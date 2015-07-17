/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/LandXMLExporter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/WString.h>
#include <BeXml/BeXml.h>
#include <Bentley\DateTime.h>
#include <list>

/*__PUBLISH_SECTION_START__*/
#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Core/DTMIterators.h>
#include <Bentley/WString.h>
/*__PUBLISH_SECTION_END__*/
#include <BeXml/BeXml.h>
/*__PUBLISH_SECTION_START__*/
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/bcDTMClass.h>

#include <TerrainModel/Formats/LandXMLExporter.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

Utf8CP cLandXMLNS ("http://www.landxml.org/schema/LandXML-1.2");
Utf8CP cLandXMLSchemaLocation ("http://www.landxml.org/schema/LandXML-1.2 http://www.landxml.org/schema/LandXML-1.2/LandXML-1.2.xsd");
Utf8CP cLandXMLVersion ("1.2");
Utf8CP cXSIPrefixAndNamespaceURI ("http://www.w3.org/2001/XMLSchema-instance");

struct DTMLandXMLFeatureInfo
    {
    DTMUserTag DtmUserTag;
    DTMFeatureId DtmFeatureId;
    DTMFeatureType DtmFeatureType;
    };

struct CallbackData
    {
    private:
        LandXMLExporter* m_exporter;
        std::function<bool (LandXMLExporter&, const DTMLandXMLFeatureInfo&, DPoint3dP pts, int numPts)> m_method;
    public:
        CallbackData (LandXMLExporter* exporter, std::function<bool (LandXMLExporter&, const DTMLandXMLFeatureInfo&, DPoint3dP pts, int numPts)> method) : m_exporter (exporter), m_method (method)
            {
            }

        static int Callback (DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId featureId, DPoint3d *points, size_t numPoints, void* userArg)
            {
            CallbackData& data = *(static_cast<CallbackData*>(userArg));
            DTMLandXMLFeatureInfo info;
            info.DtmFeatureId = featureId;
            info.DtmFeatureType = dtmFeatureType;
            info.DtmUserTag = userTag;
            return data.m_method (*data.m_exporter, info, points, (int)numPoints) ? DTM_SUCCESS : DTM_ERROR;
            }
    };

LandXMLExporter::LandXMLExporter ()
    {
    m_applicationName = L"OpenRoads";
    m_version = L"1.0";
    m_linearUnit = FileUnit::Meter;
    m_addExtraInformation = true;
    m_hasBreakLines = false;
    m_hasContours = false;
    m_hasBoundaries = false;
    }

BENTLEYDTMFORMATS_EXPORT LandXMLExporterPtr LandXMLExporter::Create ()
    {
    LandXMLExporter* writer = new LandXMLExporter ();
    return writer;
    }

void LandXMLExporter::CreateXML (WCharCP filename, bvector<NamedDTM> const& dtms)
    {
    WriteXML (filename, dtms);
    }

void LandXMLExporter::WriteXML (WCharCP filename, bvector<NamedDTM> const& dtms)
    {
    m_writer = BeXmlWriter::CreateFileWriter (filename);
    m_writer->WriteDocumentStart (XML_CHAR_ENCODING_UTF8);
    m_writer->SetIndentation (2);

    m_writer->WriteElementStart ("LandXML"); // Todo , LandXMLNS);
    m_writer->WriteAttribute ("xsi:schemaLocation", cLandXMLSchemaLocation);
    m_writer->WriteAttribute ("xmlns", cLandXMLNS);
    m_writer->WriteAttribute ("xmlns:xsi", cXSIPrefixAndNamespaceURI);
    m_writer->WriteAttribute ("version", cLandXMLVersion);
    DateTime now = DateTime::GetCurrentTime ();
    WString date, time;

    date.Sprintf (L"%d-%02d-%02d", now.GetYear (), now.GetMonth (), now.GetDay ());
    time.Sprintf (L"%02d:%02d:%02d", now.GetHour (), now.GetMinute (), now.GetSecond ());
    m_writer->WriteAttribute ("date", date.GetWCharCP ());
    m_writer->WriteAttribute ("time", time.GetWCharCP ());

    WriteUnits ();
    WriteProject ();
    WriteApplication ();
    WriteSurfaces (dtms);

    m_writer->WriteElementEnd ();
    m_writer = nullptr;
    }

void LandXMLExporter::WriteUnits ()
    {
    m_writer->WriteElementStart ("Units");

    if (m_linearUnit == FileUnit::Millimeter ||
        m_linearUnit == FileUnit::Centimeter ||
        m_linearUnit == FileUnit::Meter ||
        m_linearUnit == FileUnit::Kilometer ||
        m_linearUnit == FileUnit::Custom ||
        m_linearUnit == FileUnit::Unknown)
        {
        m_writer->WriteElementStart ("Metric");
        m_writer->WriteAttribute ("areaUnit", L"squareMeter");
        m_writer->WriteAttribute ("linearUnit", GetFileUnitString ());
        m_writer->WriteAttribute ("volumeUnit", L"cubicMeter");
        m_writer->WriteAttribute ("temperatureUnit", L"celsius");
        m_writer->WriteAttribute ("pressureUnit", L"HPA");
        m_writer->WriteElementEnd ();
        }
    else
        {
        m_writer->WriteElementStart ("Imperial");
        m_writer->WriteAttribute ("areaUnit", L"squareFoot");
        m_writer->WriteAttribute ("linearUnit", GetFileUnitString ());
        m_writer->WriteAttribute ("volumeUnit", L"cubicYard");
        m_writer->WriteAttribute ("temperatureUnit", L"fahrenheit");
        m_writer->WriteAttribute ("pressureUnit", L"inHG");
        m_writer->WriteElementEnd ();
        }
    m_writer->WriteElementEnd ();
    }

void LandXMLExporter::WriteProject ()
    {
    if (!m_projectName.empty ())
        {
        m_writer->WriteElementStart ("Project");
        m_writer->WriteAttribute ("name", m_projectName.GetWCharCP ());
        m_writer->WriteAttribute ("desc", m_projectDescription.GetWCharCP ());
        m_writer->WriteElementEnd ();
        }
    }

void LandXMLExporter::WriteApplication ()
    {
    m_writer->WriteElementStart ("Application");
    m_writer->WriteAttribute ("name", m_applicationName.GetWCharCP ());
    m_writer->WriteAttribute ("version", m_version.GetWCharCP ());
    m_writer->WriteAttribute ("manufacturer", L"Bentley Systems, Inc.");
    m_writer->WriteAttribute ("manufacturerURL", L"http://www.bentley.com");
    m_writer->WriteElementEnd ();
    }

void LandXMLExporter::WriteSurfaces (bvector<NamedDTM> const& dtms)
    {
    m_writer->WriteElementStart ("Surfaces");
    for (bvector<NamedDTM>::const_iterator dtm = dtms.begin (); dtm != dtms.end (); dtm++)
        WriteSurface (*dtm);

    m_writer->WriteElementEnd ();
    }

void LandXMLExporter::WriteSurface (const NamedDTM& namedDtm)
    {
    if (nullptr != m_featureInfoCallback)
        m_featureInfoCallback->StartTerrain (namedDtm);
    m_writer->WriteElementStart ("Surface");
    m_writer->WriteAttribute ("name", namedDtm.GetName ());

    if (!WString::IsNullOrEmpty (namedDtm.GetDescription ()))
        m_writer->WriteAttribute ("desc", namedDtm.GetDescription ());

    BcDTMR dtm = *namedDtm.GetBcDTMPtr ().get ();
    if (dtm.GetDTMState () != DTMState::Tin)
        WriteSourceData (dtm);
    else
        {
        if ((m_options & LandXMLOptions::Source) == LandXMLOptions::Source)
            WriteSourceData (dtm);

        if ((m_options & LandXMLOptions::Definition) == LandXMLOptions::Definition)
            WriteDefinition (dtm);
        }
    m_writer->WriteElementEnd ();
    if (nullptr != m_featureInfoCallback)
        m_featureInfoCallback->EndTerrain ();
    }

void LandXMLExporter::WriteSourceData (BcDTMR dtm)
    {
    m_writer->WriteElementStart ("SourceData");
    WriteDataPoints (dtm);
    WriteBoundaries (dtm);
    WriteBreakLines (dtm);
    WriteContours (dtm);
    m_writer->WriteElementEnd ();
    }

bool LandXMLExporter::WriteBreakLine (const DTMFeatureInfo& featureInfo)
    {
    if (!m_hasBreakLines)
        {
        m_writer->WriteElementStart ("Breaklines");
        m_hasBreakLines = true;
        }

    m_writer->WriteElementStart ("Breakline");
    m_writer->WriteAttribute ("brkType", L"standard");
    WString featureStyle = WriteFeatureNameAndDescription (featureInfo);
    // attribute name
    m_writer->WriteElementStart ("PntList3D");

    bvector<DPoint3d> points;
    featureInfo.GetFeaturePoints (points);
    WritePoints (points.data(), points.size());
    m_writer->WriteElementEnd ();
    WriteFeatureStuff (L"Breakline", featureInfo, featureStyle);
    m_writer->WriteElementEnd ();
    return true;
    }

void LandXMLExporter::WriteBreakLines (BcDTMR dtm)
    {
    m_hasBreakLines = false;

    DTMFeatureEnumerator featureEnumerator (dtm);

    featureEnumerator.ExcludeAllFeatures ();
    featureEnumerator.IncludeFeature (DTMFeatureType::Breakline);

    featureEnumerator.SetSort (false);

    for (DTMFeatureInfo feature : featureEnumerator)
        WriteBreakLine (feature);

    if (m_hasBreakLines)
        m_writer->WriteElementEnd ();
    }

bool LandXMLExporter::WriteBoundary (const DTMFeatureInfo& featureInfo)
    {
    if (!m_hasBoundaries)
        {
        m_writer->WriteElementStart ("Boundaries");
        m_hasBoundaries = true;
        }

    m_writer->WriteElementStart ("Boundary");
    switch (featureInfo.FeatureType())
        {
        case DTMFeatureType::TinHull:    // Calculated Boundary Hull
        case DTMFeatureType::Hull:
        case DTMFeatureType::DrapeHull:
            m_writer->WriteAttribute ("bndType", L"outer");
            break;
        case DTMFeatureType::Island:
            m_writer->WriteAttribute ("bndType", L"island");
            break;
        case DTMFeatureType::Void:
        case DTMFeatureType::Hole:
        case DTMFeatureType::DrapeVoid:
            m_writer->WriteAttribute ("bndType", L"void");
            break;
        }
    m_writer->WriteAttribute ("edgeTrim", L"true");

    WString featureStyle = WriteFeatureNameAndDescription (featureInfo);

    // ToDo? Write Drape features as PntList2D?
    // attribute name
    m_writer->WriteElementStart ("PntList3D");

    bvector<DPoint3d> points;
    featureInfo.GetFeaturePoints (points);
    WritePoints (points.data (), points.size ());
    m_writer->WriteElementEnd ();
    WriteFeatureStuff (L"Boundary", featureInfo, featureStyle);
    m_writer->WriteElementEnd ();
    return true;
    }

void LandXMLExporter::WriteBoundaries (BcDTMR dtm)
    {
    m_hasBoundaries = false;
    DTMFeatureEnumerator featureEnumerator (dtm);

    featureEnumerator.ExcludeAllFeatures ();
    featureEnumerator.IncludeFeature (DTMFeatureType::Hull);
    featureEnumerator.IncludeFeature (DTMFeatureType::Void);
    featureEnumerator.IncludeFeature (DTMFeatureType::BreakVoid);
    featureEnumerator.IncludeFeature (DTMFeatureType::DrapeVoid);
    featureEnumerator.IncludeFeature (DTMFeatureType::Island);
    featureEnumerator.IncludeFeature (DTMFeatureType::Hole);
// What should we do with VoidLine and HoleLine?
    featureEnumerator.SetSort (false);
    featureEnumerator.SetReadSourceFeatures (false); // Only get the features in the Tin.

    for (const DTMFeatureInfo feature : featureEnumerator)
        WriteBoundary (feature);

    if (m_hasBoundaries)
        m_writer->WriteElementEnd ();
    }

