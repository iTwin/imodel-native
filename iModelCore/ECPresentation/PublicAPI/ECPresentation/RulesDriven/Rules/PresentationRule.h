/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
        mutable BeMutex m_mutex;

    private:
        void ComputeHash();

    protected:
        HashableBase() : m_parent(nullptr) {}
        HashableBase(HashableBase const& other) : m_parent(other.m_parent), m_hash(other.m_hash) {}
        virtual ~HashableBase() {}
        virtual MD5 _ComputeHash(Utf8CP parentHash) const = 0;

/*__PUBLISH_SECTION_END__*/
    public:
        Utf8StringCR GetHash(Utf8CP parentHash) const;
/*__PUBLISH_SECTION_START__*/

    public:
        //! Get hash.
        ECPRESENTATION_EXPORT Utf8StringCR GetHash() const;

        //! Invalidates current hash.
        ECPRESENTATION_EXPORT void InvalidateHash();

        //! Sets parent.
        void SetParent(HashableBase* parent) {BeMutexHolder lock(m_mutex); m_parent = parent;}
    };

#define ADD_HASHABLE_CHILD(container, childR) \
    InvalidateHash(); \
    childR.SetParent(this); \
    container.push_back(&childR); 


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

    virtual bool _ShallowEqual(PresentationKeyCR other) const {return m_priority == other.m_priority;}

    virtual Utf8CP _GetXmlElementName () const = 0;
    virtual bool _ReadXml(BeXmlNodeP xmlNode) {return true;}
    virtual void _WriteXml(BeXmlNodeP xmlNode) const {}
    
    virtual Utf8CP _GetJsonElementType() const = 0;
    virtual bool _ReadJson(JsonValueCR json) {return true;}
    virtual void _WriteJson(JsonValueR) const {}

    //! Computes rule hash.
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    //! Virtual destructor.
    virtual ~PresentationKey(){}

    //! Does shallow comparison between this PresentationRule and other PresentationRule
    bool ShallowEqual(PresentationKeyCR other) const {return _ShallowEqual(other);}
    
    //! Reads PresentationRule from xml node.
    ECPRESENTATION_EXPORT bool ReadXml(BeXmlNodeP xmlNode);

    //! Writes PresentationRule to xml node.
    ECPRESENTATION_EXPORT void WriteXml(BeXmlNodeP parentXmlNode) const;

    //! Reads rule information from json, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);

    //! Writes rule information to json
    ECPRESENTATION_EXPORT Json::Value WriteJson() const;

    //! Priority of the rule, defines the order in which rules are evaluated and executed.
    ECPRESENTATION_EXPORT int GetPriority() const;
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

    ECPRESENTATION_EXPORT virtual bool _ShallowEqual(PresentationKeyCR other) const override;

    ECPRESENTATION_EXPORT virtual bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT virtual void _WriteXml (BeXmlNodeP xmlNode) const override;
    
    ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR) const override;

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

    ECPRESENTATION_EXPORT virtual bool _ShallowEqual(PresentationKeyCR other) const override;

    ECPRESENTATION_EXPORT virtual bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT virtual void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR) const override;

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

    virtual bool _ShallowEqual(PresentationRuleSpecification const& other) const { return true; }

    virtual Utf8CP _GetXmlElementName() const = 0;
    virtual bool _ReadXml(BeXmlNodeP xmlNode) {return true;}
    virtual void _WriteXml(BeXmlNodeP xmlNode) const {}

    virtual Utf8CP _GetJsonElementType() const = 0;
    virtual bool _ReadJson(JsonValueCR json) {return true;}
    virtual void _WriteJson(JsonValueR json) const {}

public:
    //! Does shallow comparison between this specification and other specification
    bool ShallowEqual(PresentationRuleSpecification const& other) const {return _ShallowEqual(other);}

    //! Allows the visitor to visit this specification.
    ECPRESENTATION_EXPORT void Accept(PresentationRuleSpecificationVisitor& visitor) const;

    //! Reads specification from XML.
    ECPRESENTATION_EXPORT bool ReadXml(BeXmlNodeP xmlNode);

    //! Writes specification to xml node.
    ECPRESENTATION_EXPORT void WriteXml(BeXmlNodeP parentXmlNode) const;

    //! Reads specification from json
    ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);

    //! Writes specification to json
    ECPRESENTATION_EXPORT Json::Value WriteJson() const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
