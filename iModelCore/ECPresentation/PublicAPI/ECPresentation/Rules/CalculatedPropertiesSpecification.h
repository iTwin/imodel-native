/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Specification for specifying single property that is Calculated.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct CalculatedPropertiesSpecification : PrioritizedPresentationKey
{
    DEFINE_T_SUPER(PrioritizedPresentationKey)

private:
    Utf8String m_label;
    Utf8String m_value;
    CustomRendererSpecificationCP m_renderer;
    PropertyEditorSpecificationCP m_editor;
    std::unique_ptr<PropertyCategoryIdentifier> m_categoryId;
    Utf8String m_type;
    bmap<Utf8String, Utf8String> m_extendedData;

protected:
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(BeJsConst json) override;
    ECPRESENTATION_EXPORT void _WriteJson(BeJsValue) const override;

public:
    CalculatedPropertiesSpecification() : m_renderer(nullptr), m_editor(nullptr) {}
    CalculatedPropertiesSpecification(Utf8String label, int priority, Utf8String value, CustomRendererSpecificationP rendererOverride = nullptr,
        PropertyEditorSpecificationP editorOverride = nullptr, std::unique_ptr<PropertyCategoryIdentifier> categoryId = nullptr)
        : PrioritizedPresentationKey(priority), m_label(label), m_value(value), m_renderer(rendererOverride),
        m_editor(editorOverride), m_categoryId(std::move(categoryId))
        {}
    CalculatedPropertiesSpecification(Utf8String label, int priority, Utf8String value, Utf8String type)
        : PrioritizedPresentationKey(priority), m_label(label), m_value(value), m_renderer(nullptr),
        m_editor(nullptr), m_categoryId(std::move(nullptr)), m_type(type)
        {}
    ECPRESENTATION_EXPORT CalculatedPropertiesSpecification(CalculatedPropertiesSpecification const& other);
    ECPRESENTATION_EXPORT CalculatedPropertiesSpecification(CalculatedPropertiesSpecification&& other);
    ECPRESENTATION_EXPORT ~CalculatedPropertiesSpecification();

    //! Get label expression.
    Utf8StringCR GetLabel() const {return m_label;}
    void SetLabel(Utf8String label) { m_label = label; InvalidateHash(); }

    //! Get property value expression.
    Utf8StringCR GetValue() const {return m_value;}
    void SetValue(Utf8String value) { m_value = value; InvalidateHash(); }

    CustomRendererSpecificationCP GetRenderer() const { return m_renderer; }
    ECPRESENTATION_EXPORT void SetRenderer(CustomRendererSpecificationP renderer);

    PropertyEditorSpecificationCP GetEditor() const { return m_editor; }
    ECPRESENTATION_EXPORT void SetEditor(PropertyEditorSpecificationP editor);

    PropertyCategoryIdentifier const* GetCategoryId() const { return m_categoryId.get(); }
    void SetCategoryId(std::unique_ptr<PropertyCategoryIdentifier> categoryId) { m_categoryId = std::move(categoryId); InvalidateHash(); }

    Utf8StringCR GetType() const { return m_type; }
    void SetType(Utf8String type) { m_type = type; InvalidateHash(); }

    //! Get key-value pairs for extended data value definitions in this rule
    bmap<Utf8String, Utf8String> const& GetExtendedDataMap() const { return m_extendedData; }

    //! Set key-value pairs for extended data value definitions in this rule
    ECPRESENTATION_EXPORT void SetExtendedDataMap(bmap<Utf8String, Utf8String> map);

    //! Set a single extended data value definition. The `key` property must be
    //! unique. The `value` property is an ECExpression.
    ECPRESENTATION_EXPORT void AddExtendedData(Utf8String key, Utf8String value);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