bool LandXMLExporter::WriteContour (const DTMFeatureInfo& featureInfo)
    {
    if (!m_hasContours)
        {
        m_writer->WriteElementStart ("Contours");
        m_hasContours = true;
        }

    m_writer->WriteElementStart ("Contour");
    bvector<DPoint3d> points;
    featureInfo.GetFeaturePoints (points);

    m_writer->WriteAttribute ("elev", FormatDouble (points[0].z).GetWCharCP ());
    WString featureStyle = WriteFeatureNameAndDescription (featureInfo);

    m_writer->WriteElementStart ("PntList2D");

    WritePoints (points.data (), points.size (), false);
    m_writer->WriteElementEnd ();

    WriteFeatureStuff (L"Contour", featureInfo, featureStyle);
    m_writer->WriteElementEnd ();
    return true;
    }

void LandXMLExporter::WriteContours (BcDTMR dtm)
    {
    m_hasContours = false;
    DTMFeatureEnumerator featureEnumerator (dtm);

    featureEnumerator.ExcludeAllFeatures ();
    featureEnumerator.IncludeFeature (DTMFeatureType::Contour);

    featureEnumerator.SetSort (false);

    for (DTMFeatureInfo feature : featureEnumerator)
        WriteContour (feature);

    if (m_hasContours)
        m_writer->WriteElementEnd ();
    }

bool LandXMLExporter::WriteRandomPoints (const DTMLandXMLFeatureInfo& featureInfo, DPoint3d* tPoint, int numPts)
    {
    m_writer->WriteElementStart ("DataPoints");

    WString featureStyle = WriteFeatureNameAndDescription (featureInfo);

    m_writer->WriteElementStart ("PntList3D");

    WritePoints (tPoint, numPts);
    m_writer->WriteElementEnd ();

    if (m_addExtraInformation)
        WriteFeatureStuff (L"RandomPoints", featureInfo, featureStyle);

    m_writer->WriteElementEnd ();

    return true;
    }

bool LandXMLExporter::WriteGroupPoints (const DTMFeatureInfo& featureInfo)
    {
    m_writer->WriteElementStart ("DataPoints");

    WString featureStyle = WriteFeatureNameAndDescription (featureInfo);

    m_writer->WriteElementStart ("PntList3D");

    bvector<DPoint3d> points;
    featureInfo.GetFeaturePoints (points);
    WritePoints (points.data (), points.size ());
    m_writer->WriteElementEnd ();

    if (m_addExtraInformation)
            WriteFeatureStuff (L"GroupPoints", featureInfo, featureStyle);
    m_writer->WriteElementEnd ();

    return true;
    }

void LandXMLExporter::WriteDataPoints (BcDTMR dtm)
    {
    CallbackData callbackData (this, std::mem_fn (&LandXMLExporter::WriteRandomPoints));
    dtm.BrowseFeatures (DTMFeatureType::RandomSpots, 1000, &callbackData, &CallbackData::Callback);
    DTMFeatureEnumerator featureEnumerator (dtm);

    featureEnumerator.ExcludeAllFeatures ();
    featureEnumerator.IncludeFeature (DTMFeatureType::GroupSpots);

    featureEnumerator.SetSort (false);

    for (const DTMFeatureInfo feature : featureEnumerator)
        WriteGroupPoints (feature);
    }

bool LandXMLExporter::WritePnts (const DTMLandXMLFeatureInfo& featureInfo, DPoint3dP tPoint, int numPts)
    {
    for (int i = 0; i < numPts; i++)
        {
        m_writer->WriteElementStart ("P");
        m_pntNum++;
        m_writer->WriteAttribute ("id", FormatInt (m_pntNum).GetWCharCP());
        WritePoint (tPoint[i]);
        m_writer->WriteElementEnd ();
        }
    return true;
    }

bool LandXMLExporter::WriteFaces (const DTMLandXMLFeatureInfo& featureInfo, DPoint3dP tPoint, int numPts)
    {

    for (int i = 0; i < numPts; i++)
        {
        m_writer->WriteElementStart ("F");
        int adjTri1 = (int)tPoint[i].x + 1;    // (int) - VS2012
        int adjTri2 = (int)tPoint[i].y + 1;
        int adjTri3 = (int)tPoint[i].z + 1;
        WString text = FormatInt (adjTri1) + L" " + FormatInt (adjTri2) + L" " + FormatInt (adjTri3);
        m_writer->WriteText (text.GetWCharCP ());
        m_writer->WriteElementEnd ();
        }
    return true;
    }

void LandXMLExporter::WriteDefinition (BcDTMR dtm)
    {
    if (dtm.GetDTMState () == DTMState::Tin)
        {
        m_writer->WriteElementStart ("Definition");
        m_writer->WriteAttribute ("surfType", L"TIN");
        m_writer->WriteElementStart ("Pnts");
        m_pntNum = 0;

        CallbackData callbackData (this, std::mem_fn (&LandXMLExporter::WritePnts));
        dtm.BrowseFeatures (DTMFeatureType::TinPoint, 1000, &callbackData, &CallbackData::Callback);
        // ToDo m_currentDtm.DTM.BrowseDynamicFeatures (new DynamicFeaturesBrowsingCriteria (), DTMDynamicFeatureType.TriangleVertex, WritePnts, this);

        m_writer->WriteElementEnd ();

        m_writer->WriteElementStart ("Faces");
        CallbackData callbackData2 (this, std::mem_fn (&LandXMLExporter::WriteFaces));
        dtm.BrowseFeatures (DTMFeatureType::TriangleIndex, 1000, &callbackData2, &CallbackData::Callback);
        // ToDo m_currentDtm.DTM.BrowseDynamicFeatures (new DynamicFeaturesBrowsingCriteria (), (DTMDynamicFeatureType)3, WriteFaces, this);
        m_writer->WriteElementEnd ();
        m_writer->WriteElementEnd ();
        }
    }

WString LandXMLExporter::WriteFeatureNameAndDescription (const DTMFeatureInfo& featureInfo)
    {
    WString name;
    WString desc;
    WString featureStyle;
    DTMFeatureType featureType;
    DTMFeatureId featureId;
    DTMUserTag userTag;
    featureInfo.GetFeatureInfo (featureType, featureId, userTag);

    if (nullptr != m_featureInfoCallback &&
        m_featureInfoCallback->GetFeatureInfo (name, desc, featureStyle, featureType, featureId, userTag))
        {
        if (!name.empty ())
            m_writer->WriteAttribute ("name", name.GetWCharCP());
        if (!desc.empty())
            m_writer->WriteAttribute ("desc", desc.GetWCharCP());
        }
    return featureStyle;
    }

void LandXMLExporter::WriteFeatureStuff (WStringCR nodeType, const DTMFeatureInfo& featureInfo, WStringCR featureStyle)
    {
    if (m_addExtraInformation)
        {
        m_writer->WriteElementStart ("Feature");
        m_writer->WriteAttribute ("code", nodeType.GetWCharCP ());
        DTMFeatureType featureType;
        DTMFeatureId featureId;
        DTMUserTag userTag;
        featureInfo.GetFeatureInfo (featureType, featureId, userTag);

        if (featureId != DTM_NULL_FEATURE_ID)
            {
            m_writer->WriteElementStart ("Property");
            m_writer->WriteAttribute ("label", L"id");
            m_writer->WriteAttribute ("value", FormatInt (featureId).GetWCharCP ());
            m_writer->WriteElementEnd ();
            }
        if (!featureStyle.empty ())
            {
            m_writer->WriteElementStart ("Property");
            m_writer->WriteAttribute ("label", L"style");
            m_writer->WriteAttribute ("value", featureStyle.GetWCharCP ());
            m_writer->WriteElementEnd ();
            }
        if (userTag != DTM_NULL_USER_TAG)
            {
            m_writer->WriteElementStart ("Property");
            m_writer->WriteAttribute ("label", L"UsrID");
            m_writer->WriteAttribute ("value", FormatInt (userTag).GetWCharCP ());
            m_writer->WriteElementEnd ();
            }
        m_writer->WriteElementEnd ();
        }
    }

