/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/TerrainImporter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "LandXML.h"
#include "MX.h"
#include "Lidar.h"
#include "Inroads.h"

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

struct UnmanagedCallback : BENTLEY_NAMESPACE_NAME::TerrainModel::TerrainImporter::ICallback
    {
    private: gcroot<IImporterCallback^> m_callback;

    public: UnmanagedCallback (gcroot<IImporterCallback^> callback) : m_callback(callback)
        {
        }

    public: void SetCallback (gcroot<IImporterCallback^> callback)
        {
        m_callback = callback;
        }

    public: virtual bool StartTerrain (WCharCP name, WCharCP description, BcDTMPtr& dtm)
                {
                return m_callback->StartTerrain(gcnew System::String (name), gcnew System::String (description));
                }
    public: virtual void AddFeature (::DTMFeatureId id, WCharCP DTMAttribute, WCharCP featureDefinitionName, WCharCP featureName, WCharCP description, ::DTMFeatureType featureType, DPoint3dCP points, size_t numPoints)
                {
                array<Bentley::GeometryNET::DPoint3d>^ managedPoints = gcnew array<Bentley::GeometryNET::DPoint3d>((int)numPoints);
                pin_ptr<Bentley::GeometryNET::DPoint3d> managedPointsPinned = &managedPoints[0];
                memcpy ((void*)managedPointsPinned, points, sizeof (::DPoint3d) * numPoints);
                m_callback->AddFeature (BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTMFeatureId::FromId(id), gcnew System::String (DTMAttribute), gcnew System::String (featureDefinitionName), gcnew System::String (featureName), gcnew System::String (description), (BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTMFeatureType)featureType, managedPoints);
                }
    public: virtual bool EndTerrain (WCharCP name, BcDTMP dtm)
                {
                BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ managedDTM = dtm ? BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM::FromHandle ((System::IntPtr)dtm) : nullptr;
                return m_callback->EndTerrain (gcnew System::String (name), managedDTM);
                }
    };

TerrainImporter::TerrainImporter ()
    {
    m_unmanaged = new _TerrainImporter_Unmanaged ();
    }

TerrainImporter::TerrainImporter (TerrainImporterP importer)
    {
    m_unmanaged = new _TerrainImporter_Unmanaged();
    m_unmanaged->m_importer = importer;
    }

TerrainImporter::!TerrainImporter()
    {
    TerrainImporter::~TerrainImporter ();
    }

TerrainImporter::~TerrainImporter()
    {
    delete m_unmanaged;
    m_unmanaged = nullptr;
    }

System::String^ TerrainImporter::FileUnitString::get()
    {
    return gcnew System::String (m_unmanaged->m_importer->GetFileUnitString ());
    }

BENTLEY_NAMESPACE_NAME::TerrainModelNET::Formats::FileUnit TerrainImporter::FileUnit::get ()
    {
    return (BENTLEY_NAMESPACE_NAME::TerrainModelNET::Formats::FileUnit)m_unmanaged->m_importer->GetFileUnit ();
    }

System::Collections::Generic::IEnumerable<TerrainInfo^>^ TerrainImporter::Terrains::get ()
    {
    const BENTLEY_NAMESPACE_NAME::TerrainModel::TerrainInfoList& list = m_unmanaged->m_importer->GetTerrains ();

    System::Collections::Generic::List<TerrainInfo^>^ retList = gcnew System::Collections::Generic::List<TerrainInfo^>();

    for (BENTLEY_NAMESPACE_NAME::TerrainModel::TerrainInfoList::const_iterator iter = list.begin (); iter != list.end (); iter++)
        {
        System::String^ description = iter->GetDescription ().GetWCharCP() ? gcnew System::String (iter->GetDescription ().GetWCharCP ()) : nullptr;
        retList->Add (gcnew TerrainInfo (gcnew System::String (iter->GetName().GetWCharCP()), description, iter->HasDefinition()));
        }

    return retList;
    }

IImporterCallback^ TerrainImporter::Callback::get()
    {
    return m_callback;
    }

void TerrainImporter::Callback::set (IImporterCallback^ value)
    {
    m_callback = value;
    
    if (m_unmanaged->m_unmanagedCallback)
        {
        m_unmanaged->m_importer->SetCallback (nullptr);
        delete m_unmanaged->m_unmanagedCallback;
        m_unmanaged->m_unmanagedCallback = nullptr;
        }
    if (value)
        {
        m_unmanaged->m_unmanagedCallback = new UnmanagedCallback (value);
        m_unmanaged->m_importer->SetCallback (m_unmanaged->m_unmanagedCallback);
        }
    }

