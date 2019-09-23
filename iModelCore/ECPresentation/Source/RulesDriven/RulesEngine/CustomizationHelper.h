/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "RulesEngineTypes.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2016
+===============+===============+===============+===============+===============+======*/
struct ICustomizablePropertiesSetter
    {
    virtual ~ICustomizablePropertiesSetter() {}
    virtual void _SetLabel(Utf8StringCR label) const = 0;
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
* @bsiclass                                     Grigas.Petraitis                08/2015
+===============+===============+===============+===============+===============+======*/
struct NavNodeCustomizer
{
private:
    RulesDrivenProviderContextCR m_context;
    JsonNavNodeCR m_node;
    JsonNavNodeCP m_parentNode;
    ICustomizablePropertiesSetter const& m_setter;
    ExpressionContextPtr m_nodeExpressionContext;

private:
    ExpressionContext& GetNodeExpressionContext();

public:
    NavNodeCustomizer(RulesDrivenProviderContextCR context, JsonNavNodeCR node, JsonNavNodeCP parentNode, ICustomizablePropertiesSetter const& setter);
    ~NavNodeCustomizer();
    bool ApplyLabelAndDescriptionOverride(bool customizeLabel);
    bool ApplyStyleOverride();
    bool ApplyImageIdOverride();
    bool ApplyCheckboxRules();
    bool ApplyLocalization();
    bool ApplyExtendedDataRules();
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2015
+===============+===============+===============+===============+===============+======*/
struct CustomizationHelper
    {
    ECPRESENTATION_EXPORT static void Customize(NavNodesProviderContextCR, JsonNavNode&, bool customizeLabel);
    ECPRESENTATION_EXPORT static void Customize(ContentProviderContextCR, ContentDescriptorCR, ContentSetItemR);
    ECPRESENTATION_EXPORT static NodeArtifacts EvaluateArtifacts(NavNodesProviderContextCR, JsonNavNodeCR);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
