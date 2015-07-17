/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/TerrainExporter.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "LandXML.h"

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

struct UnmanagedExportCallback : Bentley::TerrainModel::TerrainExporter::IFeatureInfoCallback
    {
    private: gcroot<Bentley::TerrainModelNET::Formats::IFeatureInfoCallback^> m_callback;

    public: UnmanagedExportCallback (gcroot<Bentley::TerrainModelNET::Formats::IFeatureInfoCallback^> callback) : m_callback (callback)
        {
        }

    public: void SetCallback (gcroot<Bentley::TerrainModelNET::Formats::IFeatureInfoCallback^> callback)
        {
        m_callback = callback;
        }

    public: virtual void StartTerrain (Bentley::TerrainModel::TerrainExporter::NamedDTM const & dtm)
                {
//                Bentley::TerrainModelNET::DTM^ managedDTM = dtm ? Bentley::TerrainModelNET::DTM::FromHandle ((System::IntPtr)dtm) : nullptr;
                m_callback->StartTerrain (gcnew NamedTerrain (nullptr, gcnew System::String (dtm.GetName ()), gcnew System::String (dtm.GetDescription ())));
                }
    public: virtual bool GetFeatureInfo (WStringR name, WStringR desc, WStringR featureStyle, ::DTMFeatureType type, ::DTMFeatureId id, ::DTMUserTag userTag)
        {
        System::String^ managed_name;
        System::String^ managed_desc;
        System::String^ managed_featureStyle;
        bool ret = m_callback->GetFeatureInfo (managed_name, managed_desc, managed_featureStyle, (DTMFeatureType)type, DTMFeatureId::FromId (id), userTag);
        if (ret)
            {
            pin_ptr<const wchar_t> uName = PtrToStringChars (managed_name);
            pin_ptr<const wchar_t> uDesc = PtrToStringChars (managed_desc);
            pin_ptr<const wchar_t> uFeatureStyle = PtrToStringChars (managed_featureStyle);
            name = uName;
            desc = uDesc;
            featureStyle = uFeatureStyle;
            }
        return ret;
        }
    public: virtual void EndTerrain ()
                {
                m_callback->EndTerrain();
                }
    };

TerrainExporter::TerrainExporter ()
    {
    m_unmanaged = new _TerrainExporter_Unmanaged ();
    }

TerrainExporter::TerrainExporter (TerrainExporterP exporter)
    {
    m_unmanaged = new _TerrainExporter_Unmanaged ();
    m_unmanaged->m_exporter = exporter;
    }

TerrainExporter::!TerrainExporter()
    {
    TerrainExporter::~TerrainExporter();
    }

TerrainExporter::~TerrainExporter()
    {
    delete m_unmanaged;
    m_unmanaged = nullptr;
    }

void TerrainExporter::SetTerrainExporter (TerrainExporterP exporter)
    {
    m_unmanaged->m_exporter = exporter;
    }

System::String^ TerrainExporter::FileUnitString::get()
    {
    return gcnew System::String (m_unmanaged->m_exporter->GetFileUnitString());
    }

Bentley::TerrainModelNET::Formats::FileUnit TerrainExporter::FileUnit::get ()
    {
    return (Bentley::TerrainModelNET::Formats::FileUnit)m_unmanaged->m_exporter->GetFileUnit ();
    }

void TerrainExporter::FileUnit::set (Bentley::TerrainModelNET::Formats::FileUnit value)
    {
    m_unmanaged->m_exporter->SetFileUnit ((Bentley::TerrainModel::FileUnit)value);
    }

IFeatureInfoCallback^ TerrainExporter::Callback::get ()
    {
    return m_callback;
    }

void TerrainExporter::Callback::set (IFeatureInfoCallback^ value)
    {
    m_callback = value;
    
    if (m_unmanaged->m_unmanagedCallback)
        {
        m_unmanaged->m_exporter->SetFeatureInfoCallback (nullptr);
        delete m_unmanaged->m_unmanagedCallback;
        m_unmanaged->m_unmanagedCallback = nullptr;
        }
    if (value)
        {
        m_unmanaged->m_unmanagedCallback = new UnmanagedExportCallback (value);
        m_unmanaged->m_exporter->SetFeatureInfoCallback (m_unmanaged->m_unmanagedCallback);
        }
    }

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE