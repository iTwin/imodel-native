/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/LandXML.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "LandXML.h"

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

LandXMLImporter^ LandXMLImporter::Create (System::String^ filename)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (filename);
    LandXMLImporterPtr importer = BENTLEY_NAMESPACE_NAME::TerrainModel::LandXMLImporter::Create (p);
    if (importer.IsValid ())
        return gcnew LandXMLImporter (importer.get ());
    return nullptr;
    }

LandXMLImporter::LandXMLImporter (BENTLEY_NAMESPACE_NAME::TerrainModel::LandXMLImporter* importer) : TerrainImporter (importer)
    {
    m_importer = importer;
    }

LandXMLOptions LandXMLImporter::Options::get ()
    {
    return (LandXMLOptions)m_importer->GetOptions ();
    }

void LandXMLImporter::Options::set (LandXMLOptions value)
    {
    m_importer->SetOptions ((Bentley::TerrainModel::LandXMLOptions)value);
    }

LandXMLExporter::LandXMLExporter ()
    {
    LandXMLExporterPtr exporter = BENTLEY_NAMESPACE_NAME::TerrainModel::LandXMLExporter::Create ();
    m_exporter = exporter.get ();
    SetTerrainExporter (m_exporter);
    }

LandXMLOptions LandXMLExporter::Options::get ()
    {
    return (LandXMLOptions)m_exporter->GetOptions ();
    }

void LandXMLExporter::Options::set (LandXMLOptions value)
    {
    m_exporter->SetExportType ((Bentley::TerrainModel::LandXMLExporter::LandXMLExportTypes)value);
    }

System::String^ LandXMLExporter::ApplicationName::get ()
    {
    WCharCP value = m_exporter->GetApplicationName ().GetWCharCP();

    if (value)
        return gcnew System::String (value);
    return nullptr;
    }

void LandXMLExporter::ApplicationName::set (System::String^ value)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (value);
    m_exporter->SetApplicationName (p);
    }

System::String^ LandXMLExporter::Version::get ()
    {
    WCharCP value = m_exporter->GetVersion ().GetWCharCP ();

    if (value)
        return gcnew System::String (value);
    return nullptr;
    }

void LandXMLExporter::Version::set (System::String^ value)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (value);
    m_exporter->SetVersion (p);
    }

System::String^ LandXMLExporter::ProjectName::get ()
    {
    WCharCP value = m_exporter->GetProjectName ().GetWCharCP ();

    if (value)
        return gcnew System::String (value);
    return nullptr;
    }

void LandXMLExporter::ProjectName::set (System::String^ value)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (value);
    m_exporter->SetProjectName (p);
    }

System::String^ LandXMLExporter::ProjectDescription::get ()
    {
    WCharCP value = m_exporter->GetProjectDescription ().GetWCharCP ();

    if (value)
        return gcnew System::String (value);
    return nullptr;
    }

void LandXMLExporter::ProjectDescription::set (System::String^ value)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (value);
    m_exporter->SetProjectDescription (p);
    }

void LandXMLExporter::CreateXML (System::String^ filename, System::Collections::Generic::IEnumerable<NamedTerrain^>^ terrains)
    {
    bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::LandXMLExporter::NamedDTM> uTerrains;

    for each (NamedTerrain^ terrain in terrains)
        {
        pin_ptr<const wchar_t> uName = PtrToStringChars (terrain->Name);
        pin_ptr<const wchar_t> uDescription = PtrToStringChars (terrain->Description);
        BcDTMP dtmP = (BcDTMP)terrain->Terrain->ExternalHandle.ToPointer();
        BENTLEY_NAMESPACE_NAME::TerrainModel::LandXMLExporter::NamedDTM uTerrain (dtmP, uName, uDescription);
        uTerrains.push_back (uTerrain);
        }
    pin_ptr<const wchar_t> uFilename = PtrToStringChars (filename);
    m_exporter->CreateXML (uFilename, uTerrains);
    }

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE