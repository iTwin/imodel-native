#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Formats/TerrainImporter.h>
#include <TerrainModel/Formats/LandXMLImporter.h>
#include <TerrainModel/Formats/InroadsImporter.h>
#include <TerrainModel/Formats/LidarImporter.h>
#include <TerrainModel/Formats/MX.h>
#include <Bentley/BeFileName.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

TerrainInfo::TerrainInfo (WCharCP name, WCharCP description, bool hasDefinition) : m_name (name), m_hasDefinition (hasDefinition)
    {
    }

ImportedTerrain::ImportedTerrain (BcDTMP dtm, WCharCP name, WCharCP description, bool hasDefinition) : TerrainInfo (name, description, hasDefinition), m_dtm (dtm)
    {
    }

struct StandardImporter;
typedef RefCountedPtr<StandardImporter> StandardImporterPtr;

struct StandardImporter : SingleTerrainImporter
    {
    protected: StandardImporter (WCharCP filename) : SingleTerrainImporter (filename)
                   {
                   }

    protected: virtual ImportedTerrain _ImportTerrain (WCharCP name) const override
                   {
                   if (name == m_name)
                       {
                       if (BeFileName::GetExtension (m_fileName.GetWCharCP()).CompareToI (L"xyz") == 0)
                           return ImportedTerrain (BcDTM::CreateFromXyzFile (m_fileName.GetWCharCP ()).get(), m_name.GetWCharCP(), nullptr, true);
                       else if (BeFileName::GetExtension (m_fileName.GetWCharCP()).CompareToI (L"dat") == 0)
                           return ImportedTerrain (BcDTM::CreateFromGeopakDatFile (m_fileName.GetWCharCP ()).get (), m_name.GetWCharCP (), nullptr, true);
                       else
                           return ImportedTerrain (BcDTM::CreateFromTinFile (m_fileName.GetWCharCP ()).get (), m_name.GetWCharCP (), nullptr, true);
                       }
                   return ImportedTerrain (nullptr, name, nullptr, false);
                   }

    public: static bool IsFileSupported (WCharCP filename)
        {
        WString extension = BeFileName::GetExtension (filename);
        return (extension.CompareToI (L"tin") == 0 || extension.CompareToI (L"bcdtm") == 0 || extension.CompareToI (L"xyz") == 0 || extension.CompareToI (L"dat") == 0);
        }

    public: static StandardImporterPtr Create (WCharCP filename)
                {
                return new StandardImporter (filename);
                }
    };


WCharCP TerrainImporter::GetFileUnitString () const
    {
    return _GetFileUnitString ();
    }

FileUnit TerrainImporter::GetFileUnit () const
    {
    return _GetFileUnit ();
    }

const TerrainInfoList& TerrainImporter::GetTerrains () const
    {
    return _GetTerrains ();
    }

ImportedTerrain TerrainImporter::ImportTerrain (WCharCP name) const
    {
    return _ImportTerrain (name);
    }

ImportedTerrainList TerrainImporter::ImportTerrains () const
    {
    return _ImportTerrains ();
    }

ImportedTerrainList TerrainImporter::ImportTerrains (bvector<WString>& names) const
    {
    return _ImportTerrains (names);
    }

Bentley::GeoCoordinates::BaseGCSPtr TerrainImporter::GetGCS () const
    {
    return _GetGCS ();
    }

TerrainImporterPtr TerrainImporter::CreateImporter (WCharCP filename)
    {
    if (LandXMLImporter::IsFileSupported (filename))
        return LandXMLImporter::Create (filename);
    if (LidarImporter::IsFileSupported (filename))
        return LidarImporter::Create (filename);
    if (MXFilImporter::IsFileSupported (filename))
        return MXFilImporter::Create (filename);
    if (InroadsImporter::IsFileSupported (filename))
        return InroadsImporter::Create (filename);
    if (StandardImporter::IsFileSupported (filename))
        return StandardImporter::Create (filename);
    return nullptr;
    }