WString LandXMLExporter::WriteFeatureNameAndDescription (const DTMLandXMLFeatureInfo& featureInfo)
    {
    WString name;
    WString desc;
    WString featureStyle;

    if (nullptr != m_featureInfoCallback &&
        m_featureInfoCallback->GetFeatureInfo (name, desc, featureStyle, featureInfo.DtmFeatureType, featureInfo.DtmFeatureId, featureInfo.DtmUserTag))
        {
        if (!name.empty ())
            m_writer->WriteAttribute ("name", name.GetWCharCP ());
        if (!desc.empty ())
            m_writer->WriteAttribute ("desc", desc.GetWCharCP ());
        }
    return featureStyle;
    }

void LandXMLExporter::WriteFeatureStuff (WStringCR nodeType, const DTMLandXMLFeatureInfo& featureInfo, WStringCR featureStyle)
    {
    if (m_addExtraInformation)
        {
        m_writer->WriteElementStart ("Feature");
        m_writer->WriteAttribute ("code", nodeType.GetWCharCP ());
        if (featureInfo.DtmFeatureId != DTM_NULL_FEATURE_ID)
            {
            m_writer->WriteElementStart ("Property");
            m_writer->WriteAttribute ("label", L"id");
            m_writer->WriteAttribute ("value", FormatInt (featureInfo.DtmFeatureId).GetWCharCP ());
            m_writer->WriteElementEnd ();
            }
        if (!featureStyle.empty ())
            {
            m_writer->WriteElementStart ("Property");
            m_writer->WriteAttribute ("label", L"style");
            m_writer->WriteAttribute ("value", featureStyle.GetWCharCP ());
            m_writer->WriteElementEnd ();
            }
        if (featureInfo.DtmUserTag != DTM_NULL_USER_TAG)
            {
            m_writer->WriteElementStart ("Property");
            m_writer->WriteAttribute ("label", L"UsrID");
            m_writer->WriteAttribute ("value", FormatInt (featureInfo.DtmUserTag).GetWCharCP ());
            m_writer->WriteElementEnd ();
            }
        m_writer->WriteElementEnd ();
        }
    }

WString LandXMLExporter::FormatDouble (double const& val)
    {
    WString sVal;
    sVal.Sprintf (L"%f", val);
    return sVal;
    }

WString LandXMLExporter::FormatInt (int val)
    {
    WString sVal;
    sVal.Sprintf (L"%d", val);
    return sVal;
    }

WString LandXMLExporter::FormatInt (Int64 const& val)
    {
    WString sVal;
    sVal.Sprintf (L"%d", val);
    return sVal;
    }

void LandXMLExporter::WritePoint (DPoint3dCR pt, bool is3d)
    {
    WString text;

    if (is3d)
        text = FormatDouble (pt.y) + L" " + FormatDouble (pt.x) + L" " + FormatDouble (pt.z);
    else
        text = FormatDouble (pt.y) + L" " + FormatDouble (pt.x);
    m_writer->WriteText (text.GetWCharCP ());
    }

void LandXMLExporter::WritePoints (DPoint3dCP tPoint, size_t numPts, bool is3d)
    {
    for (size_t i = 0; i < numPts; i++)
        {
        if (i != 0)
            m_writer->WriteText (L" ");
        WritePoint (tPoint[i], is3d);
        }
    }

END_BENTLEY_TERRAINMODEL_NAMESPACE