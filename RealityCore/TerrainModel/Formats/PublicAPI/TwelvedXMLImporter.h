/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

TERRAINMODEL_TYPEDEFS(TwelvedXMLImporter)

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<TwelvedXMLImporter> TwelvedXMLImporterPtr;

enum class TwelvedXMLOptions
    {
    Source = 1 << 0,
    Definition = 1 << 1,
    SourceAndDefinition = Definition | Source,
    };

ENUM_IS_FLAGS(TwelvedXMLOptions);

struct TwelvedXMLImporter : TerrainImporter
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

        TwelvedXMLOptions m_options = TwelvedXMLOptions::SourceAndDefinition;
        bool m_initialized;
        mutable TerrainInfoList m_surfaces;
        BeXmlReaderPtr m_reader;
        WString m_filename;
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
        bool m_isGeopakProject;
        mutable BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr m_gcs;

    private: TwelvedXMLImporter(WCharCP filename);
    private: bool Read12dFaceIndexes(WStringCR value, int& p1, int& p2, int& p3);
    private: DPoint3d Read12dPoint(WStringCR value, bool is3D);
    private: DTMFeatureId _AddLinearFeature(DPoint3d* points, DTMFeatureType featureType);
    private: int _BuildDTMFromTriangles();
    private: void Read12dUnits();
    private: void Read12dApplication();
    private: void Read12dComments();
    private: void Read12dModel();
    private: void Read12dSurface();
    private: void Read12dAttributes(WStringR styleName, int& weedValue, int& facesValue);
    private: void ReadToEndOfElements(Utf8StringCR endNodeName);
    private: WString GetAttribute(char* attributeName);
    private: void Read12dXML();
    private: void Initialize();
    protected: virtual WCharCP _GetFileUnitString() const override;
    protected: virtual FileUnit _GetFileUnit () const override;
    protected: virtual const TerrainInfoList& _GetTerrains() const override;
    protected: virtual ImportedTerrain _ImportTerrain(WCharCP name) const override;
    protected: virtual ImportedTerrainList _ImportTerrains() const override;
    protected: virtual ImportedTerrainList _ImportTerrains(bvector <WString>& names) const override;
    protected: virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr _GetGCS() const override
        {
        return m_gcs;
        }
               /*__PUBLISH_SECTION_START__*/

    public: BENTLEYDTMFORMATS_EXPORT static bool IsFileSupported(WCharCP filename);
    public: BENTLEYDTMFORMATS_EXPORT static TwelvedXMLImporterPtr Create(WCharCP filename);

    public: BENTLEYDTMFORMATS_EXPORT TwelvedXMLOptions GetOptions();
    public: BENTLEYDTMFORMATS_EXPORT void SetOptions(TwelvedXMLOptions options);

    };

END_BENTLEY_TERRAINMODEL_NAMESPACE
