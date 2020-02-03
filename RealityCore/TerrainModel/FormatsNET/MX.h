/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

public ref class MXFilImporter : TerrainImporter
    {
    MXFilImporterP m_importer;

    public: static MXFilImporter^ Create (System::String^ filename);
    internal: MXFilImporter (MXFilImporterP importer);
    };

public ref class MXFilExporter : TerrainExporter
    {
    MXFilExporterP m_exporter;
    public:
        enum class MXExportError
            {
            Success,
            CantOpenFile,
            StringExists,
            Error
            };
    private: MXFilExporter();

    public: static MXFilExporter^ Create()
        {
        return gcnew MXFilExporter();
        }

    public: MXExportError Export(System::String^ filename, System::String^ modelName, System::String^ stringName, NamedTerrain^ dtm, bool allowOverwrite);
    };

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE