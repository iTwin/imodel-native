/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <Bentley/md5.h>
#include <ECPresentation/Rules/PresentationRulesTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Base class for providing hashing functionality.
* @bsiclass
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
    virtual MD5 _ComputeHash() const = 0;

public:
    ECPRESENTATION_EXPORT Utf8StringCR GetHash() const;
    ECPRESENTATION_EXPORT void InvalidateHash();
    void SetParent(HashableBase* parent) {BeMutexHolder lock(m_mutex); m_parent = parent;}
};

#define ADD_PRIMITIVE_VALUE_TO_HASH(md5, identifier, var) \
    { \
    md5.Add(identifier, strlen(identifier)); \
    md5.Add(&var, sizeof(var)); \
    }

#define ADD_STR_VALUE_TO_HASH(md5, identifier, var) \
    { \
    md5.Add(identifier, strlen(identifier)); \
    md5.Add(var.c_str(), var.size()); \
    }

#define ADD_STR_VALUES_TO_HASH(md5, identifier, values) \
    if (!values.empty()) \
        { \
        md5.Add(identifier, strlen(identifier)); \
        for (auto const& value : values) \
            md5.Add(value.c_str(), value.size()); \
        }

#define ADD_CSTR_VALUE_TO_HASH(md5, identifier, var) \
    { \
    md5.Add(identifier, strlen(identifier)); \
    md5.Add(var, strlen(var)); \
    }

#define ADD_RULES_TO_HASH(md5, identifier, rules) \
    if (!rules.empty()) \
        { \
        md5.Add(identifier, strlen(identifier)); \
        for (auto rule : rules) \
            { \
            Utf8StringCR h = rule->GetHash(); \
            md5.Add(h.c_str(), h.size()); \
            } \
        }

#define ADD_HASHABLE_CHILD(container, childR) \
    InvalidateHash(); \
    (childR).SetParent(this); \
    container.push_back(&childR);

#define SET_RULES_INDEX(rules, indexR) \
    { \
    for (auto rule : rules) \
        rule->SetIndex(indexR); \
    }

/*---------------------------------------------------------------------------------**//**
Base class for all custom PresentationKeys. It represents any presentation configuration.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationKey : HashableBase
{
private:
    Nullable<int> m_index;

protected:
    PresentationKey() : m_index(nullptr) {}

    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash() const override;
    virtual void _SetIndex(int& index) { m_index = index++; InvalidateHash(); }

    virtual Utf8CP _GetJsonElementTypeAttributeName() const = 0;
    virtual Utf8CP _GetJsonElementType() const = 0;
    virtual bool _ReadJson(BeJsConst json) {return true;}
    virtual void _WriteJson(BeJsValue) const {}

public:
    void SetIndex(int& index) { _SetIndex(index); }

    //! Get JSON element type
    Utf8CP GetJsonElementType() const {return _GetJsonElementType();}

    //! Reads rule information from json, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT bool ReadJson(BeJsConst json);

    //! Writes rule information to json
    ECPRESENTATION_EXPORT BeJsDocument WriteJson() const;
    ECPRESENTATION_EXPORT void WriteJson(BeJsValue json) const;

    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE PrioritizedPresentationKey : PresentationKey
{
    DEFINE_T_SUPER(PresentationKey)

private:
    int m_priority;

protected:
    PrioritizedPresentationKey() : m_priority(1000) {}
    PrioritizedPresentationKey(int priority) : m_priority(priority) {}

    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash() const override;

    ECPRESENTATION_EXPORT virtual bool _ReadJson(BeJsConst json) override;
    ECPRESENTATION_EXPORT virtual void _WriteJson(BeJsValue) const override;

public:
    int GetPriority() const {return m_priority;}
    void SetPriority(int value) {m_priority = value; InvalidateHash();}
};

/*---------------------------------------------------------------------------------**//**
Base class for all PresentationRules.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationRule : PrioritizedPresentationKey
{
    DEFINE_T_SUPER(PrioritizedPresentationKey)

private:
    bool m_onlyIfNotHandled;
    RequiredSchemaSpecificationsList m_requiredSchemas;

protected:
    ECPRESENTATION_EXPORT PresentationRule();
    ECPRESENTATION_EXPORT PresentationRule(int priority, bool onlyIfNotHandled);
    ECPRESENTATION_EXPORT PresentationRule(PresentationRule const&);
    ECPRESENTATION_EXPORT PresentationRule(PresentationRule&&);

    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash() const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementTypeAttributeName() const override;
    ECPRESENTATION_EXPORT virtual bool _ReadJson(BeJsConst json) override;
    ECPRESENTATION_EXPORT virtual void _WriteJson(BeJsValue) const override;

public:
    ECPRESENTATION_EXPORT ~PresentationRule();

    //! Returns true if this rule should be executed only in the case where there are no other higher priority rules for this particular cotext.
    ECPRESENTATION_EXPORT bool GetOnlyIfNotHandled (void) const;

    RequiredSchemaSpecificationsList const& GetRequiredSchemaSpecifications() const {return m_requiredSchemas;}
    ECPRESENTATION_EXPORT void ClearRequiredSchemaSpecifications();
    ECPRESENTATION_EXPORT void AddRequiredSchemaSpecification(RequiredSchemaSpecificationR spec);
};

/*---------------------------------------------------------------------------------**//**
Class for PresentationRules with conditions.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConditionalPresentationRule : PresentationRule
{
    DEFINE_T_SUPER(PresentationRule)

private:
    Utf8String m_condition;

protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ConditionalPresentationRule() {}

    //! Constructor.
    ConditionalPresentationRule(Utf8String condition, int priority, bool onlyIfNotHandled)
        : PresentationRule(priority, onlyIfNotHandled), m_condition(condition)
        {}

    ECPRESENTATION_EXPORT virtual bool _ReadJson(BeJsConst json) override;
    ECPRESENTATION_EXPORT virtual void _WriteJson(BeJsValue) const override;

    //! Compute rule hash.
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash() const override;

public:
    //! Condition is an ECExpression string, which will be evaluated against the given context in order to decide whether to apply this rule or not.
    ECPRESENTATION_EXPORT Utf8StringCR GetCondition (void) const;

    //! Set condition ECExpression string, which will be evaluated against the given context in order to decide whether to apply this rule or not.
    ECPRESENTATION_EXPORT void SetCondition (Utf8String value);
};

struct PresentationRuleSpecificationVisitor;
/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationRuleSpecification : PrioritizedPresentationKey
{
protected:
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementTypeAttributeName() const override;
    virtual void _Accept(PresentationRuleSpecificationVisitor& visitor) const {}

protected:
    PresentationRuleSpecification() {}
    PresentationRuleSpecification(int priority) : PrioritizedPresentationKey(priority) {}

public:
    //! Allows the visitor to visit this specification.
    ECPRESENTATION_EXPORT void Accept(PresentationRuleSpecificationVisitor& visitor) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
