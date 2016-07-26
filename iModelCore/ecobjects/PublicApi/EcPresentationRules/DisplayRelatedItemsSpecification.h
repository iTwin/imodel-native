/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/DisplayRelatedItemsSpecification.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Specification for including related items into display commands.
* @bsiclass                                    dmitrijus.tiazlovas                11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DisplayRelatedItemsSpecification : PresentationRuleSpecification
    {
    private:
        bool                    m_logicalChildren;
        int                     m_nestingDepth;
        Utf8String                 m_relationshipClasses;

    public:
        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT bool        ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT void        WriteXml (BeXmlNodeP parentXmlNode) const;

        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT DisplayRelatedItemsSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT DisplayRelatedItemsSpecification (bool logicalChildren, int nestingDepth, Utf8StringCR relationshipClasses);

        //! Only include logical children of selected items.
        ECOBJECTS_EXPORT bool           GetLogicalChildren (void) const;

        //! Nesting depth of relationships.
        ECOBJECTS_EXPORT int            GetNestingDepth (void) const;

        //! Supported relationsip classes.
        ECOBJECTS_EXPORT Utf8StringCR      GetRelationshipClasses (void) const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
