/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/LandXMLImporter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/WString.h>
#include <BeXml/BeXml.h>
#include <list>
#include "TerrainModel/Formats/LandXMLImporter.h"
#include <TerrainModel/Core/bcdtmInlines.h> 
#include "TriangulationPreserver.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

Utf8CP cApplication("Application");
Utf8CP cName ("name");
Utf8CP cDesc ("desc");
Utf8CP cDTMAttribute ("DTMAttribute");
Utf8CP cElev("elev");
Utf8CP cLabel("label");
Utf8CP cValue ("value");
Utf8CP cBndType("bndType");
Utf8CP cPointGeometry ("pointGeometry");
Utf8CP cCode ("code");
Utf8CP cId ("id");
Utf8CP cPntRef ("pntRef");
Utf8CP cSurfType("surfType");
Utf8CP cLinearUnit("linearUnit");
Utf8CP cSurface("Surface");
Utf8CP cPntList2D ("PntList2D");
Utf8CP cPntList3D ("PntList3D"); 
Utf8CP cContour ("Contour");
Utf8CP cContours ("Contours");
Utf8CP cBreakline ("Breakline");
Utf8CP cBreaklines ("Breaklines");
Utf8CP cProperty ("Property");
Utf8CP cDataPoints ("DataPoints");
Utf8CP cFeature ("Feature");
Utf8CP cSourceData ("SourceData");
Utf8CP cBoundary ("Boundary");
Utf8CP cBoundaries ("Boundaries");
Utf8CP cDefinition ( "Definition");
Utf8CP cP ("P");
Utf8CP cPnts ("Pnts");
Utf8CP cFaces ("Faces");
Utf8CP cF ("F");
Utf8CP cUnits ("Units");
Utf8CP cCoordinateSystem ("CoordinateSystem");
Utf8CP cImperial ("Imperial");
Utf8CP cMetric ("Metric");
Utf8CP cCgPoints ("CgPoints");
Utf8CP cCgPoint ("CgPoint");

WCharCP wDonotinclude(L"donotinclude");
WCharCP wOuter (L"outer");
WCharCP wVoid (L"void");
WCharCP wIsland (L"island");
WCharCP wStyle (L"style");
WCharCP wId (L"id");
WCharCP wUsrID (L"usrID");
WCharCP wTriangluate (L"triangulate");
WCharCP wRandomPoints (L"randomPoints");
WCharCP wTIN (L"TIN");

WCharCP wMillimeter (L"millimeter");
WCharCP wCentimeter (L"centimeter");
WCharCP wMeter (L"meter");
WCharCP wKilometer (L"kilometer");
WCharCP wFoot (L"foot");
WCharCP wUSSurveyFoot (L"USSurveyFoot");
WCharCP wInch (L"inch");
WCharCP wMile (L"mile");

class XmlAttributes
    {
    bmap<Utf8String, WString> m_attributes;
    WString m_empty;
    public:
        XmlAttributes (BeXmlReaderPtr& reader)
            {
            Utf8String name;
            WString value;
            while (reader->ReadToNextAttribute (&name, &value) == BEXML_Success)
                {
                m_attributes[name] = value;
                }
            }
        WStringCR GetAttribute (Utf8String name)
            {
            bmap<Utf8String, WString>::const_iterator attribute = m_attributes.find (name);
            
            if (attribute != m_attributes.end())
                return attribute->second;

            return m_empty;
            }
    };

LandXMLImporter::LandXMLImporter (WCharCP filename)
        {
        m_secondPass = false;  // Second pass for pntRef resolution.
        m_importAllDTMS = false;
        m_2delevation = -999;
        m_initialized = false;
        m_filename = filename;
        m_pntRefs = NULL;
        m_callback = nullptr;
        m_isMXProject = false;

        m_currentPointList = 0;
        }

bool LandXMLImporter::IsFileSupported (WCharCP filename)
    {
    return BeFileName::GetExtension (filename).CompareToI (L"xml") == 0; // ToDo: Check contents
    }

LandXMLImporterPtr LandXMLImporter::Create (WCharCP filename)
    {
    if (LandXMLImporter::IsFileSupported (filename))
        return new LandXMLImporter (filename);
    return nullptr;
    }

const TerrainInfoList& LandXMLImporter::_GetTerrains () const
        {
        const_cast<LandXMLImporterP>(this)->Initalize ();
        return m_surfaces;
        }

WCharCP LandXMLImporter::_GetFileUnitString () const
        {
        const_cast<LandXMLImporterP>(this)->Initalize ();
        return m_linearUnit.GetWCharCP();
        }

FileUnit LandXMLImporter::_GetFileUnit () const
    {
    const_cast<LandXMLImporterP>(this)->Initalize ();
    if (m_isMetric)
        {
        if (m_linearUnit.EqualsI (wMillimeter)) return FileUnit::Millimeter;
        if (m_linearUnit.EqualsI (wCentimeter)) return FileUnit::Centimeter;
        if (m_linearUnit.EqualsI (wMeter))      return FileUnit::Meter;
        if (m_linearUnit.EqualsI (wKilometer))  return FileUnit::Kilometer;
        }
    if (m_linearUnit.EqualsI (wFoot))         return FileUnit::Foot;
    if (m_linearUnit.EqualsI (wUSSurveyFoot)) return FileUnit::USSurveyFoot;
    if (m_linearUnit.EqualsI (wInch))         return FileUnit::Inch;
    if (m_linearUnit.EqualsI (wMile))         return FileUnit::Mile;
    return FileUnit::Unknown;
    }


Int64 ConvertToInt64 (WCharCP s)
    {
#if defined (_WIN32)
    return _wtoi64 (s);
#elif defined (__unix__)
    Int64 i = 0;
    Swscanf (s, L"%I64d", &i);
    return i;
#endif
    }

