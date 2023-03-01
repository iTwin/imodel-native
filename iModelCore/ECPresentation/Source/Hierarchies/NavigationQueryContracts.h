/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../Shared/Queries/QueryContracts.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE NavigationQueryContract : PresentationQueryContract
{
public:
    ECPRESENTATION_EXPORT static Utf8CP SpecificationIdentifierFieldName;
    ECPRESENTATION_EXPORT static Utf8CP SkippedInstanceKeysFieldName;
    ECPRESENTATION_EXPORT static Utf8CP SkippedInstanceKeysInternalFieldName;

private:
    Utf8String m_specificationIdentifier;
    mutable PresentationQueryContractFieldCPtr m_skippedInstanceKeysField;
    mutable PresentationQueryContractFieldCPtr m_skippedInstanceKeysInternalField;
    RelatedClassPath m_pathFromSelectToParentClass;

protected:
    NavigationQueryContract(Utf8String specificationIdentifier)
        : PresentationQueryContract(0), m_specificationIdentifier(specificationIdentifier)
        {}
    NavigationQueryContract(uint64_t id, Utf8String specificationIdentifier)
        : PresentationQueryContract(id), m_specificationIdentifier(specificationIdentifier)
        {}
    NavigationQueryContract const* _AsNavigationQueryContract() const override {return this;}
    virtual NavigationQueryResultType _GetResultType() const = 0;
    virtual RefCountedPtr<NavigationQueryContract> _Clone() const = 0;
    ECPRESENTATION_EXPORT virtual bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;

public:
    RefCountedPtr<NavigationQueryContract> Clone() const {return _Clone();}
    Utf8StringCR GetSpecificationIdentifier() const {return m_specificationIdentifier;}
    NavigationQueryResultType GetResultType() const {return _GetResultType();}
    RelatedClassPath const& GetPathFromSelectToParentClass() const {return m_pathFromSelectToParentClass;}
    void SetPathFromSelectToParentClass(RelatedClassPath path) {m_pathFromSelectToParentClass = path;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavigationQuerySelectContract : NavigationQueryContract
{
public:
    ECPRESENTATION_EXPORT static Utf8CP ContractIdFieldName;
private:
    PresentationQueryBuilderCPtr m_instanceKeysSelectQuery;
protected:
    NavigationQuerySelectContract(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery)
        : NavigationQueryContract(id, specificationIdentifier), m_instanceKeysSelectQuery(&instanceKeysSelectQuery)
        {}
    ECPRESENTATION_EXPORT virtual bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
public:
    PresentationQueryBuilderCR GetInstanceKeysSelectQuery() const {return *m_instanceKeysSelectQuery;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InstanceKeysSelectContract : NavigationQueryContract
{
public:
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECClassIdFieldName;

protected:
    InstanceKeysSelectContract(Utf8String specificationIdentifier)
        : NavigationQueryContract(specificationIdentifier)
        {}
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::Invalid;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECInstanceNodesQueryContract : NavigationQuerySelectContract
{
DEFINE_T_SUPER(NavigationQuerySelectContract);
friend struct NavigationQuerySelectContract;

public:
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECClassIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP DisplayLabelFieldName;

private:
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecClassIdField;
    PresentationQueryContractFieldPtr m_displayLabelField;
    PresentationQueryContractFieldPtr m_relatedInstanceInfoField;

private:
    ECPRESENTATION_EXPORT ECInstanceNodesQueryContract(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery,
        ECClassCP, PresentationQueryContractFieldPtr displayLabelField, bvector<RelatedClassPath> const&);

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new ECInstanceNodesQueryContract(*this);}
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::ECInstanceNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<ECInstanceNodesQueryContract> Create(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery, ECClassCP ecClass,
        PresentationQueryContractFieldPtr displayLabelField, bvector<RelatedClassPath> const& relatedInstancePaths = bvector<RelatedClassPath>())
        {
        return new ECInstanceNodesQueryContract(id, specificationIdentifier, instanceKeysSelectQuery, ecClass, displayLabelField, relatedInstancePaths);
        }
    ECPRESENTATION_EXPORT PresentationQueryBuilderPtr CreateInstanceKeysSelectQuery(ECInstanceKey const&) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE MultiECInstanceNodesQueryContract : NavigationQuerySelectContract
{
DEFINE_T_SUPER(NavigationQuerySelectContract);
friend struct NavigationQuerySelectContract;

public:
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECClassIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP InstanceKeysFieldName;
    ECPRESENTATION_EXPORT static Utf8CP DisplayLabelFieldName;

private:
    bool m_includeIdFields;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecClassIdField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_instanceKeysField;
    PresentationQueryContractFieldPtr m_displayLabelField;
    PresentationQueryContractFieldPtr m_relatedInstanceInfoField;

private:
    ECPRESENTATION_EXPORT MultiECInstanceNodesQueryContract(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery,
        ECClassCP, PresentationQueryContractFieldPtr displayLabelField, bool, bvector<RelatedClassPath> const&);

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new MultiECInstanceNodesQueryContract(*this);}
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::MultiECInstanceNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<MultiECInstanceNodesQueryContract> Create(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery, ECClassCP ecClass,
        PresentationQueryContractFieldPtr displayLabelField, bool aggregateInstanceKeys, bvector<RelatedClassPath> const& relatedInstancePaths = bvector<RelatedClassPath>())
        {
        return new MultiECInstanceNodesQueryContract(id, specificationIdentifier, instanceKeysSelectQuery, ecClass, displayLabelField, aggregateInstanceKeys, relatedInstancePaths);
        }
    ECPRESENTATION_EXPORT PresentationQueryBuilderPtr CreateInstanceKeysSelectQuery(bvector<ECInstanceKey> const&) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECClassGroupedInstancesQueryContract : InstanceKeysSelectContract
{
DEFINE_T_SUPER(InstanceKeysSelectContract);

private:
    ECClassGroupedInstancesQueryContract() : T_Super("") {}

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new ECClassGroupedInstancesQueryContract(*this);}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;

public:
    static RefCountedPtr<ECClassGroupedInstancesQueryContract> Create() {return new ECClassGroupedInstancesQueryContract();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECClassGroupingNodesQueryContract : NavigationQuerySelectContract
{
DEFINE_T_SUPER(NavigationQuerySelectContract);
friend struct NavigationQuerySelectContract;

public:
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECClassIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP IsClassPolymorphicFieldName;
    ECPRESENTATION_EXPORT static Utf8CP DisplayLabelFieldName;
    ECPRESENTATION_EXPORT static Utf8CP GroupedInstancesCountFieldName;

private:
    ECClassId m_customClassId;
    bool m_isPolymorphic;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecClassIdField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_displayLabelField;
    PresentationQueryContractFieldPtr m_groupedInstancesCountField;

private:
    ECPRESENTATION_EXPORT ECClassGroupingNodesQueryContract(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery,
        ECClassId customClassId, bool isPolymorphic);

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new ECClassGroupingNodesQueryContract(*this);}
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::ClassGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<ECClassGroupingNodesQueryContract> Create(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery)
        {
        return new ECClassGroupingNodesQueryContract(id, specificationIdentifier, instanceKeysSelectQuery, ECClassId(), false);
        }
    static RefCountedPtr<ECClassGroupingNodesQueryContract> Create(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery, ECClassId customClassId, bool isPolymorphic)
        {
        return new ECClassGroupingNodesQueryContract(id, specificationIdentifier, instanceKeysSelectQuery, customClassId, isPolymorphic);
        }
    ECPRESENTATION_EXPORT PresentationQueryBuilderPtr CreateInstanceKeysSelectQuery(ECClassCR, bool isPolymorphic) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DisplayLabelGroupedInstancesQueryContract : InstanceKeysSelectContract
{
DEFINE_T_SUPER(InstanceKeysSelectContract);

private:
    PresentationQueryContractFieldPtr m_displayLabelField;

private:
    DisplayLabelGroupedInstancesQueryContract(PresentationQueryContractFieldPtr displayLabelField) : T_Super(""), m_displayLabelField(displayLabelField) {}

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new DisplayLabelGroupedInstancesQueryContract(*this);}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;

public:
    static RefCountedPtr<DisplayLabelGroupedInstancesQueryContract> Create(PresentationQueryContractFieldPtr displayLabelField) {return new DisplayLabelGroupedInstancesQueryContract(displayLabelField);}
};

#define MAX_LABEL_GROUPED_INSTANCE_KEYS 5
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE DisplayLabelGroupingNodesQueryContract : NavigationQuerySelectContract
{
DEFINE_T_SUPER(NavigationQuerySelectContract);
friend struct NavigationQuerySelectContract;

private:
    static Utf8CP ECInstanceIdFieldName;
    static Utf8CP ECClassIdFieldName;
public:
    ECPRESENTATION_EXPORT static Utf8CP GroupedInstancesCountFieldName;
    ECPRESENTATION_EXPORT static Utf8CP GroupedInstanceKeysFieldName;
    ECPRESENTATION_EXPORT static Utf8CP DisplayLabelFieldName;

private:
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecClassIdField;
    PresentationQueryContractFieldPtr m_displayLabelField;
    PresentationQueryContractFieldPtr m_groupedInstancesCountField;
    RefCountedPtr<PresentationQueryContractFunctionField> m_groupedInstanceKeysField;
    PresentationQueryBuilderCPtr m_instanceKeysSelectQueryBase;

private:
    ECPRESENTATION_EXPORT DisplayLabelGroupingNodesQueryContract(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery,
        ECClassCP, PresentationQueryContractFieldPtr displayLabelField);

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new DisplayLabelGroupingNodesQueryContract(*this);}
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::DisplayLabelGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<DisplayLabelGroupingNodesQueryContract> Create(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery,
        ECClassCP ecClass, PresentationQueryContractFieldPtr displayLabelField)
        {
        return new DisplayLabelGroupingNodesQueryContract(id, specificationIdentifier, instanceKeysSelectQuery, ecClass, displayLabelField);
        }
    ECPRESENTATION_EXPORT PresentationQueryBuilderPtr CreateInstanceKeysSelectQuery(LabelDefinitionCR label) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPropertyGroupedInstancesQueryContract : InstanceKeysSelectContract
{
DEFINE_T_SUPER(InstanceKeysSelectContract);

private:
    Utf8String m_propertyValueSelector;

private:
    ECPropertyGroupedInstancesQueryContract(Utf8String propertyValueSelector) : T_Super(""), m_propertyValueSelector(propertyValueSelector) {}

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new ECPropertyGroupedInstancesQueryContract(*this);}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;

public:
    static RefCountedPtr<ECPropertyGroupedInstancesQueryContract> Create(Utf8String propertyValueSelector) {return new ECPropertyGroupedInstancesQueryContract(propertyValueSelector);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECPropertyGroupingNodesQueryContract : NavigationQuerySelectContract
{
DEFINE_T_SUPER(NavigationQuerySelectContract);
friend struct NavigationQuerySelectContract;

public:
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECClassIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECPropertyClassIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECPropertyNameFieldName;
    ECPRESENTATION_EXPORT static Utf8CP DisplayLabelFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ImageIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP IsRangeFieldName;
    ECPRESENTATION_EXPORT static Utf8CP GroupingValueFieldName;
    ECPRESENTATION_EXPORT static Utf8CP GroupingValuesFieldName;
    ECPRESENTATION_EXPORT static Utf8CP GroupedInstancesCountFieldName;

private:
    ECPropertyCR m_property;
    PropertyGroupCR m_specification;
    RefCountedPtr<PresentationQueryContractSimpleField> m_ecInstanceIdField;
    PresentationQueryContractFieldPtr m_ecClassIdField;
    PresentationQueryContractFieldPtr m_ecPropertyClassIdField;
    PresentationQueryContractFieldPtr m_ecPropertyNameField;
    PresentationQueryContractFieldPtr m_displayLabelField;
    PresentationQueryContractFieldPtr m_imageIdField;
    PresentationQueryContractFieldPtr m_groupingValueField;
    PresentationQueryContractFieldPtr m_groupingValuesField;
    PresentationQueryContractFieldPtr m_isRangeField;
    PresentationQueryContractFieldPtr m_groupedInstancesCountField;
    Utf8String m_groupingPropertyClassAlias;

private:
    ECPRESENTATION_EXPORT ECPropertyGroupingNodesQueryContract(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery,
        SelectClass<ECClass> const&, ECPropertyCR, PropertyGroupCR, SelectClass<ECClass> const*);
    ECPRESENTATION_EXPORT ECPropertyGroupingNodesQueryContract(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery, ECPropertyCR, PropertyGroupCR);
    static PresentationQueryContractFieldPtr CreateImageIdField(PropertyGroupCR, PresentationQueryContractFieldCR propertyDisplayValueField);
    static PresentationQueryContractFieldPtr CreatePropertyDisplayValueField(ECPropertyCR, Utf8StringCR propertyClassAlias, Utf8StringCR foreignKeyClassAlias, PresentationQueryContractFieldCR ecClassIdField, PresentationQueryContractFieldCR ecInstanceIdField);
    static PresentationQueryContractFieldPtr CreateGroupingValueField(ECPropertyCR, Utf8StringCR propertyClassAlias, PropertyGroupCR, PresentationQueryContractFieldCR propertyDisplayValueField);
    static PresentationQueryContractFieldPtr CreateGroupingValuesField(ECPropertyCR, PropertyGroupCR, PresentationQueryContractFieldCR groupingValueField);
    static PresentationQueryContractFieldPtr CreateDisplayLabelField(PropertyGroupCR spec, PresentationQueryContractFieldCR propertyDisplayValueField, PresentationQueryContractFieldCR groupingValueField,
        PresentationQueryContractFieldCR ecPropertyClassIdField, PresentationQueryContractFieldCR ecPropertyNameField, PresentationQueryContractFieldCR ecInstanceIdField);
    static PresentationQueryContractFieldPtr CreateGroupingDisplayLabelField(PropertyGroupCR spec, PresentationQueryContractFieldCR groupingValueField,
        PresentationQueryContractFieldCR ecPropertyClassIdField, PresentationQueryContractFieldCR ecPropertyNameField, PresentationQueryContractFieldCR ecInstanceIdField);

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new ECPropertyGroupingNodesQueryContract(*this);}
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::PropertyGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<ECPropertyGroupingNodesQueryContract> Create(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery,
        SelectClass<ECClass> const& propertySelectClass, ECPropertyCR prop, PropertyGroupCR spec, SelectClass<ECClass> const* foreignKeyClass)
        {
        return new ECPropertyGroupingNodesQueryContract(id, specificationIdentifier, instanceKeysSelectQuery, propertySelectClass, prop, spec, foreignKeyClass);
        }
    static RefCountedPtr<ECPropertyGroupingNodesQueryContract> Create(uint64_t id, Utf8String specificationIdentifier, PresentationQueryBuilderCR instanceKeysSelectQuery, ECPropertyCR prop, PropertyGroupCR spec)
        {
        return new ECPropertyGroupingNodesQueryContract(id, specificationIdentifier, instanceKeysSelectQuery, prop, spec);
        }
    ECPropertyCR GetProperty() const {return m_property;}
    Utf8StringCR GetGroupingPropertyClassAlias() const {return m_groupingPropertyClassAlias;}
    ECPRESENTATION_EXPORT PresentationQueryBuilderPtr CreateInstanceKeysSelectQuery(RapidJsonValueCR groupingValuesJson) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
