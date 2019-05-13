/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

public ref class NamedTerrain
    {
    public: System::String^ Name;
    public: System::String^ Description;
    public: BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ Terrain;

    public: NamedTerrain (BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ terrain, System::String^ name)
        : Terrain (terrain), Name (name), Description (nullptr)
        {
        }
    public: NamedTerrain (BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ terrain, System::String^ name, System::String^ description)
        : Terrain (terrain), Name (name), Description (description)
        {
        }
    };

public interface class IFeatureInfoCallback
    {
    virtual void StartTerrain (NamedTerrain^ terrain) abstract;
    virtual bool GetFeatureInfo (System::String^% name, System::String^% description, System::String^% featureStyle, DTMFeatureType type, DTMFeatureId id, DTMUserTag userTag) abstract;
    virtual void EndTerrain () abstract;
    };

struct _TerrainExporter_Unmanaged
    {
    TerrainExporterPtr m_exporter;
    BENTLEY_NAMESPACE_NAME::TerrainModel::TerrainExporter::IFeatureInfoCallback* m_unmanagedCallback;
    _TerrainExporter_Unmanaged ()
        {
        m_unmanagedCallback = nullptr;
        }
    ~_TerrainExporter_Unmanaged ()
        {
        if (m_unmanagedCallback)
            {
            delete m_unmanagedCallback;
            m_unmanagedCallback = nullptr;
            }
        }
        
    };

public ref class TerrainExporter abstract
    {
    private: _TerrainExporter_Unmanaged* m_unmanaged;
             IFeatureInfoCallback^ m_callback;
    protected: TerrainExporter ();
    protected: TerrainExporter (TerrainExporterP exporter);
    public: !TerrainExporter ();
    public: ~TerrainExporter();

    protected: void SetTerrainExporter (TerrainExporterP exporter);

    public: property System::String^ FileUnitString
        {
        System::String^ get ();
        }
    public: property BENTLEY_NAMESPACE_NAME::TerrainModelNET::Formats::FileUnit FileUnit
        {
        BENTLEY_NAMESPACE_NAME::TerrainModelNET::Formats::FileUnit get ();
        void set (BENTLEY_NAMESPACE_NAME::TerrainModelNET::Formats::FileUnit value);
        }
            
    public: property IFeatureInfoCallback^ Callback
                {
                IFeatureInfoCallback^ get ();
                void set (IFeatureInfoCallback^ value);
                }

    };

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE