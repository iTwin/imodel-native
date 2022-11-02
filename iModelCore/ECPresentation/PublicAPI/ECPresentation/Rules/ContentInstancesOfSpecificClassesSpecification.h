/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/ContentSpecification.h>
#include <ECPresentation/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Specification that creates content ECQueries for predefined ECClasses.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE  ContentInstancesOfSpecificClassesSpecification : ContentSpecification
{
    DEFINE_T_SUPER(ContentSpecification)

private:
    Utf8String  m_instanceFilter;
    bool m_handlePropertiesPolymorphically;
    bvector<MultiSchemaClass*> m_classes;
    bvector<MultiSchemaClass*> m_excludedClasses;

protected:
    //! Allows the visitor to visit this specification.
    ECPRESENTATION_EXPORT void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    //! Clones this content specification.
    ContentSpecification* _Clone() const override {return new ContentInstancesOfSpecificClassesSpecification(*this);}

    //! Compute specification hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;

public:
    ECPRESENTATION_EXPORT ContentInstancesOfSpecificClassesSpecification();
    //! @deprecated Use ContentInstancesOfSpecificClassesSpecification (int, Utf8StringCR,  bvector<MultiSchemaClass*>,  bvector<MultiSchemaClass*>, bool)
    ECPRESENTATION_EXPORT ContentInstancesOfSpecificClassesSpecification(
        int priority,
        Utf8StringCR instanceFilter,
        Utf8StringCR classNames,
        bool handleInstancesPolymorphically,
        bool handlePropertiesPolymorphically);
    ECPRESENTATION_EXPORT ContentInstancesOfSpecificClassesSpecification(
        int priority,
        bool onlyIfNotHandled,
        Utf8StringCR instanceFilter,
        bvector<MultiSchemaClass*> classes,
        bvector<MultiSchemaClass*> excludedClasses,
        bool handlePropertiesPolymorphically);

    //! Copy constructor.
    ECPRESENTATION_EXPORT ContentInstancesOfSpecificClassesSpecification(ContentInstancesOfSpecificClassesSpecification const&);
    //! Move constructor.
    ECPRESENTATION_EXPORT ContentInstancesOfSpecificClassesSpecification(ContentInstancesOfSpecificClassesSpecification&&);
    //! Destructor.
    ECPRESENTATION_EXPORT ~ContentInstancesOfSpecificClassesSpecification(void);

    //! Returns a vector of instance classes which should be included.
    bvector<MultiSchemaClass*> const&   GetClasses(void) const {return m_classes;}
    //! Sets a vector of instance classes which should be included.
    ECPRESENTATION_EXPORT void SetClasses(bvector<MultiSchemaClass*> value);

    //! Returns a vector of instance classes which should be excluded.
    bvector<MultiSchemaClass*> const& GetExcludedClasses(void) const {return m_excludedClasses;}
    //! Sets a vector of instance classes which should be excluded.
    ECPRESENTATION_EXPORT void SetExcludedClasses(bvector<MultiSchemaClass*> value);

    //! Whether to retrieve properties from derived classes. When `true`, also filters out properties of classes that
    //! do not have any instances.
    bool ShouldHandlePropertiesPolymorphically() const {return m_handlePropertiesPolymorphically;}
    void SetShouldHandlePropertiesPolymorphically(bool value) {m_handlePropertiesPolymorphically = value; InvalidateHash();}

    //! InstanceFiler is specially formated string that represents WhereCriteria in
    //! ECQuery that is used to filter query results.
    Utf8StringCR GetInstanceFilter() const {return m_instanceFilter;}
    void SetInstanceFilter(Utf8StringCR value) {m_instanceFilter = value; InvalidateHash();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
