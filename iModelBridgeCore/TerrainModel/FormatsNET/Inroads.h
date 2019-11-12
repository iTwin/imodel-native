/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

public ref class InroadsImporter : TerrainImporter
    {
    private: InroadsImporterP m_importer;

    public: static InroadsImporter^ Create (System::String^ filename);
    internal: InroadsImporter (InroadsImporterP importer);
    };

public ref class InroadsExporter : TerrainExporter
    {
    private: InroadsExporterP m_exporter;
    private: InroadsExporter();

    public: static InroadsExporter^ Create()
        {
        return gcnew InroadsExporter();
        }

    public: void InroadsExporter::CreateDTM(System::String^ filename, NamedTerrain^ terrain);

    };

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE