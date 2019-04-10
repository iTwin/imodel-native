/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/LandXML.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "TerrainExporter.h"

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE


[System::Flags] public enum class LandXMLOptions
    {
    Source = (int)Bentley::TerrainModel::LandXMLOptions::Source,
    Definition = (int)Bentley::TerrainModel::LandXMLOptions::Definition,
    SourceAndDefinition = (int)Bentley::TerrainModel::LandXMLOptions::SourceAndDefinition
    };

public ref class LandXMLImporter : public TerrainImporter
    {
    private: LandXMLImporterP m_importer;
    internal: LandXMLImporter (Bentley::TerrainModel::LandXMLImporter* importer);
    public: static LandXMLImporter^ Create (System::String^ filename);

    public: property LandXMLOptions Options
        {
        LandXMLOptions get ();
        void set (LandXMLOptions value);
        }
    };

public ref class LandXMLExporter : TerrainExporter
    {
    private: LandXMLExporterP m_exporter;

    private: LandXMLExporter ();

    public: static LandXMLExporter^ Create ()
        {
        return gcnew LandXMLExporter ();
        }

    public: property LandXMLOptions Options
        {
        LandXMLOptions get ();
        void set (LandXMLOptions value);
        }

    public: property System::String^ ApplicationName
        {
        System::String^ get ();
        void set (System::String^ value);
        }
    public: property System::String^ Version
        {
        System::String^ get ();
        void set (System::String^ value);
        }
    public: property System::String^ ProjectName
        {
        System::String^ get ();
        void set (System::String^ value);
        }
    public: property System::String^ ProjectDescription
        {
        System::String^ get ();
        void set (System::String^ value);
        }

    public: void CreateXML (System::String^ filename, System::Collections::Generic::IEnumerable<NamedTerrain^>^ terrains);
    };

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE