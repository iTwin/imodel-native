/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Nullable.h>
#include <ECPresentation/Rules/PresentationRuleSet.h>
#include "CustomRendererSpecification.h"
#include "PropertyEditorSpecification.h"
#include "PropertyCategorySpecification.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* TODO: Use `std::variant` instead of this after switching to C++17
* @bsiclass
+===============+===============+===============+===============+===============+======*/

struct BoolOrString
{
private:
    bool m_boolValue;
    Utf8String m_stringValue;
    bool m_isBool = false;
    bool m_isString = false;

public:
    BoolOrString() {}
    BoolOrString(std::nullptr_t) {}
    BoolOrString(bool const& value) : m_boolValue(value), m_isBool(true) {}
    BoolOrString(Utf8String const& value) : m_stringValue(value), m_isString(true) {}
    BoolOrString(Utf8CP const& value) : m_stringValue(value), m_isString(true) {}

    BoolOrString& operator=(std::nullptr_t rhs)
        {
        m_isBool = false;
        m_isString = false;
        return *this;
        }

    BoolOrString& operator=(bool const& rhs)
        {
        m_boolValue = rhs;
        m_isBool = true;
        m_isString = false;
        return *this;
        }

    BoolOrString& operator=(Utf8String const& rhs)
        {
        m_stringValue = rhs;
        m_isBool = false;
        m_isString = true;
        return *this;
        }

    BoolOrString& operator=(Utf8CP const& rhs)
        {
        m_stringValue = rhs;
        m_isBool = false;
        m_isString = true;
        return *this;
        }

    bool operator==(BoolOrString const& rhs) const 
        { 
        return m_isBool == rhs.m_isBool && m_isString == rhs.m_isString && 
               (!m_isBool || m_boolValue == rhs.m_boolValue) &&
               (!m_isString || m_stringValue == rhs.m_stringValue);
        }
    bool operator==(std::nullptr_t) const { return !m_isBool && !m_isString; }

    bool IsValid() const { return m_isBool || m_isString; }

    bool IsBoolean() const { return m_isBool; }
    bool IsString() const { return m_isString; }

    bool const& GetBoolean() const { BeAssert(IsBoolean()); return m_boolValue; }
    Utf8String const& GetString() const { BeAssert(IsString()); return m_stringValue; }
};

/*---------------------------------------------------------------------------------**//**
* Specification for a property and optional overrides.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertySpecification : NoXmlSupport<PresentationKey>
{
    DEFINE_T_SUPER(NoXmlSupport<PresentationKey>)

private:
    Utf8String m_propertyName;
    int m_overridesPriority;
    Utf8String m_labelOverride;
    BoolOrString m_isDisplayed;
    bool m_doNotHideOtherPropertiesOnDisplayOverride;
    CustomRendererSpecification const* m_rendererOverride;
    PropertyEditorSpecification const* m_editorOverride;
    std::unique_ptr<PropertyCategoryIdentifier> m_categoryId;
    Nullable<bool> m_isReadOnly;
    Nullable<int> m_priority;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    PropertySpecification(): m_overridesPriority(1000), m_rendererOverride(nullptr), m_editorOverride(nullptr), m_doNotHideOtherPropertiesOnDisplayOverride(false) {}
    ECPRESENTATION_EXPORT PropertySpecification(Utf8String propertyName, int overridesPriority = 1000, Utf8String labelOverride = "", std::unique_ptr<PropertyCategoryIdentifier> categoryId = nullptr,
        BoolOrString isDisplayed = nullptr, CustomRendererSpecification* rendererOverride = nullptr, PropertyEditorSpecification* editorOverride = nullptr,
        bool doNotHideOtherPropertiesOnDisplayOverride = false, Nullable<bool> isReadOnly = nullptr, Nullable<int> priority = nullptr);
    ECPRESENTATION_EXPORT PropertySpecification(PropertySpecification const& other);
    ECPRESENTATION_EXPORT PropertySpecification(PropertySpecification&& other);
    ECPRESENTATION_EXPORT ~PropertySpecification();

    //! Reads rule information from deprecated PropertyEditorSpecification json, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT bool ReadEditorSpecificationJson(JsonValueCR json);

    int GetOverridesPriority() const {return m_overridesPriority;}
    void SetOverridesPriority(int value) {m_overridesPriority = value; InvalidateHash();}

    Utf8StringCR GetPropertyName() const {return m_propertyName;}
    void SetPropertyName(Utf8String value) {m_propertyName = value; InvalidateHash();}

    Utf8StringCR GetLabelOverride() const {return m_labelOverride;}
    void SetLabelOverride(Utf8String value) {m_labelOverride = value; InvalidateHash();}

    BoolOrString const& IsDisplayed() const {return m_isDisplayed;}
    void SetIsDisplayed(BoolOrString value) {m_isDisplayed = value; InvalidateHash();}

    bool DoNotHideOtherPropertiesOnDisplayOverride() const {return m_doNotHideOtherPropertiesOnDisplayOverride;}
    void SetDoNotHideOtherPropertiesOnDisplayOverride(bool value) {m_doNotHideOtherPropertiesOnDisplayOverride = value; InvalidateHash();}

    CustomRendererSpecification const* GetRendererOverride() const {return m_rendererOverride;}
    ECPRESENTATION_EXPORT void SetRendererOverride(CustomRendererSpecification*);

    PropertyEditorSpecification const* GetEditorOverride() const {return m_editorOverride;}
    ECPRESENTATION_EXPORT void SetEditorOverride(PropertyEditorSpecification*);

    PropertyCategoryIdentifier const* GetCategoryId() const {return m_categoryId.get();}
    void SetCategoryId(std::unique_ptr<PropertyCategoryIdentifier> value) {m_categoryId = std::move(value); InvalidateHash();}

    Nullable<bool> const& IsReadOnly() const {return m_isReadOnly;}
    void SetIsReadOnly(Nullable<bool> value) {m_isReadOnly = value; InvalidateHash();}

    Nullable<int> const& GetPriority() const {return m_priority;}
    void SetPriority(Nullable<int> value) {m_priority = value; InvalidateHash();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
