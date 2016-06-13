/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/AllInstanceNodesSpecification.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/* __PUBLISH_SECTION_START__ */
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/ChildNodeSpecification.h>
#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This specification returns all instance nodes available in the repository.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct AllInstanceNodesSpecification : public ChildNodeSpecification
    {
    private:
        bool     m_groupByClass;
        bool     m_groupByLabel;
        Utf8String  m_supportedSchemas;

    protected:
        //! Allows the visitor to visit this specification.
        ECOBJECTS_EXPORT virtual void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT AllInstanceNodesSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT AllInstanceNodesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren,
                                                        bool groupByClass, bool groupByLabel, Utf8StringCR supportedSchemas);

        //! Returns true if grouping by class should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByClass (void) const;

        //! Sets GroupByClass value. Can be boolean.
        ECOBJECTS_EXPORT void                         SetGroupByClass (bool value);

        //! Returns true if grouping by label should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByLabel (void) const;

        //! Sets GroupByLabel value. Can be boolean.
        ECOBJECTS_EXPORT void                         SetGroupByLabel (bool value);

        //! Returns supported schemas that should be used by this specification.
        ECOBJECTS_EXPORT Utf8StringCR                 GetSupportedSchemas (void) const;

        //! Sets SupportedSchemas value. Can be string.
        ECOBJECTS_EXPORT void                         SetSupportedSchemas (Utf8StringCR value);

    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
