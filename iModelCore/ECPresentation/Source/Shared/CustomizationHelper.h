/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include "../RulesEngineTypes.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ICustomizablePropertiesSetter
    {
    virtual ~ICustomizablePropertiesSetter() {}
    virtual void _SetLabelDefinition(LabelDefinitionCR labelDefinition) const = 0;
    virtual void _SetDescription(Utf8StringCR description) const = 0;
    virtual void _SetForeColor(Utf8StringCR color) const = 0;
    virtual void _SetBackColor(Utf8StringCR color) const = 0;
    virtual void _SetFontStyle(Utf8StringCR style) const = 0;
    virtual void _SetImageId(Utf8StringCR imageId) const = 0;
    virtual void _SetIsCheckboxVisible(bool value) const = 0;
    virtual void _SetIsCheckboxEnabled(bool value) const = 0;
    virtual void _SetIsChecked(bool value) const = 0;
    virtual void _SetCheckboxBoundInfo(Utf8StringCR propertyName, bool inverse) const = 0;
    virtual void _AddExtendedData(Utf8StringCR key, ECValueCR value) const = 0;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavNodeCustomizer
{
private:
    RulesDrivenProviderContextCR m_context;
    NavNodeCR m_node;
    NavNodeCP m_parentNode;
    ICustomizablePropertiesSetter const& m_setter;
    ExpressionContextPtr m_nodeExpressionContext;

private:
    ExpressionContext& GetNodeExpressionContext();

public:
    NavNodeCustomizer(RulesDrivenProviderContextCR context, NavNodeCR node, NavNodeCP parentNode, ICustomizablePropertiesSetter const& setter);
    ~NavNodeCustomizer();
    bool ApplyLabelAndDescriptionOverride();
    bool ApplyStyleOverride();
    bool ApplyImageIdOverride();
    bool ApplyCheckboxRules();
    bool ApplyExtendedDataRules();
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CustomizationHelper
    {
    ECPRESENTATION_EXPORT static void Customize(NavNodesProviderContextCR, NavNodeCP, NavNode&);
    ECPRESENTATION_EXPORT static void Customize(ContentProviderContextCR, ContentDescriptorCR, ContentSetItemR);
    ECPRESENTATION_EXPORT static NodeArtifacts EvaluateArtifacts(NavNodesProviderContextCR, NavNodeCR);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
