/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/PublicAPI/LandXMLImporter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

/*__PUBLISH_SECTION_START__*/
#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Formats/TerrainImporter.h>
#include <Bentley/WString.h>
/*__PUBLISH_SECTION_END__*/
#include <BeXml/BeXml.h>
/*__PUBLISH_SECTION_START__*/
#include <list>
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/bcDTMClass.h>

TERRAINMODEL_TYPEDEFS (LandXMLImporter)

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<LandXMLImporter> LandXMLImporterPtr;

enum class LandXMLOptions
    {
    Source = 1 << 0,
    Definition = 1 << 1,
    SourceAndDefinition = Definition | Source,
    };

ENUM_IS_FLAGS (LandXMLOptions);


struct LandXMLImporter : TerrainImporter
    {
    /*__PUBLISH_SECTION_END__*/

    private:
        struct FeatureAttributes
            {
            WString featureDefinition;
            WString featureId;
            WString userTag;
            bool triangulate;

            FeatureAttributes()
                {
                triangulate = true;
                }
            };

        LandXMLOptions m_options = LandXMLOptions::SourceAndDefinition;
        bool m_initialized;
        mutable TerrainInfoList m_surfaces;
        BeXmlReaderPtr m_reader;
        WString m_filename;
        //        DateTime m_fileTime;
        WString m_linearUnit;
        bool m_isMetric;
        typedef bvector <DPoint3d> PointArray;
        int m_currentPointList;
        typedef bvector <PointArray> PointArrayList;
        PointArrayList m_segments;
        BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr m_dtm;
        double m_2delevation;
        mutable bmap <WString, BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr> m_namedDtms;
        bmap <WString, DPoint3d>* m_pntRefs;
        bool m_secondPass;  // Second pass for pntRef resolution.
        mutable bool m_importAllDTMS;
        bool m_isMXProject;
        mutable BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr m_gcs;

    private: LandXMLImporter (WCharCP filename);

    private: void AddFeatureToDTM (WStringCR name, WStringCR description, DTMFeatureType type, const FeatureAttributes& featureAttribute, WStringCR DTMAttribute);
    private: void ReadFeatureAttributes (FeatureAttributes& featureAttribute);
    private: DTMFeatureType ConvertBoundaryFeatureType (WStringCR type);

    private: void ReadFaceIndexs (WStringCR value, int& p1, int& p2, int& p3);

    private: DPoint3d ReadPoint (WStringCR value, bool is3D);

    private: void ReadPoints (WStringCR value, bool is3D);

    private: void ReadPntList2D ();
    private: void ReadPntList3D ();
    private: WString GetAttribute (char* attributeName);
    private: void ReadContour ();
    private: void ReadContours ();
    private: void ReadBreakLine ();
    private: void ReadBreakLines ();
    private: void ReadBoundary ();
    private: void ReadBoundaries ();
    private: void ReadDataPoints ();
    private: void ReadSourceData ();
    private: void ReadDefinition ();
    private: void ReadSurface ();
    private: void ReadCoordinateSystem ();
    private: void ReadUnits ();
    private: void ReadApplication ();
    private: void ReadXML ();
    private: void Initalize ();
    private: void ClearPoints ()
        {
        m_segments.clear ();
        m_currentPointList = 0;
        }

    protected: virtual WCharCP _GetFileUnitString () const override;
    protected: virtual FileUnit _GetFileUnit () const override;

    protected: virtual const TerrainInfoList& _GetTerrains () const override;
    protected: virtual ImportedTerrain _ImportTerrain (WCharCP name) const override;
    protected: virtual ImportedTerrainList _ImportTerrains () const override;
    protected: virtual ImportedTerrainList _ImportTerrains (bvector <WString>& names) const override;
    protected: virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr _GetGCS () const override { return m_gcs; }
    /*__PUBLISH_SECTION_START__*/

    public: BENTLEYDTMFORMATS_EXPORT static bool IsFileSupported (WCharCP filename);
    public: BENTLEYDTMFORMATS_EXPORT static LandXMLImporterPtr Create (WCharCP filename);

    public: BENTLEYDTMFORMATS_EXPORT LandXMLOptions GetOptions ();
    public: BENTLEYDTMFORMATS_EXPORT void SetOptions (LandXMLOptions options);

    };

END_BENTLEY_TERRAINMODEL_NAMESPACE
