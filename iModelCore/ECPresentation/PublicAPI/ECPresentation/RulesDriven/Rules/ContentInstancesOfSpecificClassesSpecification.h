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
        bool     m_arePolymorphic;

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
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT ContentInstancesOfSpecificClassesSpecification ();

        //! Constructor.
        ECPRESENTATION_EXPORT ContentInstancesOfSpecificClassesSpecification (int priority, Utf8StringCR instanceFilter, Utf8StringCR classNames, bool arePolymorphic);

        //! Class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECPRESENTATION_EXPORT Utf8StringCR                 GetClassNames (void) const;

        //! Sets the ClassNames for the specification.
        ECPRESENTATION_EXPORT void                         SetClassNames (Utf8StringCR value);

        //! This flag identifies whether ECClasses defined in this specification should be marked as polymorphic in the Query.
        ECPRESENTATION_EXPORT bool                         GetArePolymorphic (void) const;

        //! Sets the ArePolymorphic value for the specification.
        ECPRESENTATION_EXPORT void                         SetArePolymorphic (bool value);

        //! InstanceFiler is specially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results.
        ECPRESENTATION_EXPORT Utf8StringCR                 GetInstanceFilter (void) const;

        ECPRESENTATION_EXPORT void                         SetInstanceFilter (Utf8StringCR value);

    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