void LandXMLImporter::AddFeatureToDTM (WStringCR name, WStringCR description, DTMFeatureType type, const FeatureAttributes& featureAttribute, WStringCR DTMAttribute)
    {
    if (m_segments.empty ())
        return;

    m_currentPointList = 0;
    for (PointArrayList::iterator it = m_segments.begin (); it != m_segments.end (); it++)
        {
        PointArray& points = *it;

        if (m_isMXProject)
            {
            if (type == DTMFeatureType::RandomSpots || type == DTMFeatureType::GroupSpots)
                {
                size_t cur = 0;
                for (size_t i = 0; i < points.size (); i++)
                if (points[i].z >= -998)
                    points[cur++] = points[i];
                points.resize (cur);
                }
            }

        if (DTMAttribute == wDonotinclude || !featureAttribute.triangulate)
            {
            if (m_callback)
                m_callback->AddFeature (DTM_NULL_FEATURE_ID, DTMAttribute.GetWCharCP (), featureAttribute.featureDefinition.GetWCharCP (), name.GetWCharCP (), description.GetWCharCP (), type, &points[0], points.size ());
            continue;
            }

        DTMUserTag dtmUserTag = DTM_NULL_USER_TAG;

        if (!WString::IsNullOrEmpty (featureAttribute.userTag.GetWCharCP ()))
            dtmUserTag = ConvertToInt64 (featureAttribute.userTag.GetWCharCP ());

        if (!WString::IsNullOrEmpty (featureAttribute.featureId.GetWCharCP ()))
            {
            DTMFeatureId id = ConvertToInt64 (featureAttribute.featureId.GetWCharCP ());
            bcdtmObject_storeDtmFeatureInDtmObject (m_dtm->GetTinHandle (), type, dtmUserTag, 2, &id, &points[0], (int)points.size ());
            if (m_callback)
                m_callback->AddFeature (id, DTMAttribute.GetWCharCP (), featureAttribute.featureDefinition.GetWCharCP (), name.GetWCharCP (), description.GetWCharCP (), type, &points[0], points.size ());
            }
        else
            {
            // Add to DTM.
            DTMFeatureId id;
            if (type == DTMFeatureType::RandomSpots)
                m_dtm->AddPoints (&points[0], (int)points.size ());
            else
                {
                if (type == DTMFeatureType::GroupSpots)
                    m_dtm->AddPointFeature (&points[0], (int)points.size (), dtmUserTag, &id);
                else
                    m_dtm->AddLinearFeature (type, &points[0], (int)points.size (), dtmUserTag, &id);
                if (m_callback)
                    m_callback->AddFeature (id, DTMAttribute.GetWCharCP (), featureAttribute.featureDefinition.GetWCharCP (), name.GetWCharCP (), description.GetWCharCP (), type, &points[0], points.size ());
                }
            }
        }
        }

    DTMFeatureType LandXMLImporter::ConvertBoundaryFeatureType (WStringCR type)
        {
        if (type == wOuter)
            return DTMFeatureType::Hull;
        if (type == wVoid)
            return DTMFeatureType::Void;
        if (type == wIsland)
            return DTMFeatureType::Island;
        return DTMFeatureType::Void;
        }

    static int ConvertToInt (WCharCP start, size_t off, size_t len)
                    {
                    return BeStringUtilities::Wtoi (&start[off]);
                    }
    static int ConvertToInt (WStringCR string)
                    {
                    return BeStringUtilities::Wtoi (string.GetWCharCP());
                    }
    static double ConvertToDouble (WCharCP start, size_t off, size_t len)
                    {
                    return BeStringUtilities::Wtof (&start[off]);
                    }
    static double ConvertToDouble (WStringCR string)
                    {
                    return BeStringUtilities::Wtof (string.GetWCharCP());
                    }
    void LandXMLImporter::ReadFaceIndexs (WStringCR value, int& p1, int& p2, int& p3)
        {
        WCharCP chars = value.GetWCharCP();
        size_t charsLength = value.length ();
        size_t charPos = 0;
        size_t startChar = 0;

        p1 = p2 = p3 = 0;
        while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
            charPos++;

        if (charPos >= charsLength)
            return; // Error

        startChar = charPos;
        while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
            charPos++;

        p1 = ConvertToInt(chars, startChar, charPos - startChar);

        while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
            charPos++;

        if (charPos >= charsLength)
            return; // Error

        startChar = charPos;
        while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
            charPos++;
        p2 = ConvertToInt(chars, startChar, charPos - startChar);

        while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
            charPos++;

        if (charPos >= charsLength)
            return; // Error

        startChar = charPos;
        while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
            charPos++;
        p3 = ConvertToInt (chars, startChar, charPos - startChar);
        }

    DPoint3d LandXMLImporter::ReadPoint (WStringCR value, bool is3D)
        {
        WCharCP chars = value.GetWCharCP();
        size_t charsLength = value.length ();
        size_t charPos = 0;
        size_t startChar = 0;
        double x;
        double y;
        double z = 0;

        while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
            charPos++;

        if (charPos >= charsLength)
            return DPoint3d (); // Error

        startChar = charPos;
        while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
            charPos++;

        y = ConvertToDouble (chars, startChar, charPos - startChar);

        while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
            charPos++;

        if (charPos >= charsLength)
            return DPoint3d (); // Error

        startChar = charPos;
        while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
            charPos++;
        x = ConvertToDouble (chars, startChar, charPos - startChar);

        if (is3D)
            {
            while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
                charPos++;

            if (charPos >= charsLength)
                return DPoint3d (); // Error

            startChar = charPos;
            while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
                charPos++;
            z = ConvertToDouble (chars, startChar, charPos - startChar);
            }
        return DPoint3d::From (x, y, z);
        }

    void LandXMLImporter::ReadPoints (WStringCR value, bool is3D)
        {
        bool isFirstPoint = true;
        WCharCP chars = value.GetWCharCP();
        size_t charsLength = value.length ();
        size_t charPos = 0;
        size_t startChar = 0;
        double x;
        double y;
        double z = 0;

        while (charPos < charsLength)
            {
            while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
                charPos++;

            if (charPos >= charsLength)
                break;

            startChar = charPos;
            while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
                charPos++;

            y = ConvertToDouble (chars, startChar, charPos - startChar);

            while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
                charPos++;

            if (charPos >= charsLength)
                break;// Error??

            startChar = charPos;
            while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
                charPos++;
            x = ConvertToDouble (chars, startChar, charPos - startChar);

            if (is3D)
                {
                while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
                    charPos++;

                if (charPos >= charsLength)
                    break;// Error??

                startChar = charPos;
                while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
                    charPos++;
                z = ConvertToDouble (chars, startChar, charPos - startChar);
                }
            else
                z = m_2delevation;

            if (isFirstPoint)
                {
                isFirstPoint = false;
                if (m_segments.empty ())
                    {
                    PointArray newPointArray;

                    newPointArray.push_back (DPoint3d::From (x, y, z));
                    m_segments.push_back (newPointArray);
                    }
                else
                    {
                    PointArray& pointArray = m_segments[m_currentPointList];
                    PointArray::const_iterator lastPt = pointArray.end ();

                    if (pointArray.size () != 0 && (lastPt->x != x || lastPt->y != y || lastPt->z != z))
                        {
                        m_currentPointList++;
                        PointArray newPointArray;

                        newPointArray.push_back (DPoint3d::From (x, y, z));
                        m_segments.push_back (newPointArray);
                        }
                    else
                        m_segments[m_currentPointList].push_back (DPoint3d::From (x, y, z));
                    }
                }
            else
                m_segments[m_currentPointList].push_back (DPoint3d::From (x, y, z));
            }
        }

    void LandXMLImporter::ReadPntList2D ()
        {
        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_EndElement:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cPntList2D)
                        return;
                    }
                    break;
                case BeXmlReader::NODE_TYPE_Text:
                    {
                    WString value;
                    m_reader->GetCurrentNodeValue (value);

                    ReadPoints (value, false);
                    }
                    break;
                }
            }
        }
    void LandXMLImporter::ReadPntList3D ()
        {
        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_EndElement:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cPntList3D)
                        return;
                    }
                    break;
                case BeXmlReader::NODE_TYPE_Text:
                    {
                    WString value;
                    m_reader->GetCurrentNodeValue (value);
                    ReadPoints (value, true);
                    }
                    break;
                }
            }
        }

void LandXMLImporter::ReadFeatureAttributes (FeatureAttributes& featureAttribute)
    {
    if (m_reader->IsCurrentElementEmpty ())
        return;
    while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
        {
        switch (m_reader->GetCurrentNodeType())
            {
            case BeXmlReader::NODE_TYPE_EndElement:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName (nodeName);
                if (nodeName == cFeature)
                    return;
                }
                break;
            case BeXmlReader::NODE_TYPE_Element:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName (nodeName);
                if (nodeName == cProperty)
                    {
                    XmlAttributes attrs (m_reader);
                    WString propertyName = attrs.GetAttribute (cLabel);
                    if (propertyName == wStyle)
                        featureAttribute.featureDefinition = attrs.GetAttribute (cValue);
                    else if (propertyName == wId)
                        featureAttribute.featureId = attrs.GetAttribute (cValue);
                    else if (propertyName == wUsrID)
                        featureAttribute.userTag = attrs.GetAttribute (cValue);
                    else if (propertyName == wTriangluate)
                        featureAttribute.triangulate = attrs.GetAttribute (cValue) == L"true";
                    }
                }
                break;
            }
        }
    }

void LandXMLImporter::ReadContour ()
        {
        XmlAttributes attrs (m_reader);
        //ConvertFeatureType (attrs.GetAttribute (cBrkType));
        WString name = attrs.GetAttribute (cName);
        WString DTMAttribute = attrs.GetAttribute (cDTMAttribute);
        FeatureAttributes featureAttributes;

        m_2delevation = ConvertToDouble (attrs.GetAttribute (cElev));
        ClearPoints ();
        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_EndElement:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cContour)
                        {
                        AddFeatureToDTM (name, name, DTMFeatureType::ContourLine, featureAttributes, DTMAttribute);
                        return;
                        }
                    }
                    break;
                case BeXmlReader::NODE_TYPE_Element:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cPntList3D)
                        ReadPntList3D ();
                    else if (nodeName == cPntList2D)
                        ReadPntList2D ();
                    else if (nodeName == cFeature)
                        ReadFeatureAttributes (featureAttributes);
                    }
                    break;
                }
            }
        }
    void LandXMLImporter::ReadContours ()
        {
        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_EndElement:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);

                    if (nodeName == cContours)
                        {
                        return;
                        }
                    }
                    break;
                case BeXmlReader::NODE_TYPE_Element:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cContour)
                        ReadContour ();
                    }
                    break;
                }
            }
        }
    void LandXMLImporter::ReadBreakLine ()
        {
        XmlAttributes attrs (m_reader);
        //ConvertFeatureType (attrs.GetAttribute (cBrkType));
        WString name = attrs.GetAttribute (cName);
        WString DTMAttribute = attrs.GetAttribute (cDTMAttribute);
        FeatureAttributes featureAttributes;

        ClearPoints ();
        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_EndElement:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cBreakline)
                        {
                        AddFeatureToDTM (name, name, DTMFeatureType::Breakline, featureAttributes, DTMAttribute);
                        return;
                        }
                    }
                    break;
                case BeXmlReader::NODE_TYPE_Element:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);

                    if (nodeName == cPntList3D)
                        ReadPntList3D ();
                    else if (nodeName == cPntList3D)
                        ReadPntList3D ();
                    else if (nodeName == cFeature)
                        ReadFeatureAttributes (featureAttributes);
                    }
                    break;
                }
            }
        }
    void LandXMLImporter::ReadBreakLines ()
        {
        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_EndElement:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);

                    if (nodeName == cBreaklines)
                        {
                        return;
                        }
                    }
                    break;
                case BeXmlReader::NODE_TYPE_Element:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cBreakline)
                        ReadBreakLine ();
                    }
                    break;
                }
            }
        }

    void LandXMLImporter::ReadBoundary ()
        {
        XmlAttributes attrs (m_reader);
        DTMFeatureType bndType = ConvertBoundaryFeatureType (attrs.GetAttribute (cBndType));
        WString name = attrs.GetAttribute (cName);
        WString DTMAttribute = attrs.GetAttribute (cDTMAttribute);
        FeatureAttributes featureAttributes;

        ClearPoints ();
        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_EndElement:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cBoundary)
                        {
                        AddFeatureToDTM (name, name, bndType, featureAttributes, DTMAttribute);
                        return;
                        }
                    }
                    break;
                case BeXmlReader::NODE_TYPE_Element:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cPntList3D)
                        ReadPntList3D ();
                    else if (nodeName == cPntList2D)
                        ReadPntList2D ();
                    else if (nodeName == cFeature)
                        ReadFeatureAttributes (featureAttributes);
                    }
                    break;
                }
            }
        }
    void LandXMLImporter::ReadBoundaries ()
        {
        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_EndElement:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cBoundaries)
                        {
                        return;
                        }
                    }
                    break;
                case BeXmlReader::NODE_TYPE_Element:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cBoundary)
                        ReadBoundary ();
                    }
                    break;
                }
            }
        }

    void LandXMLImporter::ReadDataPoints ()
        {
        XmlAttributes attrs (m_reader);
        WString type = attrs.GetAttribute (cPointGeometry);
        WString name = attrs.GetAttribute (cName);
        WString DTMAttribute = attrs.GetAttribute (cDTMAttribute);
        FeatureAttributes featureAttributes;

        bool randomPoints = false;
        ClearPoints ();
        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_EndElement:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cDataPoints)
                        {
                        AddFeatureToDTM (name, name, randomPoints ? DTMFeatureType::RandomSpots: DTMFeatureType::GroupSpots, featureAttributes, DTMAttribute);
                        return;
                        }
                    }
                    break;
                case BeXmlReader::NODE_TYPE_Element:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cPntList3D)
                        ReadPntList3D ();
                    else if (nodeName == cFeature)
                        {
                        XmlAttributes attrs (m_reader);
                        if (attrs.GetAttribute (cCode) == wRandomPoints)
                            randomPoints = true;
                        ReadFeatureAttributes (featureAttributes);
                        }
                    }
                    break;
                }
            }
        }


    void LandXMLImporter::ReadSourceData ()
        {
        if ((m_options & LandXMLOptions::Source) != LandXMLOptions::Source)
            {
            m_reader->ReadToEndOfElement ();
            return;
            }

        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_EndElement:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cSourceData)
                        return;
                    }
                    break;
                case BeXmlReader::NODE_TYPE_Element:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cBreaklines)
                        ReadBreakLines ();
                    else if (nodeName == cContours)
                        ReadContours ();
                    else if (nodeName == cDataPoints)
                        ReadDataPoints ();
                    else if (nodeName == cBoundaries)
                        ReadBoundaries ();
                    break;
                    }
                }
            }
        }

    void LandXMLImporter::ReadDefinition ()
        {
        TriangulationPreserver adjust (*m_dtm);

        if ((m_options & LandXMLOptions::Definition) != LandXMLOptions::Definition)
            {
            m_reader->ReadToEndOfElement ();
            return;
            }

        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_EndElement:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cDefinition)
                        {
                        adjust.Finish ();
                        return;
                        }
                    }
                    break;
                case BeXmlReader::NODE_TYPE_Element:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cP)
                        {
                        WString pntRef;
                        int ptNum = -1;
                        Utf8String name;
                        WString value;

                        while (m_reader->ReadToNextAttribute (&name, &value) == BEXML_Success)
                            {
                            if (name == cPntRef)
                                pntRef = value;
                            else if (name == cId)
                                ptNum = ConvertToInt (value);
                            }

                        //XmlAttributes attrs (m_reader);
                        //WString pntRef = attrs.GetAttribute (cPntRef);
                        if (m_secondPass || pntRef.length() == 0)
                            {
                            if (pntRef.length() == 0)
                                {
                                m_reader->Read ();  // ToDo needs work.
                                
                                m_reader->GetCurrentNodeValue (value);
                                adjust.AddPoint (ReadPoint (value, true), ptNum);
                                }
                            else
                                {
                                adjust.AddPoint ((*m_pntRefs)[pntRef], ptNum);
                                }
                            }
                        else
                            {
                            if (m_pntRefs == NULL)
                                m_pntRefs = new bmap<WString, DPoint3d> ();
                            (*m_pntRefs)[pntRef] = DPoint3d ();
                            }
                        }
                    else if (nodeName == cFaces && m_pntRefs != NULL && !m_secondPass)
                        m_reader->ReadToEndOfElement  ();
                    else if (nodeName == cF)
                        {
                        if (m_pntRefs == NULL || m_secondPass)
                            {
                            m_reader->Read ();  // ToDo needs work. Are these always 3?
                            WString value;
                            m_reader->GetCurrentNodeValue (value);

                            int ptNums[3];
                            ReadFaceIndexs (value, ptNums[0], ptNums[1], ptNums[2]);

                            adjust.AddTriangle (ptNums, 3);
                            }
                        }
                    }
                    break;
                }
            }
        }

    void LandXMLImporter::ReadSurface ()
        {
        WString currentName;
        WString currentDesc;
        bool hasDefinition = false;

        XmlAttributes attrs (m_reader);
        currentName = attrs.GetAttribute (cName);
        currentDesc = attrs.GetAttribute (cDesc);

        if (!m_importAllDTMS && m_namedDtms.size() && m_namedDtms.find (currentName) == m_namedDtms.end())
            {
            m_reader->ReadToEndOfElement  ();
            return;
            }


        if (m_importAllDTMS || m_namedDtms.size())
            {
            if (!m_secondPass)
                {
                m_dtm = nullptr;
                if (m_callback)
                    if (!m_callback->StartTerrain (currentName.GetWCharCP(), L"", m_dtm))
                        {
                        m_reader->ReadToEndOfElement  ();
                        return;
                        }
                if (m_dtm.IsNull())
                    m_dtm = BcDTM::Create ();
                m_namedDtms[currentName] = m_dtm;
                }
            else
                m_dtm = m_namedDtms[currentName];
            }

        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_Element:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cSourceData)
                        {
                        if (!m_secondPass && m_dtm.IsValid ())
                            ReadSourceData ();
                        else
                            m_reader->ReadToEndOfElement  ();
                        }
                    else if (nodeName == cDefinition)
                        {
                        XmlAttributes attrs (m_reader);
                        WString surfType = attrs.GetAttribute (cSurfType);
                        if (surfType == wTIN)
                            hasDefinition = true;

                        if (m_dtm.IsValid())
                            ReadDefinition ();
                        else
                            m_reader->ReadToEndOfElement  ();
                        }
                    }
                    break;
                case BeXmlReader::NODE_TYPE_EndElement:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cSurface)
                        {
                        m_surfaces.push_back (TerrainInfo (currentName.GetWCharCP (), currentDesc.GetWCharCP (), hasDefinition));
                        m_dtm = NULL;
                        return;
                        }
                    }
                }
            }

        if (m_callback)
            {
            if (!m_callback->EndTerrain (currentName.GetWCharCP(), m_dtm.get()))
                m_namedDtms[currentName] = nullptr;
            }
        }

    void LandXMLImporter::ReadCoordinateSystem ()
        {
        Utf8CP cDesc ("desc");
        Utf8CP cName ("name");
        Utf8CP cEpsgCode ("epsgCode");
        Utf8CP cOgcWktCode ("ogcWktCode");
//horizontalDatum
//verticalDatum
//ellipsoidName
//horizontalCoordinateSystemName
//geocentricCoordinateSystemName
//fileLocation
//rotationAngle
//datum
//fittedCoordinateSystemName
//compoundCoordinateSystemName
//localCoordinateSystemName
//geographicCoordinateSystemName
//projectedCoordinateSystemName
//verticalCoordinateSystemName

        XmlAttributes attrs (m_reader);
        WString name = attrs.GetAttribute (cName);
        WString desc = attrs.GetAttribute (cDesc);
        WString epsgCode = attrs.GetAttribute (cEpsgCode);
        WString ogcWktCode = attrs.GetAttribute (cOgcWktCode);

        m_reader->ReadToEndOfElement ();

        m_gcs = Bentley::GeoCoordinates::BaseGCS::CreateGCS ();

        m_gcs->SetName (name.GetWCharCP ());
        m_gcs->SetDescription (desc.GetWCharCP ());

        StatusInt ret = ERROR;
        if (!epsgCode.empty ())
            ret = m_gcs->InitFromEPSGCode (nullptr, nullptr, ConvertToInt (epsgCode));

        if (ret != SUCCESS && !ogcWktCode.empty())// Try a different method
            ret = m_gcs->InitFromWellKnownText (nullptr, nullptr, Bentley::GeoCoordinates::BaseGCS::WktFlavor::wktFlavorUnknown, ogcWktCode.GetWCharCP());

        if (ret != SUCCESS)
            m_gcs = nullptr;
        }
    void LandXMLImporter::ReadUnits ()
        {
        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
            case BeXmlReader::NODE_TYPE_EndElement:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName (nodeName);

                if (nodeName == cUnits)
                    return;
                }
                break;
            case BeXmlReader::NODE_TYPE_Element:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName (nodeName);
                if (nodeName == cImperial)
                    {
                    XmlAttributes attrs (m_reader);
                    m_isMetric = false;
                    m_linearUnit  = attrs.GetAttribute (cLinearUnit);
                    }
                else if (nodeName == cMetric)
                    {
                    XmlAttributes attrs (m_reader);
                    m_isMetric = true;
                    m_linearUnit = attrs.GetAttribute (cLinearUnit);
                    }
                }
                break;
                }
            }
        }

    void LandXMLImporter::ReadApplication ()
        {
        WString applicationName;
        XmlAttributes attrs (m_reader);
        applicationName = attrs.GetAttribute (cName);
        if (applicationName == L"MX")
            m_isMXProject = true;
        }

    ImportedTerrain LandXMLImporter::_ImportTerrain (WCharCP name) const
        {
        m_namedDtms.clear ();
        m_namedDtms[name] = nullptr;
        const_cast<LandXMLImporterP>(this)->ReadXML ();

        return ImportedTerrain (m_namedDtms[name].get(), name, nullptr, false);
        }

    ImportedTerrainList LandXMLImporter::_ImportTerrains () const
        {
        m_namedDtms.clear ();

        m_importAllDTMS = true;
        const_cast<LandXMLImporterP>(this)->ReadXML ();
        m_importAllDTMS = false;
        ImportedTerrainList ret;

        for (bmap <WString, Bentley::TerrainModel::BcDTMPtr>::const_iterator iter = m_namedDtms.begin(); iter != m_namedDtms.end(); iter++)
            ret.push_back (ImportedTerrain (iter->second.get(), iter->first.GetWCharCP(), nullptr, false));
        return ret;
        }

    ImportedTerrainList LandXMLImporter::_ImportTerrains (bvector<WString>& names) const
        {
        m_namedDtms.clear ();
        for (bvector<WString>::const_iterator iter = names.begin(); iter != names.end(); iter++)
            m_namedDtms[*iter] = nullptr;

        const_cast<LandXMLImporterP>(this)->ReadXML ();

        ImportedTerrainList ret;
        for (bvector<WString>::const_iterator iter = names.begin(); iter != names.end(); iter++)
            ret.push_back (ImportedTerrain (m_namedDtms[*iter].get(), iter->GetWCharCP(), nullptr, false));
        return ret;
        }

    void LandXMLImporter::ReadXML ()
        {
        do
            {
            if (m_pntRefs != NULL)
                m_secondPass = true;

            BeXmlStatus status;
            m_reader = BeXmlReader::CreateAndReadFromFile (status, m_filename.GetWCharCP());
            // Parse the file and display each of the nodes.
            while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
                {
                switch (m_reader->GetCurrentNodeType())
                    {
                    case BeXmlReader::NODE_TYPE_Element:
                        {
                        Utf8String nodeName;
                        m_reader->GetCurrentNodeName (nodeName);
                        if (nodeName == cUnits)
                            ReadUnits ();
                        else if (nodeName == cCoordinateSystem)
                            ReadCoordinateSystem ();
                        else if (nodeName == cSurface)
                            ReadSurface ();
                        else if (nodeName == cCgPoint)
                            {
                            if (m_pntRefs == NULL)
                                {
                                m_reader->ReadToEndOfElement  ();
                                }
                            else
                                {
                                XmlAttributes attrs (m_reader);
                                WString pntRef = attrs.GetAttribute (cName);
                                if (pntRef.length() && m_pntRefs->find (pntRef) != m_pntRefs->end())
                                    {
                                    m_reader->Read ();
                                    WString value;
                                    m_reader->GetCurrentNodeValue (value);
                                    (*m_pntRefs)[pntRef] = ReadPoint (value, true);
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            m_reader = NULL;
            } while (m_pntRefs != NULL && !m_secondPass);
        m_importAllDTMS = false;
        if (m_pntRefs != NULL)
            {
            delete m_pntRefs;
            m_pntRefs = NULL;
            }
        m_secondPass = false;
        }

    void LandXMLImporter::Initalize ()
        {
        if (m_initialized)
            {
//                if (m_fileTime == System.IO.File.GetCreationTime (m_filename))
                return;
            }

        m_initialized = true;
        // ToDo Need to check that the time and date are the same.
//            m_fileTime = System.IO.File.GetCreationTime (m_filename);

        m_surfaces.clear();

        // if the file does not exist, assume were using DTM Export
//            if (!System.IO.File.Exists (m_filename))
//                return;

        BeXmlStatus status;
        m_reader = BeXmlReader::CreateAndReadFromFile (status, m_filename.GetWCharCP());

        // Parse the file and display each of the nodes.
        while (m_reader->Read () == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_Element:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName (nodeName);
                    if (nodeName == cApplication)
                        ReadApplication ();
                    else if (nodeName == cSurface)
                        ReadSurface ();
                    else if (nodeName == cUnits)
                        ReadUnits ();
                    else if (nodeName == cCoordinateSystem)
                        ReadCoordinateSystem ();
                    else if (nodeName == cCgPoints)
                        m_reader->ReadToEndOfElement  ();
                    break;
                }
            }
        }
        m_reader = NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Daryl.Holmwood  06/2015
//---------------------------------------------------------------------------------------
LandXMLOptions LandXMLImporter::GetOptions ()
    {
    return m_options;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Daryl.Holmwood  06/2015
//---------------------------------------------------------------------------------------
void LandXMLImporter::SetOptions (LandXMLOptions options)
    {
    m_options = options;
    }
