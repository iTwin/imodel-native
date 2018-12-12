/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/TerrainImporter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

public ref class TerrainInfo
    {
    private: System::String^ m_name;
    private: System::String^ m_description;
    private: bool m_hasDefinition;

    internal: TerrainInfo (System::String^ name, System::String^ description, bool hasDefinition) : m_name (name), m_hasDefinition (hasDefinition), m_description (description)
                  {
                  }

    public: property System::String^ Name
        {
        System::String^ get () { return m_name; }
        }
    public: property System::String^ Description
        {
        System::String^ get () { return m_name; }
        }
    public: property bool HasDefinition
        {
        bool get() { return m_hasDefinition; }
        }
    };

public ref class ImportedTerrain : TerrainInfo
    {
    BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ m_dtm;
    internal: ImportedTerrain (BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ dtm, System::String^ name, System::String^ description, bool hasDefinition) : TerrainInfo (name, description, hasDefinition), m_dtm (dtm)
                  {
                  }

    public: property BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ Terrain
        {
        BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ get() { return m_dtm; }
        }
    };

public interface class IImporterCallback
    {
    virtual bool StartTerrain (System::String^ name, System::String^ description) abstract;
    virtual void AddFeature (DTMFeatureId id, System::String^ DTMAttribute, System::String^ featureDefinitionName, System::String^ featureName, System::String^ description, DTMFeatureType featureType, array<Bentley::GeometryNET::DPoint3d>^ points) abstract;
    virtual bool EndTerrain (System::String^ name, BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ dtm) abstract;
    };

struct _TerrainImporter_Unmanaged
    {
    BENTLEY_NAMESPACE_NAME::TerrainModel::TerrainImporterPtr m_importer;
    BENTLEY_NAMESPACE_NAME::TerrainModel::TerrainImporter::ICallback* m_unmanagedCallback;
    _TerrainImporter_Unmanaged ()
        {
        m_unmanagedCallback = nullptr;
        }
    ~_TerrainImporter_Unmanaged ()
        {
        delete m_unmanagedCallback;
        m_unmanagedCallback = nullptr;
        }
    };

/* units */
public enum class FileUnit
    {
    Unknown = BENTLEY_NAMESPACE_NAME::TerrainModel::FileUnit::Unknown,

    // Metric units
    Millimeter = BENTLEY_NAMESPACE_NAME::TerrainModel::FileUnit::Millimeter,
    Centimeter = BENTLEY_NAMESPACE_NAME::TerrainModel::FileUnit::Centimeter,
    Meter = BENTLEY_NAMESPACE_NAME::TerrainModel::FileUnit::Meter,
    Kilometer = BENTLEY_NAMESPACE_NAME::TerrainModel::FileUnit::Kilometer,

    // Imperial units
    Inch = BENTLEY_NAMESPACE_NAME::TerrainModel::FileUnit::Inch,
    Foot = BENTLEY_NAMESPACE_NAME::TerrainModel::FileUnit::Foot,
    USSurveyFoot = BENTLEY_NAMESPACE_NAME::TerrainModel::FileUnit::USSurveyFoot,
    Mile = BENTLEY_NAMESPACE_NAME::TerrainModel::FileUnit::Mile,

    // Others
    Custom = BENTLEY_NAMESPACE_NAME::TerrainModel::FileUnit::Custom
    };

public ref class TerrainImporter abstract
    {
    private: _TerrainImporter_Unmanaged* m_unmanaged;
    IImporterCallback^ m_callback;
    protected: TerrainImporter ();
    protected: TerrainImporter (TerrainImporterP importer);
    public: !TerrainImporter ();
    public: ~TerrainImporter();

    public: static TerrainImporter^ CreateImporter (System::String^ fileName);

    public: property System::String^ FileUnitString
        {
        System::String^ get ();
        }
    public: property BENTLEY_NAMESPACE_NAME::TerrainModelNET::Formats::FileUnit FileUnit
        {
        BENTLEY_NAMESPACE_NAME::TerrainModelNET::Formats::FileUnit get ();
        }

    public: property Bentley::GeoCoordinatesNET::BaseGCS^ GCS
        {
        Bentley::GeoCoordinatesNET::BaseGCS^ get ();
        }
    public: property System::Collections::Generic::IEnumerable<TerrainInfo^>^ Terrains
                {
                System::Collections::Generic::IEnumerable<TerrainInfo^>^ get();
                }

    public: property IImporterCallback^ Callback
                {
                IImporterCallback^ get();
                void set (IImporterCallback^ value);
                }

    public: ImportedTerrain^ ImportTerrain (System::String^ name);
    public: System::Collections::Generic::IEnumerable<ImportedTerrain^>^ ImportTerrains ();
    public: System::Collections::Generic::IEnumerable<ImportedTerrain^>^ ImportTerrains (System::Collections::Generic::IEnumerable<System::String^>^ names);
    };

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE