/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/SelectionSyncHandler.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/SelectionSyncHandler.h>

const Utf8CP RulesDrivenSelectionSyncHandler::SelectionExtendedData::OPTION_NAME_RulesetId = "RulesetId";
const Utf8CP RulesDrivenSelectionSyncHandler::SelectionExtendedData::OPTION_NAME_UseSelectionScope = "UseSelectionScope";

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    08/2016
//---------------------------------------------------------------------------------------
Json::Value RulesDrivenSelectionSyncHandler::_CreateContentOptionsForSelection(SelectionChangedEventCR evt) const
    {
    // if available, use the ruleset specified in the selection event
    SelectionExtendedData extendedData(evt);
    if (extendedData.HasRulesetId())
        return RulesDrivenECPresentationManager::ContentOptions(extendedData.GetRulesetId()).GetJson();

    // otherwise, use the ruleset of this handler
    return RulesDrivenECPresentationManager::ContentOptions(m_rulesetId.c_str()).GetJson();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    08/2016
//---------------------------------------------------------------------------------------
rapidjson::Document RulesDrivenSelectionSyncHandler::_CreateSelectionEventExtendedData() const
    {
    SelectionExtendedData extendedData;
    if (!m_rulesetId.empty())
        extendedData.SetRulesetId(m_rulesetId.c_str());

    rapidjson::Document d;
    d.CopyFrom(extendedData.GetJson(), d.GetAllocator());
    return d;
    }
