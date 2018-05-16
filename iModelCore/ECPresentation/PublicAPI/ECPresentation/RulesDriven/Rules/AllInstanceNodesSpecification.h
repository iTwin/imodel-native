/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/AllInstanceNodesSpecification.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/* __PUBLISH_SECTION_START__ */

#include <ECPresentation/RulesDriven/Rules/ChildNodeSpecification.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This specification returns all instance nodes available in the repository.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE AllInstanceNodesSpecification : public ChildNodeSpecification
    {
    private:
        bool     m_groupByClass;
        bool     m_groupByLabel;
        Utf8String  m_supportedSchemas;

    protected:
        //! Allows the visitor to visit this specification.
        ECPRESENTATION_EXPORT virtual void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT virtual CharCP               _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode) override;

        //! Reads rule information from Json, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool                 _ReadJson(JsonValueCR json) override;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode) const override;

        //! Clones this specification.
        virtual ChildNodeSpecification* _Clone() const override {return new AllInstanceNodesSpecification(*this);}

        //! Computes specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT AllInstanceNodesSpecification ();

        //! Constructor.
        ECPRESENTATION_EXPORT AllInstanceNodesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren,
                                                        bool groupByClass, bool groupByLabel, Utf8StringCR supportedSchemas);

        //! Returns true if grouping by class should be applied.
        ECPRESENTATION_EXPORT bool                         GetGroupByClass (void) const;

        //! Sets GroupByClass value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetGroupByClass (bool value);

        //! Returns true if grouping by label should be applied.
        ECPRESENTATION_EXPORT bool                         GetGroupByLabel (void) const;

        //! Sets GroupByLabel value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetGroupByLabel (bool value);

        //! Returns supported schemas that should be used by this specification.
        ECPRESENTATION_EXPORT Utf8StringCR                 GetSupportedSchemas (void) const;

        //! Sets SupportedSchemas value. Can be string.
        ECPRESENTATION_EXPORT void                         SetSupportedSchemas (Utf8StringCR value);

    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
