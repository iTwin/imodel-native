/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/NavNodeKey.h>
#include "Queries/CustomFunctions.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodeLabelCalculator : INodeLabelCalculator
{
private:
    std::unique_ptr<CustomFunctionsContext> m_ctx;
    ECSchemaHelper const& m_schemaHelper;
    IRulesPreprocessorR m_rulesPreprocessor;
protected:
    LabelDefinitionPtr _GetNodeLabel(ECClassInstanceKeyCR key, bvector<ECInstanceKey> const& prevLabelRequestsStack) const override;
public:
    NodeLabelCalculator(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, IConnectionCR connection, Utf8StringCR rulesetId, IRulesPreprocessorR preprocessor, 
        RulesetVariables const& rulesetVariables, ECExpressionsCache& ecexpressionsCache, NavNodesFactory const& navNodeFactory) 
        : m_schemaHelper(schemaHelper), m_rulesPreprocessor(preprocessor)
        {
        m_ctx = std::make_unique<CustomFunctionsContext>(schemaHelper, connections, connection, rulesetId, preprocessor, rulesetVariables, nullptr, ecexpressionsCache, navNodeFactory, nullptr, nullptr, nullptr);
        }
    NodeLabelCalculator(ECSchemaHelper const& schemaHelper, IRulesPreprocessorR preprocessor)
        : m_schemaHelper(schemaHelper), m_rulesPreprocessor(preprocessor)
        {}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE