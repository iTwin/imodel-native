/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ImporterBaseFixture.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      05/16
//----------------------------------------------------------------------------------------
struct ImportConfigEditor 
{
    BentleyApi::BeFileName m_importFileName;
    BentleyApi::BeXmlDomPtr m_dom;

    struct
    {
        bool SyncBlockChanges;
    } m_options;

    struct FontConfig
    {
        Utf8String m_type;
        Utf8String m_name;
        Utf8String m_action;
        FontConfig(){}
        FontConfig(Utf8String type, Utf8String name, Utf8String action){ m_type = type; m_action = action; m_name = name; }
    };
    
    struct ModelImportRule
    {
        Utf8String m_name;
        Utf8String m_newName;
        ModelImportRule(){}
        ModelImportRule(Utf8String name, Utf8String newName){ m_name = name; m_newName = newName; }
    };

   
    Utf8String m_LayerCopy;
    Utf8String m_referencesNewLevelDisplay;
    Utf8String m_referencesMergeIntoParent;
    Utf8String m_referencesCloneDuplicates;
    Utf8String m_rasterImportAttachments;
    Utf8String m_rasterExportNonPortableFormats;
    Utf8String m_fontsSearchPaths;
    bvector<FontConfig> m_fontConfigList;
    bvector<ModelImportRule> m_modelImportRules;
    // TODO: Models/UnspecifiedUnits
    Utf8String m_thumbnailsViewTypes;
    Utf8String m_thumbnailsPixelResolution;
    Utf8String m_thumbnailsRenderModeOverride;

    // TODO: LineWeightMapping 

    ImportConfigEditor();
    void CreateConfig();
};
