/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/RuleSetEmbedder.h>
#include <BeSQLite/BeSQLite.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

const Utf8CP RuleSetEmbedder::FILE_TYPE_PresentationRuleSet = "PresentationRuleSet";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RuleSetEmbedder::Embed(PresentationRuleSetR ruleset)
    {
    DbEmbeddedFileTable& embeddedFiles = m_connection.EmbeddedFiles();

    Utf8String rulesetName = ruleset.GetFullRuleSetId();
    Utf8String rulesetXml = ruleset.WriteToXmlString();
    DbResult result = embeddedFiles.AddEntry(rulesetName.c_str(), FILE_TYPE_PresentationRuleSet);
    if (BE_SQLITE_OK != result)
        return result;

    return embeddedFiles.Save(rulesetXml.c_str(), rulesetXml.SizeInBytes(), rulesetName.c_str());
    }