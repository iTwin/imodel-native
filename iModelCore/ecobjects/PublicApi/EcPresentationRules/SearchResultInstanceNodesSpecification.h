/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/SearchResultInstanceNodesSpecification.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE QuerySpecificationVisitor
{
friend struct StringQuerySpecification;
friend struct ECPropertyValueQuerySpecification;

protected:
    virtual void _Visit(StringQuerySpecificationCR) {}
    virtual void _Visit(ECPropertyValueQuerySpecificationCR) {}
public:
    virtual ~QuerySpecificationVisitor() {}
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE QuerySpecification
{
private:
    Utf8String m_schemaName;
    Utf8String m_className;

protected:
    QuerySpecification(Utf8String schemaName, Utf8String className)
        : m_schemaName(schemaName), m_className(className)
        {}

    virtual void _Accept(QuerySpecificationVisitor& visitor) const = 0;
    virtual Utf8CP _GetXmlElementName() const = 0;
    ECOBJECTS_EXPORT virtual bool _ReadXml(BeXmlNodeP xmlNode);
    ECOBJECTS_EXPORT virtual void _WriteXml(BeXmlNodeP xmlNode) const;

public:
    QuerySpecification() {}
    virtual ~QuerySpecification() {}

    //! Returns XmlElement name that is used to read/save this rule information.
    Utf8CP GetXmlElementName() const {return _GetXmlElementName();}

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    bool ReadXml(BeXmlNodeP xmlNode) {return _ReadXml(xmlNode);}

    //! Writes rule information to given XmlNode.
    void WriteXml(BeXmlNodeP xmlNode) const {_WriteXml(xmlNode);}

    //! Implements the visitor pattern
    void Accept(QuerySpecificationVisitor& visitor) const {_Accept(visitor);}

    //! Sets name of the schema whose instances will be returned by this query specification.
    void SetSchemaName(Utf8StringCR schemaName) {m_schemaName = schemaName;}

    //! Returns name of the schema whose instances will be returned by this query specification.
    Utf8StringCR GetSchemaName() const {return m_schemaName;}
    
    //! Sets name of the class whose instances will be returned by this query specification.
    void SetClassName(Utf8StringCR className) {m_className = className;}

    //! Returns name of the class whose instances will be returned by this query specification.
    Utf8StringCR GetClassName() const {return m_className;}
};
typedef bvector<QuerySpecificationP> QuerySpecificationList;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE StringQuerySpecification : QuerySpecification
{
private:
    Utf8String m_query;

protected:
    virtual void _Accept(QuerySpecificationVisitor& visitor) const override {visitor._Visit(*this);}
    ECOBJECTS_EXPORT virtual Utf8CP _GetXmlElementName() const override;
    ECOBJECTS_EXPORT virtual bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECOBJECTS_EXPORT virtual void _WriteXml(BeXmlNodeP xmlNode) const override;

public:
    //! Constructor
    StringQuerySpecification() {}

    //! Constructor
    StringQuerySpecification(Utf8String query, Utf8String schemaName, Utf8String className)
        : QuerySpecification(schemaName, className), m_query(query)
        {}

    //! Sets the query.
    void SetQuery(Utf8String query) {m_query = query;}

    //! Returns the query.
    Utf8StringCR GetQuery() const {return m_query;}
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ECPropertyValueQuerySpecification : QuerySpecification
{
private:
    Utf8String m_parentPropertyName;

protected:
    virtual void _Accept(QuerySpecificationVisitor& visitor) const override {visitor._Visit(*this);}
    ECOBJECTS_EXPORT virtual Utf8CP _GetXmlElementName() const override;
    ECOBJECTS_EXPORT virtual bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECOBJECTS_EXPORT virtual void _WriteXml(BeXmlNodeP xmlNode) const override;

public:
    //! Constructor
    ECPropertyValueQuerySpecification() {}

    //! Constructor
    ECPropertyValueQuerySpecification(Utf8String schemaName, Utf8String className, Utf8String parentPropertyName)
        : QuerySpecification(schemaName, className), m_parentPropertyName(parentPropertyName)
        {}

    //! Sets the name of parent ECInstance property whose value is the search query.
    void SetParentPropertyName(Utf8String parentPropertyName) {m_parentPropertyName = parentPropertyName;}

    //! Returns the name of parent ECInstance property whose value is the search query.
    Utf8StringCR GetParentPropertyName() const {return m_parentPropertyName;}
};

/*---------------------------------------------------------------------------------**//**
This specification returns search results instance nodes. Nodes are returned only if 
parent node is SearchNodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE SearchResultInstanceNodesSpecification : public ChildNodeSpecification
    {
    private:
        QuerySpecificationList m_querySpecifications;
        bool     m_groupByClass;
        bool     m_groupByLabel;

    protected:
        //! Allows the visitor to visit this specification.
        ECOBJECTS_EXPORT virtual void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode) const override;
        
        //! Clones this specification.
        virtual ChildNodeSpecification* _Clone() const override {return new SearchResultInstanceNodesSpecification(*this);}

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT SearchResultInstanceNodesSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT SearchResultInstanceNodesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, 
                                                                 bool hideIfNoChildren, bool groupByClass, bool groupByLabel);

        //! Destructor.
        ECOBJECTS_EXPORT ~SearchResultInstanceNodesSpecification();
        
        //! Returns the list of query specifications that are responsible for the results of this rule.
        QuerySpecificationList const& GetQuerySpecifications() const {return m_querySpecifications;}

        //! Returns the list of query specifications that are responsible for the results of this rule.
        QuerySpecificationList& GetQuerySpecificationsR() {return m_querySpecifications;}

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
