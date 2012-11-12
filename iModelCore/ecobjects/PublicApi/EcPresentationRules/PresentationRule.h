/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/PresentationRule.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/

//This enumerator contains trees for which the rule can be applied.
enum RuleTargetTree
    {
    //Apply rule for main tree. (default)
    TargetTree_MainTree,
    //Apply rule for selection tree.
    TargetTree_SelectionTree,
    //Apply rule for both trees.
    TargetTree_Both
    };

/*---------------------------------------------------------------------------------**//**
Base class for all custom PresentationKeys. It represents any presentation configuration.
* @bsiclass                                     Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PresentationKey
    {
//__PUBLISH_SECTION_END__
private:
    int m_priority;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
protected:
    ECOBJECTS_EXPORT PresentationKey ()
        : m_priority (1000)
        {
        }

    ECOBJECTS_EXPORT PresentationKey (int priority) 
        : m_priority (priority)
        {
        }

    ECOBJECTS_EXPORT virtual CharCP         _GetXmlElementName () = 0;
    ECOBJECTS_EXPORT virtual bool           _ReadXml (BeXmlNodeP xmlNode) = 0;
    ECOBJECTS_EXPORT virtual void           _WriteXml (BeXmlNodeP xmlNode) = 0;

public:
    //! Reads PresentationRule from xml node.
    ECOBJECTS_EXPORT bool                   ReadXml (BeXmlNodeP xmlNode);

    //! Writes PresentationRule to xml node.
    ECOBJECTS_EXPORT void                   WriteXml (BeXmlNodeP parentXmlNode);

    //! Priority of the rule, defines the order in which rules are evaluated and executed.
    ECOBJECTS_EXPORT int                    GetPriority (void) const             { return m_priority; }

    };

/*__PUBLISH_SECTION_START__*/
/*---------------------------------------------------------------------------------**//**
Base class for all PresentationRules.
* @bsiclass                                     Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PresentationRule : public PresentationKey
    {
//__PUBLISH_SECTION_END__
private:
    WString   m_condition;
    bool      m_onlyIfNotHandled;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
protected:
    ECOBJECTS_EXPORT PresentationRule ()
        : PresentationKey (), m_condition (L""), m_onlyIfNotHandled (false)
        {
        }

    ECOBJECTS_EXPORT PresentationRule (WStringCR condition, int priority, bool onlyIfNotHandled)
        : PresentationKey (priority), m_condition (condition), m_onlyIfNotHandled (onlyIfNotHandled)
        {
        }

    ECOBJECTS_EXPORT virtual bool           _ReadXml (BeXmlNodeP xmlNode);
    ECOBJECTS_EXPORT virtual void           _WriteXml (BeXmlNodeP xmlNode);

public:
    //! Condition is an ECExpression string, which will be evaluated against the given context in order to decide whether to apply this rule or not.
    ECOBJECTS_EXPORT WStringCR              GetCondition (void) const            { return m_condition; }

    //! Returns true if this rule should be executed only in the case where there are no other higher priority rules for this particular cotext.
    ECOBJECTS_EXPORT bool                   GetOnlyIfNotHandled (void) const     { return m_onlyIfNotHandled; }
    };

END_BENTLEY_ECOBJECT_NAMESPACE