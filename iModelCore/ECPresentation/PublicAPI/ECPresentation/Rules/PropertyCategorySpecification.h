/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Enum for ways of identifying a category.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
enum class PropertyCategoryIdentifierType
    {
    Root,
    DefaultParent,
    Id,
    };

struct IdPropertyCategoryIdentifier;
/*---------------------------------------------------------------------------------**//**
* Specification for identifying a category.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyCategoryIdentifier : NoXmlSupport<PresentationKey>
{
    DEFINE_T_SUPER(NoXmlSupport<PresentationKey>)

private:
    PropertyCategoryIdentifierType m_type;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR) const override;

    virtual std::unique_ptr<PropertyCategoryIdentifier> _Clone() const {return std::make_unique<PropertyCategoryIdentifier>(*this);}
    virtual IdPropertyCategoryIdentifier const* _AsIdIdentifier() const {return nullptr;}

protected:
    PropertyCategoryIdentifier(PropertyCategoryIdentifierType type) : m_type(type) {}

public:
    ECPRESENTATION_EXPORT static std::unique_ptr<PropertyCategoryIdentifier> Create(JsonValueCR);
    ECPRESENTATION_EXPORT static std::unique_ptr<PropertyCategoryIdentifier> CreateForRoot();
    ECPRESENTATION_EXPORT static std::unique_ptr<PropertyCategoryIdentifier> CreateForDefaultParent();
    ECPRESENTATION_EXPORT static std::unique_ptr<PropertyCategoryIdentifier> CreateForId(Utf8String id);

    std::unique_ptr<PropertyCategoryIdentifier> Clone() const {return _Clone();}

    IdPropertyCategoryIdentifier const* AsIdIdentifier() const {return _AsIdIdentifier();}
    PropertyCategoryIdentifierType GetType() const {return m_type;}
};

/*---------------------------------------------------------------------------------**//**
* Specification for identifying a category by id.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct IdPropertyCategoryIdentifier : PropertyCategoryIdentifier
{
    DEFINE_T_SUPER(PropertyCategoryIdentifier)
    friend struct PropertyCategoryIdentifier;

private:
    Utf8String m_categoryId;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR) const override;
    std::unique_ptr<PropertyCategoryIdentifier> _Clone() const override {return std::make_unique<IdPropertyCategoryIdentifier>(*this);}
    IdPropertyCategoryIdentifier const* _AsIdIdentifier() const override {return this;}

protected:
    IdPropertyCategoryIdentifier(Utf8String id = "")
        : PropertyCategoryIdentifier(PropertyCategoryIdentifierType::Id), m_categoryId(id)
        {}

public:
    Utf8StringCR GetCategoryId() const {return m_categoryId;}
};

/*---------------------------------------------------------------------------------**//**
* Specification for a custom property category
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyCategorySpecification : NoXmlSupport<PrioritizedPresentationKey>
{
    DEFINE_T_SUPER(NoXmlSupport<PrioritizedPresentationKey>)

private:
    Utf8String m_id;
    std::unique_ptr<PropertyCategoryIdentifier> m_parentId;
    Utf8String m_label;
    Utf8String m_description;
    std::unique_ptr<CustomRendererSpecification> m_rendererOverride;
    bool m_autoExpand;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    PropertyCategorySpecification(): m_autoExpand(false), m_rendererOverride(nullptr) {}
    ECPRESENTATION_EXPORT PropertyCategorySpecification(Utf8String id, Utf8String label, Utf8String description = "", int priority = 1000,
        bool autoExpand = false, std::unique_ptr<CustomRendererSpecification> rendererOverride = nullptr, std::unique_ptr<PropertyCategoryIdentifier> parentId = nullptr);
    ECPRESENTATION_EXPORT PropertyCategorySpecification(PropertyCategorySpecification const&);
    ECPRESENTATION_EXPORT PropertyCategorySpecification(PropertyCategorySpecification&&);

    ECPRESENTATION_EXPORT PropertyCategorySpecification& operator=(PropertyCategorySpecification const&);
    ECPRESENTATION_EXPORT PropertyCategorySpecification& operator=(PropertyCategorySpecification&&);

    Utf8StringCR GetId() const {return m_id;}
    void SetId(Utf8String value) {m_id = value; InvalidateHash();}

    PropertyCategoryIdentifier const* GetParentId() const {return m_parentId.get();}
    void SetParentId(std::unique_ptr<PropertyCategoryIdentifier> value) {m_parentId = std::move(value); InvalidateHash();}

    Utf8StringCR GetLabel() const {return m_label;}
    void SetLabel(Utf8String value) {m_label = value; InvalidateHash();}

    Utf8StringCR GetDescription() const {return m_description;}
    void SetDescription(Utf8String value) {m_description = value; InvalidateHash();}

    bool ShouldAutoExpand() const {return m_autoExpand;}
    void SetShouldAutoExpand(bool value) {m_autoExpand = value; InvalidateHash();}

    CustomRendererSpecification const* GetRendererOverride() const {return m_rendererOverride.get();}
    void SetRendererOverride(std::unique_ptr<CustomRendererSpecification> value) {m_rendererOverride = std::move(value);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
