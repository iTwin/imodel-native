/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/DisplayRelatedItemsSpecification.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

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

    protected:
        //! Computes specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT bool        ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT void        WriteXml (BeXmlNodeP parentXmlNode) const;

        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT DisplayRelatedItemsSpecification ();

        //! Constructor.
        ECPRESENTATION_EXPORT DisplayRelatedItemsSpecification (bool logicalChildren, int nestingDepth, Utf8StringCR relationshipClasses);

        //! Only include logical children of selected items.
        ECPRESENTATION_EXPORT bool           GetLogicalChildren (void) const;

        //! Nesting depth of relationships.
        ECPRESENTATION_EXPORT int            GetNestingDepth (void) const;

        //! Supported relationsip classes.
        ECPRESENTATION_EXPORT Utf8StringCR      GetRelationshipClasses (void) const;
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
