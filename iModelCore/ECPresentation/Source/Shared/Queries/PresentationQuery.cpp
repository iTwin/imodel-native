/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include "QueryExecutor.h"
#include "QueryContracts.h"
#include "PresentationQuery.h"
#include "../ValueHelpers.h"
#include "../../Hierarchies/NavigationQuery.h"

// *** NEEDS WORK: clang for android complains that GetECClassClause and ContractHasNonAggregateFields are unused
// ***              and that ConstraintSupportsClass "is not needed and will not be emitted". I don't understand
// ***              any of that. GetECClassClause, for example, appears be used. For now, I will disable these clang warnings.
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#endif

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BoundQueryValuesList::Bind(ECSqlStatement& stmt) const
    {
    for (size_t i = 0; i < size(); ++i)
        {
        auto const& value = at(i);
        ECSqlStatus status = value->Bind(stmt, (uint32_t)(i + 1));
        if (!status.IsSuccess())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to bind value. Result: %d", status.GetSQLiteError()));
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document BoundQueryValuesList::ToJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetArray();
    for (size_t i = 0; i < size(); ++i)
        {
        auto const& value = at(i);
        json.PushBack(value->ToJson(&json.GetAllocator()), json.GetAllocator());
        }
    return json;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BoundQueryValuesList::FromJson(RapidJsonValueCR json)
    {
    clear();
    if (!json.IsArray())
        return ERROR;

    for (rapidjson::SizeType i = 0; i < json.Size(); ++i)
        {
        auto value = BoundQueryValue::FromJson(json[i]);
        if (value)
            push_back(std::move(value));
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
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

    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unhandled ECValue type: %d", (int)m_value.GetPrimitiveType()));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
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
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BoundQueryECValue::_Equals(BoundQueryValue const& other) const
    {
    BoundQueryECValue const* otherECValue = dynamic_cast<BoundQueryECValue const*>(&other);
    return (nullptr != otherECValue) ? AreEqual(this, otherECValue) : false;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document BoundQueryECValue::_ToJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("type", "ec-value", json.GetAllocator());
    json.AddMember("value-type", (int)m_value.GetPrimitiveType(), json.GetAllocator());
    json.AddMember("value", ValueHelpers::GetJsonFromECValue(m_value, &json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatus BoundQueryId::_Bind(ECSqlStatement& stmt, uint32_t index) const
    {
    return stmt.BindId((int)index, m_id);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BoundQueryId::_Equals(BoundQueryValue const& other) const
    {
    BoundQueryId const* otherId = dynamic_cast<BoundQueryId const*>(&other);
    if (nullptr == otherId)
        return false;

    return m_id == otherId->m_id;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document BoundQueryId::_ToJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("type", "id", json.GetAllocator());
    json.AddMember("value", rapidjson::Value(m_id.ToString(BeInt64Id::UseHex::Yes).c_str(), json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatus BoundQueryIdSet::_Bind(ECSqlStatement& stmt, uint32_t index) const
    {
    return stmt.BindVirtualSet((int)index, m_set);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BoundQueryIdSet::_Equals(BoundQueryValue const& other) const
    {
    BoundQueryIdSet const* otherVirtualSet = dynamic_cast<BoundQueryIdSet const*>(&other);
    if (nullptr == otherVirtualSet)
        return false;

    return m_set == otherVirtualSet->m_set;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document BoundQueryIdSet::_ToJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("type", "id-set", json.GetAllocator());
    rapidjson::Value idsJson;
    idsJson.SetArray();
    for (auto const& id : m_set)
        idsJson.PushBack(rapidjson::Value(id.ToString(BeInt64Id::UseHex::Yes).c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("value", idsJson, json.GetAllocator());
    return json;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NoValuesHandler : FilteredValuesHandler
{
protected:
    Utf8String _GetWhereClause(Utf8CP idSelector, bool inverse) const override {return inverse ? "TRUE" : "FALSE";}
    void _Accept(BeInt64Id id) override {}
    void _Accept(ECValue) override {}
    BoundQueryValuesList _GetBoundValues() override {return BoundQueryValuesList();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct VirtualSetHandler : FilteredValuesHandler
{
protected:
    Utf8String _GetWhereClause(Utf8CP valueSelector, bool inverse) const override
        {
        Utf8String clause;
        if (inverse)
            clause.append("NOT ");
        clause.append("InVirtualSet(?, ").append(valueSelector).append(")");
        return clause;
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct VirtualSetIdsHandler : VirtualSetHandler
{
private:
    bvector<BeInt64Id> m_ids;
protected:
    void _Accept(BeInt64Id id) override {m_ids.push_back(id);}
    void _Accept(ECValue) override {DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Binding a non-ID value using VirtualSetIdsHandler");}
    BoundQueryValuesList _GetBoundValues() override {return {std::make_shared<BoundQueryIdSet>(m_ids)};}
public:
    VirtualSetIdsHandler(size_t inputSize) {m_ids.reserve(inputSize);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct VirtualSetValuesHandler : VirtualSetHandler
{
private:
    bvector<ECValue> m_values;
protected:
    void _Accept(BeInt64Id id) override {m_values.push_back(ECValue(id));}
    void _Accept(ECValue value) override {m_values.push_back(std::move(value));}
    BoundQueryValuesList _GetBoundValues() override {return {std::make_shared<BoundECValueSet>(m_values)};}
public:
    VirtualSetValuesHandler(size_t inputSize) {m_values.reserve(inputSize);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct BoundValuesHandler : FilteredValuesHandler
{
private:
    BoundQueryValuesList m_values;
protected:
    Utf8String _GetWhereClause(Utf8CP valueSelector, bool inverse) const override
        {
        if (m_values.empty())
            return "FALSE";

        Utf8String bindingsArg(m_values.size() * 2 - 1, '?');
        for (size_t i = 1; i < bindingsArg.size(); i += 2)
            bindingsArg[i] = ',';
        Utf8String clause(valueSelector);
        if (inverse)
            clause.append(" NOT");
        clause.append(" IN (").append(bindingsArg).append(")");
        return clause;
        }
    void _Accept(BeInt64Id id) override {m_values.push_back(std::make_unique<BoundQueryId>(id));}
    void _Accept(ECValue value) override {m_values.push_back(std::make_unique<BoundQueryECValue>(value));}
    BoundQueryValuesList _GetBoundValues() override {return m_values;}
public:
    BoundValuesHandler(size_t inputSize) {m_values.reserve(inputSize);}
};

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<FilteredValuesHandler> FilteredValuesHandler::Create(size_t inputSize, bool onlyIds)
    {
    // choosing when it's worth using virtual set vs bound values depends on 2 factors:
    // - rows count in the table we're selecting from (more rows make virtual set more expensive)
    // - bound values count (more values make binding them individually more expensive)
    // assuming that selecting from large tables is a common case and large number of
    // bounds values - not.
    static const size_t BOUNDARY_VirtualSet = 100;

    if (0 == inputSize)
        return std::make_unique<NoValuesHandler>();
    if (inputSize > BOUNDARY_VirtualSet)
        {
        if (onlyIds)
            {
            // in most cases we're using this for IDs - use a more optimal implementation when
            // we know we're only going to have IDs
            return std::make_unique<VirtualSetIdsHandler>(inputSize);
            }
        return std::make_unique<VirtualSetValuesHandler>(inputSize);
        }
    return std::make_unique<BoundValuesHandler>(inputSize);
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryHelpers::IsLiteral(Utf8StringCR clause)
    {
    return clause.EqualsI("null")
        || clause.EqualsI("false") || clause.EqualsI("true")
        || IsNumeric(clause)
        || clause.StartsWith("'") && clause.EndsWith("'");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryHelpers::IsWrapped(Utf8StringCR clause)
    {
    return (clause.StartsWith("[") || clause.StartsWith("+["))
        && clause.EndsWith("]");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String QueryHelpers::Wrap(Utf8StringCR str)
    {
    if (str.empty() || IsFunction(str) || IsLiteral(str) || IsWrapped(str))
        return str;

    if (!str.Contains(" "))
        return Utf8String("[").append(str).append("]");

    return str;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendQuery(PresentationQuery& target, PresentationQuery const& source)
    {
    target.GetQueryString().append(source.GetQueryString());
    ContainerHelpers::Push(target.GetBindings(), source.GetBindings());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentation::JoinClassClause : JoinClause
    {
    SelectClassWithExcludes<ECClass> m_join;
    JoinClassClause(SelectClassWithExcludes<ECClass> join, QueryClauseAndBindings joinFilter, JoinType joinType)
        : JoinClause(joinFilter, joinType), m_join(join)
        {}
    virtual JoinClassClause const* _AsClassJoin() const override { return this; }
    };
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentation::JoinClassWithRelationshipClause : JoinClassClause
    {
    SelectClassWithExcludes<ECRelationshipClass> m_using;
    bool m_isForward;
    bool m_shouldJoinTargetClass;
    JoinClassWithRelationshipClause(SelectClassWithExcludes<ECClass> join, QueryClauseAndBindings joinFilter, SelectClass<ECRelationshipClass> usingRelationship, bool isForward, JoinType joinType, bool shouldJoinTargetClass = true)
        : JoinClassClause(join, joinFilter, joinType), m_using(usingRelationship), m_isForward(isForward), m_shouldJoinTargetClass(shouldJoinTargetClass)
        {}
    JoinClassWithRelationshipClause const* _AsClassWithRelationshipJoin() const override { return this; }
    };
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentation::JoinQueryClause : JoinClause
    {
    std::unique_ptr<PresentationQuery> m_query;
    Utf8String m_alias;
    JoinQueryClause(std::unique_ptr<PresentationQuery> query, Utf8String alias, QueryClauseAndBindings joinFilter, JoinType joinType)
        : JoinClause(joinFilter, joinType), m_query(std::move(query)), m_alias(alias)
        {}
    JoinQueryClause const* _AsQueryJoin() const override { return this; }
    };

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AreEqual(JoinClause const& lhs, JoinClause const& rhs)
    {
    if (auto lhsAsClass = lhs._AsClassJoin())
        {
        auto rhsAsClass = rhs._AsClassJoin();
        if (!rhsAsClass)
            return false;
        if (lhsAsClass->m_join != rhsAsClass->m_join)
            return false;
        }
    if (auto lhsAsClassWithRelationship = lhs._AsClassWithRelationshipJoin())
        {
        auto rhsAsClassWithRelationship = rhs._AsClassWithRelationshipJoin();
        if (!rhsAsClassWithRelationship)
            return false;
        if (lhsAsClassWithRelationship->m_using != rhsAsClassWithRelationship->m_using)
            return false;
        if (lhsAsClassWithRelationship->m_isForward != rhsAsClassWithRelationship->m_isForward)
            return false;
        if (lhsAsClassWithRelationship->m_shouldJoinTargetClass != rhsAsClassWithRelationship->m_shouldJoinTargetClass)
            return false;
        }
    if (auto lhsAsQuery = lhs._AsQueryJoin())
        {
        auto rhsAsQuery = rhs._AsQueryJoin();
        if (!rhsAsQuery)
            return false;
        if (!lhsAsQuery->m_query->IsEqual(*rhsAsQuery->m_query))
            return false;
        if (lhsAsQuery->m_alias != lhsAsQuery->m_alias)
            return false;
        }
    return lhs.GetJoinFilter()  == rhs.GetJoinFilter()
        && lhs.GetJoinType()  == rhs.GetJoinType();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document PresentationQuery::ToJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("query", rapidjson::Value(GetQueryString().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("bindings", GetBindings().ToJson(&json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<PresentationQuery> PresentationQuery::FromJson(RapidJsonValueCR json)
    {
    BoundQueryValuesList bindings;
    if (SUCCESS == bindings.FromJson(json["bindings"]))
        return std::make_unique<PresentationQuery>(json["query"].GetString(), bindings);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// Note: need these here to avoid compiler from complaining regarding unknown type of
// `m_navigationResultParams`.
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryBuilder::PresentationQueryBuilder() : m_isOuterQuery(true) {}
PresentationQueryBuilder::PresentationQueryBuilder(PresentationQueryBuilder const& other)
    : m_isOuterQuery(other.m_isOuterQuery)
    {
    if (other.m_navigationResultParams != nullptr)
        m_navigationResultParams = std::make_unique<NavigationQueryResultParameters>(*other.m_navigationResultParams);
    }
PresentationQueryBuilder::~PresentationQueryBuilder() {}

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PresentationQueryBuilder> PresentationQueryBuilder::Clone() const
    {
    if (nullptr != AsComplexQueryBuilder())
        return new ComplexQueryBuilder(*AsComplexQueryBuilder());
    if (nullptr != AsUnionQueryBuilder())
        return new UnionQueryBuilder(*AsUnionQueryBuilder());
    if (nullptr != AsExceptQueryBuilder())
        return new ExceptQueryBuilder(*AsExceptQueryBuilder());
    return StringQueryBuilder::Create(*this);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryResultParameters const& PresentationQueryBuilder::GetNavigationResultParameters() const
    {
    if (m_navigationResultParams != nullptr)
        return *m_navigationResultParams;
    static NavigationQueryResultParameters const s_empty;
    return s_empty;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryResultParameters& PresentationQueryBuilder::GetNavigationResultParameters()
    {
    if (m_navigationResultParams == nullptr)
        m_navigationResultParams = std::make_unique<NavigationQueryResultParameters>();
    return *m_navigationResultParams;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
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
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContract const* ComplexQueryBuilder::_GetContract(size_t contractId) const
    {
    if (m_selectContract.IsValid() && (0 == contractId || m_selectContract->GetId() == contractId))
        return m_selectContract.get();

    if (m_nestedQuery.IsValid())
        return m_nestedQuery->GetContract(contractId);

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContract const* ComplexQueryBuilder::_GetGroupingContract() const
    {
    if (m_groupingContract.IsValid())
        return m_groupingContract.get();

    if (m_nestedQuery.IsValid())
        return m_nestedQuery->GetGroupingContract();

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
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
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComplexQueryBuilder::HasClause(PresentationQueryClauses clauses) const
    {
    if (CLAUSE_Select == (CLAUSE_Select & clauses))
        return m_isSelectAll || m_selectContract.IsValid();

    if (CLAUSE_From == (CLAUSE_From & clauses))
        return m_nestedQuery.IsValid() || !m_from.empty();

    if (CLAUSE_Where == (CLAUSE_Where & clauses))
        return !m_whereClause.GetClause().empty();

    if (CLAUSE_JoinUsing == (CLAUSE_JoinUsing & clauses))
        return !m_joins.empty();

    if (CLAUSE_OrderBy == (CLAUSE_OrderBy & clauses))
        return !m_orderByClause.empty();

    if (CLAUSE_Limit == (CLAUSE_Limit & clauses))
        return nullptr != m_limit;

    if (CLAUSE_GroupBy == (CLAUSE_GroupBy & clauses))
        return m_groupingContract.IsValid() && ContractHasNonAggregateFields(*m_groupingContract);

    if (CLAUSE_Having == (CLAUSE_Having & clauses))
        return !m_havingClause.GetClause().empty();

    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unhandled presentation query clause type: %d", (int)clauses));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void SelectField(bvector<QueryClauseAndBindings>& output, QueryClauseAndBindings clause, Utf8StringCR alias)
    {
    if (clause.GetClause().empty())
        {
        output.push_back(QueryClauseAndBindings(QueryHelpers::Wrap(alias)));
        return;
        }

    if (!alias.empty())
        clause.GetClause().append(" AS ").append(QueryHelpers::Wrap(alias));

    output.push_back(clause);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::SelectAll()
    {
    InvalidateQuery();
    m_isSelectAll = true;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::SelectContract(PresentationQueryContract const& contract, Utf8CP prefix)
    {
    InvalidateQuery();
    m_selectContract = &contract;
    m_selectPrefix = prefix;
    if (contract.AsNavigationQueryContract())
        GetNavigationResultParameters().OnContractSelected(*contract.AsNavigationQueryContract());
    return *this;
    }

enum QueryPosition
    {
    None  = 0,
    Inner = 1 << 0,
    Outer = 1 << 1,
    };

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ShouldSelectWithClause(PresentationQueryContractFieldCR field, PresentationQueryContractCP nestedContract)
    {
    if (!nestedContract)
        return true;

    bvector<PresentationQueryContractFieldCPtr> nestedFields = nestedContract->GetFields();
    for (PresentationQueryContractFieldCPtr const& nestedField : nestedFields)
        {
        if (0 == strcmp(nestedField->GetName(), field.GetName()) && (FieldVisibility::Outer != nestedField->GetVisibility()))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::function<bool(Utf8CP)> CreateQueryFieldLookupFunc(PresentationQueryBuilder const* nestedQuery)
    {
    if (!nestedQuery || !nestedQuery->GetContract())
        return nullptr;

    return [nestedQuery](Utf8CP fieldName)
        {
        auto field = nestedQuery->GetContract()->GetField(fieldName);
        if (field.IsValid() && !field->IsAggregateField())
            return true;

        auto groupingContract = nestedQuery->GetGroupingContract();
        if (!groupingContract)
            return false;

        field = groupingContract->GetField(fieldName);
        return field.IsValid() && field->IsAggregateField();
        };
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DoesQuerySelectInnerFields(PresentationQueryBuilder const& query)
    {
    auto contract = query.GetContract();
    if (!contract || !contract->HasInnerFields())
        return false;

    // at this point we know the query has a select contract and it has inner fields.
    // now we need to check whether the query is 'inner' itself. that means checking if it doesn't have any nested queries
    bool hasNestedQuery = query.AsComplexQueryBuilder() && query.AsComplexQueryBuilder()->GetNestedQuery();
    return !hasNestedQuery;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ComplexQueryBuilder::InitSelectClause() const
    {
    if (!m_selectClause.GetClause().empty())
        return;

    int position = QueryPosition::None;
    if (nullptr == GetGroupingContract())
        position |= QueryPosition::Inner;
    if (IsOuterQuery())
        position |= QueryPosition::Outer;

    bool hasNestedInnerFields = false;
    PresentationQueryContract const* contract = m_selectContract.get();
    PresentationQueryContract const* nestedContract = nullptr;
    if (m_nestedQuery.IsValid())
        {
        nestedContract = m_nestedQuery->GetContract();
        if (nullptr != nestedContract)
            {
            if (m_isSelectAll)
                contract = nestedContract;

            hasNestedInnerFields = DoesQuerySelectInnerFields(*m_nestedQuery);
            }
        }

    bvector<QueryClauseAndBindings> selectClauseFields;

    if (m_isSelectAll && !hasNestedInnerFields)
        {
        selectClauseFields.push_back("*");
        }
    else
        {
        bvector<PresentationQueryContractFieldCPtr> selectFields = contract->GetFields();
        for (PresentationQueryContractFieldCPtr const& field : selectFields)
            {
            if (field->IsAggregateField())
                continue;

            if (FieldVisibility::Inner == field->GetVisibility() && 0 == (QueryPosition::Inner & position))
                continue;

            if (FieldVisibility::Outer == field->GetVisibility() && 0 == (QueryPosition::Outer & position))
                continue;

            bool selectWithClause = ShouldSelectWithClause(*field, nestedContract);
            Utf8String wrappedName = QueryHelpers::Wrap(field->GetName());
            if (selectWithClause)
                {
                auto skipNestedClauses = CreateQueryFieldLookupFunc(m_nestedQuery.get());
                SelectField(selectClauseFields, field->GetSelectClause(m_selectPrefix.c_str(), skipNestedClauses), wrappedName);
                }
            else
                {
                SelectField(selectClauseFields, QueryClauseAndBindings(), wrappedName);
                }
            }
        }

    PresentationQueryContract const* groupingContract = m_groupingContract.get();
    if (nullptr != groupingContract)
        {
        bvector<PresentationQueryContractFieldCPtr> groupingFields = groupingContract->GetFields();
        for (PresentationQueryContractFieldCPtr const& field : groupingFields)
            {
            if (!field->IsAggregateField())
                continue;

            if (m_groupingContract.get() == groupingContract)
                {
                auto skipNestedClauses = CreateQueryFieldLookupFunc(m_nestedQuery.get());
                SelectField(selectClauseFields, field->GetSelectClause(m_selectPrefix.c_str(), skipNestedClauses), field->GetName());
                }
            else
                {
                SelectField(selectClauseFields, QueryClauseAndBindings(), QueryHelpers::Wrap(field->GetName()));
                }
            }
        }

    for (auto const& fieldClause : selectClauseFields)
        m_selectClause.Append(fieldClause, ", ");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static QueryClauseAndBindings CreateExcludedClassesQueryClause(bvector<SelectClass<ECClass>> const& excludedClasses, Utf8CP alias)
    {
    if (excludedClasses.empty())
        return QueryClauseAndBindings();

    bvector<Utf8String> multiSchemaClassECSqlExpressionList;

    for (auto const& excludeSelectClass : excludedClasses)
        {
        Utf8String singleClassSqlExpression;

        if (!excludeSelectClass.IsSelectPolymorphic())
            singleClassSqlExpression.append("ONLY ");

        singleClassSqlExpression
            .append("[").append(excludeSelectClass.GetClass().GetSchema().GetAlias()).append("]")
            .append(".")
            .append("[").append(excludeSelectClass.GetClass().GetName()).append("]");

        multiSchemaClassECSqlExpressionList.push_back(singleClassSqlExpression);
        }
    Utf8String combinedECSqlExpression;
    combinedECSqlExpression.append("[").append(alias).append("].[ECClassId] IS NOT (")
        .append(BeStringUtilities::Join(multiSchemaClassECSqlExpressionList, ", ")).append(")");

    return QueryClauseAndBindings(combinedECSqlExpression, BoundQueryValuesList());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateClassSelectorClause(SelectClassWithExcludes<ECClass> const& select)
    {
    Utf8String clause;
    if (!select.GetDerivedExcludedClasses().empty())
        {
        SelectClassWithExcludes<ECClass> noExcludes(select);
        noExcludes.GetDerivedExcludedClasses().clear();
        auto nested = ComplexQueryBuilder::Create();
        nested->SelectAll().From(noExcludes)
            .Where(CreateExcludedClassesQueryClause(select.GetDerivedExcludedClasses(), select.GetAlias().c_str()));
        clause.append("(").append(nested->GetQuery()->GetQueryString()).append(")");
        }
    else
        {
        if (!select.IsSelectPolymorphic())
            clause.append("ONLY ");
        if (select.ShouldDisqualify())
            clause.append("+");
        clause.append(GetECClassClause(select.GetClass()));
        }
    if (!select.GetAlias().empty())
        clause.append(" ").append(QueryHelpers::Wrap(select.GetAlias()));
    return clause;
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateClassSelectorClause(SelectClassWithExcludes<ECRelationshipClass> const& select)
    {
    return CreateClassSelectorClause(reinterpret_cast<SelectClassWithExcludes<ECClass> const&>(select));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ComplexQueryBuilder::InitFromClause() const
    {
    if (!m_fromClause.GetClause().empty())
        return;

    for (auto const& from : m_from)
        m_fromClause.Append(CreateClassSelectorClause(*from), ", ");
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::From(SelectClassWithExcludes<ECClass> const& fromClass)
    {
    InvalidateQuery();
    m_fromClause.Reset();
    m_from.push_back(std::make_unique<SelectClassWithExcludes<ECClass>>(fromClass));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::From(ECClassCR fromClass, bool polymorphic, Utf8CP alias)
    {
    return From(SelectClass<ECClass>(fromClass, alias, polymorphic));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::From(PresentationQueryBuilder& nestedQuery, Utf8CP alias)
    {
    InvalidateQuery();
    nestedQuery.SetIsOuterQuery(false);

    m_nestedQuery = &nestedQuery;
    m_nestedQueryAlias = alias;

    if (nestedQuery.GetContract() && nestedQuery.GetContract()->AsNavigationQueryContract())
        {
        GetNavigationResultParameters().MergeWith(m_nestedQuery->GetNavigationResultParameters());
        nestedQuery.GetNavigationResultParameters() = GetNavigationResultParameters();
        }
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::From(PresentationQuery const& nestedQuery, Utf8CP alias)
    {
    return From(*StringQueryBuilder::Create(nestedQuery.GetQueryString(), nestedQuery.GetBindings()), alias);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::Where(Utf8CP whereClause, BoundQueryValuesListCR bindings)
    {
    return Where(QueryClauseAndBindings(whereClause, BoundQueryValuesList(bindings)));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::Where(QueryClauseAndBindings clause)
    {
    InvalidateQuery();
    clause.GetClause().assign(Utf8String("(").append(clause.GetClause()).append(")"));
    m_whereClause.Append(clause, " AND ");
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::Join(RelatedClass const& relatedClass)
    {
    RelatedClassPath path;
    path.push_back(relatedClass);
    return Join(path);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::Join(RelatedClassPath const& path, bool shouldJoinLastTargetClass)
    {
    InvalidateQuery();
    m_joinClause.Reset();
    bool isJoinOptional = false; // inner join by default
    bvector<std::shared_ptr<JoinClause>> rootJoins;
    for (RelatedClass const& relatedClass : path)
        {
        bool shouldJoinTargetClass = (&relatedClass != &path[path.size() - 1]) || shouldJoinLastTargetClass;
        isJoinOptional |= relatedClass.IsTargetOptional(); // make it outer join as soon as we find an optional target
        std::unique_ptr<JoinClause> clause;
        if (relatedClass.GetTargetIds().empty())
            {
            if (!relatedClass.GetRelationship().IsValid())
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Skipping JOIN as relationship is not specified");
            if (relatedClass.GetRelationship().GetAlias().empty())
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Skipping JOIN as relationship doesn't have an alias");
            clause = std::make_unique<JoinClassWithRelationshipClause>(relatedClass.GetTargetClass(), QueryClauseAndBindings(relatedClass.GetTargetInstanceFilter()),
                relatedClass.GetRelationship(), relatedClass.IsForwardRelationship(), isJoinOptional ? JoinType::Outer : JoinType::Inner, shouldJoinTargetClass);
            }
        else if (relatedClass.GetRelationship().IsValid())
            {
            if (relatedClass.GetRelationship().GetAlias().empty())
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Skipping JOIN as relationship doesn't have an alias");
            clause = std::make_unique<JoinClassWithRelationshipClause>(relatedClass.GetTargetClass(),
                ValuesFilteringHelper(relatedClass.GetTargetIds()).Create(QueryHelpers::Wrap(relatedClass.GetTargetClass().GetAlias()).append(".[ECInstanceId]").c_str()),
                relatedClass.GetRelationship(), relatedClass.IsForwardRelationship(), isJoinOptional ? JoinType::Outer : JoinType::Inner, shouldJoinTargetClass);
            }
        else
            {
            clause = std::make_unique<JoinClassClause>(relatedClass.GetTargetClass(),
                ValuesFilteringHelper(relatedClass.GetTargetIds()).Create(QueryHelpers::Wrap(relatedClass.GetTargetClass().GetAlias()).append(".[ECInstanceId]").c_str()), isJoinOptional ? JoinType::Outer : JoinType::Inner);
            }
        rootJoins.push_back(std::move(clause));
        }

    if (rootJoins.empty())
        return *this;

    m_joins.push_back(std::move(rootJoins));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::Join(SelectClass<ECClass> const& join, QueryClauseAndBindings joinClause, bool isOuter)
    {
    InvalidateQuery();
    m_joinClause.Reset();
    m_joins.push_back({ std::make_unique<JoinClassClause>(join, joinClause, isOuter ? JoinType::Outer : JoinType::Inner) });
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::Join(PresentationQueryBuilder const& nestedQuery, Utf8CP alias, QueryClauseAndBindings joinClause, bool isOuter)
    {
    InvalidateQuery();
    m_joinClause.Reset();
    m_joins.push_back({ std::make_unique<JoinQueryClause>(nestedQuery.CreateQuery(), alias, joinClause, isOuter ? JoinType::Outer : JoinType::Inner) });
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::Join(PresentationQuery const& nestedQuery, Utf8CP alias, QueryClauseAndBindings joinClause, bool isOuter)
    {
    InvalidateQuery();
    m_joinClause.Reset();
    m_joins.push_back({ std::make_unique<JoinQueryClause>(std::make_unique<PresentationQuery>(nestedQuery), alias, joinClause, isOuter ? JoinType::Outer : JoinType::Inner) });
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::OrderBy(Utf8CP orderByClause)
    {
    InvalidateQuery();
    m_orderByClause = orderByClause;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::GroupByContract(PresentationQueryContract const& contract)
    {
    InvalidateQuery();
    m_groupingContract = &contract;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComplexQueryBuilder::CreateGroupByClause() const
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
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::Having(Utf8CP havingClause, BoundQueryValuesListCR bindings)
    {
    InvalidateQuery();
    return Having(QueryClauseAndBindings(havingClause, BoundQueryValuesList(bindings)));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::Having(QueryClauseAndBindings clause)
    {
    InvalidateQuery();
    m_havingClause = clause;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ComplexQueryBuilder& ComplexQueryBuilder::Limit(uint64_t limit, uint64_t offset)
    {
    InvalidateQuery();
    m_limit = std::make_unique<BoundQueryECValue>(ECValue(limit));
    m_offset = std::make_unique<BoundQueryECValue>(ECValue(offset));
    return *this;
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct JoinInfo
    {
    ECClassCR m_class;
    Utf8String m_alias;
    bool m_isForward;
    JoinInfo(ECClassCR ecClass, Utf8String alias, bool isForward)
        : m_class(ecClass), m_alias(alias), m_isForward(isForward)
        {}
    };

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsTargetClassSupported(ECClassCR targetClass, ECRelationshipConstraintCR oppositeConstraint)
    {
    if (oppositeConstraint.SupportsClass(targetClass))
        return true;
    if (!targetClass.IsEntityClass())
        return false;
    for (auto const& constraintClass : oppositeConstraint.GetConstraintClasses())
        {
        if (constraintClass->IsMixin() && targetClass.GetEntityClassCP()->CanApply(*constraintClass->GetEntityClassCP()))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::unique_ptr<JoinInfo> DetermineJoinTarget(bvector<std::shared_ptr<SelectClassWithExcludes<ECClass>>> const& fromClauses, JoinClassWithRelationshipClause const& joinClause)
    {
    ECRelationshipConstraintCR constraint = joinClause.m_isForward ? joinClause.m_using.GetClass().GetSource() : joinClause.m_using.GetClass().GetTarget();
    ECRelationshipConstraintCR oppositeConstraint = joinClause.m_isForward ? joinClause.m_using.GetClass().GetTarget() : joinClause.m_using.GetClass().GetSource();
    for (auto const& fromClausePtr : fromClauses)
        {
        auto const& fromClause = *fromClausePtr;
        if (constraint.SupportsClass(fromClause.GetClass()))
            {
            DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, IsTargetClassSupported(joinClause.m_join.GetClass(), oppositeConstraint), Utf8PrintfString("Expected opposite constraint to support joined class, but it doesn't. "
                "Relationship: '%s', joined class: '%s'", joinClause.m_using.GetClass().GetFullName(), joinClause.m_join.GetClass().GetFullName()));
            return std::make_unique<JoinInfo>(fromClause.GetClass(), fromClause.GetAlias().empty() ? fromClause.GetClass().GetName() : fromClause.GetAlias(), true);
            }
        }
    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Tried to JOIN on a relationship whose neither target nor source exists in the FROM clause"
        "Relationship: '%s'", joinClause.m_using.GetClass().GetFullName()));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void DetermineJoinClauses(Utf8StringR thisClause, Utf8StringR nextClause, ECClassCR ecClass, bool wasPreviousJoinForward, JoinClassWithRelationshipClause const& joinClause)
    {
    Utf8CP thisClauseECInstanceIdPropertyName,
        nextClauseECInstanceIdPropertyName,
        joinECInstanceIdPropertyName;
    if (joinClause.m_isForward)
        {
        thisClauseECInstanceIdPropertyName = "SourceECInstanceId";
        nextClauseECInstanceIdPropertyName = "TargetECInstanceId";
        }
    else
        {
        thisClauseECInstanceIdPropertyName = "TargetECInstanceId";
        nextClauseECInstanceIdPropertyName = "SourceECInstanceId";
        }

    if (ecClass.IsRelationshipClass())
        {
        if (wasPreviousJoinForward)
            joinECInstanceIdPropertyName = "TargetECInstanceId";
        else
            joinECInstanceIdPropertyName = "SourceECInstanceId";
        }
    else
        {
        joinECInstanceIdPropertyName = "ECInstanceId";
        }

    static Utf8CP pattern = "[%%s].[%s] = [%%s].[%s]";
    thisClause = Utf8PrintfString(pattern, joinECInstanceIdPropertyName, thisClauseECInstanceIdPropertyName);
    nextClause = Utf8PrintfString(pattern, "ECInstanceId", nextClauseECInstanceIdPropertyName);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetNavigationPropertyName(NavigationECPropertyCR navigationProperty)
    {
    return QueryHelpers::Wrap(navigationProperty.GetName()).append(".[Id]");
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
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

    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Previously joined class is not an Entity and not a Relationship class: '%s'", previouslyJoinedClass.GetFullName()));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateJoinClause(JoinType type)
    {
    return JoinType::Outer == type ? " LEFT JOIN " : " INNER JOIN ";
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static QueryClauseAndBindings CreateJoinsClause(bvector<std::shared_ptr<JoinClause>> const& joins, bvector<std::shared_ptr<SelectClassWithExcludes<ECClass>>> const& fromClauses,
    bmap<Utf8String, std::shared_ptr<JoinInfo>>& joinedRelationships)
    {
    Utf8String joinClause;
    BoundQueryValuesList bindings;

    std::shared_ptr<JoinInfo> prev;
    for (auto const& joinPtr : joins)
        {
        auto const& join = *joinPtr;

        if (auto classJoin = join._AsClassJoin())
            {
            // first check if we already have the required relationship joined and attempt to reuse it if possible
            auto joinedRelationshipInfoIter = joinedRelationships.find(classJoin->m_join.GetAlias());
            if (joinedRelationships.end() != joinedRelationshipInfoIter)
                {
                prev = joinedRelationshipInfoIter->second;
                continue;
                }
            }

        // if we're joining using a relationship, we need to know what we're joining to - if we don't have a previous join,
        // use "from" as our join target
        if (auto classWithRelationshipJoin = join._AsClassWithRelationshipJoin())
            {
            if (!prev)
                {
                prev = DetermineJoinTarget(fromClauses, *classWithRelationshipJoin);
                if (!prev)
                    continue;
                }
            }

        Utf8String joinFilterClause;
        Utf8String andJoinFilterClause;
        if (!join.GetJoinFilter().GetClause().empty())
            {
            joinFilterClause.append("(").append(join.GetJoinFilter().GetClause()).append(")");
            andJoinFilterClause.append(" AND ").append(joinFilterClause);
            ContainerHelpers::Push(bindings, join.GetJoinFilter().GetBindings());
            }

        std::shared_ptr<JoinInfo> joinedClassInfo;
        if (auto classJoin = join._AsClassJoin())
            joinedClassInfo = std::make_shared<JoinInfo>(classJoin->m_join.GetClass(), classJoin->m_join.GetAlias(), true);

        if (auto classWithRelationshipJoin = join._AsClassWithRelationshipJoin())
            {
            NavigationECPropertyCP navigationProperty = RelatedClass(prev->m_class, classWithRelationshipJoin->m_using, classWithRelationshipJoin->m_isForward, classWithRelationshipJoin->m_join).GetNavigationProperty();
            if (nullptr != navigationProperty)
                {
                // when possible, use navigation property for the join
                bool isForward = (classWithRelationshipJoin->m_isForward == (ECRelatedInstanceDirection::Forward == navigationProperty->GetDirection()));
                joinClause.append(CreateJoinClause(classWithRelationshipJoin->GetJoinType()));
                joinClause.append(CreateClassSelectorClause(classWithRelationshipJoin->m_join));
                joinClause.append(" ON ").append(QueryHelpers::Wrap(classWithRelationshipJoin->m_join.GetAlias())).append(".");
                joinClause.append(isForward ? GetOppositeNavigationPropertyName(*navigationProperty, prev->m_class, prev->m_isForward) : GetNavigationPropertyName(*navigationProperty));
                joinClause.append(" = ").append(QueryHelpers::Wrap(prev->m_alias)).append(".");
                joinClause.append(isForward ? GetNavigationPropertyName(*navigationProperty) : GetOppositeNavigationPropertyName(*navigationProperty, prev->m_class, prev->m_isForward));
                joinClause.append(andJoinFilterClause);
                joinedRelationships.Insert(classWithRelationshipJoin->m_join.GetAlias(), joinedClassInfo);
                prev = joinedClassInfo;
                }
            else
                {
                // determine the join clause based on relationship direction
                Utf8String previousClassJoinClause;
                Utf8String joinedClassJoinClause;
                DetermineJoinClauses(previousClassJoinClause, joinedClassJoinClause, prev->m_class, prev->m_isForward, *classWithRelationshipJoin);

                // if relationship is not already joined, do it
                auto relationshipJoinInfo = std::make_shared<JoinInfo>(classWithRelationshipJoin->m_using.GetClass(), classWithRelationshipJoin->m_using.GetAlias(), classWithRelationshipJoin->m_isForward);
                if (joinedRelationships.end() == joinedRelationships.find(classWithRelationshipJoin->m_using.GetAlias()))
                    {
                    if (JoinType::Outer == classWithRelationshipJoin->GetJoinType())
                        {
                        // note: we always want to inner join the class to the relationship, because relationship may be targeting a base class or our
                        // target class and cause duplicate results
                        joinClause.append(CreateJoinClause(JoinType::Outer)).append("(")
                            .append("SELECT [").append(classWithRelationshipJoin->m_using.GetAlias()).append("].* ")
                            .append("FROM ").append(CreateClassSelectorClause(classWithRelationshipJoin->m_using))
                            .append(CreateJoinClause(JoinType::Inner)).append(CreateClassSelectorClause(classWithRelationshipJoin->m_join))
                            .append(" ON ").append(Utf8PrintfString(joinedClassJoinClause.c_str(), classWithRelationshipJoin->m_join.GetAlias().c_str(), classWithRelationshipJoin->m_using.GetAlias().c_str()))
                            .append(andJoinFilterClause)
                            .append(") [").append(classWithRelationshipJoin->m_using.GetAlias()).append("]")
                            .append(" ON ").append(Utf8PrintfString(previousClassJoinClause.c_str(), prev->m_alias.c_str(), classWithRelationshipJoin->m_using.GetAlias().c_str()));
                        }
                    else
                        {
                        joinClause.append(CreateJoinClause(JoinType::Inner))
                            .append(CreateClassSelectorClause(classWithRelationshipJoin->m_using))
                            .append(" ON ").append(Utf8PrintfString(previousClassJoinClause.c_str(), prev->m_alias.c_str(), classWithRelationshipJoin->m_using.GetAlias().c_str()));
                        }
                    joinedRelationships.Insert(classWithRelationshipJoin->m_using.GetAlias(), relationshipJoinInfo);
                    prev = relationshipJoinInfo;
                    }

                // determine if we need to join the target class (when joining multiple relationships, sometimes we may skip the target class
                // and just join relationship to relationship)
                bool shouldJoinTargetClass =
                    (classWithRelationshipJoin->m_shouldJoinTargetClass && // it is not explicitly specified that target class should not be joined
                        (joinPtr == joins[joins.size() - 1] // this is the last join clause in the group
                            || !classWithRelationshipJoin->m_join.GetAlias().empty())) // join has an assigned alias
                    || !classWithRelationshipJoin->GetJoinFilter().GetClause().empty(); // join has a filter
                if (shouldJoinTargetClass)
                    {
                    Utf8String joinedClassName = classWithRelationshipJoin->m_join.GetClass().GetName();
                    joinClause.append(CreateJoinClause(classWithRelationshipJoin->GetJoinType()));
                    joinClause.append(CreateClassSelectorClause(classWithRelationshipJoin->m_join));
                    if (!classWithRelationshipJoin->m_join.GetAlias().empty())
                        joinedClassName = classWithRelationshipJoin->m_join.GetAlias();
                    joinClause.append(" ON ").append(Utf8PrintfString(joinedClassJoinClause.c_str(), joinedClassName.c_str(), classWithRelationshipJoin->m_using.GetAlias().c_str()));
                    joinClause.append(andJoinFilterClause);
                    joinedRelationships.Insert(joinedClassName, relationshipJoinInfo);
                    prev = joinedClassInfo;
                    }
                }
            }
        else if (auto classJoin = join._AsClassJoin())
            {
            Utf8String joinedClassName = classJoin->m_join.GetClass().GetName();
            joinClause.append(CreateJoinClause(classJoin->GetJoinType()));
            joinClause.append(CreateClassSelectorClause(classJoin->m_join));
            if (!classJoin->m_join.GetAlias().empty())
                joinedClassName = classJoin->m_join.GetAlias();
            if (!joinFilterClause.empty())
                joinClause.append(" ON ").append(joinFilterClause);
            joinedRelationships.Insert(joinedClassName, joinedClassInfo);
            prev = joinedClassInfo;
            }
        else if (auto queryJoin = join._AsQueryJoin())
            {
            joinClause.append(CreateJoinClause(queryJoin->GetJoinType()));
            joinClause.append("(").append(queryJoin->m_query->GetQueryString()).append(") ").append(queryJoin->m_alias);
            if (!joinFilterClause.empty())
                joinClause.append(" ON ").append(joinFilterClause);
            }
        }

    return QueryClauseAndBindings(joinClause, bindings);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ComplexQueryBuilder::InitJoinClause() const
    {
    if (!m_joinClause.GetClause().empty() || m_joins.empty())
        return;

    // TODO: we should not need to have a FROM clause to create a JOIN clause... especially when
    // thinking about joining on wrapped queries
    bvector<std::shared_ptr<SelectClassWithExcludes<ECClass>>> const* fromClauses = nullptr;
    RefCountedCPtr<ComplexQueryBuilder> fromClauseSource = this;
    while (!fromClauses && fromClauseSource.IsValid())
        {
        fromClauses = &fromClauseSource->m_from;
        RefCountedCPtr nestedQuery = fromClauseSource->GetNestedQuery();
        fromClauseSource = (nestedQuery.IsValid() && nullptr != nestedQuery->AsComplexQueryBuilder()) ? nestedQuery->AsComplexQueryBuilder() : nullptr;
        }
    if (!fromClauses || fromClauses->empty())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Join is only valid when used with 'FROM'"));

    bmap<Utf8String, std::shared_ptr<JoinInfo>> joinedRelationships;
    for (auto joinGroupIter = m_joins.begin(); joinGroupIter != m_joins.end(); ++joinGroupIter)
        {
        auto joinsClause = CreateJoinsClause(*joinGroupIter, *fromClauses, joinedRelationships);
        m_joinClause.Append(joinsClause, "");
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ComplexQueryBuilder::GetClause(PresentationQueryClauses clause) const
    {
    if (CLAUSE_Select == (CLAUSE_Select & clause))
        {
        InitSelectClause();
        return m_selectClause.GetClause();
        }

    if (CLAUSE_From == (CLAUSE_From & clause))
        {
        if (m_nestedQuery.IsValid())
            {
            Utf8String nestedClause;
            nestedClause.append("(").append(m_nestedQuery->GetQuery()->GetQueryString()).append(")");
            if (!m_nestedQueryAlias.empty())
                nestedClause.append(" ").append(QueryHelpers::Wrap(m_nestedQueryAlias));
            return nestedClause;
            }
        InitFromClause();
        return m_fromClause.GetClause();
        }

    if (CLAUSE_Where == (CLAUSE_Where & clause))
        return m_whereClause.GetClause();

    if (CLAUSE_JoinUsing == (CLAUSE_JoinUsing & clause))
        {
        InitJoinClause();
        return m_joinClause.GetClause();
        }

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
        return m_havingClause.GetClause();

    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Invalid presentation query clause: '%d'", (int)clause));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<PresentationQuery> ComplexQueryBuilder::_CreateQuery() const
    {
    auto query = std::make_unique<PresentationQuery>();

    // SELECT
    if (HasClause(CLAUSE_Select))
        {
        query->GetQueryString().append("SELECT ").append(GetClause(CLAUSE_Select));
        ContainerHelpers::Push(query->GetBindings(), m_selectClause.GetBindings());
        }

    // FROM
    if (HasClause(CLAUSE_From))
        {
        query->GetQueryString().append(" FROM ").append(GetClause(CLAUSE_From));
        if (m_nestedQuery.IsValid())
            ContainerHelpers::Push(query->GetBindings(), m_nestedQuery->GetQuery()->GetBindings());
        }

    // JOIN
    if (HasClause(CLAUSE_JoinUsing))
        {
        query->GetQueryString().append(GetClause(CLAUSE_JoinUsing));
        ContainerHelpers::Push(query->GetBindings(), m_joinClause.GetBindings());
        }

    // WHERE
    if (HasClause(CLAUSE_Where))
        {
        query->GetQueryString().append(" WHERE ").append(GetClause(CLAUSE_Where));
        ContainerHelpers::Push(query->GetBindings(), m_whereClause.GetBindings());
        }

    // GROUP BY
    if (HasClause(CLAUSE_GroupBy))
        query->GetQueryString().append(" GROUP BY ").append(GetClause(CLAUSE_GroupBy));

    // HAVING
    if (HasClause(CLAUSE_Having))
        {
        query->GetQueryString().append(" HAVING ").append(GetClause(CLAUSE_Having));
        ContainerHelpers::Push(query->GetBindings(), m_havingClause.GetBindings());
        }

    // ORDER BY
    if (HasClause(CLAUSE_OrderBy))
        query->GetQueryString().append(" ORDER BY ").append(GetClause(CLAUSE_OrderBy));

    // LIMIT
    if (HasClause(CLAUSE_Limit))
        {
        query->GetQueryString().append(" LIMIT ").append(GetClause(CLAUSE_Limit));
        if (nullptr != m_limit)
            query->GetBindings().push_back(m_limit);
        if (nullptr != m_offset)
            query->GetBindings().push_back(m_offset);
        }

    return query;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComplexQueryBuilder::_IsEqual(PresentationQueryBuilder const& otherBase) const
    {
    if (!PresentationQueryBuilder::_IsEqual(otherBase))
        return false;

    auto other = otherBase.AsComplexQueryBuilder();
    if (!other)
        return false;

    // SELECT
    InitSelectClause();
    other->InitSelectClause();
    if (m_selectClause != other->m_selectClause)
        return false;

    // FROM
    if (m_nestedQuery.IsValid())
        {
        if (other->m_nestedQuery.IsNull() || !m_nestedQuery->IsEqual(*other->m_nestedQuery))
            return false;

        if (!m_nestedQueryAlias.Equals(other->m_nestedQueryAlias))
            return false;
        }
    else
        {
        if (m_from.size() != other->m_from.size())
            return false;

        for (auto const& clause : m_from)
            {
            bool found = false;
            for (auto const& otherFromClause : other->m_from)
                {
                if (*otherFromClause == *clause)
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
    if (m_joins.size() != other->m_joins.size())
        return false;

    for (auto const& joinGroup : m_joins)
        {
        bool foundGroup = false;
        for (auto const& otherJoinGroup : other->m_joins)
            {
            if (joinGroup.size() != otherJoinGroup.size())
                continue;

            bool foundAllGroupClauses = true;
            for (auto const& clause : joinGroup)
                {
                bool foundClause = false;
                for (auto const& otherClause : otherJoinGroup)
                    {
                    if (AreEqual(*clause, *otherClause))
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
    if (m_whereClause != other->m_whereClause)
        return false;

    // LIMIT
    if (!AreEqual(m_limit.get(), other->m_limit.get()))
        return false;
    if (!AreEqual(m_offset.get(), other->m_offset.get()))
        return false;

    // GROUP BY
    if (!CreateGroupByClause().Equals(other->CreateGroupByClause()))
        return false;

    // HAVING
    if (m_havingClause != other->m_havingClause)
        return false;

    // ORDER BY
    if (!m_orderByClause.Equals(other->m_orderByClause))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8CP> ComplexQueryBuilder::GetAliases(int flags, bool isRelationship) const
    {
    bvector<Utf8CP> list;

    if (0 != (IQueryInfoProvider::SELECTION_SOURCE_From & flags))
        {
        for (auto const& from : m_from)
            {
            if (!from->GetAlias().empty())
                list.push_back(from->GetAlias().c_str());
            }
        if (!m_nestedQueryAlias.empty())
            list.push_back(m_nestedQueryAlias.c_str());
        }

    if (0 != (IQueryInfoProvider::SELECTION_SOURCE_Join & flags))
        {
        for (auto const& joinPath : m_joins)
            {
            for (auto const& join : joinPath)
                {
                if (auto classWithRelationshipJoin = join->_AsClassWithRelationshipJoin())
                    {
                    if (isRelationship && !classWithRelationshipJoin->m_using.GetAlias().empty())
                        list.push_back(classWithRelationshipJoin->m_using.GetAlias().c_str());
                    else if (!isRelationship && !classWithRelationshipJoin->m_join.GetAlias().empty())
                        list.push_back(classWithRelationshipJoin->m_join.GetAlias().c_str());
                    }
                else if (auto classJoin = join->_AsClassJoin())
                    {
                    if (!isRelationship && !classJoin->m_join.GetAlias().empty())
                        list.push_back(classJoin->m_join.GetAlias().c_str());
                    }
                }
            }
        }

    return list;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8CP> ComplexQueryBuilder::_GetSelectAliases(int flags) const {return GetAliases(flags, false);}

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8CP> ComplexQueryBuilder::_GetRelationshipAliases(int flags) const {return GetAliases(flags, true);}

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UnionQueryBuilder::Init(PresentationQueryBuilder* initQuery)
    {
    if (nullptr == initQuery)
        {
        for (auto& query : m_queries)
            Init(query.get());
        return;
        }

    if (initQuery->GetContract() && initQuery->GetContract()->AsNavigationQueryContract())
        {
        GetNavigationResultParameters().MergeWith(initQuery->GetNavigationResultParameters());
        initQuery->GetNavigationResultParameters() = NavigationQueryResultParameters();
        }

    if (nullptr != initQuery->AsComplexQueryBuilder())
        {
        m_orderByClause = initQuery->AsComplexQueryBuilder()->GetClause(CLAUSE_OrderBy);
        initQuery->AsComplexQueryBuilder()->OrderBy("");
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnionQueryBuilder& UnionQueryBuilder::OrderBy(Utf8CP orderByClause)
    {
    InvalidateQuery();
    m_orderByClause = orderByClause;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnionQueryBuilder& UnionQueryBuilder::Limit(uint64_t limit, uint64_t offset)
    {
    InvalidateQuery();
    m_limit = std::make_unique<BoundQueryECValue>(ECValue(limit));
    m_offset = std::make_unique<BoundQueryECValue>(ECValue(offset));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContract const* UnionQueryBuilder::_GetContract(size_t contractId) const
    {
    for (auto const& query : m_queries)
        {
        PresentationQueryContract const* contract = query->GetContract(contractId);
        if (nullptr != contract)
            return contract;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContract const* UnionQueryBuilder::_GetGroupingContract() const
    {
    for (auto const& query : m_queries)
        {
        PresentationQueryContract const* contract = query->GetGroupingContract();
        if (nullptr != contract)
            return contract;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UnionQueryBuilder::_OnIsOuterQueryValueChanged()
    {
    for (auto& query : m_queries)
        query->SetIsOuterQuery(IsOuterQuery());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8CP> UnionQueryBuilder::_GetSelectAliases(int flags) const
    {
    bvector<Utf8CP> list;
    for (auto const& query : m_queries)
        ContainerHelpers::Push(list, query->GetSelectAliases(flags));
    return list;
    }

// /*---------------------------------------------------------------------------------**//**
// @bsimethod
// +---------------+---------------+---------------+---------------+---------------+------*/
bool UnionQueryBuilder::_IsEqual(PresentationQueryBuilder const& otherBase) const
    {
    if (!PresentationQueryBuilder::_IsEqual(otherBase))
        return false;

    auto other = otherBase.AsUnionQueryBuilder();
    if (!other)
        return false;

    if (m_queries.size() != other->m_queries.size())
        return false;

    for (auto const& query : m_queries)
        {
        bool found = false;
        for (auto const& otherQuery : other->m_queries)
            {
            if (query->IsEqual(*otherQuery))
                {
                found = true;
                break;
                }
            }
        if (!found)
            return false;
        }

    return m_orderByClause.Equals(other->m_orderByClause)
        && AreEqual(m_limit.get(), other->m_limit.get())
        && AreEqual(m_offset.get(), other->m_offset.get());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<PresentationQuery> UnionQueryBuilder::_CreateQuery() const
    {
    auto query = std::make_unique<PresentationQuery>();

    if (!m_orderByClause.empty())
        {
        // note: wrapping queries in a subquery is required until the TFS#291221 is fixed
        query->GetQueryString().append("SELECT * FROM (");
        }

    for (size_t i = 0; i < m_queries.size(); ++i)
        {
        if (i > 0)
            query->GetQueryString().append(" UNION ALL ");
        AppendQuery(*query, *m_queries[i]->GetQuery());
        }

    if (!m_orderByClause.empty())
        query->GetQueryString().append(") ORDER BY ").append(m_orderByClause);

    if (nullptr != m_limit)
        {
        query->GetQueryString().append(" LIMIT ?");
        query->GetBindings().push_back(m_limit);

        if (nullptr != m_offset)
            {
            query->GetQueryString().append(" OFFSET ?");
            query->GetBindings().push_back(m_offset);
            }
        }

    return query;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExceptQueryBuilder::Init()
    {
    if (m_base->GetContract() && m_base->GetContract()->AsNavigationQueryContract())
        {
        GetNavigationResultParameters().MergeWith(m_base->GetNavigationResultParameters());
        m_base->GetNavigationResultParameters() = NavigationQueryResultParameters();
        m_except->GetNavigationResultParameters() = NavigationQueryResultParameters();
        }

    if (nullptr != m_base->AsComplexQueryBuilder())
        {
        m_orderByClause = m_base->AsComplexQueryBuilder()->GetClause(CLAUSE_OrderBy);
        m_base->AsComplexQueryBuilder()->OrderBy("");
        }
    if (nullptr != m_except->AsComplexQueryBuilder())
        {
        m_orderByClause = m_except->AsComplexQueryBuilder()->GetClause(CLAUSE_OrderBy);
        m_except->AsComplexQueryBuilder()->OrderBy("");
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExceptQueryBuilder& ExceptQueryBuilder::OrderBy(Utf8CP orderByClause)
    {
    InvalidateQuery();
    m_orderByClause = orderByClause;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ExceptQueryBuilder& ExceptQueryBuilder::Limit(uint64_t limit, uint64_t offset)
    {
    InvalidateQuery();
    m_limit = std::make_unique<BoundQueryECValue>(ECValue(limit));
    m_offset = std::make_unique<BoundQueryECValue>(ECValue(offset));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContract const* ExceptQueryBuilder::_GetContract(size_t contractId) const
    {
    PresentationQueryContract const* contract = m_base->GetContract(contractId);
    if (nullptr != contract)
        return contract;

    contract = m_except->GetContract(contractId);
    if (nullptr != contract)
        return contract;

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContract const* ExceptQueryBuilder::_GetGroupingContract() const
    {
    PresentationQueryContract const* contract = m_base->GetGroupingContract();
    if (nullptr != contract)
        return contract;

    contract = m_except->GetGroupingContract();
    if (nullptr != contract)
        return contract;

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExceptQueryBuilder::_OnIsOuterQueryValueChanged()
    {
    m_base->SetIsOuterQuery(IsOuterQuery());
    m_except->SetIsOuterQuery(IsOuterQuery());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8CP> ExceptQueryBuilder::_GetSelectAliases(int flags) const
    {
    bvector<Utf8CP> first = m_base->GetSelectAliases(flags);
    bvector<Utf8CP> second = m_except->GetSelectAliases(flags);
    bvector<Utf8CP> list;
    list.insert(list.end(), first.begin(), first.end());
    list.insert(list.end(), second.begin(), second.end());
    return list;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExceptQueryBuilder::_IsEqual(PresentationQueryBuilder const& otherBase) const
    {
    if (!PresentationQueryBuilder::_IsEqual(otherBase))
        return false;

    auto other = otherBase.AsExceptQueryBuilder();
    if (!other)
        return false;

    return m_orderByClause.Equals(other->m_orderByClause)
        && AreEqual(m_limit.get(), other->m_limit.get())
        && AreEqual(m_offset.get(), other->m_offset.get())
        && m_base->IsEqual(*other->m_base) && m_except->IsEqual(*other->m_except);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<PresentationQuery> ExceptQueryBuilder::_CreateQuery() const
    {
    auto query = std::make_unique<PresentationQuery>();

    AppendQuery(*query, *m_base->GetQuery());
    query->GetQueryString().append(" EXCEPT ");
    AppendQuery(*query, *m_except->GetQuery());

    if (!m_orderByClause.empty())
        {
        // note: wrapping queries in a subquery is required until the TFS#291221 is fixed
        query->GetQueryString() = Utf8PrintfString("SELECT * FROM (%s) this ORDER BY %s",
            query->GetQueryString().c_str(), m_orderByClause.c_str());
        }

    if (nullptr != m_limit)
        {
        query->GetQueryString().append(" LIMIT ?");
        query->GetBindings().push_back(m_limit);

        if (nullptr != m_offset)
            {
            query->GetQueryString().append(" OFFSET ?");
            query->GetBindings().push_back(m_offset);
            }
        }

    return query;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool StringQueryBuilder::_IsEqual(PresentationQueryBuilder const& otherBase) const
    {
    if (!PresentationQueryBuilder::_IsEqual(otherBase))
        return false;

    auto other = otherBase.AsStringQueryBuilder();
    if (!other)
        return false;

    return m_query->IsEqual(*other->m_query);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<PresentationQuery> StringQueryBuilder::_CreateQuery() const
    {
    return std::make_unique<PresentationQuery>(*m_query);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RapidJsonValueComparer
{
    bool operator() (rapidjson::Value const* left, rapidjson::Value const* right) const
        {
        if (left->IsNull())
            return !right->IsNull();
        if (right->IsNull())
            return false;

        switch (left->GetType())
            {
            case rapidjson::kFalseType:
            case rapidjson::kTrueType:
                {
                return (int)left->GetBool() < (int)right->GetBool();
                }
            case rapidjson::kNumberType:
                {
                if (left->IsInt())
                    return left->GetInt() < right->GetInt();
                if (left->IsInt64())
                    return left->GetInt64() < right->GetInt64();
                if (left->IsDouble())
                    return (fabs(left->GetDouble() - right->GetDouble()) > 0.0000001 && (left->GetDouble() - right->GetDouble()) < 0);
                }
            case rapidjson::kStringType:
                {
                return strcmp(left->GetString(), right->GetString()) < 0;
                }
            case rapidjson::kObjectType:
            case rapidjson::kArrayType:
                {
                return BeRapidJsonUtilities::ToString(*left).CompareTo(BeRapidJsonUtilities::ToString(*right));
                }
            }
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unhandled rapidjson value type: %d", (int)left->GetType()));
        }
};

/*=================================================================================**//**
* @bsiclass
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
        m_jsonValues.SetArray();
        for (rapidjson::SizeType i = 0; i < values.Size(); i++)
            {
            if (PRIMITIVETYPE_Point2d == m_type || PRIMITIVETYPE_Point3d == m_type)
                {
                if (values[i].IsString())
                    m_jsonValues.PushBack(rapidjson::Value(values[i], m_jsonValues.GetAllocator()), m_jsonValues.GetAllocator());
                else if (values[i].IsObject())
                    m_jsonValues.PushBack(rapidjson::Value(BeRapidJsonUtilities::ToString(values[i]).c_str(), m_jsonValues.GetAllocator()), m_jsonValues.GetAllocator());
                else
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Expected Point value type to be a JSON object or JSON string. Actual: %d", (int)values[i].GetType()));
                }
            else
                m_jsonValues.PushBack(rapidjson::Value(values[i], m_jsonValues.GetAllocator()), m_jsonValues.GetAllocator());
            m_keys.insert(&m_jsonValues[i]);
            }
        }
    RapidJsonValueSet(RapidJsonValueSet const& other)
        : m_type(other.m_type)
        {
        m_jsonValues.CopyFrom(other.m_jsonValues, m_jsonValues.GetAllocator());
        for (rapidjson::SizeType i = 0; i < m_jsonValues.Size(); i++)
            m_keys.insert(&m_jsonValues[i]);
        }
    PrimitiveType GetValuesType() const {return m_type;}
    RapidJsonDocumentCR GetValuesJson() const {return m_jsonValues;}
    bool Equals(RapidJsonValueSet const& otherSet) const
        {
        return m_jsonValues == otherSet.m_jsonValues;
        }
    bool _IsInSet(int nVals, BeSQLite::DbValue const* vals) const override
        {
        if (nVals < 1 || nVals > 1)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Invalid number of arguments. Expected 1, got: %d", nVals));

        rapidjson::Document jsonValue;
        if (!vals[0].IsNull())
            {
            switch (m_type)
                {
                case PRIMITIVETYPE_Double:
                case PRIMITIVETYPE_DateTime:
                    jsonValue.SetDouble(vals[0].GetValueDouble());
                    break;
                case PRIMITIVETYPE_Boolean:
                    jsonValue.SetBool(vals[0].GetValueInt() != 0);
                    break;
                case PRIMITIVETYPE_Integer:
                    jsonValue.SetInt(vals[0].GetValueInt());
                    break;
                case PRIMITIVETYPE_Long:
                    jsonValue.SetInt64(vals[0].GetValueInt64());
                    break;
                case PRIMITIVETYPE_String:
                    jsonValue.SetString(vals[0].GetValueText(), jsonValue.GetAllocator());
                    break;
                case PRIMITIVETYPE_Point2d:
                case PRIMITIVETYPE_Point3d:
                    jsonValue.SetString(vals[0].GetValueText(), jsonValue.GetAllocator());
                    break;
                default:
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unhandled primitive value type: %d", (int)m_type));
                }
            }
        return (m_keys.end() != m_keys.find(&jsonValue));
        }
};

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BoundRapidJsonValueSet::BoundRapidJsonValueSet(RapidJsonValueCR values, PrimitiveType type)
    {
    m_set = std::make_unique<RapidJsonValueSet>(values, type);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BoundRapidJsonValueSet::BoundRapidJsonValueSet(BoundRapidJsonValueSet const& other)
    {
    RapidJsonValueSet const* otherVirtualSet = static_cast<RapidJsonValueSet const*>(other.m_set.get());
    m_set = std::make_unique<RapidJsonValueSet>(*otherVirtualSet);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatus BoundRapidJsonValueSet::_Bind(ECSqlStatement& stmt, uint32_t index) const
    {
    return stmt.BindVirtualSet((int)index, *m_set);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BoundRapidJsonValueSet::_Equals(BoundQueryValue const& other) const
    {
    BoundRapidJsonValueSet const* otherVirtualSet = dynamic_cast<BoundRapidJsonValueSet const*>(&other);
    if (nullptr == otherVirtualSet)
        return false;

    RapidJsonValueSet const* firstSet = static_cast<RapidJsonValueSet const*>(m_set.get());
    RapidJsonValueSet const* secondSet = static_cast<RapidJsonValueSet const*>(otherVirtualSet->m_set.get());
    return firstSet->Equals(*secondSet);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document BoundRapidJsonValueSet::_ToJson(rapidjson::Document::AllocatorType* allocator) const
    {
    auto const& set = static_cast<RapidJsonValueSet const&>(*m_set);
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("type", "value-set", json.GetAllocator());
    json.AddMember("value-type", (int)set.GetValuesType(), json.GetAllocator());
    json.AddMember("value", rapidjson::Value(set.GetValuesJson(), json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PrimitiveECValueHasher
    {
    size_t operator()(ECValueCR value) const
        {
        if (!value.IsPrimitive())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Expected only primitive values, got: %s", value.ToString().c_str()));

        if (value.IsNull())
            return 0;

        PrimitiveType type = value.GetPrimitiveType();
        size_t hash = type;
        switch (type)
            {
            case PRIMITIVETYPE_Boolean:
                hash ^= std::hash<bool>{}(value.GetBoolean()) << 2;
                break;
            case PRIMITIVETYPE_DateTime:
                hash ^= std::hash<int64_t>{}(value.GetDateTimeTicks()) << 2;
                break;
            case PRIMITIVETYPE_Double:
                hash ^= std::hash<double>{}(value.GetDouble()) << 2;
                break;
            case PRIMITIVETYPE_Integer:
                hash ^= std::hash<int32_t>{}(value.GetInteger()) << 2;
                break;
            case PRIMITIVETYPE_Long:
                hash ^= std::hash<int64_t>{}(value.GetLong()) << 2;
                break;
            case PRIMITIVETYPE_String:
                hash ^= std::hash<std::string>{}(value.GetUtf8CP()) << 2;
                break;
            case PRIMITIVETYPE_Point2d:
                {
                DPoint2d point2d = value.GetPoint2d();
                hash ^= (std::hash<double>{}(point2d.x) ^ (std::hash<double>{}(point2d.y) << 8)) << 2;
                break;
                }
            case PRIMITIVETYPE_Point3d:
                {
                DPoint3d point3d = value.GetPoint3d();
                hash ^= (std::hash<double>{}(point3d.x) ^ (std::hash<double>{}(point3d.y) << 8) ^ (std::hash<double>{}(point3d.z) << 16)) << 2;
                break;
                }
            case PRIMITIVETYPE_Binary:
            case PRIMITIVETYPE_IGeometry:
                break;
            default:
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unrecognized primitive property type: %d", (int)type));
            }
        return hash;
        }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECValueVirtualSet : BeSQLite::VirtualSet
{
private:
    std::unordered_set<ECValue, PrimitiveECValueHasher> m_values;
public:
    ECValueVirtualSet(bvector<ECValue> values)
        : m_values(ContainerHelpers::MoveTransformContainer<std::unordered_set<ECValue, PrimitiveECValueHasher>>(values))
        {}
    std::unordered_set<ECValue, PrimitiveECValueHasher> const& GetValues() const {return m_values;}
    bool Equals(ECValueVirtualSet const& otherSet) const
        {
        return m_values == otherSet.m_values;
        }
    void Insert(ECValue value) {m_values.insert(std::move(value));}
    bool _IsInSet(int nVals, BeSQLite::DbValue const* vals) const override
        {
        if (nVals < 1 || nVals > 1)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Invalid number of arguments. Expected 1, got: %d", nVals));

        if (m_values.empty())
            return false;

        // note: we expect all values to be of the same type - just pick the first
        // value and use it's type to parse sql value
        ECValueCR firstValue = *m_values.begin();
        PrimitiveType type = firstValue.GetPrimitiveType();

        ECValue value = ValueHelpers::GetECValueFromSqlValue(type, vals[0]);
        return (m_values.end() != m_values.find(value));
        }
};

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BoundECValueSet::BoundECValueSet(bvector<ECValue> values)
    : m_set(std::make_unique<ECValueVirtualSet>(std::move(values)))
    {}

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BoundECValueSet::BoundECValueSet(BoundECValueSet const& other)
    {
    ECValueVirtualSet const* otherVirtualSet = static_cast<ECValueVirtualSet const*>(other.m_set.get());
    m_set = std::make_unique<ECValueVirtualSet>(*otherVirtualSet);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatus BoundECValueSet::_Bind(ECSqlStatement& stmt, uint32_t index) const
    {
    return stmt.BindVirtualSet((int)index, *m_set);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BoundECValueSet::_Equals(BoundQueryValue const& other) const
    {
    BoundECValueSet const* otherVirtualSet = dynamic_cast<BoundECValueSet const*>(&other);
    if (nullptr == otherVirtualSet)
        return false;

    ECValueVirtualSet const* firstSet = static_cast<ECValueVirtualSet const*>(m_set.get());
    ECValueVirtualSet const* secondSet = static_cast<ECValueVirtualSet const*>(otherVirtualSet->m_set.get());
    return firstSet->Equals(*secondSet);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document BoundECValueSet::_ToJson(rapidjson::Document::AllocatorType* allocator) const
    {
    auto const& values = static_cast<ECValueVirtualSet const*>(m_set.get())->GetValues();
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("type", "value-set", json.GetAllocator());
    json.AddMember("value-type", values.empty() ? 0 : (int)(*values.begin()).GetPrimitiveType(), json.GetAllocator());
    rapidjson::Value valuesJson;
    valuesJson.SetArray();
    for (auto const& value : values)
        valuesJson.PushBack(ValueHelpers::GetJsonFromECValue(value, &json.GetAllocator()), json.GetAllocator());
    json.AddMember("value", valuesJson, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<BoundQueryValue> BoundQueryValue::FromJson(RapidJsonValueCR json)
    {
    if (!json.IsObject() || !json.HasMember("type"))
        return nullptr;

    Utf8CP type = json["type"].GetString();
    if (0 == strcmp("ec-value", type))
        {
        ECValue value = ValueHelpers::GetECValueFromJson((PrimitiveType)json["value-type"].GetInt(), json["value"]);
        return std::make_unique<BoundQueryECValue>(std::move(value));
        }
    if (0 == strcmp("value-set", type))
        {
        int valueType = json["value-type"].GetInt();
        if (0 == valueType)
            return std::make_unique<BoundECValueSet>(bvector<ECValue>());
        return std::make_unique<BoundRapidJsonValueSet>(json["value"], (PrimitiveType)valueType);
        }
    if (0 == strcmp("id", type))
        {
        return std::make_unique<BoundQueryId>(json["value"].GetString());
        }
    if (0 == strcmp("id-set", type))
        {
        RapidJsonValueCR idsJson = json["value"];
        bvector<BeInt64Id> ids;
        for (rapidjson::SizeType i = 0; i < idsJson.Size(); ++i)
            ids.push_back(BeInt64Id::FromString(idsJson[i].GetString()));
        return std::make_unique<BoundQueryIdSet>(ids);
        }
    return nullptr;
    }
