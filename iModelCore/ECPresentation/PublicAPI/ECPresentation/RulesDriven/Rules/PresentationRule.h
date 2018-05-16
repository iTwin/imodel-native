/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/PresentationRule.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
#include <Bentley/md5.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Base class for providing hashing functionality.
* @bsiclass                                     Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct HashableBase
    {
    private:
        HashableBase* m_parent;
        mutable Utf8String m_hash;

    private:
        void ComputeHash();

    protected:
        HashableBase() : m_parent(nullptr) {}
        virtual ~HashableBase() {}
        virtual MD5 _ComputeHash(Utf8CP parentHash) const = 0;

/*__PUBLISH_SECTION_END__*/
    public:
        Utf8StringCR GetHash(Utf8CP parentHash);
/*__PUBLISH_SECTION_START__*/

    public:
        //! Get hash.
        ECPRESENTATION_EXPORT Utf8StringCR GetHash() const;

        //! Invalidates current hash.
        ECPRESENTATION_EXPORT void InvalidateHash();

        //! Sets parent.
        void SetParent(HashableBase* parent) {m_parent = parent;}
    };

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
struct EXPORT_VTABLE_ATTRIBUTE PresentationKey : HashableBase
    {
private:
    int m_priority;

protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT PresentationKey ();

    //! Constructor.
    ECPRESENTATION_EXPORT PresentationKey (int priority);

    //! Returns XmlElement name that is used to read/save this rule information.
    virtual CharCP _GetXmlElementName () const = 0;

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    virtual bool _ReadXml (BeXmlNodeP xmlNode) = 0;


    //! Reads rule information from Json, returns true if it can read it successfully.
    virtual bool _ReadJson(JsonValueCR json) = 0;

    //! Writes rule information to given XmlNode.
    virtual void _WriteXml (BeXmlNodeP xmlNode) const = 0;

    //! Computes rule hash.
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    //! Virtual destructor.
    virtual ~PresentationKey(){}
    
    //! Reads PresentationRule from xml node.
    ECPRESENTATION_EXPORT bool                   ReadXml (BeXmlNodeP xmlNode);

    //! Reads rule information from Json, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT bool                   ReadJson(JsonValueCR json);

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
    bool      m_onlyIfNotHandled;

protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT PresentationRule ();

    //! Constructor.
    ECPRESENTATION_EXPORT PresentationRule (int priority, bool onlyIfNotHandled);

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT virtual bool _ReadXml (BeXmlNodeP xmlNode) override;

    //! Reads rule information from Json, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json) override;

    //! Writes rule information to given XmlNode.
    ECPRESENTATION_EXPORT virtual void _WriteXml (BeXmlNodeP xmlNode) const override;
    //! Compute rule hash.
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    //! Returns true if this rule should be executed only in the case where there are no other higher priority rules for this particular cotext.
    ECPRESENTATION_EXPORT bool GetOnlyIfNotHandled (void) const;
    };

/*---------------------------------------------------------------------------------**//**
Class for PresentationRules with conditions.
* @bsiclass                                     Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ConditionalPresentationRule : public PresentationRule
    {
private:
    Utf8String m_condition;

protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ConditionalPresentationRule() {}

    //! Constructor.
    ConditionalPresentationRule(Utf8String condition, int priority, bool onlyIfNotHandled) 
        : PresentationRule(priority, onlyIfNotHandled), m_condition(condition)
        {}

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT virtual bool _ReadXml (BeXmlNodeP xmlNode) override;

    //! Reads rule information from Json, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json) override;

    //! Writes rule information to given XmlNode.
    ECPRESENTATION_EXPORT virtual void _WriteXml (BeXmlNodeP xmlNode) const override;

    //! Compute rule hash.
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    //! Condition is an ECExpression string, which will be evaluated against the given context in order to decide whether to apply this rule or not.
    ECPRESENTATION_EXPORT Utf8StringCR GetCondition (void) const;

    //! Set condition ECExpression string, which will be evaluated against the given context in order to decide whether to apply this rule or not.
    ECPRESENTATION_EXPORT void SetCondition (Utf8String value);
    };

struct PresentationRuleSpecificationVisitor;
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationRuleSpecification : HashableBase
{
protected:
    virtual ~PresentationRuleSpecification() {}
    virtual void _Accept(PresentationRuleSpecificationVisitor& visitor) const {}
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    //! Allows the visitor to visit this specification.
    ECPRESENTATION_EXPORT void Accept(PresentationRuleSpecificationVisitor& visitor) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
