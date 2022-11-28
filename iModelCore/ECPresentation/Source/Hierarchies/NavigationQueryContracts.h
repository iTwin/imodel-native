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
        : m_specificationIdentifier(specificationIdentifier)
        {}
    virtual NavigationQueryResultType _GetResultType() const = 0;
    virtual RefCountedPtr<NavigationQueryContract> _Clone() const = 0;
    ECPRESENTATION_EXPORT virtual bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;

public:
    RefCountedPtr<NavigationQueryContract> Clone() const {return _Clone();}
    Utf8StringCR GetSpecificationIdentifier() const {return m_specificationIdentifier;}
    NavigationQueryResultType GetResultType() const {return _GetResultType();}
    RelatedClassPath const& GetPathFromSelectToParentClass() const {return m_pathFromSelectToParentClass;}
    void SetPathFromSelectToParentClass(RelatedClassPath path) { m_pathFromSelectToParentClass = path; }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECInstanceNodesQueryContract : NavigationQueryContract
{
DEFINE_T_SUPER(NavigationQueryContract);
friend struct NavigationQueryContract;

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
    ECPRESENTATION_EXPORT ECInstanceNodesQueryContract(Utf8String specificationIdentifier,
        ECClassCP, PresentationQueryContractFieldPtr displayLabelField, bvector<RelatedClassPath> const&);

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new ECInstanceNodesQueryContract(*this);}
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::ECInstanceNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<ECInstanceNodesQueryContract> Create(Utf8String specificationIdentifier, ECClassCP ecClass,
        PresentationQueryContractFieldPtr displayLabelField, bvector<RelatedClassPath> const& relatedInstancePaths = bvector<RelatedClassPath>())
        {
        return new ECInstanceNodesQueryContract(specificationIdentifier, ecClass, displayLabelField, relatedInstancePaths);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE MultiECInstanceNodesQueryContract : NavigationQueryContract
{
DEFINE_T_SUPER(NavigationQueryContract);
friend struct NavigationQueryContract;

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
    ECPRESENTATION_EXPORT MultiECInstanceNodesQueryContract(Utf8String specificationIdentifier,
        ECClassCP, PresentationQueryContractFieldPtr displayLabelField, bool, bvector<RelatedClassPath> const&);

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new MultiECInstanceNodesQueryContract(*this);}
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::MultiECInstanceNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<MultiECInstanceNodesQueryContract> Create(Utf8String specificationIdentifier, ECClassCP ecClass,
        PresentationQueryContractFieldPtr displayLabelField, bool aggregateInstanceKeys, bvector<RelatedClassPath> const& relatedInstancePaths = bvector<RelatedClassPath>())
        {
        return new MultiECInstanceNodesQueryContract(specificationIdentifier, ecClass, displayLabelField, aggregateInstanceKeys, relatedInstancePaths);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECClassGroupedInstancesQueryContract : NavigationQueryContract
{
DEFINE_T_SUPER(NavigationQueryContract);

private:
    ECClassGroupedInstancesQueryContract() : T_Super("") {}

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override { return new ECClassGroupedInstancesQueryContract(*this); }
    NavigationQueryResultType _GetResultType() const override { return NavigationQueryResultType::Invalid; }
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;

public:
    static RefCountedPtr<ECClassGroupedInstancesQueryContract> Create() {return new ECClassGroupedInstancesQueryContract();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECClassGroupingNodesQueryContract : NavigationQueryContract
{
DEFINE_T_SUPER(NavigationQueryContract);
friend struct NavigationQueryContract;

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
    NavigationQueryCPtr m_instanceKeysSelectQueryBase;

private:
    ECPRESENTATION_EXPORT ECClassGroupingNodesQueryContract(Utf8String specificationIdentifier, NavigationQueryCPtr instanceKeysSelectQueryBase,
        ECClassId customClassId, bool isPolymorphic);

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new ECClassGroupingNodesQueryContract(*this);}
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::ClassGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<ECClassGroupingNodesQueryContract> Create(Utf8String specificationIdentifier, NavigationQueryCPtr instanceKeysSelectQueryBase)
        {
        return new ECClassGroupingNodesQueryContract(specificationIdentifier, instanceKeysSelectQueryBase, ECClassId(), false);
        }
    static RefCountedPtr<ECClassGroupingNodesQueryContract> Create(Utf8String specificationIdentifier, NavigationQueryCPtr instanceKeysSelectQueryBase, ECClassId customClassId, bool isPolymorphic)
        {
        return new ECClassGroupingNodesQueryContract(specificationIdentifier, instanceKeysSelectQueryBase, customClassId, isPolymorphic);
        }
    ECPRESENTATION_EXPORT NavigationQueryPtr CreateInstanceKeysSelectQuery() const;
    ECPRESENTATION_EXPORT NavigationQueryPtr CreateInstanceKeysSelectQuery(ECClassCR, bool isPolymorphic) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DisplayLabelGroupedInstancesQueryContract : NavigationQueryContract
{
DEFINE_T_SUPER(NavigationQueryContract);

private:
    PresentationQueryContractFieldPtr m_displayLabelField;

private:
    DisplayLabelGroupedInstancesQueryContract(PresentationQueryContractFieldPtr displayLabelField) : T_Super(""), m_displayLabelField(displayLabelField) {}

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override { return new DisplayLabelGroupedInstancesQueryContract(*this); }
    NavigationQueryResultType _GetResultType() const override { return NavigationQueryResultType::Invalid; }
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;

public:
    static RefCountedPtr<DisplayLabelGroupedInstancesQueryContract> Create(PresentationQueryContractFieldPtr displayLabelField) {return new DisplayLabelGroupedInstancesQueryContract(displayLabelField);}
};

#define MAX_LABEL_GROUPED_INSTANCE_KEYS 5
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE DisplayLabelGroupingNodesQueryContract : NavigationQueryContract
{
DEFINE_T_SUPER(NavigationQueryContract);
friend struct NavigationQueryContract;

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
    NavigationQueryCPtr m_instanceKeysSelectQueryBase;

private:
    ECPRESENTATION_EXPORT DisplayLabelGroupingNodesQueryContract(Utf8String specificationIdentifier, NavigationQueryCPtr instanceKeysSelectQueryBase,
        ECClassCP, PresentationQueryContractFieldPtr displayLabelField);

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new DisplayLabelGroupingNodesQueryContract(*this);}
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::DisplayLabelGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECClassIdFieldName(Utf8CP name) override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<DisplayLabelGroupingNodesQueryContract> Create(Utf8String specificationIdentifier, NavigationQueryCPtr instanceKeysSelectQueryBase,
        ECClassCP ecClass, PresentationQueryContractFieldPtr displayLabelField)
        {
        return new DisplayLabelGroupingNodesQueryContract(specificationIdentifier, instanceKeysSelectQueryBase, ecClass, displayLabelField);
        }
    ECPRESENTATION_EXPORT NavigationQueryPtr CreateInstanceKeysSelectQuery() const;
    ECPRESENTATION_EXPORT NavigationQueryPtr CreateInstanceKeysSelectQuery(LabelDefinitionCR label) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPropertyGroupedInstancesQueryContract : NavigationQueryContract
{
DEFINE_T_SUPER(NavigationQueryContract);

private:
    Utf8String m_propertyValueSelector;

private:
    ECPropertyGroupedInstancesQueryContract(Utf8String propertyValueSelector) : T_Super(""), m_propertyValueSelector(propertyValueSelector) {}

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override { return new ECPropertyGroupedInstancesQueryContract(*this); }
    NavigationQueryResultType _GetResultType() const override { return NavigationQueryResultType::Invalid; }
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;

public:
    static RefCountedPtr<ECPropertyGroupedInstancesQueryContract> Create(Utf8String propertyValueSelector) {return new ECPropertyGroupedInstancesQueryContract(propertyValueSelector);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECPropertyGroupingNodesQueryContract : NavigationQueryContract
{
DEFINE_T_SUPER(NavigationQueryContract);
friend struct NavigationQueryContract;

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
    NavigationQueryCPtr m_instanceKeysSelectQueryBase;

private:
    ECPRESENTATION_EXPORT ECPropertyGroupingNodesQueryContract(Utf8String specificationIdentifier, NavigationQueryCPtr instanceKeysSelectQueryBase,
        SelectClass<ECClass> const&, ECPropertyCR, PropertyGroupCR, SelectClass<ECClass> const*);
    static PresentationQueryContractFieldPtr CreateImageIdField(PropertyGroupCR, PresentationQueryContractFieldCR propertyDisplayValueField);
    static PresentationQueryContractFieldPtr CreatePropertyDisplayValueField(ECPropertyCR, Utf8StringCR propertyClassAlias, Utf8StringCR foreignKeyClassAlias, PresentationQueryContractFieldCR ecClassIdField, PresentationQueryContractFieldCR ecInstanceIdField);
    static PresentationQueryContractFieldPtr CreateGroupingValueField(ECPropertyCR, Utf8StringCR propertyClassAlias, PropertyGroupCR, PresentationQueryContractFieldCR propertyDisplayValueField);
    static PresentationQueryContractFieldPtr CreateGroupingValuesField(ECPropertyCR, PropertyGroupCR, PresentationQueryContractFieldCR groupingValueField);
    static PresentationQueryContractFieldPtr CreateDisplayLabelField(PropertyGroupCR spec, PresentationQueryContractFieldCR propertyDisplayValueField, PresentationQueryContractFieldCR groupingValueField,
        PresentationQueryContractFieldCR ecPropertyClassIdField, PresentationQueryContractFieldCR ecPropertyNameField, PresentationQueryContractFieldCR ecInstanceIdField);

protected:
    RefCountedPtr<NavigationQueryContract> _Clone() const override {return new ECPropertyGroupingNodesQueryContract(*this);}
    NavigationQueryResultType _GetResultType() const override {return NavigationQueryResultType::PropertyGroupingNodes;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;
    ECPRESENTATION_EXPORT void _SetECInstanceIdFieldName(Utf8CP name) override;

public:
    static RefCountedPtr<ECPropertyGroupingNodesQueryContract> Create(Utf8String specificationIdentifier, NavigationQueryCPtr instanceKeysSelectQueryBase,
        SelectClass<ECClass> const& propertySelectClass, ECPropertyCR prop, PropertyGroupCR spec, SelectClass<ECClass> const* foreignKeyClass)
        {
        return new ECPropertyGroupingNodesQueryContract(specificationIdentifier, instanceKeysSelectQueryBase, propertySelectClass, prop, spec, foreignKeyClass);
        }
    ECPropertyCR GetProperty() const {return m_property;}
    Utf8StringCR GetGroupingPropertyClassAlias() const {return m_groupingPropertyClassAlias;}
    ECPRESENTATION_EXPORT NavigationQueryPtr CreateInstanceKeysSelectQuery() const;
    ECPRESENTATION_EXPORT NavigationQueryPtr CreateInstanceKeysSelectQuery(RapidJsonValueCR groupingValuesJson) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
