/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "ImportConfigEditor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfigEditor::ImportConfigEditor()
    {
    // Setting default values
     m_levelCopy = "Never";
     m_referencesNewLevelDisplay = "false";
     m_referencesMergeIntoParent = "false";
     m_referencesCloneDuplicates = "true";
     m_rasterImportAttachments = "true";

     // False by default because exporting takes a long time, and if we work on a Windows platform, we don't need to export. So this will be true 
     // for applications that target multiple platforms.
     m_rasterExportNonPortableFormats = "false";

     m_fontsSearchPaths = "$(AppRoot)DgnV8/Fonts/";
     m_fontConfigList.push_back(FontConfig("*","*","IfUsed"));

     m_modelImportRules.push_back(ModelImportRule("Model", "%file"));
     m_modelImportRules.push_back(ModelImportRule("Design Model", "%file"));
     m_modelImportRules.push_back(ModelImportRule("Default", "%file"));
     
     m_thumbnailsViewTypes = "Physical Drawing";
     m_thumbnailsPixelResolution = "768";
     m_thumbnailsRenderModeOverride = "None";

     m_options.CompactDatabase          = false;
     m_options.Consider2dModelsSpatial  = false;
     m_options.EmbedIssues              = false;
     m_options.EmbedConfigFile          = false;
     m_options.EmbedDgnLinkFiles        = true;
     m_options.SaveIdsAsCodes           = false;
     m_options.SaveAllDrawings          = false;
     m_options.CreateECClassViews       = true;
    }

#define ADD_OPTION(name , value) \
    node = m_dom->AddNewElement("OptionBool", L"", optionsNode); \
    node->AddAttributeStringValue("name", name); \
    node->AddAttributeBooleanValue("value", value);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportConfigEditor::CreateConfig()
    {
        m_dom = BentleyApi::BeXmlDom::CreateEmpty();
        BentleyApi::BeXmlNodeP root = m_dom->AddNewElement("ImportConfig", L"", NULL);
        root->AddAttributeStringValue("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
        root->AddAttributeStringValue("xsi:schemaLocation", "urn:schemas-bentley-com:xml-dgndbimportconfiguration ImportConfig.xsd");

        BentleyApi::BeXmlNodeP node = m_dom->AddNewElement("Levels", L"", root);
        node->AddAttributeStringValue("copy", m_levelCopy.c_str());
        
        node = m_dom->AddNewElement("Models", L"", root);
        node = m_dom->AddNewElement("ImportRules", L"", node);
        for (ModelImportRule config : m_modelImportRules)
        {
            BentleyApi::BeXmlNodeP ifNode = m_dom->AddNewElement("If", L"", node);
            ifNode->AddAttributeStringValue("name", config.m_name.c_str());
            BentleyApi::BeXmlNodeP thenNode = m_dom->AddNewElement("Then", L"", ifNode);
            thenNode->AddAttributeStringValue("newName", config.m_newName.c_str());
        }

        node = m_dom->AddNewElement("References", L"", root);
        node->AddAttributeStringValue("cloneDuplicates", m_referencesCloneDuplicates.c_str());
        node->AddAttributeStringValue("mergeIntoParent", m_referencesMergeIntoParent.c_str());
        node->AddAttributeStringValue("newLevelDisplay", m_referencesNewLevelDisplay.c_str());

        node = m_dom->AddNewElement("Raster", L"", root);
        node->AddAttributeStringValue("importAttachments", m_rasterImportAttachments.c_str());
        node->AddAttributeStringValue("exportNonPortableFormats", m_rasterExportNonPortableFormats.c_str());

        node = m_dom->AddNewElement("Thumbnails", L"", root);
        node->AddAttributeStringValue("viewTypes", m_thumbnailsViewTypes.c_str());
        node->AddAttributeStringValue("pixelResolution", m_thumbnailsPixelResolution.c_str());
        node->AddAttributeStringValue("renderModeOverride", m_thumbnailsRenderModeOverride.c_str());

        node = m_dom->AddNewElement("Fonts", L"", root);
        node->AddAttributeStringValue("searchPaths", m_fontsSearchPaths.c_str());
        for (FontConfig config : m_fontConfigList)
        {
            BentleyApi::BeXmlNodeP font = m_dom->AddNewElement("Font", L"", node);
            font->AddAttributeStringValue("type", config.m_type.c_str());
            font->AddAttributeStringValue("name", config.m_name.c_str());
            m_dom->AddNewElement("EmbedAction", WString(config.m_action.c_str()).c_str(), font);
        }
        
        BentleyApi::BeXmlNodeP optionsNode = m_dom->AddNewElement("Options", L"", root);
        ADD_OPTION("CompactDatabase", m_options.CompactDatabase)
        ADD_OPTION("Consider2dModelsSpatial", m_options.Consider2dModelsSpatial)
        ADD_OPTION("EmbedConfigFile", m_options.EmbedConfigFile)
        ADD_OPTION("EmbedDgnLinkFiles", m_options.EmbedDgnLinkFiles)
        ADD_OPTION("EmbedIssues", m_options.EmbedIssues)
        ADD_OPTION("SaveIdsAsCodes", m_options.SaveIdsAsCodes)
        ADD_OPTION("SaveAllDrawings", m_options.SaveAllDrawings)
        ADD_OPTION("CreateECClassViews", m_options.CreateECClassViews)
        
        BentleyApi::BeFileName configFileName_out(ConverterTestBaseFixture::GetOutputDir());
        configFileName_out.AppendToPath(L"ImportConfig.xml");

        m_dom->ToFile(BentleyApi::WString(configFileName_out.c_str()), (BentleyApi::BeXmlDom::ToStringOption)(BentleyApi::BeXmlDom::ToStringOption::TO_STRING_OPTION_Indent | BentleyApi::BeXmlDom::ToStringOption::TO_STRING_OPTION_Formatted), BentleyApi::BeXmlDom::FileEncodingOption::FILE_ENCODING_Utf8);
        m_importFileName = configFileName_out;
    }
