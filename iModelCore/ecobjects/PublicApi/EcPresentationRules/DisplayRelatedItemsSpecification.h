/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/DisplayRelatedItemsSpecification.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/
/*---------------------------------------------------------------------------------**//**
Specification for including related items into display commands.
* @bsiclass                                    dmitrijus.tiazlovas                11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DisplayRelatedItemsSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        bool                    m_logicalChildren;
        int                     m_nestingDepth;
        WString                 m_relationshipClasses;

    public:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT bool        ReadXml (BeXmlNodeP xmlNode);
        ECOBJECTS_EXPORT void        WriteXml (BeXmlNodeP parentXmlNode);

        ECOBJECTS_EXPORT DisplayRelatedItemsSpecification ()
            : m_logicalChildren (false), m_nestingDepth (0), m_relationshipClasses (L"")
            {
            }

        ECOBJECTS_EXPORT DisplayRelatedItemsSpecification (bool logicalChildren, int nestingDepth, WStringCR relationshipClasses)
            : m_logicalChildren (logicalChildren), m_nestingDepth (nestingDepth), m_relationshipClasses (relationshipClasses)
            {
            }

        //! Only include logical children of selected items.
        ECOBJECTS_EXPORT bool           GetLogicalChildren (void) const         { return m_logicalChildren; }

        //! Nesting depth of relationships.
        ECOBJECTS_EXPORT int            GetNestingDepth (void) const            { return m_nestingDepth; }

        //! Supported relationsip classes.
        ECOBJECTS_EXPORT WStringCR      GetRelationshipClasses (void) const     { return m_relationshipClasses; }
    };

END_BENTLEY_ECOBJECT_NAMESPACE