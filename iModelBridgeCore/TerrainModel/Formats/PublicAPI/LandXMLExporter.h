/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/PublicAPI/LandXMLExporter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

/*__PUBLISH_SECTION_START__*/
#include <TerrainModel/Formats/Formats.h>
#include <Bentley/WString.h>
/*__PUBLISH_SECTION_END__*/
#include <BeXml/BeXml.h>
/*__PUBLISH_SECTION_START__*/
#include <list>
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <TerrainModel/Formats/TerrainExporter.h>
#include <TerrainModel/Formats/LandXMLImporter.h>

TERRAINMODEL_TYPEDEFS (LandXMLExporter)
ADD_BENTLEY_TYPEDEFS (Bentley::TerrainModel, LandXMLExporter);

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<LandXMLExporter> LandXMLExporterPtr;

struct DTMLandXMLFeatureInfo;
struct DTMFeatureInfo;

struct LandXMLExporter : TerrainExporter
    {
private:
    WString m_applicationName;
    WString m_version;
    WString m_projectName;
    WString m_projectDescription;
    bool m_addExtraInformation;
    BeXmlWriterPtr m_writer;
    bool m_hasBreakLines;
    bool m_hasContours;
    bool m_hasBoundaries;
    int m_pntNum;
    LandXMLOptions m_options = LandXMLOptions::SourceAndDefinition;

    protected: LandXMLExporter ();

    /*__PUBLISH_SECTION_START__*/

    public: BENTLEYDTMFORMATS_EXPORT static LandXMLExporterPtr Create ();

    public: void SetApplicationName (WCharCP value)
                {
                m_applicationName = value;
                }
            WStringCR GetApplicationName ()
                {
                return m_applicationName;
                }

    public:
        void SetOptions (LandXMLOptions value)
            {
            m_options = value;
            }
        LandXMLOptions GetOptions ()
            {
            return m_options;
            }
            

    public: void SetVersion (WCharCP value)
                {
                m_version = value;
                }
            WStringCR GetVersion ()
                {
                return m_version;
                }

    public: void SetProjectName (WCharCP value)
                {
                m_projectName = value;
                }
            WStringCR GetProjectName ()
                {
                return m_projectName;
                }

    public: void SetProjectDescription (WCharCP value)
                {
                m_projectDescription = value;
                }
            WStringCR GetProjectDescription ()
                {
                return m_projectDescription;
                }

    public: BENTLEYDTMFORMATS_EXPORT void CreateXML (WCharCP filename, WCharCP surfaceName, BcDTMPtr dtm)
                {
                CreateXML (filename, NamedDTM (dtm, surfaceName, surfaceName));
                }

    public: BENTLEYDTMFORMATS_EXPORT void CreateXML (WCharCP filename, NamedDTM const& dtm)
        {
        bvector<NamedDTM> dtms;
        dtms.push_back (dtm);
        }

    public: BENTLEYDTMFORMATS_EXPORT void CreateXML (WCharCP filename, bvector<NamedDTM> const& dtms);

    /*__PUBLISH_SECTION_END__*/
    protected: virtual FileUnit _GetFileUnit () { return m_linearUnit; }
    protected: virtual void _SetFileUnit (FileUnit value) { m_linearUnit = value; }

    private: void WriteXML (WCharCP filename, bvector<NamedDTM> const& dtms);
    private: void WriteUnits ();
    private: void WriteProject ();
    private: void WriteApplication ();
    private: void WriteSurfaces (bvector<NamedDTM> const& dtms);
    private: void WriteSurface (const NamedDTM& namedDtm);
    private: void WriteSourceData (BcDTMR dtm);
    private: bool WriteBreakLine (const DTMFeatureInfo& featureInfo);
    private: void WriteBreakLines (BcDTMR dtm);
    private: bool WriteBoundary (const DTMFeatureInfo& featureInfo);
    private: void WriteBoundaries (BcDTMR dtm);
    private: bool WriteContour (const DTMFeatureInfo& featureInfo);
    private: void WriteContours (BcDTMR dtm);
    private: bool WriteRandomPoints (const DTMLandXMLFeatureInfo& featureInfo, DPoint3dP tPoint, int numPts);
    private: bool WriteGroupPoints (const DTMFeatureInfo& featureInfo);
    private: void WriteDataPoints (BcDTMR dtm);
    private: bool WritePnts (const DTMLandXMLFeatureInfo& featureInfo, DPoint3dP tPoint, int numPts);
    private: bool WriteFaces (const DTMLandXMLFeatureInfo& featureInfo, DPoint3dP tPoint, int numPts);
    private: void WriteDefinition (BcDTMR dtm);
    private: WString WriteFeatureNameAndDescription (const DTMLandXMLFeatureInfo& featureInfo);
    private: WString WriteFeatureNameAndDescription (const DTMFeatureInfo& featureInfo);
    private: void WriteFeatureStuff (WStringCR nodeType, const DTMFeatureInfo& featureInfo, WStringCR featureStyle);
    private: void WriteFeatureStuff (WStringCR nodeType, const DTMLandXMLFeatureInfo& featureInfo, WStringCR featureStyle);
    private: WString FormatDouble (double const& val);
    private: WString FormatInt (int val);
    private: WString FormatInt (Int64 const& val);
    private: void WritePoint (DPoint3dCR pt, bool is3d = true);
    private: void WritePoints (DPoint3dCP tPoint, size_t numPts, bool is3d = true);

    FileUnit m_linearUnit;
    /*__PUBLISH_SECTION_START__*/
    };

END_BENTLEY_TERRAINMODEL_NAMESPACE

