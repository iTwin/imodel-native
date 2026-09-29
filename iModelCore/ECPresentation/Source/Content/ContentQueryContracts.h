/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../Shared/Queries/QueryContracts.h"
#include "../Shared/ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentQueryContract : PresentationQueryContract
{
public:
    ECPRESENTATION_EXPORT static Utf8CP ContractIdFieldName;
    ECPRESENTATION_EXPORT static Utf8CP ECInstanceKeysFieldName;
    ECPRESENTATION_EXPORT static Utf8CP InputECInstanceKeysFieldName;

    using RelatedInstanceDisplayLabelFieldFactory = std::function<PresentationQueryContractFieldPtr(Utf8CP fieldName, SelectClass<ECClass> const& relatedInstanceSelectClass)>;

private:
    PresentationQueryContractFieldPtr m_ecInstanceKeysField;
    PresentationQueryContractFieldPtr m_displayLabelField;
    ContentDescriptorCPtr m_descriptor;
    ECClassCP m_class;
    ECClassCP m_relationshipClass;
    Utf8String m_relationshipClassAlias;
    IQueryInfoProvider const& m_queryInfo;
    RelatedInstanceDisplayLabelFieldFactory m_relatedInstanceDisplayLabelFieldFactory;
    bvector<RelatedClassPath> m_relatedInstancePaths;
    bool m_skipCompositePropertyFields;
    bool m_skipXToManyRelatedContentFields;
    Utf8String m_inputClassAlias;
    ECInstanceKey m_inputInstanceKey;
    mutable std::unique_ptr<bvector<PresentationQueryContractFieldCPtr>> m_fields;

private:
    ECPRESENTATION_EXPORT ContentQueryContract(uint64_t id, ContentDescriptorCR descriptor, ECClassCP ecClass, IQueryInfoProvider const&,
        RelatedInstanceDisplayLabelFieldFactory, PresentationQueryContractFieldPtr, bvector<RelatedClassPath>, bool, bool);
    PresentationQueryContractFieldCPtr GetCalculatedPropertyField(Utf8StringCR, Utf8StringCR, Utf8StringCR, PrimitiveType) const;
    PresentationQueryContractFieldCPtr CreateInputKeysField(Utf8CP selectAlias) const;
    bool CreateContractFields(bvector<PresentationQueryContractFieldCPtr>&, bvector<ContentDescriptor::Field*> const&, ContentDescriptor::RelatedContentField const*, bool) const;
    ECClassCR GetSelectClass(ContentDescriptor::RelatedContentField const* parentField) const;

protected:
    ContentQueryContract const* _AsContentQueryContract() const override {return this;}
    ECPRESENTATION_EXPORT bvector<PresentationQueryContractFieldCPtr> _GetFields() const override;

public:
    static ContentQueryContractPtr Create(uint64_t id, ContentDescriptorCR descriptor, ECClassCP ecClass, IQueryInfoProvider const& queryInfo, RelatedInstanceDisplayLabelFieldFactory displayLabelFieldFactory,
        PresentationQueryContractFieldPtr displayLabelField = nullptr, bvector<RelatedClassPath> relatedInstancePaths = {}, bool skipCompositePropertyFields = true, bool skipXToManyRelatedContentFields = true)
        {
        return new ContentQueryContract(id, descriptor, ecClass, queryInfo, std::move(displayLabelFieldFactory), displayLabelField, relatedInstancePaths, skipCompositePropertyFields, skipXToManyRelatedContentFields);
        }
    ContentDescriptorCR GetDescriptor() const {return *m_descriptor;}
    ContentDescriptor::Property const* FindMatchingProperty(ContentDescriptor::ECPropertiesField const&, ECClassCP) const;
    bool ShouldSkipCompositePropertyFields() const {return m_skipCompositePropertyFields;}
    bool ShouldSkipXToManyRelatedContentFields() const {return m_skipXToManyRelatedContentFields;}
    bool ShouldHandleRelatedContentField(ContentDescriptor::RelatedContentField const& field) const;
    void SetInputClassAlias(Utf8String value) {m_inputClassAlias = std::move(value);}
    void SetInputInstanceKey(ECInstanceKey value) {m_inputInstanceKey = std::move(value);}
    void SetRelationshipClass(ECClassCP value) {m_relationshipClass = value;}
    void SetRelationshipClassAlias(Utf8String value) {m_relationshipClassAlias = value;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
