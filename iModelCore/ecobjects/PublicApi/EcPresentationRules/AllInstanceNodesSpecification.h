/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/AllInstanceNodesSpecification.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
This specification returns all instance nodes available in the repository.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct AllInstanceNodesSpecification : public ChildNodeSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        bool     m_groupByClass;
        bool     m_groupByLabel;
        WString  m_supportedSchemas;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

    public:
        ECOBJECTS_EXPORT AllInstanceNodesSpecification ()
            : ChildNodeSpecification (), m_groupByClass (true), m_groupByLabel (true), m_supportedSchemas (L"")
            {
            }

        ECOBJECTS_EXPORT AllInstanceNodesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, 
                                       bool groupByClass, bool groupByLabel, WStringCR supportedSchemas)
            : ChildNodeSpecification (priority, alwaysReturnsChildren, hideNodesInHierarchy), 
            m_groupByClass (groupByClass), m_groupByLabel (groupByLabel), m_supportedSchemas (supportedSchemas)
            {
            }

        //! Returns true if grouping by class should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByClass (void) const       { return m_groupByClass; }

        //! Returns true if grouping by label should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByLabel (void) const       { return m_groupByLabel; }

        //! Returns supported schemas that should be used by this specification.
        ECOBJECTS_EXPORT WStringCR                    GetSupportedSchemas (void) const   { return m_supportedSchemas; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE