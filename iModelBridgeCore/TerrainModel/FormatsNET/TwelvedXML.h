/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include "TerrainExporter.h"

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE


[System::Flags] public enum class TwelvedXMLOptions
    {
    Source = (int)Bentley::TerrainModel::TwelvedXMLOptions::Source,
    Definition = (int)Bentley::TerrainModel::TwelvedXMLOptions::Definition,
    SourceAndDefinition = (int)Bentley::TerrainModel::TwelvedXMLOptions::SourceAndDefinition
    };

public ref class TwelvedXMLImporter : public TerrainImporter
    {
    private: TwelvedXMLImporterP m_importer;
    internal: TwelvedXMLImporter (Bentley::TerrainModel::TwelvedXMLImporter* importer);
    public: static TwelvedXMLImporter^ Create (System::String^ filename);

    public: property TwelvedXMLOptions Options
        {
        TwelvedXMLOptions get ();
        void set (TwelvedXMLOptions value);
        }
    };

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE