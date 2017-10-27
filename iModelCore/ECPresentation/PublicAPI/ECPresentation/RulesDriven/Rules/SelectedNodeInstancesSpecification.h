/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/SelectedNodeInstancesSpecification.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Specification that creates content ECQueries for selected items.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE SelectedNodeInstancesSpecification : public ContentSpecification
    {
    private:
        Utf8String  m_acceptableSchemaName;
        Utf8String  m_acceptableClassNames;
        bool     m_acceptablePolymorphically;
        bool     m_onlyIfNotHandled;

    protected:
        //! Allows the visitor to visit this specification.
        ECPRESENTATION_EXPORT virtual void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT virtual CharCP               _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode) const override;
        
        //! Clones this content specification.
        virtual ContentSpecification* _Clone() const override {return new SelectedNodeInstancesSpecification(*this);}

        //! Computes specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT SelectedNodeInstancesSpecification ();

        //! Constructor.
        ECPRESENTATION_EXPORT SelectedNodeInstancesSpecification (int priority, bool onlyIfNotHandled, Utf8StringCR acceptableSchemaName, Utf8StringCR acceptableClassNames, bool acceptablePolymorphically);

        //! Returns true if this rule should be executed only in the case where there are no other higher priority rules for this particular cotext.
        ECPRESENTATION_EXPORT bool                         GetOnlyIfNotHandled (void) const;

        //! Sets OnlyIfNotHandled value for the specification.
        ECPRESENTATION_EXPORT void                         SetOnlyIfNotHandled (bool value);

        //! Acceptable schema name of ECInstances that will be shown in the content.
        ECPRESENTATION_EXPORT Utf8StringCR                    GetAcceptableSchemaName (void) const;

        //! Sets the acceptable schema name of the specification.
        ECPRESENTATION_EXPORT void                         SetAcceptableSchemaName (Utf8StringCR value);

        //! Acceptable class names of ECInstances that will be shown in the content.
        ECPRESENTATION_EXPORT Utf8StringCR                    GetAcceptableClassNames (void) const;

        //! Sets the acceptable class names of the specification.
        ECPRESENTATION_EXPORT void                         SetAcceptableClassNames (Utf8StringCR value);

        //! Identifies whether AcceptableClasses should be check polymorphically.
        ECPRESENTATION_EXPORT bool                         GetAcceptablePolymorphically (void) const;
        //! Sets the AcceptablePolymorphically value for the specification.
        ECPRESENTATION_EXPORT void                         SetAcceptablePolymorphically (bool value);

    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
