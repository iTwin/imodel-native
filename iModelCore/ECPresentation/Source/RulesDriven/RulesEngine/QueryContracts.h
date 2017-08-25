/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/QueryContracts.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/Content.h>
#include <ECPresentationRules/PresentationRulesTypes.h>
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

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct PresentationQueryContractField : RefCountedBase
{
private:
    Utf8String m_defaultName;
    Utf8String m_name;
    bool m_isDistinct;
    bool m_allowsPrefix;
    bool m_isAggregateField;
    FieldVisibility m_fieldVisibility;
    Utf8String m_prefixOverride;
 
protected:
    PresentationQueryContractField(Utf8CP defaultName, bool allowsPrefix, bool isDistinct, bool isAggregateField, FieldVisibility fieldVisibility) 
        : m_defaultName(defaultName), m_allowsPrefix(allowsPrefix), m_isAggregateField(isAggregateField), m_fieldVisibility(fieldVisibility), m_isDistinct(isDistinct) {}
    ~PresentationQueryContractField() {}
    bool AllowsPrefix() const {return m_allowsPrefix;}
    virtual Utf8String _GetSelectClause(Utf8CP prefix, bool useFieldNames) const = 0;

public:
    Utf8CP     GetDefaultName() const {return m_defaultName.c_str();}
    Utf8CP     GetName() const {return m_name.empty() ? GetDefaultName() : m_name.c_str();}
    void       SetName(Utf8CP name) {m_name = name;}
    bool       IsAggregateField() const {return m_isAggregateField;}
    FieldVisibility GetVisibility() const {return m_fieldVisibility;}
    bool       IsDistinct() const {return m_isDistinct;}
    void       SetPrefixOverride(Utf8String prefix) {m_prefixOverride = prefix;}
    Utf8String GetSelectClause(Utf8CP prefix = nullptr, bool useFieldNames = false) const {return _GetSelectClause(m_prefixOverride.empty() ? prefix : m_prefixOverride.c_str(), useFieldNames);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryContractSimpleField : PresentationQueryContractField
{
private:
    Utf8String m_selectClause;

private:
    PresentationQueryContractSimpleField(Utf8CP name, Utf8CP selectClause, bool allowsPrefix, bool isDistinct, bool isAggregateField, FieldVisibility fieldVisibility) 
        : PresentationQueryContractField(name, allowsPrefix, isDistinct, isAggregateField, fieldVisibility), m_selectClause(selectClause)
        {}

protected:
    ECPRESENTATION_EXPORT Utf8String _GetSelectClause(Utf8CP prefix, bool) const override;

public:
    static RefCountedPtr<PresentationQueryContractSimpleField> Create(Utf8CP name, Utf8CP selectClause, bool allowsPrefix = true, bool isDistinct = false, bool isAggregateField = false, FieldVisibility fieldVisibility = FieldVisibility::Both)
        {
        return new PresentationQueryContractSimpleField(name, selectClause, allowsPrefix, isDistinct, isAggregateField, fieldVisibility);
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
    bvector<Utf8String> m_functionParameters;
    
protected:
    PresentationQueryContractFunctionField(Utf8CP name, Utf8CP functionName, bvector<Utf8String> const& functionParameters, bool allowsPrefix, bool isDistinct, bool isAggregateField, FieldVisibility fieldVisibility) 
        : PresentationQueryContractField(name, allowsPrefix, isDistinct, isAggregateField, fieldVisibility), m_functionName(functionName), m_functionParameters(functionParameters)
        {}
    virtual ECPRESENTATION_EXPORT Utf8String _GetSelectClause(Utf8CP prefix, bool) const override;

public:
    static RefCountedPtr<PresentationQueryContractFunctionField> Create(Utf8CP name, Utf8CP functionName, bvector<Utf8String> const& functionParameters, bool allowsPrefix = true, bool isDistinct = false, bool isAggregateField = false, FieldVisibility fieldVisibility = FieldVisibility::Both)
        {
        return new PresentationQueryContractFunctionField(name, functionName, functionParameters, allowsPrefix, isDistinct, isAggregateField, fieldVisibility);
        }
    bvector<Utf8String>& GetFunctionParametersR() {return m_functionParameters;}
    bvector<Utf8String> const& GetFunctionParameters() const {return m_functionParameters;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryContractDynamicField : PresentationQueryContractField
{
private:
    std::function<Utf8String(Utf8CP)> m_getSelectClauseHandler;

private:
    PresentationQueryContractDynamicField(Utf8CP name, std::function<Utf8String(Utf8CP)> handler, bool allowsPrefix, bool isDistinct, bool isAggregateField, FieldVisibility fieldVisibility) 
        : PresentationQueryContractField(name, allowsPrefix, isDistinct, isAggregateField, fieldVisibility), m_getSelectClauseHandler(handler)
        {}

protected:
    ECPRESENTATION_EXPORT Utf8String _GetSelectClause(Utf8CP prefix, bool) const override;

public:
    static RefCountedPtr<PresentationQueryContractDynamicField> Create(Utf8CP name, std::function<Utf8String(Utf8CP)> handler, bool allowsPrefix = true, bool isDistinct = false, bool isAggregateField = false, FieldVisibility fieldVisibility = FieldVisibility::Both)
        {
        return new PresentationQueryContractDynamicField(name, handler, allowsPrefix, isDistinct, isAggregateField, fieldVisibility);
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
    PresentationQueryMergeField(Utf8CP name, PresentationQueryContractFieldCR mergedField, Utf8String mergedValueResult, bool allowsPrefix, bool isDistinct, bool isAggregateField, FieldVisibility fieldVisibility) 
        : PresentationQueryContractField(name, allowsPrefix, isDistinct, isAggregateField, fieldVisibility), m_mergedField(&mergedField), m_mergedValueResult(mergedValueResult)
        {}

protected:
    ECPRESENTATION_EXPORT Utf8String _GetSelectClause(Utf8CP prefix, bool) const override;

public:
    static RefCountedPtr<PresentationQueryMergeField> Create(Utf8CP name, PresentationQueryContractFieldCR mergedField, Utf8String mergedValueResult = Utf8String(),
        bool allowsPrefix = true, bool isDistinct = false, bool isAggregateField = false, FieldVisibility fieldVisibility = FieldVisibility::Both)
        {
        return new PresentationQueryMergeField(name, mergedField, mergedValueResult, allowsPrefix, isDistinct, isAggregateField, fieldVisibility);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryContract : RefCountedBase
    {
protected:
    PresentationQueryContract() {}
    virtual ~PresentationQueryContract() {}
    virtual bvector<PresentationQueryContractFieldCPtr> _GetFields() const = 0;
    virtual void _SetECInstanceIdFieldName(Utf8CP name){}
    virtual void _SetECClassIdFieldName(Utf8CP name){}
    virtual ECN::ECClassCP _GetSelectClass() const {return nullptr;}

public:
    ECPRESENTATION_EXPORT uint8_t GetIndex(Utf8CP fieldName) const;
    void SetECInstanceIdFieldName(Utf8CP name) {_SetECInstanceIdFieldName(name);}
    void SetECClassIdFieldName(Utf8CP name) {_SetECClassIdFieldName(name);}
    ECN::ECClassCP GetSelectClass() const {return _GetSelectClass();}
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
    ECPRESENTATION_EXPORT static Utf8CP RelatedInstanceInfoFieldName;

private:
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_displayLabelField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecClassIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_relatedInstanceInfoField;
    
private:
    ECPRESENTATION_EXPORT ECInstanceNodesQueryContract(ECN::ECClassCP, bvector<RelatedClass> const&);
    static RefCountedPtr<PresentationQueryContractFunctionField> CreateDisplayLabelField(ECN::ECClassCP, bvector<RelatedClass> const&);
    static RefCountedPtr<PresentationQueryContractSimpleField> CreateRelatedInstanceInfoField(bvector<RelatedClass> const&);

protected:
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::ECInstanceNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<ECInstanceNodesQueryContract> Create(ECN::ECClassCP ecClass, bvector<RelatedClass> const& relatedClasses = bvector<RelatedClass>())
        {
        return new ECInstanceNodesQueryContract(ecClass, relatedClasses);
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
    ECPRESENTATION_EXPORT DisplayLabelGroupingNodesQueryContract(ECN::ECClassCP, bool, bvector<RelatedClass> const&);
    static RefCountedPtr<PresentationQueryContractFunctionField> CreateDisplayLabelField(ECN::ECClassCP, bvector<RelatedClass> const&);

protected:
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::DisplayLabelGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<DisplayLabelGroupingNodesQueryContract> Create(ECN::ECClassCP ecClass, 
        bool includeGroupedInstanceIdsField = true, bvector<RelatedClass> const& relatedClasses = bvector<RelatedClass>())
        {
        return new DisplayLabelGroupingNodesQueryContract(ecClass, includeGroupedInstanceIdsField, relatedClasses);
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
    ECN::ECClassId m_baseClassId;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecClassIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_baseClassIdField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_displayLabelField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_groupedInstanceIdsField;

private:
    ECPRESENTATION_EXPORT BaseECClassGroupingNodesQueryContract(ECN::ECClassId baseClassId);
    Utf8String GetDisplayLabelClause(Utf8CP) const;

protected:
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::BaseClassGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<BaseECClassGroupingNodesQueryContract> Create(ECN::ECClassId baseClassId) {return new BaseECClassGroupingNodesQueryContract(baseClassId);}
    ECN::ECClassId GetBaseClassId() const {return m_baseClassId;}
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
    ECN::ECPropertyCR m_property;
    ECN::PropertyGroupCR m_specification;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecClassIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecPropertyClassIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecPropertyNameField;
    RefCountedPtr<PresentationQueryContractDynamicField> m_displayLabelField;
    RefCountedPtr<PresentationQueryContractDynamicField> m_imageIdField;
    RefCountedPtr<PresentationQueryContractField> m_groupingValueField;
    RefCountedPtr<PresentationQueryContractDynamicField> m_groupingValuesField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_isRangeField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_groupedInstanceIdsField;
    Utf8String m_ecInstanceFieldName;
    ECN::ECClassCP m_foreignKeyClass;
    Utf8String m_groupingPropertyClassAlias;

private:
    ECPRESENTATION_EXPORT ECPropertyGroupingNodesQueryContract(ECN::ECClassCR, ECN::ECPropertyCR, Utf8String, ECN::PropertyGroupCR, ECN::ECClassCP);
    static Utf8String GetGroupingValueClause(ECN::ECPropertyCR);
    Utf8String GetDisplayLabelClause(Utf8CP) const;
    Utf8String GetImageIdClause(Utf8CP) const;
    Utf8String GetGroupingValuesClause(Utf8CP) const;
    Utf8String GetPropertyValueClause(Utf8CP) const;

protected:
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::PropertyGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<ECPropertyGroupingNodesQueryContract> Create(ECN::ECClassCR ecClass, ECN::ECPropertyCR prop, Utf8String groupingPropertyClassAlias, ECN::PropertyGroupCR spec, ECN::ECClassCP foreignKeyClass)
        {
        return new ECPropertyGroupingNodesQueryContract(ecClass, prop, groupingPropertyClassAlias, spec, foreignKeyClass);
        }
    ECN::ECPropertyCR GetProperty() const {return m_property;}
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
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceKeysFieldName;
    ECPRESENTATION_EXPORT static Utf8CP DisplayLabelFieldName;

private:
    PresentationQueryContractFieldPtr m_ecInstanceKeysField;
    mutable RefCountedPtr<PresentationQueryContractFunctionField> m_displayLabelField;
    ContentDescriptorCPtr m_descriptor;
    ECN::ECClassCP m_class;
    IQueryInfoProvider const& m_queryInfo;

private:
    ECPRESENTATION_EXPORT ContentQueryContract(ContentDescriptorCR descriptor, ECN::ECClassCP ecClass, IQueryInfoProvider const&);
    PresentationQueryContractFunctionField const& GetDisplayLabelField() const;
    PresentationQueryContractFieldCPtr GetCalculatedPropertyField(Utf8String const&, Utf8String const&) const;
    PresentationQueryContractFieldCPtr CreateInstanceKeyField(Utf8CP fieldName, Utf8CP alias, ECN::ECClassId defaultClassId, bool isMerging) const;
    PresentationQueryContractFieldCPtr CreateInstanceKeyField(ContentDescriptor::ECInstanceKeyField const&, bool isMerging) const;

protected:
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECN::ECClassCP _GetSelectClass() const override {return m_class;}

public:
    static ContentQueryContractPtr Create(ContentDescriptorCR descriptor, ECN::ECClassCP ecClass, IQueryInfoProvider const& queryInfo)
        {
        return new ContentQueryContract(descriptor, ecClass, queryInfo);
        }
    ContentDescriptorCR GetDescriptor() const {return *m_descriptor;}
    ContentDescriptor::Property const* FindMatchingProperty(ContentDescriptor::ECPropertiesField const&, ECN::ECClassCP) const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                05/2016
+===============+===============+===============+===============+===============+======*/
struct CountQueryContract : PresentationQueryContract
{
public:
    ECPRESENTATION_EXPORT static Utf8CP CountFieldName;

private:
    RefCountedPtr<PresentationQueryContractSimpleField> m_countField;

private:
    ECPRESENTATION_EXPORT CountQueryContract();

public:
    static RefCountedPtr<CountQueryContract> Create() {return new CountQueryContract();}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
