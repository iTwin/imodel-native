/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/PresentationRule.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//! This enumerator contains trees for which the rule can be applied.
enum RuleTargetTree
    {
    //! Apply rule for main tree. (default)
    TargetTree_MainTree,
    //! Apply rule for selection tree.
    TargetTree_SelectionTree,
    //! Apply rule for both trees.
    TargetTree_Both
    };

/*---------------------------------------------------------------------------------**//**
Base class for all custom PresentationKeys. It represents any presentation configuration.
* @bsiclass                                     Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationKey
    {
private:
    int m_priority;

protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT PresentationKey ();

    //! Constructor.
    ECPRESENTATION_EXPORT PresentationKey (int priority);

    //! Returns XmlElement name that is used to read/save this rule information.
    ECPRESENTATION_EXPORT virtual CharCP         _GetXmlElementName () const = 0;

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT virtual bool           _ReadXml (BeXmlNodeP xmlNode) = 0;

    //! Writes rule information to given XmlNode.
    ECPRESENTATION_EXPORT virtual void           _WriteXml (BeXmlNodeP xmlNode) const = 0;

public:
    //! Virtual destructor.
    virtual ~PresentationKey(){}
    
    //! Reads PresentationRule from xml node.
    ECPRESENTATION_EXPORT bool                   ReadXml (BeXmlNodeP xmlNode);

    //! Writes PresentationRule to xml node.
    ECPRESENTATION_EXPORT void                   WriteXml (BeXmlNodeP parentXmlNode) const;

    //! Priority of the rule, defines the order in which rules are evaluated and executed.
    ECPRESENTATION_EXPORT int                    GetPriority (void) const;

    };

/*---------------------------------------------------------------------------------**//**
Base class for all PresentationRules.
* @bsiclass                                     Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationRule : public PresentationKey
    {
private:
    Utf8String   m_condition;
    bool      m_onlyIfNotHandled;

protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT PresentationRule ();

    //! Constructor.
    ECPRESENTATION_EXPORT PresentationRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled);

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT virtual bool           _ReadXml (BeXmlNodeP xmlNode) override;

    //! Writes rule information to given XmlNode.
    ECPRESENTATION_EXPORT virtual void           _WriteXml (BeXmlNodeP xmlNode) const override;

public:
    //! Condition is an ECExpression string, which will be evaluated against the given context in order to decide whether to apply this rule or not.
    ECPRESENTATION_EXPORT Utf8StringCR              GetCondition (void) const;

    //! Set condition ECExpression string, which will be evaluated against the given context in order to decide whether to apply this rule or not.
    ECPRESENTATION_EXPORT void                   SetCondition (Utf8String value);

    //! Returns true if this rule should be executed only in the case where there are no other higher priority rules for this particular cotext.
    ECPRESENTATION_EXPORT bool                   GetOnlyIfNotHandled (void) const;
    };

struct PresentationRuleSpecificationVisitor;
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationRuleSpecification
{
protected:
    virtual ~PresentationRuleSpecification() {}
    virtual void _Accept(PresentationRuleSpecificationVisitor& visitor) const {}

public:
    //! Allows the visitor to visit this specification.
    ECPRESENTATION_EXPORT void Accept(PresentationRuleSpecificationVisitor& visitor) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
