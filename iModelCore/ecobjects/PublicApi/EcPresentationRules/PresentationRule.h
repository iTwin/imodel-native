/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/PresentationRule.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

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
struct PresentationKey
    {
private:
    int m_priority;

protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ECOBJECTS_EXPORT PresentationKey ();

    //! Constructor.
    ECOBJECTS_EXPORT PresentationKey (int priority);

    //! Returns XmlElement name that is used to read/save this rule information.
    ECOBJECTS_EXPORT virtual CharCP         _GetXmlElementName () = 0;

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    ECOBJECTS_EXPORT virtual bool           _ReadXml (BeXmlNodeP xmlNode) = 0;

    //! Writes rule information to given XmlNode.
    ECOBJECTS_EXPORT virtual void           _WriteXml (BeXmlNodeP xmlNode) = 0;

public:
    //! Virtual destructor.
    virtual ~PresentationKey(){}
    
    //! Reads PresentationRule from xml node.
    ECOBJECTS_EXPORT bool                   ReadXml (BeXmlNodeP xmlNode);

    //! Writes PresentationRule to xml node.
    ECOBJECTS_EXPORT void                   WriteXml (BeXmlNodeP parentXmlNode);

    //! Priority of the rule, defines the order in which rules are evaluated and executed.
    ECOBJECTS_EXPORT int                    GetPriority (void) const;

    };


/*---------------------------------------------------------------------------------**//**
Base class for all PresentationRules.
* @bsiclass                                     Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PresentationRule : public PresentationKey
    {
private:
    WString   m_condition;
    bool      m_onlyIfNotHandled;

protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ECOBJECTS_EXPORT PresentationRule ();

    //! Constructor.
    ECOBJECTS_EXPORT PresentationRule (WStringCR condition, int priority, bool onlyIfNotHandled);

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    ECOBJECTS_EXPORT virtual bool           _ReadXml (BeXmlNodeP xmlNode);

    //! Writes rule information to given XmlNode.
    ECOBJECTS_EXPORT virtual void           _WriteXml (BeXmlNodeP xmlNode);

public:
    //! Condition is an ECExpression string, which will be evaluated against the given context in order to decide whether to apply this rule or not.
    ECOBJECTS_EXPORT WStringCR              GetCondition (void) const;

    //! Returns true if this rule should be executed only in the case where there are no other higher priority rules for this particular cotext.
    ECOBJECTS_EXPORT bool                   GetOnlyIfNotHandled (void) const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE