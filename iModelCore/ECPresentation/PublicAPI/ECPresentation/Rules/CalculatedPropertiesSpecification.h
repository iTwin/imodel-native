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
    Nullable<Utf8String> m_value;
    CustomRendererSpecificationCP m_renderer;
    PropertyEditorSpecificationCP m_editor;
    std::unique_ptr<PropertyCategoryIdentifier> m_categoryId;

protected:
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP xmlNode) const override;

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
    CalculatedPropertiesSpecification(Utf8String label, int priority)
        : PrioritizedPresentationKey(priority), m_label(label), m_value(nullptr)
        {}
    ECPRESENTATION_EXPORT CalculatedPropertiesSpecification(CalculatedPropertiesSpecification const& other);
    ECPRESENTATION_EXPORT CalculatedPropertiesSpecification(CalculatedPropertiesSpecification&& other);
    ECPRESENTATION_EXPORT ~CalculatedPropertiesSpecification();

    //! Get label expression.
    Utf8StringCR GetLabel() const {return m_label;}
    void SetLabel(Utf8String label) { m_label = label; InvalidateHash(); }

    //! Get property value expression.
    Nullable<Utf8String> GetValue() const {return m_value;}
    void SetValue(Utf8CP value) { m_value = value ? Utf8String(value) : nullptr; InvalidateHash(); }

    CustomRendererSpecificationCP GetRenderer() const { return m_renderer; }
    ECPRESENTATION_EXPORT void SetRenderer(CustomRendererSpecificationP renderer);

    PropertyEditorSpecificationCP GetEditor() const { return m_editor; }
    ECPRESENTATION_EXPORT void SetEditor(PropertyEditorSpecificationP editor);

    PropertyCategoryIdentifier const* GetCategoryId() const { return m_categoryId.get(); }
    void SetCategoryId(std::unique_ptr<PropertyCategoryIdentifier> categoryId) { m_categoryId = std::move(categoryId); InvalidateHash(); }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
