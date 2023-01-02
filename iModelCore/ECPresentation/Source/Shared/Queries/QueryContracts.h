/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/Content.h>
#include <ECPresentation/Rules/PresentationRulesTypes.h>
#include "../../RulesEngineTypes.h"
#include "QueryBuilding.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class FieldVisibility
    {
    Inner = 1 << 0,
    Outer = 1 << 1,
    Both  = Inner | Outer,
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class PresentationQueryFieldType
    {
    Unknown,
    Primitive,
    Enum,
    LabelDefinition,
    NavigationPropertyValue,
    };

struct PresentationQueryContractSimpleField;
struct PresentationQueryContractFunctionField;
struct PresentationQueryContractDynamicField;
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PresentationQueryContractField : RefCountedBase
{
private:
    Utf8String m_defaultName;
    Utf8String m_name;
    Utf8String m_groupingClause;
    bool m_allowsPrefix;
    bool m_isAggregateField;
    FieldVisibility m_fieldVisibility;
    Utf8String m_prefixOverride;
    PresentationQueryFieldType m_resultType;
    BoundQueryValuesList m_bindings;

protected:
    PresentationQueryContractField(Utf8CP defaultName, bool allowsPrefix, bool isAggregateField, FieldVisibility fieldVisibility)
        : m_defaultName(defaultName), m_allowsPrefix(allowsPrefix), m_isAggregateField(isAggregateField), m_fieldVisibility(fieldVisibility), m_resultType(PresentationQueryFieldType::Unknown)
        {}
    ~PresentationQueryContractField() {}
    bool AllowsPrefix() const {return m_allowsPrefix;}
    virtual QueryClauseAndBindings _GetSelectClause(Utf8CP prefix, std::function<bool(Utf8CP)> const& useFieldNames) const = 0;
    virtual PresentationQueryContractSimpleField* _AsPresentationQueryContractSimpleField() {return nullptr;}
    virtual PresentationQueryContractFunctionField* _AsPresentationQueryContractFunctionField() {return nullptr;}

public:
    Utf8CP     GetDefaultName() const {return m_defaultName.c_str();}
    Utf8CP     GetName() const {return m_name.empty() ? GetDefaultName() : m_name.c_str();}
    void       SetName(Utf8CP name) {m_name = name;}
    Utf8CP     GetGroupingClause() const {return !m_groupingClause.empty() ? m_groupingClause.c_str() : GetName();}
    void       SetGroupingClause(Utf8String name) {m_groupingClause = name;}
    bool       IsAggregateField() const {return m_isAggregateField;}
    FieldVisibility GetVisibility() const {return m_fieldVisibility;}
    void       SetPrefixOverride(Utf8String prefix) {m_prefixOverride = prefix;}
    QueryClauseAndBindings GetSelectClause(Utf8CP prefix = nullptr, std::function<bool(Utf8CP)> const& useFieldNames = nullptr) const {return _GetSelectClause(m_prefixOverride.empty() ? prefix : m_prefixOverride.c_str(), useFieldNames);}
    PresentationQueryFieldType GetResultType() const {return m_resultType;}
    void SetResultType(PresentationQueryFieldType type) {m_resultType = type;}

    bool IsPresentationQueryContractSimpleField() const {return nullptr != AsPresentationQueryContractSimpleField();}
    PresentationQueryContractSimpleField* AsPresentationQueryContractSimpleField() {return _AsPresentationQueryContractSimpleField();}
    PresentationQueryContractSimpleField const* AsPresentationQueryContractSimpleField() const {return const_cast<PresentationQueryContractField*>(this)->_AsPresentationQueryContractSimpleField();}

    bool IsPresentationQueryContractFunctionField() const {return nullptr != AsPresentationQueryContractFunctionField();}
    PresentationQueryContractFunctionField* AsPresentationQueryContractFunctionField() {return _AsPresentationQueryContractFunctionField();}
    PresentationQueryContractFunctionField const* AsPresentationQueryContractFunctionField() const {return const_cast<PresentationQueryContractField*>(this)->_AsPresentationQueryContractFunctionField();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryContractSimpleField : PresentationQueryContractField
{
private:
    QueryClauseAndBindings m_selectClause;

private:
    PresentationQueryContractSimpleField(Utf8CP name, QueryClauseAndBindings selectClause, bool allowsPrefix, bool isAggregateField, FieldVisibility fieldVisibility)
        : PresentationQueryContractField(name, allowsPrefix, isAggregateField, fieldVisibility), m_selectClause(selectClause)
        {}

protected:
    ECPRESENTATION_EXPORT QueryClauseAndBindings _GetSelectClause(Utf8CP prefix, std::function<bool(Utf8CP)> const&) const override;
    PresentationQueryContractSimpleField* _AsPresentationQueryContractSimpleField() override {return this;}

public:
    static RefCountedPtr<PresentationQueryContractSimpleField> Create(Utf8CP name, QueryClauseAndBindings selectClause, bool allowsPrefix = true, bool isAggregateField = false, FieldVisibility fieldVisibility = FieldVisibility::Both)
        {
        return new PresentationQueryContractSimpleField(name, selectClause, allowsPrefix, isAggregateField, fieldVisibility);
        }
    void SetClause(QueryClauseAndBindings clause) {m_selectClause = clause;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryContractFunctionField : PresentationQueryContractField
{
private:
    Utf8String m_functionName;
    bvector<RefCountedPtr<PresentationQueryContractField const>> m_parameters;
    bool m_distinctArguments;

protected:
    PresentationQueryContractFunctionField(Utf8CP name, Utf8CP functionName, bvector<RefCountedPtr<PresentationQueryContractField const>> const& parameters, bool allowsPrefix,
        bool isAggregateField, FieldVisibility fieldVisibility)
        : PresentationQueryContractField(name, allowsPrefix, isAggregateField, fieldVisibility), m_functionName(functionName), m_parameters(parameters), m_distinctArguments(false)
        {}
    virtual ECPRESENTATION_EXPORT QueryClauseAndBindings _GetSelectClause(Utf8CP prefix, std::function<bool(Utf8CP)> const&) const override;
    PresentationQueryContractFunctionField* _AsPresentationQueryContractFunctionField() override {return this;}

public:
    static RefCountedPtr<PresentationQueryContractFunctionField> Create(Utf8CP name, Utf8CP functionName, bvector<PresentationQueryContractFieldCPtr> const& functionParameters,
        bool allowsPrefix = true, bool isAggregateField = false, FieldVisibility fieldVisibility = FieldVisibility::Both)
        {
        return new PresentationQueryContractFunctionField(name, functionName, functionParameters, allowsPrefix, isAggregateField, fieldVisibility);
        }

    bvector<RefCountedPtr<PresentationQueryContractField const>>& GetFunctionParameters() {return m_parameters;}
    bvector<RefCountedPtr<PresentationQueryContractField const>> const& GetFunctionParameters() const {return m_parameters;}

    void SetDistinctArguments(bool value) {m_distinctArguments = value;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IPresentationQueryFieldTypesProvider
{
protected:
    virtual PresentationQueryFieldType _GetFieldType(Utf8StringCR name) const = 0;
public:
    virtual ~IPresentationQueryFieldTypesProvider() {}
    PresentationQueryFieldType GetFieldType(Utf8StringCR name) const {return _GetFieldType(name);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryContract : RefCountedBase, IPresentationQueryFieldTypesProvider
{
public:
    ECPRESENTATION_EXPORT static Utf8CP RelatedInstanceInfoFieldName;

private:
    uint64_t m_id;

protected:
    PresentationQueryContract(uint64_t id = 0) : m_id(id) {}
    virtual ~PresentationQueryContract() {}
    virtual NavigationQueryContract const* _AsNavigationQueryContract() const {return nullptr;}
    virtual ContentQueryContract const* _AsContentQueryContract() const {return nullptr;}
    virtual bvector<PresentationQueryContractFieldCPtr> _GetFields() const = 0;
    ECPRESENTATION_EXPORT PresentationQueryFieldType _GetFieldType(Utf8StringCR name) const override;
    virtual void _SetECInstanceIdFieldName(Utf8CP name){}
    virtual void _SetECClassIdFieldName(Utf8CP name){}

public:
    NavigationQueryContract const* AsNavigationQueryContract() const {return _AsNavigationQueryContract();}
    ContentQueryContract const* AsContentQueryContract() const {return _AsContentQueryContract();}
    uint64_t GetId() const {return m_id;}
    ECPRESENTATION_EXPORT uint8_t GetIndex(Utf8CP fieldName) const;
    void SetECInstanceIdFieldName(Utf8CP name) {_SetECInstanceIdFieldName(name);}
    void SetECClassIdFieldName(Utf8CP name) {_SetECClassIdFieldName(name);}
    ECPRESENTATION_EXPORT bool IsAggregating() const;
    ECPRESENTATION_EXPORT bool HasInnerFields() const;
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> GetFields() const;
    ECPRESENTATION_EXPORT PresentationQueryContractFieldCPtr GetField(Utf8CP name) const;
    ECPRESENTATION_EXPORT bvector<Utf8String> GetGroupingAliases() const;

public:
    static PresentationQueryContractFieldPtr CreateRelatedInstanceInfoField(bvector<RelatedClassPath> const&);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SimpleQueryContract : PresentationQueryContract
{
private:
    bvector<PresentationQueryContractFieldCPtr> m_fields;
protected:
    ECPRESENTATION_EXPORT SimpleQueryContract(bvector<PresentationQueryContractFieldCPtr> = bvector<PresentationQueryContractFieldCPtr>());
public:
    static RefCountedPtr<SimpleQueryContract> Create() {return new SimpleQueryContract();}
    static RefCountedPtr<SimpleQueryContract> Create(PresentationQueryContractFieldCR field) {return new SimpleQueryContract({&field});}
    static RefCountedPtr<SimpleQueryContract> Create(bvector<PresentationQueryContractFieldCPtr> fields) {return new SimpleQueryContract(fields);}
    bvector<PresentationQueryContractFieldCPtr> _GetFields() const {return m_fields;}
    void AddField(PresentationQueryContractFieldCR field) {m_fields.push_back(&field);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CountQueryContract : SimpleQueryContract
{
public:
    ECPRESENTATION_EXPORT static Utf8CP CountFieldName;
private:
    ECPRESENTATION_EXPORT CountQueryContract(Utf8CP groupingFieldName);
public:
    static RefCountedPtr<CountQueryContract> Create(Utf8CP groupingFieldName = nullptr) {return new CountQueryContract(groupingFieldName);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TContract>
struct IContractProvider
{
protected:
    virtual TContract const* _GetContract(size_t contractId) const = 0;
public:
    virtual ~IContractProvider() {}
    TContract const* GetContract(size_t contractId = 0) const {return _GetContract(contractId);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryContractsFilter : IContractProvider<NavigationQueryContract>
{
private:
    IContractProvider<PresentationQueryContract> const& m_source;
protected:
    NavigationQueryContract const* _GetContract(size_t contractId) const override
        {
        auto contract = m_source.GetContract(contractId);
        return contract ? contract->AsNavigationQueryContract() : nullptr;
        }
public:
    NavigationQueryContractsFilter(IContractProvider<PresentationQueryContract> const& source)
        : m_source(source)
        {}
};
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentQueryContractsFilter : IContractProvider<ContentQueryContract>
{
private:
    IContractProvider<PresentationQueryContract> const& m_source;
protected:
    ContentQueryContract const* _GetContract(size_t contractId) const override
        {
        auto contract = m_source.GetContract(contractId);
        return contract ? contract->AsContentQueryContract() : nullptr;
        }
public:
    ContentQueryContractsFilter(IContractProvider<PresentationQueryContract> const& source)
        : m_source(source)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryContractHelpers
    {
    QueryContractHelpers() = delete;
    QueryContractHelpers(QueryContractHelpers const&) = delete;
    QueryContractHelpers(QueryContractHelpers&&) = delete;
    static PresentationQueryContractFieldPtr CreatePointAsJsonStringSelectField(Utf8StringCR propertyName, Utf8StringCR prefix, int dimensions);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
