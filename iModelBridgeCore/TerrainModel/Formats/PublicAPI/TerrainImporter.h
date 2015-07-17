/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/PublicAPI/TerrainImporter.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

class TerrainInfo
    {
    private: WString m_name;
    private: WString m_description;
    private: bool m_hasDefinition;

    public: TerrainInfo (WCharCP name, WCharCP description, bool hasDefinition);

    public: WString GetName () const
        {
        return m_name;
        }
    public: WString GetDescription() const
        {
        return m_description;
        }
    public: bool HasDefinition () const
                {
                return m_hasDefinition;
                }
    };

class ImportedTerrain : public TerrainInfo
    {
    private: Bentley::TerrainModel::BcDTMPtr m_dtm;

    public: ImportedTerrain (BcDTMP dtm, WCharCP name, WCharCP description, bool hasDefinition);
    public: BcDTMP GetTerrain() const
                {
                return m_dtm.get();
                }

    };

typedef bvector <TerrainInfo> TerrainInfoList;
typedef bvector <ImportedTerrain> ImportedTerrainList;

struct TerrainImporter : RefCountedBase
    {
    struct ICallback
        {
        virtual bool StartTerrain (WCharCP name, WCharCP description, BcDTMPtr& dtm) = 0;
        virtual void AddFeature (DTMFeatureId id, WCharCP DTMAttribute, WCharCP featureDefinitionName, WCharCP featureName, WCharCP description, DTMFeatureType featureType, DPoint3dCP points, size_t numPoints) = 0;
        virtual bool EndTerrain (WCharCP name, BcDTMP dtm) = 0;
        };
        ICallback* m_callback;

    protected: TerrainImporter () : m_callback (nullptr)
        {
        }
    public: BENTLEYDTMFORMATS_EXPORT ICallback* GetCallback() const
                {
                return m_callback;
                }
    public: BENTLEYDTMFORMATS_EXPORT void SetCallback (ICallback* value)
                {
                WString l;
                m_callback = value;
                }
    protected: virtual WCharCP _GetFileUnitString () const { return L"unknown"; }
    protected: virtual FileUnit _GetFileUnit () const { return FileUnit::Unknown; }
    protected: virtual const TerrainInfoList& _GetTerrains () const = 0;
    protected: virtual ImportedTerrain _ImportTerrain (WCharCP name) const = 0;
    protected: virtual ImportedTerrainList _ImportTerrains () const = 0;
    protected: virtual ImportedTerrainList _ImportTerrains (bvector<WString>& names) const = 0;
    protected: virtual Bentley::GeoCoordinates::BaseGCSPtr _GetGCS () const { return nullptr; }
    public: BENTLEYDTMFORMATS_EXPORT WCharCP GetFileUnitString () const;
    public: BENTLEYDTMFORMATS_EXPORT FileUnit GetFileUnit () const;

    public: BENTLEYDTMFORMATS_EXPORT const TerrainInfoList& GetTerrains () const;

    public: BENTLEYDTMFORMATS_EXPORT ImportedTerrain ImportTerrain (WCharCP name) const;
    public: BENTLEYDTMFORMATS_EXPORT ImportedTerrainList ImportTerrains () const;
    public: BENTLEYDTMFORMATS_EXPORT ImportedTerrainList ImportTerrains (bvector<WString>& names) const;
    public: BENTLEYDTMFORMATS_EXPORT Bentley::GeoCoordinates::BaseGCSPtr GetGCS () const;

    public: BENTLEYDTMFORMATS_EXPORT static TerrainImporterPtr CreateImporter (WCharCP filename);
               //    public: BENTLEYDTMFORMATS_EXPORT static GetSupportedFiles
    };

/*__PUBLISH_SECTION_END__*/

struct SingleTerrainImporter : TerrainImporter
    {
    protected: TerrainInfoList m_surfaces;
    protected: WString m_fileName;
    protected: WString m_name;

    protected: SingleTerrainImporter (WCharCP filename) : m_fileName (filename)
        {
        m_name = BeFileName::GetFileNameWithoutExtension (filename);
        m_surfaces.push_back (TerrainInfo (m_name.GetWCharCP (), nullptr, true)); // ToDo need to work this out.
        }

    protected: virtual const TerrainInfoList& _GetTerrains () const override
        {
        return m_surfaces;
        }

    protected: virtual ImportedTerrainList _ImportTerrains () const override
        {
        ImportedTerrainList ret;
        ret.push_back (_ImportTerrain (m_name.GetWCharCP ()));
        return ret;
        }

    protected: virtual ImportedTerrainList _ImportTerrains (bvector <WString>& names) const override
        {
        ImportedTerrainList ret;
        for (bvector<WString>::const_iterator iter = names.begin (); iter != names.end (); iter++)
            ret.push_back (_ImportTerrain (iter->GetWCharCP ()));
        return ret;
        }
    };
/*__PUBLISH_SECTION_START__*/

END_BENTLEY_TERRAINMODEL_NAMESPACE
