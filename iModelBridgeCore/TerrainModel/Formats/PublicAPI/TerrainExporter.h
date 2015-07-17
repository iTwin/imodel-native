/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/PublicAPI/TerrainExporter.h $
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

struct TerrainExporter : RefCountedBase
    {
    public: struct NamedDTM
        {
        private:
            BcDTMPtr m_dtm;
            WString m_name;
            WString m_description;

        public: NamedDTM (BcDTMPtr dtm, WCharCP name, WCharCP description)
            {
            m_dtm = dtm;
            m_name = name;
            m_description = description;
            }

        public: BcDTMPtr GetBcDTMPtr () const
            {
            return m_dtm;
            }

        public: WCharCP GetName () const
            {
            return m_name.GetWCharCP();
            }
        public: WCharCP GetDescription () const
            {
            return m_description.GetWCharCP();
            }
        };
    public: struct IFeatureInfoCallback
        {
        virtual void StartTerrain (NamedDTM const& dtm) {};
        virtual bool GetFeatureInfo (WStringR name, WStringR desc, WStringR featureStyle, DTMFeatureType type, DTMFeatureId id, DTMUserTag userTag) = 0;
        virtual void EndTerrain () {};
        };

    protected: TerrainExporter ()
        {
        m_featureInfoCallback = nullptr;
        }
    protected: virtual WCharCP _GetFileUnitString ();
    protected: virtual FileUnit _GetFileUnit () { return FileUnit::Unknown; }
    protected: virtual void _SetFileUnit (FileUnit value) {  }

    public: BENTLEYDTMFORMATS_EXPORT void SetFeatureInfoCallback (IFeatureInfoCallback* value);
    public: BENTLEYDTMFORMATS_EXPORT IFeatureInfoCallback* GetFeatureInfoCallback ();

    public: BENTLEYDTMFORMATS_EXPORT WCharCP GetFileUnitString ();

    public: BENTLEYDTMFORMATS_EXPORT FileUnit GetFileUnit ();
    public: BENTLEYDTMFORMATS_EXPORT void SetFileUnit (FileUnit value);

    protected:
        IFeatureInfoCallback* m_featureInfoCallback;
    };

END_BENTLEY_TERRAINMODEL_NAMESPACE