ImportedTerrain^ TerrainImporter::ImportTerrain (System::String^ name)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (name);

    BENTLEY_NAMESPACE_NAME::TerrainModel::ImportedTerrain surface = m_unmanaged->m_importer->ImportTerrain (p);

    BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ dtm = surface.GetTerrain() ? BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM::FromHandle ((System::IntPtr)surface.GetTerrain()) : nullptr;
    System::String^ description = surface.GetDescription ().GetWCharCP () ? gcnew System::String (surface.GetDescription ().GetWCharCP ()) : nullptr;
    return gcnew ImportedTerrain (dtm, gcnew System::String (surface.GetName ().GetWCharCP ()), description, surface.HasDefinition ());
    }

System::Collections::Generic::IEnumerable<ImportedTerrain^>^ TerrainImporter::ImportTerrains ()
    {
    BENTLEY_NAMESPACE_NAME::TerrainModel::ImportedTerrainList list = m_unmanaged->m_importer->ImportTerrains ();

    System::Collections::Generic::List<ImportedTerrain^>^ retList = gcnew System::Collections::Generic::List<ImportedTerrain^>();

    for (BENTLEY_NAMESPACE_NAME::TerrainModel::ImportedTerrainList::const_iterator iter = list.begin(); iter != list.end(); iter++)
        {
        BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ dtm = iter->GetTerrain() ? BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM::FromHandle ((System::IntPtr)iter->GetTerrain()) : nullptr;
        System::String^ description = iter->GetDescription ().GetWCharCP () ? gcnew System::String (iter->GetDescription ().GetWCharCP ()) : nullptr;
        retList->Add (gcnew ImportedTerrain (dtm, gcnew System::String (iter->GetName ().GetWCharCP ()), description, iter->HasDefinition ()));
        }
    return retList;
    }

System::Collections::Generic::IEnumerable<ImportedTerrain^>^ TerrainImporter::ImportTerrains (System::Collections::Generic::IEnumerable<System::String^>^ names)
    {
    bvector<WString> importList;

    for each (System::String^ name in names)
        {
        pin_ptr<const wchar_t> p = PtrToStringChars (name);

        importList.push_back (WString(p));
        }
    BENTLEY_NAMESPACE_NAME::TerrainModel::ImportedTerrainList list = m_unmanaged->m_importer->ImportTerrains (importList);

    System::Collections::Generic::List<ImportedTerrain^>^ retList = gcnew System::Collections::Generic::List<ImportedTerrain^>();

    for (BENTLEY_NAMESPACE_NAME::TerrainModel::ImportedTerrainList::const_iterator iter = list.begin(); iter != list.end(); iter++)
        {
        BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ dtm = iter->GetTerrain() ? BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM::FromHandle ((System::IntPtr)iter->GetTerrain()) : nullptr;
        System::String^ description = iter->GetDescription ().GetWCharCP () ? gcnew System::String (iter->GetDescription ().GetWCharCP ()) : nullptr;
        retList->Add (gcnew ImportedTerrain (dtm, gcnew System::String (iter->GetName ().GetWCharCP ()), description, iter->HasDefinition ()));
        }
    return retList;
    }

Bentley::GeoCoordinatesNET::BaseGCS^ TerrainImporter::GCS::get ()
    {
    Bentley::GeoCoordinates::BaseGCSPtr gcs = m_unmanaged->m_importer->GetGCS ();

    if (gcs.IsValid())
        return gcnew Bentley::GeoCoordinatesNET::BaseGCS (gcs.get());
    return nullptr;
    }

ref class UnknownImporter : TerrainImporter
    {
    public: UnknownImporter (BENTLEY_NAMESPACE_NAME::TerrainModel::TerrainImporter* importer) : TerrainImporter (importer)
                {
                }
    };

TerrainImporter^ TerrainImporter::CreateImporter (System::String^ filename)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (filename);
    TerrainImporterPtr importer = BENTLEY_NAMESPACE_NAME::TerrainModel::TerrainImporter::CreateImporter (p);
    // ToDo work out what importer it is and create an instance of that.
    if (importer.IsValid ())
        {
        LandXMLImporterP landXMLImporter = dynamic_cast<LandXMLImporterP>(importer.get ());
        if (landXMLImporter != nullptr)
            return gcnew LandXMLImporter (landXMLImporter);

        MXFilImporterP mxFilImporter = dynamic_cast<MXFilImporterP>(importer.get ());
        if (mxFilImporter != nullptr)
            return gcnew MXFilImporter (mxFilImporter);

        LidarImporterP lidarImporter = dynamic_cast<LidarImporterP>(importer.get ());
        if (lidarImporter != nullptr)
            return gcnew LidarImporter (lidarImporter);

        InroadsImporterP inroadsImporter = dynamic_cast<InroadsImporterP>(importer.get ());
        if (inroadsImporter != nullptr)
            return gcnew InroadsImporter (inroadsImporter);

        return gcnew UnknownImporter (importer.get ());
        }
    return nullptr;
    }

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE