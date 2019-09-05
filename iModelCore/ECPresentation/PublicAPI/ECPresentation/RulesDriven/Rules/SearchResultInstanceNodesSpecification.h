/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

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
struct EXPORT_VTABLE_ATTRIBUTE QuerySpecification : HashableBase
{
private:
    Utf8String m_schemaName;
    Utf8String m_className;

protected:
    QuerySpecification() {}
    QuerySpecification(Utf8String schemaName, Utf8String className)
        : m_schemaName(schemaName), m_className(className)
        {}

    virtual void _Accept(QuerySpecificationVisitor& visitor) const = 0;
    virtual Utf8CP _GetXmlElementName() const = 0;
    ECPRESENTATION_EXPORT virtual bool _ReadXml(BeXmlNodeP xmlNode);
    ECPRESENTATION_EXPORT virtual void _WriteXml(BeXmlNodeP xmlNode) const;
    virtual Utf8CP _GetJsonElementType() const = 0;
    ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json);
    ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR json) const;
    virtual QuerySpecification* _Clone() const = 0;
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    virtual ~QuerySpecification() {}
    ECPRESENTATION_EXPORT static QuerySpecification* Create(JsonValueCR);

    QuerySpecification* Clone() const {return _Clone();}

    //! Returns XmlElement name that is used to read/save this rule information.
    Utf8CP GetXmlElementName() const {return _GetXmlElementName();}

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    bool ReadXml(BeXmlNodeP xmlNode) {return _ReadXml(xmlNode);}

    //! Writes rule information to given XmlNode.
    void WriteXml(BeXmlNodeP xmlNode) const {_WriteXml(xmlNode);}

    //! Reads rule information from Json, returns true if it can read it successfully.
    bool ReadJson(JsonValueCR json) { return _ReadJson(json); }

    //! Reads rule information from Json, returns true if it can read it successfully.
    Json::Value WriteJson() const {Json::Value json; _WriteJson(json); return json;}

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
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP xmlNode) const override;
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
    QuerySpecification* _Clone() const override {return new StringQuerySpecification(*this);}
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

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
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP xmlNode) const override;
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
    QuerySpecification* _Clone() const override{return new ECPropertyValueQuerySpecification(*this);}
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

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
        ECPRESENTATION_EXPORT void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

        ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationRuleSpecification const& other) const override;
        
        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;
        
        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

        //! Clones this specification.
        ChildNodeSpecification* _Clone() const override {return new SearchResultInstanceNodesSpecification(*this);}

        //! Computes specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT SearchResultInstanceNodesSpecification ();

        //! Constructor.
        //! @deprecated Use SearchResultInstanceNodesSpecification(int, ChildrenHint, bool, bool, bool, bool)
        ECPRESENTATION_EXPORT SearchResultInstanceNodesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, 
            bool hideIfNoChildren, bool groupByClass, bool groupByLabel);

        //! Constructor.
        ECPRESENTATION_EXPORT SearchResultInstanceNodesSpecification (int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy,
            bool hideIfNoChildren, bool groupByClass, bool groupByLabel);
        
        //! Copy Constructor.
        ECPRESENTATION_EXPORT SearchResultInstanceNodesSpecification (SearchResultInstanceNodesSpecification const&);

        //! Destructor.
        ECPRESENTATION_EXPORT ~SearchResultInstanceNodesSpecification();
        
        //! Returns the list of query specifications that are responsible for the results of this rule.
        QuerySpecificationList const& GetQuerySpecifications() const {return m_querySpecifications;}

        //! Add query specification.
        ECPRESENTATION_EXPORT void AddQuerySpecification(QuerySpecificationR specification);

        //! Returns true if grouping by class should be applied.
        ECPRESENTATION_EXPORT bool                         GetGroupByClass (void) const;

        //! Sets the GroupByClass value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetGroupByClass (bool value);

        //! Returns true if grouping by label should be applied.
        ECPRESENTATION_EXPORT bool                         GetGroupByLabel (void) const;

        //! Sets the GroupByLabel value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetGroupByLabel (bool value);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
