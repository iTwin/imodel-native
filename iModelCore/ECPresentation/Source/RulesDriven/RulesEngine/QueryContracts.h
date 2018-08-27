/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/QueryContracts.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/Content.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRulesTypes.h>
#include "RulesEngineTypes.h"
#include "ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2016
+===============+===============+===============+===============+===============+======*/
enum class FieldVisibility
    {
    Inner = 1 << 0,
    Outer = 1 << 1,
    Both  = Inner | Outer,
    };

struct PresentationQueryContractSimpleField;
struct PresentationQueryContractFunctionField;
struct PresentationQueryContractDynamicField;
struct PresentationQueryMergeField;
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
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
 
protected:
    PresentationQueryContractField(Utf8CP defaultName, bool allowsPrefix, bool isAggregateField, FieldVisibility fieldVisibility) 
        : m_defaultName(defaultName), m_allowsPrefix(allowsPrefix), m_isAggregateField(isAggregateField), m_fieldVisibility(fieldVisibility) {}
    ~PresentationQueryContractField() {}
    bool AllowsPrefix() const {return m_allowsPrefix;}
    virtual Utf8String _GetSelectClause(Utf8CP prefix, bool useFieldNames) const = 0;
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
    Utf8String GetSelectClause(Utf8CP prefix = nullptr, bool useFieldNames = false) const {return _GetSelectClause(m_prefixOverride.empty() ? prefix : m_prefixOverride.c_str(), useFieldNames);}

    bool IsPresentationQueryContractSimpleField() const {return nullptr != AsPresentationQueryContractSimpleField();}
    PresentationQueryContractSimpleField* AsPresentationQueryContractSimpleField() {return _AsPresentationQueryContractSimpleField();}
    PresentationQueryContractSimpleField const* AsPresentationQueryContractSimpleField() const {return const_cast<PresentationQueryContractField*>(this)->_AsPresentationQueryContractSimpleField();}

    bool IsPresentationQueryContractFunctionField() const {return nullptr != AsPresentationQueryContractFunctionField();}
    PresentationQueryContractFunctionField* AsPresentationQueryContractFunctionField() {return _AsPresentationQueryContractFunctionField();}
    PresentationQueryContractFunctionField const* AsPresentationQueryContractFunctionField() const {return const_cast<PresentationQueryContractField*>(this)->_AsPresentationQueryContractFunctionField();}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryContractSimpleField : PresentationQueryContractField
{
private:
    Utf8String m_selectClause;

private:
    PresentationQueryContractSimpleField(Utf8CP name, Utf8CP selectClause, bool allowsPrefix, bool isAggregateField, FieldVisibility fieldVisibility) 
        : PresentationQueryContractField(name, allowsPrefix, isAggregateField, fieldVisibility), m_selectClause(selectClause)
        {}

protected:
    ECPRESENTATION_EXPORT Utf8String _GetSelectClause(Utf8CP prefix, bool) const override;
    PresentationQueryContractSimpleField* _AsPresentationQueryContractSimpleField() override {return this;}

public:
    static RefCountedPtr<PresentationQueryContractSimpleField> Create(Utf8CP name, Utf8CP selectClause, bool allowsPrefix = true, bool isAggregateField = false, FieldVisibility fieldVisibility = FieldVisibility::Both)
        {
        return new PresentationQueryContractSimpleField(name, selectClause, allowsPrefix, isAggregateField, fieldVisibility);
        }
    void SetClause(Utf8CP clause) {m_selectClause = clause;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryContractFunctionField : PresentationQueryContractField
{
private:
    Utf8String m_functionName;
    bvector<RefCountedPtr<PresentationQueryContractField const>> m_parameters;

protected:
    PresentationQueryContractFunctionField(Utf8CP name, Utf8CP functionName, bvector<RefCountedPtr<PresentationQueryContractField const>> const& parameters, bool allowsPrefix, 
        bool isAggregateField, FieldVisibility fieldVisibility) 
        : PresentationQueryContractField(name, allowsPrefix, isAggregateField, fieldVisibility), m_functionName(functionName), m_parameters(parameters)
        {}
    virtual ECPRESENTATION_EXPORT Utf8String _GetSelectClause(Utf8CP prefix, bool) const override;
    PresentationQueryContractFunctionField* _AsPresentationQueryContractFunctionField() override {return this;}

public:
    static RefCountedPtr<PresentationQueryContractFunctionField> Create(Utf8CP name, Utf8CP functionName, bvector<RefCountedPtr<PresentationQueryContractField const>> const& functionParameters, bool allowsPrefix = true, 
        bool isAggregateField = false, FieldVisibility fieldVisibility = FieldVisibility::Both)
        {
        return new PresentationQueryContractFunctionField(name, functionName, functionParameters, allowsPrefix, isAggregateField, fieldVisibility);
        }

    bvector<RefCountedPtr<PresentationQueryContractField const>>& GetFunctionParameters() {return m_parameters;}
    bvector<RefCountedPtr<PresentationQueryContractField const>> const& GetFunctionParameters() const {return m_parameters;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryContractDynamicField : PresentationQueryContractField
{
private:
    std::function<Utf8String(Utf8CP)> m_getSelectClauseHandler;

private:
    PresentationQueryContractDynamicField(Utf8CP name, std::function<Utf8String(Utf8CP)> handler, bool allowsPrefix, bool isAggregateField, FieldVisibility fieldVisibility) 
        : PresentationQueryContractField(name, allowsPrefix, isAggregateField, fieldVisibility), m_getSelectClauseHandler(handler)
        {}

protected:
    ECPRESENTATION_EXPORT Utf8String _GetSelectClause(Utf8CP prefix, bool) const override;

public:
    static RefCountedPtr<PresentationQueryContractDynamicField> Create(Utf8CP name, std::function<Utf8String(Utf8CP)> handler, bool allowsPrefix = true, bool isAggregateField = false, FieldVisibility fieldVisibility = FieldVisibility::Both)
        {
        return new PresentationQueryContractDynamicField(name, handler, allowsPrefix, isAggregateField, fieldVisibility);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryMergeField : PresentationQueryContractField
{
private:
    PresentationQueryContractFieldCPtr m_mergedField;
    Utf8String m_mergedValueResult;

private:
    PresentationQueryMergeField(Utf8CP name, PresentationQueryContractFieldCR mergedField, Utf8String mergedValueResult, bool allowsPrefix, bool isAggregateField, FieldVisibility fieldVisibility) 
        : PresentationQueryContractField(name, allowsPrefix, isAggregateField, fieldVisibility), m_mergedField(&mergedField), m_mergedValueResult(mergedValueResult)
        {}

protected:
    ECPRESENTATION_EXPORT Utf8String _GetSelectClause(Utf8CP prefix, bool) const override;

public:
    static RefCountedPtr<PresentationQueryMergeField> Create(Utf8CP name, PresentationQueryContractFieldCR mergedField, Utf8String mergedValueResult = Utf8String(),
        bool allowsPrefix = true, bool isAggregateField = false, FieldVisibility fieldVisibility = FieldVisibility::Both)
        {
        return new PresentationQueryMergeField(name, mergedField, mergedValueResult, allowsPrefix, isAggregateField, fieldVisibility);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryContract : RefCountedBase
{
public:
    ECPRESENTATION_EXPORT static Utf8CP RelatedInstanceInfoFieldName;

private:
    uint64_t m_id;

protected:
    PresentationQueryContract(uint64_t id = 0) : m_id(id) {}
    virtual ~PresentationQueryContract() {}
    virtual bvector<PresentationQueryContractFieldCPtr> _GetFields() const = 0;
    virtual void _SetECInstanceIdFieldName(Utf8CP name){}
    virtual void _SetECClassIdFieldName(Utf8CP name){}
    static RefCountedPtr<PresentationQueryContractSimpleField> CreateRelatedInstanceInfoField(bvector<RelatedClass> const&);

public:
    uint64_t GetId() const {return m_id;}
    ECPRESENTATION_EXPORT uint8_t GetIndex(Utf8CP fieldName) const;
    void SetECInstanceIdFieldName(Utf8CP name) {_SetECInstanceIdFieldName(name);}
    void SetECClassIdFieldName(Utf8CP name) {_SetECClassIdFieldName(name);}
    ECPRESENTATION_EXPORT bool IsAggregating() const;
    ECPRESENTATION_EXPORT bool HasInnerFields() const;
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> GetFields() const;
    ECPRESENTATION_EXPORT PresentationQueryContractFieldCPtr GetField(Utf8CP name) const;
    ECPRESENTATION_EXPORT bvector<Utf8CP> GetGroupingAliases() const;
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE NavigationQueryContract : PresentationQueryContract
{
public:
    ECPRESENTATION_EXPORT static Utf8CP SkippedInstanceKeysFieldName;
    ECPRESENTATION_EXPORT static Utf8CP SkippedInstanceKeysInternalFieldName;

private:
    mutable PresentationQueryContractFieldCPtr m_skippedInstanceKeysField;
    mutable PresentationQueryContractFieldCPtr m_skippedInstanceKeysInternalField;
    RelatedClassPath m_relationshipPath;

protected:
    virtual NavigationQueryResultType _GetResultType() const = 0;
    ECPRESENTATION_EXPORT virtual bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;

public:
    NavigationQueryResultType GetResultType() const {return _GetResultType();}
    RelatedClassPath const& GetRelationshipPath() const {return m_relationshipPath;}
    void SetRelationshipPath(RelatedClassPath path) {m_relationshipPath = path;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECInstanceNodesQueryContract : NavigationQueryContract
{
friend struct NavigationQueryContract;

public:
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECClassIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP DisplayLabelFieldName;

private:
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_displayLabelField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecClassIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_relatedInstanceInfoField;
    
private:
    ECPRESENTATION_EXPORT ECInstanceNodesQueryContract(ECClassCP, bvector<RelatedClass> const&, bvector<ECPropertyCP> const&);
    RefCountedPtr<PresentationQueryContractFunctionField> CreateDisplayLabelField(ECClassCP, bvector<RelatedClass> const&, bvector<ECPropertyCP> const&) const;

protected:
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::ECInstanceNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<ECInstanceNodesQueryContract> Create(ECClassCP ecClass, bvector<RelatedClass> const& relatedClasses = bvector<RelatedClass>(), bvector<ECPropertyCP> const& labelOverrides = bvector<ECPropertyCP>())
        {
        return new ECInstanceNodesQueryContract(ecClass, relatedClasses, labelOverrides);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECRelationshipGroupingNodesQueryContract : NavigationQueryContract
{
friend struct NavigationQueryContract;

private:
    ECRelationshipGroupingNodesQueryContract() {}

protected:
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::ECRelationshipClassNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;

public:
    static RefCountedPtr<ECRelationshipGroupingNodesQueryContract> Create() {return new ECRelationshipGroupingNodesQueryContract();}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECClassGroupingNodesQueryContract : NavigationQueryContract
{
friend struct NavigationQueryContract;

public:
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECClassIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP DisplayLabelFieldName;
    ECPRESENTATION_EXPORT static Utf8CP GroupedInstanceIdsFieldName;

private:
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecClassIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_dummyClassIdField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_displayLabelField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_groupedInstanceIdsField;

private:
    ECPRESENTATION_EXPORT ECClassGroupingNodesQueryContract();

protected:
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::ClassGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<ECClassGroupingNodesQueryContract> Create() {return new ECClassGroupingNodesQueryContract();}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE DisplayLabelGroupingNodesQueryContract : NavigationQueryContract
{
friend struct NavigationQueryContract;

public:
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECClassIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP DisplayLabelFieldName;
    ECPRESENTATION_EXPORT static Utf8CP GroupedInstanceIdsFieldName;

private:
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecClassIdField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_displayLabelField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_groupedInstanceIdsField;

private:
    ECPRESENTATION_EXPORT DisplayLabelGroupingNodesQueryContract(ECClassCP, bool, bvector<RelatedClass> const&, bvector<ECPropertyCP> const&);
    RefCountedPtr<PresentationQueryContractFunctionField> CreateDisplayLabelField(ECClassCP, bvector<RelatedClass> const&, bvector<ECPropertyCP> const&) const;

protected:
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::DisplayLabelGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<DisplayLabelGroupingNodesQueryContract> Create(ECClassCP ecClass, 
        bool includeGroupedInstanceIdsField = true, bvector<RelatedClass> const& relatedClasses = bvector<RelatedClass>(), bvector<ECPropertyCP> const& labelOverrides = bvector<ECPropertyCP>())
        {
        return new DisplayLabelGroupingNodesQueryContract(ecClass, includeGroupedInstanceIdsField, relatedClasses, labelOverrides);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BaseECClassGroupingNodesQueryContract : NavigationQueryContract
{
friend struct NavigationQueryContract;

public:
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECClassIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP BaseClassIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP DisplayLabelFieldName;
    ECPRESENTATION_EXPORT static Utf8CP GroupedInstanceIdsFieldName;

private:
    ECClassId m_baseClassId;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecClassIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_baseClassIdField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_displayLabelField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_groupedInstanceIdsField;

private:
    ECPRESENTATION_EXPORT BaseECClassGroupingNodesQueryContract(ECClassId baseClassId);
    Utf8String GetDisplayLabelClause(Utf8CP);

protected:
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::BaseClassGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<BaseECClassGroupingNodesQueryContract> Create(ECClassId baseClassId) {return new BaseECClassGroupingNodesQueryContract(baseClassId);}
    ECClassId GetBaseClassId() const {return m_baseClassId;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECPropertyGroupingNodesQueryContract : NavigationQueryContract
{
friend struct NavigationQueryContract;

public:
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECClassIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECPropertyClassIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECPropertyNameFieldName;
    ECPRESENTATION_EXPORT static Utf8CP DisplayLabelFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ImageIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP IsRangeFieldName;
    ECPRESENTATION_EXPORT static Utf8CP GroupedInstanceIdsFieldName;
    ECPRESENTATION_EXPORT static Utf8CP GroupingValueFieldName;
    ECPRESENTATION_EXPORT static Utf8CP GroupingValuesFieldName;

private:
    ECPropertyCR m_property;
    PropertyGroupCR m_specification;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecClassIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecPropertyClassIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecPropertyNameField;
    RefCountedPtr<PresentationQueryContractField> m_displayLabelField;
    RefCountedPtr<PresentationQueryContractDynamicField> m_imageIdField;
    RefCountedPtr<PresentationQueryContractField> m_groupingValueField;
    RefCountedPtr<PresentationQueryContractDynamicField> m_groupingValuesField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_isRangeField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_groupedInstanceIdsField;
    ECClassCP m_foreignKeyClass;
    Utf8String m_groupingPropertyClassAlias;

private:
    ECPRESENTATION_EXPORT ECPropertyGroupingNodesQueryContract(ECClassCR, ECPropertyCR, Utf8String, PropertyGroupCR, ECClassCP);
    static Utf8String GetGroupingValueClause(ECPropertyCR);
    RefCountedPtr<PresentationQueryContractField> CreateDisplayLabelField() const;
    Utf8String GetImageIdClause(Utf8CP) const;
    Utf8String GetGroupingValuesClause(Utf8CP) const;
    Utf8String GetPropertyValueClause(Utf8CP) const;

protected:
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::PropertyGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<ECPropertyGroupingNodesQueryContract> Create(ECClassCR ecClass, ECPropertyCR prop, Utf8String groupingPropertyClassAlias, PropertyGroupCR spec, ECClassCP foreignKeyClass)
        {
        return new ECPropertyGroupingNodesQueryContract(ecClass, prop, groupingPropertyClassAlias, spec, foreignKeyClass);
        }
    ECPropertyCR GetProperty() const {return m_property;}
    Utf8StringCR GetGroupingPropertyClassAlias() const {return m_groupingPropertyClassAlias;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
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
public:
    bvector<Utf8CP> GetSelectAliases(int flags = SELECTION_SOURCE_All) const {return _GetSelectAliases(flags);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct ContentQueryContract : PresentationQueryContract
{
public:
    ECPRESENTATION_EXPORT static Utf8CP ContractIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceKeysFieldName;

private:
    PresentationQueryContractFieldPtr m_ecInstanceKeysField;
    mutable RefCountedPtr<PresentationQueryContractFunctionField> m_displayLabelField;
    ContentDescriptorCPtr m_descriptor;
    ECClassCP m_class;
    IQueryInfoProvider const& m_queryInfo;
    bvector<RelatedClass> m_relatedClasses;
    bool m_skipCompositePropertyFields;

private:
    ECPRESENTATION_EXPORT ContentQueryContract(uint64_t id, ContentDescriptorCR descriptor, ECClassCP ecClass, IQueryInfoProvider const&, bvector<RelatedClass> const&, bool);
    PresentationQueryContractFunctionField const& GetDisplayLabelField(ContentDescriptor::DisplayLabelField const& field) const;
    PresentationQueryContractFieldCPtr GetCalculatedPropertyField(Utf8String const&, Utf8String const&, bool) const;
    PresentationQueryContractFieldCPtr CreateInstanceKeyField(Utf8CP fieldName, Utf8CP alias, ECClassId defaultClassId, bool isMerging) const;
    PresentationQueryContractFieldCPtr CreateInstanceKeyField(ContentDescriptor::ECInstanceKeyField const&, bool isMerging) const;

protected:
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;

public:
    static ContentQueryContractPtr Create(uint64_t id, ContentDescriptorCR descriptor, ECClassCP ecClass, IQueryInfoProvider const& queryInfo, bvector<RelatedClass> const& relatedClasses = bvector<RelatedClass>(), bool skipCompositePropertyFields = true)
        {
        return new ContentQueryContract(id, descriptor, ecClass, queryInfo, relatedClasses, skipCompositePropertyFields);
        }
    ContentDescriptorCR GetDescriptor() const {return *m_descriptor;}
    ContentDescriptor::Property const* FindMatchingProperty(ContentDescriptor::ECPropertiesField const&, ECClassCP) const;
    bool ShouldSkipCompositePropertyFields() const {return m_skipCompositePropertyFields;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
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
* @bsiclass                                     Grigas.Petraitis                05/2016
+===============+===============+===============+===============+===============+======*/
struct CountQueryContract : SimpleQueryContract
{
public:
    ECPRESENTATION_EXPORT static Utf8CP CountFieldName;
private:
    ECPRESENTATION_EXPORT CountQueryContract();
public:
    static RefCountedPtr<CountQueryContract> Create() {return new CountQueryContract();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
