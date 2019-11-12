/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ImportConfigEditor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
ImportConfigEditor::ImportConfigEditor()
    {
    // Setting default values
     m_LayerCopy = "Never";
     m_rasterImportAttachments = "true";
     m_rasterExportNonPortableFormats = "false";

     m_fontsSearchPaths = "$(AppRoot)Fonts/";
     m_fontConfigList.push_back(FontConfig("SHX", "*", "IfUsed"));
     m_fontConfigList.push_back(FontConfig("*","*","Never"));

     m_modelImportRules.push_back(ModelImportRule("*ModelSpace", "%file"));
     m_modelImportRules.push_back(ModelImportRule("*PaperSpace", "%file"));
     m_modelImportRules.push_back(ModelImportRule("Default", "%file"));
     
     m_thumbnailsViewTypes = "Physical Sheet";
     m_thumbnailsPixelResolution = "768";
     m_thumbnailsRenderModeOverride = "None";

     m_options.SyncBlockChanges         = true;
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

        BentleyApi::BeXmlNodeP node = m_dom->AddNewElement("Layers", L"", root);
        node->AddAttributeStringValue("copy", m_LayerCopy.c_str());
        
        node = m_dom->AddNewElement("Models", L"", root);
        node = m_dom->AddNewElement("ImportRules", L"", node);
        for (ModelImportRule config : m_modelImportRules)
        {
            BentleyApi::BeXmlNodeP ifNode = m_dom->AddNewElement("If", L"", node);
            ifNode->AddAttributeStringValue("name", config.m_name.c_str());
            BentleyApi::BeXmlNodeP thenNode = m_dom->AddNewElement("Then", L"", ifNode);
            thenNode->AddAttributeStringValue("newName", config.m_newName.c_str());
        }

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
            m_dom->AddNewElement("EmbedAction", BentleyApi::WString(config.m_action.c_str(),false).c_str(), font);
        }
        
        BentleyApi::BeXmlNodeP optionsNode = m_dom->AddNewElement("Options", L"", root);
        ADD_OPTION("SyncBlockChanges", m_options.SyncBlockChanges)
        
        BentleyApi::BeFileName configFileName_out(ImporterTests::GetOutputDir());
        configFileName_out.AppendToPath(L"ImportConfig.xml");

        m_dom->ToFile(BentleyApi::WString(configFileName_out.c_str()), (BentleyApi::BeXmlDom::ToStringOption)(BentleyApi::BeXmlDom::ToStringOption::TO_STRING_OPTION_Indent | BentleyApi::BeXmlDom::ToStringOption::TO_STRING_OPTION_Formatted), BentleyApi::BeXmlDom::FileEncodingOption::FILE_ENCODING_Utf8);
        
    }
