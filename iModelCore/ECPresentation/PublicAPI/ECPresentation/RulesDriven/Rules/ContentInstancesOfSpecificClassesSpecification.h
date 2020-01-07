/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/ContentSpecification.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Specification that creates content ECQueries for predefined ECClasses.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE  ContentInstancesOfSpecificClassesSpecification : public ContentSpecification
{
private:
    Utf8String  m_instanceFilter;
    Utf8String  m_classNames;
    bool m_handleInstancesPolymorphically;
    // TODO: bool m_handlePropertiesPolymorphically

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
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    ECPRESENTATION_EXPORT ContentInstancesOfSpecificClassesSpecification ();
    ECPRESENTATION_EXPORT ContentInstancesOfSpecificClassesSpecification (int priority, Utf8StringCR instanceFilter, Utf8StringCR classNames, bool arePolymorphic);

    //! Class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
    Utf8StringCR GetClassNames() const {return m_classNames;}
    void SetClassNames(Utf8StringCR value) {m_classNames = value;}

    //! A flag indicating whether ECInstances of all ECClasses that derive from the specified class should be loaded in 
    //! addition to ECInstances of exactly the given class
    bool ShouldHandleInstancesPolymorphically() const {return m_handleInstancesPolymorphically;}
    void SetShouldHandleInstancesPolymorphically(bool value) { m_handleInstancesPolymorphically = value;}

    //! InstanceFiler is specially formated string that represents WhereCriteria in 
    //! ECQuery that is used to filter query results.
    Utf8StringCR GetInstanceFilter() const {return m_instanceFilter;}
    void SetInstanceFilter(Utf8StringCR value) {m_instanceFilter = value;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
