/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/SearchResultInstanceNodesSpecification.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct SearchQuerySpecification
{
private:
    Utf8String m_query;
    Utf8String m_schemaName;
    Utf8String m_className;

public:
    SearchQuerySpecification() {}
    SearchQuerySpecification(Utf8StringCR query, Utf8StringCR schemaName, Utf8StringCR className)
        : m_query(query), m_schemaName(schemaName), m_className(className)
        {}

    //! Returns XmlElement name that is used to read/save this rule information.
    ECOBJECTS_EXPORT Utf8CP GetXmlElementName ();

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    ECOBJECTS_EXPORT bool ReadXml (BeXmlNodeP xmlNode);

    //! Writes rule information to given XmlNode.
    ECOBJECTS_EXPORT void WriteXml (BeXmlNodeP xmlNode);

    //! Sets name of the schema whose instances will be returned by this query specification.
    void SetSchemaName(Utf8StringCR schemaName) {m_schemaName = schemaName;}

    //! Returns name of the schema whose instances will be returned by this query specification.
    Utf8StringCR GetSchemaName() const {return m_schemaName;}
    
    //! Sets name of the class whose instances will be returned by this query specification.
    void SetClassName(Utf8StringCR className) {m_className = className;}

    //! Returns name of the class whose instances will be returned by this query specification.
    Utf8StringCR GetClassName() const {return m_className;}

    //! Sets the query.
    void SetQuery(Utf8StringCR query) {m_query = query;}

    //! Returns the query.
    Utf8StringCR GetQuery() const {return m_query;}
};

/*---------------------------------------------------------------------------------**//**
This specification returns search results instance nodes. Nodes are returned only if 
parent node is SearchNodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SearchResultInstanceNodesSpecification : public ChildNodeSpecification
    {
    private:
        bvector<SearchQuerySpecification*> m_querySpecifications;
        bool     m_groupByClass;
        bool     m_groupByLabel;

    protected:
        //! Allows the visitor to visit this specification.
        ECOBJECTS_EXPORT virtual void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT SearchResultInstanceNodesSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT SearchResultInstanceNodesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, 
                                                                 bool hideIfNoChildren, bool groupByClass, bool groupByLabel);

        //! Destructor.
        ECOBJECTS_EXPORT ~SearchResultInstanceNodesSpecification();
        
        //! Returns the list of query specifications that are responsible for the results of this rule.
        ECOBJECTS_EXPORT bvector<SearchQuerySpecification*> const& GetQuerySpecifications() const;

        //! Returns the list of query specifications that are responsible for the results of this rule.
        ECOBJECTS_EXPORT bvector<SearchQuerySpecification*>& GetQuerySpecificationsR();

        //! Returns true if grouping by class should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByClass (void) const;

        //! Sets the GroupByClass value. Can be boolean.
        ECOBJECTS_EXPORT void                         SetGroupByClass (bool value);

        //! Returns true if grouping by label should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByLabel (void) const;

        //! Sets the GroupByLabel value. Can be boolean.
        ECOBJECTS_EXPORT void                         SetGroupByLabel (bool value);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
