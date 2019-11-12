/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/WString.h>
#include <BeXml/BeXml.h>
#include <list>
#include "TerrainModel/Formats/TwelvedXMLImporter.h"
#include <TerrainModel/Core/bcdtmInlines.h>
#include "TriangulationPreserver.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

Utf8CP c12dUnits("units");
Utf8CP c12dMetric("metric");
Utf8CP c12dLinear("linear");
Utf8CP c12dApplication("application");
Utf8CP c12dComments("comments");
Utf8CP c12dModel("model");
Utf8CP c12dName("name");
Utf8CP c12dDesc("desc");
Utf8CP c12dValue("value");
Utf8CP c12dTin("tin");
Utf8CP c12dFull_Tin("full_tin");
Utf8CP c12dAttributes("attributes");
WString c12dAttrStyle("Style");
WString c12dAttrWeed("Weed");
WString c12dAttrFaces("Faces");
Utf8CP c12dPoints("points");
Utf8CP c12dTriangles("triangles");
Utf8CP c12dNeighbours("neighbours");
Utf8CP c12dNulling("nulling");
Utf8CP c12dP("p");
Utf8CP c12dT("t");

WCharCP w12dMillimeter(L"millimetre");
WCharCP w12dCentimeter(L"centimetre");
WCharCP w12dMeter(L"metre");
WCharCP w12dKilometer(L"kilometre");
WCharCP w12dFoot(L"foot");
WCharCP w12dUSSurveyFoot(L"USSurveyFoot");
WCharCP w12dInch(L"inch");
WCharCP w12dMile(L"mile");

class XmlAttributes
    {
    bmap<Utf8String, WString> m_attributes;
    WString m_empty;
    public:
        XmlAttributes(BeXmlReaderPtr& reader)
            {
            Utf8String name;
            WString value;
            while (reader->ReadToNextAttribute(&name, &value) == BEXML_Success)
                {
                m_attributes[name] = value;
                }
            }
        WStringCR GetAttribute(Utf8String name)
            {
            bmap<Utf8String, WString>::const_iterator attribute = m_attributes.find(name);

            if (attribute != m_attributes.end())
                return attribute->second;

            return m_empty;
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
TwelvedXMLImporter::TwelvedXMLImporter(WCharCP filename)
    {
    m_secondPass = false;  // Second pass for pntRef resolution.
    m_importAllDTMS = false;
    m_2delevation = -999;
    m_initialized = false;
    m_filename = filename;
    m_pntRefs = NULL;
    m_callback = nullptr;
    m_isMXProject = false;
    m_isGeopakProject = false;
    m_currentPointList = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
bool TwelvedXMLImporter::IsFileSupported(WCharCP filename)
    {
    return BeFileName::GetExtension(filename).CompareToI(L"12dxml") == 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
TwelvedXMLImporterPtr TwelvedXMLImporter::Create(WCharCP filename)
    {
    if (TwelvedXMLImporter::IsFileSupported(filename))
        return new TwelvedXMLImporter(filename);
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
const TerrainInfoList& TwelvedXMLImporter::_GetTerrains() const
    {
    const_cast<TwelvedXMLImporterP>(this)->Initialize();
    return m_surfaces;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
WCharCP TwelvedXMLImporter::_GetFileUnitString() const
    {
    const_cast<TwelvedXMLImporterP>(this)->Initialize();
    return m_linearUnit.GetWCharCP();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
FileUnit TwelvedXMLImporter::_GetFileUnit() const
    {
    const_cast<TwelvedXMLImporterP>(this)->Initialize();
    if (m_isMetric)
        {
        if (m_linearUnit.EqualsI(w12dMillimeter)) return FileUnit::Millimeter;
        if (m_linearUnit.EqualsI(w12dCentimeter)) return FileUnit::Centimeter;
        if (m_linearUnit.EqualsI(w12dMeter))      return FileUnit::Meter;
        if (m_linearUnit.EqualsI(w12dKilometer))  return FileUnit::Kilometer;
        }
    if (m_linearUnit.EqualsI(w12dFoot))         return FileUnit::Foot;
    if (m_linearUnit.EqualsI(w12dUSSurveyFoot)) return FileUnit::USSurveyFoot;
    if (m_linearUnit.EqualsI(w12dInch))         return FileUnit::Inch;
    if (m_linearUnit.EqualsI(w12dMile))         return FileUnit::Mile;
    return FileUnit::Unknown;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
int64_t TwelvedConvertToInt64(WCharCP s)
    {
#if defined (_WIN32)
    return _wtoi64(s);
#elif defined (__unix__)
    int64_t i = 0;
    Swscanf(s, L"%I64d", &i);
    return i;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
static int ConvertToInt(WCharCP start, size_t off, size_t len)
    {
    return BeStringUtilities::Wtoi(&start[off]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
static double ConvertToDouble(WCharCP start, size_t off, size_t len)
    {
    return BeStringUtilities::Wtof(&start[off]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
bool TwelvedXMLImporter::Read12dFaceIndexes(WStringCR value, int& p1, int& p2, int& p3)
    {
    WCharCP chars = value.GetWCharCP();
    size_t charsLength = value.length();
    size_t charPos = 0;
    size_t startChar = 0;

    p1 = p2 = p3 = 0;

    while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
        charPos++;

    if (charPos >= charsLength)
        return true; // Error

    startChar = charPos;
    while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
        charPos++;

    p1 = ConvertToInt(chars, startChar, charPos - startChar);

    while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
        charPos++;

    if (charPos >= charsLength)
        return true; // Error

    startChar = charPos;
    while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
        charPos++;
    p2 = ConvertToInt(chars, startChar, charPos - startChar);

    while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
        charPos++;

    if (charPos >= charsLength)
        return true; // Error

    startChar = charPos;
    while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
        charPos++;
    p3 = ConvertToInt(chars, startChar, charPos - startChar);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
DPoint3d TwelvedXMLImporter::Read12dPoint(WStringCR value, bool is3D)
    {
    WCharCP chars = value.GetWCharCP();
    size_t charsLength = value.length();
    size_t charPos = 0;
    size_t startChar = 0;
    double x;
    double y;
    double z = 0;

    while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
        charPos++;

    if (charPos >= charsLength)
        return DPoint3d(); // Error

    startChar = charPos;
    while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
        charPos++;

    x = ConvertToDouble(chars, startChar, charPos - startChar);

    while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
        charPos++;

    if (charPos >= charsLength)
        return DPoint3d(); // Error

    startChar = charPos;
    while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
        charPos++;
    y = ConvertToDouble(chars, startChar, charPos - startChar);

    if (is3D)
        {
        while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' '))
            charPos++;

        if (charPos >= charsLength)
            return DPoint3d(); // Error

        startChar = charPos;
        while (charPos < charsLength && (chars[charPos] != '\t' && chars[charPos] != '\n' && chars[charPos] != ' '))
            charPos++;
        z = ConvertToDouble(chars, startChar, charPos - startChar);
        }

    return DPoint3d::From(x, y, z);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
void TwelvedXMLImporter::Read12dSurface()
    {
    const int VISIBLE_TRIANGLE = 2;
    WString currentName;
    WString currentDesc = L"";
    bool hasDefinition = true; //We are inside the <full_tin> so surface is defined
    WString styleName = L"";
    int weedValue;
    int facesValue;
    bvector<int> nullings;
    int triangleCount = 0;
    int ptNum = 1;  //Implicitly numbered, starting with 1

    if (m_importAllDTMS || m_namedDtms.size())
        {
        if (!m_secondPass)
            {
            m_dtm = nullptr;
            if (m_callback)
                if (!m_callback->StartTerrain(currentName.GetWCharCP(), L"", m_dtm))
                    return;
            if (m_dtm.IsNull())
                m_dtm = BcDTM::Create();
            m_namedDtms[currentName] = m_dtm;
            }
        else
            m_dtm = m_namedDtms[currentName];
        }

    //Read all the nullings first
    if (m_dtm.IsValid())
        {
        while (m_reader->Read() == BeXmlReader::READ_RESULT_Success)
            {
            Utf8String nodeName("");
            Bentley::IBeXmlReader::NodeType nodetype = m_reader->GetCurrentNodeType();
            if (nodetype == BeXmlReader::NODE_TYPE_Element)
                {
                m_reader->GetCurrentNodeName(nodeName);  //<nulling>
                if (nodeName == c12dNulling)
                    {
                    WString value;
                    m_reader->GetCurrentNodeString(value);
                    WCharCP chars = value.GetWCharCP();
                    size_t charsLength = value.length();
                    size_t charPos = 0;
                    while (charPos < charsLength && (chars[charPos] == '\t' || chars[charPos] == '\n' || chars[charPos] == ' ' || chars[charPos] != '1' || chars[charPos] != '2'))
                        {
                        charPos++;
                        if (chars[charPos] != '1' && chars[charPos] != '2')
                            continue;
                        nullings.push_back(chars[charPos] == '1' ? 1 : 2);
                        }
                    }
                }
            else if (nodetype == BeXmlReader::NODE_TYPE_EndElement)
                {
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == c12dNulling)//</nulling>
                    break;
                }
            }
        m_reader = NULL;
        BeXmlStatus status;
        m_reader = BeXmlReader::CreateAndReadFromFile(status, m_filename.GetWCharCP());
        }

    //Use to save point coordinates and read points
    TriangulationPreserver pointBuffer(*m_dtm);
    bool readingNeighbours = false;

    while (m_reader->Read() == BeXmlReader::READ_RESULT_Success)
        {
        switch (m_reader->GetCurrentNodeType())
            {
            case BeXmlReader::NODE_TYPE_Element:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == c12dName)
                    m_reader->GetCurrentNodeString(currentName);
                else if (nodeName == c12dAttributes)
                    {
                    Read12dAttributes(styleName, weedValue, facesValue);
                    if (!m_importAllDTMS && m_namedDtms.size() && m_namedDtms.find(currentName) == m_namedDtms.end())
                        {
                        if (!m_reader->IsCurrentElementEmpty())
                            m_reader->ReadToEndOfElement();
                        return;
                        }
                    }
                else if (nodeName == c12dPoints)    //points_block - Presence is mandatory according to the document "12d XML File Format" 
                    {
                    if (!m_secondPass && !m_dtm.IsValid())
                        ReadToEndOfElements(c12dPoints);
                    }
                else if (nodeName == c12dP)
                    {
                    if (m_secondPass || m_dtm.IsValid())
                        {
                        //Read points in points_block - points_block is mandatory according to the document "12d XML File Format" 
                        //<p>475285.62385856302 6591456.9685901217 0</p>            Version v12
                        //<p>475285.62385856301989 6591456.96859012171626 0</p>     Version v14a
                        //<p>x_value+space+y_value+space+z_value</p>
                        WString value;
                        m_reader->GetCurrentNodeString(value);
                        pointBuffer.AddPoint(Read12dPoint(value, true), ptNum++);
                        }
                    }
                else if (nodeName == c12dTriangles)     //Read triangles_block - Presence is mandatory according to the document "12d XML File Format" 
                    {
                    readingNeighbours = false;
                    if (!m_dtm.IsValid())
                        ReadToEndOfElements(c12dTriangles);
                    }
                else if (nodeName == c12dT)
                    {
                    if (m_secondPass || m_dtm.IsValid())
                        {
                        //Read triangles in triangles_block - triangles_block is mandatory according to the document "12d XML File Format" 
                        WString value;
                        m_reader->GetCurrentNodeString(value);
                        //c12dT can be triangles block or neighbours block
                        if (!readingNeighbours)
                            {
                            if (nullings[triangleCount] == VISIBLE_TRIANGLE)  //If visible
                                {
                                int ptNums[3];
                                if (Read12dFaceIndexes(value, ptNums[0], ptNums[1], ptNums[2]))
                                    {
                                    DPoint3d* triangPoints;
                                    triangPoints = pointBuffer.GetTrianglePoints(ptNums, 3);
                                    if (triangPoints != NULL)
                                        _AddLinearFeature(triangPoints, DTMFeatureType::GraphicBreak);
                                    }
                                }
                            triangleCount++;
                            }
                        }
                    }
                else if (nodeName == c12dNeighbours)
                    //Neighbours reading
                    ReadToEndOfElements(c12dNeighbours);
                }
                break;

            case BeXmlReader::NODE_TYPE_EndElement:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == c12dFull_Tin || nodeName == c12dTin)  //</full_tin> or </tin>
                    {
                    if (m_dtm.IsValid())
                        {
                        m_namedDtms[currentName] = m_dtm;
                        _BuildDTMFromTriangles();
                        }
                    m_surfaces.push_back(TerrainInfo(currentName.GetWCharCP(), currentDesc.GetWCharCP(), hasDefinition));
                    m_dtm = NULL;
                    return;
                    }
                }
                break;
            }
        }

    if (m_callback)
        {
        if (!m_callback->EndTerrain(currentName.GetWCharCP(), m_dtm.get()))
            m_namedDtms[currentName] = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
void TwelvedXMLImporter::Read12dAttributes(WStringR styleName, int& weedValue, int& facesValue)
    {
    WString attributeName;

    while (m_reader->Read() == BeXmlReader::READ_RESULT_Success)
        {
        switch (m_reader->GetCurrentNodeType())
            {
            case BeXmlReader::NODE_TYPE_Element:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == c12dName)
                    m_reader->GetCurrentNodeString(attributeName);
                else if (nodeName == c12dValue)
                    {
                    WString nodeValue;
                    m_reader->GetCurrentNodeString(nodeValue);
                    if (attributeName == c12dAttrStyle)
                        styleName = nodeValue;
                    else if (attributeName == c12dAttrWeed)
                        weedValue = nodeValue == L"1" ? 1 : 0;
                    else if (attributeName == c12dAttrFaces)
                        facesValue = nodeValue == L"1" ? 1 : 0;
                    attributeName = L"";
                    }
                }
                break;
            case BeXmlReader::NODE_TYPE_EndElement:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == c12dAttributes)
                    return;
                }
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
void TwelvedXMLImporter::ReadToEndOfElements(Utf8StringCR endNodeName)
    {
    while (m_reader->Read() == BeXmlReader::READ_RESULT_Success)
        {
        switch (m_reader->GetCurrentNodeType())
            {
            case BeXmlReader::NODE_TYPE_EndElement:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == endNodeName)
                    return;
                }
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
void TwelvedXMLImporter::Read12dUnits()
    {
    //Read units from 12dXml
    //Observation: Apparently only the default option "metre" is exported
    //Read linear units from <linear> and Ignore:<area>,<volume>,<temperature>,<pressure>, <angular> and <direction>
    m_isMetric = false;
    while (m_reader->Read() == BeXmlReader::READ_RESULT_Success)
        {
        switch (m_reader->GetCurrentNodeType())
            {
            case BeXmlReader::NODE_TYPE_EndElement:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);

                if (nodeName == c12dUnits) //closing </units>
                    return;
                }
                break;
            case BeXmlReader::NODE_TYPE_Element:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == c12dMetric) //<metric> to </metric>
                    {
                    m_isMetric = true;
                    }
                else if (nodeName == c12dLinear)  //<linear>metre</linear>
                    {
                    m_reader->GetCurrentNodeString(m_linearUnit);
                    }
                }
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
void TwelvedXMLImporter::Read12dApplication()
    {
    WString applicationName;
    int applicationEndCount = 0;
    while (m_reader->Read() == BeXmlReader::READ_RESULT_Success)
        {
        switch (m_reader->GetCurrentNodeType())
            {
            case BeXmlReader::NODE_TYPE_EndElement:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == c12dApplication) //closing </application>
                    {
                    applicationEndCount++;
                    if (applicationEndCount == 2)
                        {
                        if (applicationName == L"MX")
                            m_isMXProject = true;
                        else if (applicationName == L"Geopak")
                            m_isGeopakProject = true;
                        return;
                        }
                    }
                }
                break;
            case BeXmlReader::NODE_TYPE_Element:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == c12dName) //<name></name>
                    m_reader->GetCurrentNodeString(applicationName);
                }
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
void TwelvedXMLImporter::Read12dComments()
    {
    while (m_reader->Read() == BeXmlReader::READ_RESULT_Success)
        {
        switch (m_reader->GetCurrentNodeType())
            {
            case BeXmlReader::NODE_TYPE_EndElement:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == c12dComments) //closing </comments>
                    return;
                }
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
void TwelvedXMLImporter::Read12dModel()
    {
    while (m_reader->Read() == BeXmlReader::READ_RESULT_Success)
        {
        switch (m_reader->GetCurrentNodeType())
            {
            case BeXmlReader::NODE_TYPE_EndElement:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == c12dModel) //closing </model>
                    return;
                }
                break;
            case BeXmlReader::NODE_TYPE_Element:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == c12dFull_Tin || nodeName == c12dTin)
                    Read12dSurface();
                }
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
ImportedTerrain TwelvedXMLImporter::_ImportTerrain(WCharCP name) const
    {
    m_namedDtms.clear();
    m_namedDtms[name] = nullptr;
    const_cast<TwelvedXMLImporterP>(this)->Read12dXML();

    return ImportedTerrain(m_namedDtms[name].get(), name, nullptr, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
ImportedTerrainList TwelvedXMLImporter::_ImportTerrains() const
    {
    m_namedDtms.clear();

    m_importAllDTMS = true;
    const_cast<TwelvedXMLImporterP>(this)->Read12dXML();
    m_importAllDTMS = false;
    ImportedTerrainList ret;

    for (bmap <WString, BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr>::const_iterator iter = m_namedDtms.begin(); iter != m_namedDtms.end(); iter++)
        ret.push_back(ImportedTerrain(iter->second.get(), iter->first.GetWCharCP(), nullptr, false));
    return ret;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
ImportedTerrainList TwelvedXMLImporter::_ImportTerrains(bvector<WString>& names) const
    {
    m_namedDtms.clear();
    for (bvector<WString>::const_iterator iter = names.begin(); iter != names.end(); iter++)
        m_namedDtms[*iter] = nullptr;

    const_cast<TwelvedXMLImporterP>(this)->Read12dXML();

    ImportedTerrainList ret;
    for (bvector<WString>::const_iterator iter = names.begin(); iter != names.end(); iter++)
        ret.push_back(ImportedTerrain(m_namedDtms[*iter].get(), iter->GetWCharCP(), nullptr, false));
    return ret;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
void TwelvedXMLImporter::Read12dXML()
    {
    do
        {
        if (m_pntRefs != NULL)
            m_secondPass = true;

        BeXmlStatus status;
        m_reader = BeXmlReader::CreateAndReadFromFile(status, m_filename.GetWCharCP());

        // Parse the file and display each of the nodes.
        while (m_reader->Read() == BeXmlReader::READ_RESULT_Success)
            {
            switch (m_reader->GetCurrentNodeType())
                {
                case BeXmlReader::NODE_TYPE_Element:
                    {
                    Utf8String nodeName;
                    m_reader->GetCurrentNodeName(nodeName);

                    if (nodeName == c12dApplication)
                        Read12dApplication();
                    else if (nodeName == c12dUnits)
                        Read12dUnits();
                    else if (nodeName == c12dComments)
                        Read12dComments();
                    else if (nodeName == c12dModel)
                        Read12dModel();
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
void TwelvedXMLImporter::Initialize()
    {
    if (m_initialized)
        {
        return;
        }

    m_initialized = true;
    m_surfaces.clear();

    BeXmlStatus status;
    m_reader = BeXmlReader::CreateAndReadFromFile(status, m_filename.GetWCharCP());

    // Parse the file and invetigate each of the nodes.
    while (m_reader->Read() == BeXmlReader::READ_RESULT_Success)
        {
        switch (m_reader->GetCurrentNodeType())
            {
            case BeXmlReader::NODE_TYPE_Element:
                {
                Utf8String nodeName;
                m_reader->GetCurrentNodeName(nodeName);
                if (nodeName == c12dApplication)
                    Read12dApplication();
                else if (nodeName == c12dUnits)
                    Read12dUnits();
                else if (nodeName == c12dComments)
                    Read12dComments();
                else if (nodeName == c12dModel)
                    Read12dModel();
                break;
                }
            }
        }
    m_reader = NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
TwelvedXMLOptions TwelvedXMLImporter::GetOptions()
    {
    return m_options;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
void TwelvedXMLImporter::SetOptions(TwelvedXMLOptions options)
    {
    m_options = options;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
DTMFeatureId TwelvedXMLImporter::_AddLinearFeature(DPoint3d* points, DTMFeatureType featureType)
    {
    DTMFeatureId id = DTM_NULL_FEATURE_ID;
    DTMUserTag dtmUserTag = DTM_NULL_USER_TAG;
    m_dtm->AddLinearFeature(featureType, &points[0], 4, dtmUserTag, &id);

    return DTMFeatureId(id);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Cecilio.Shirakawa  05/2019
//---------------------------------------------------------------------------------------
int TwelvedXMLImporter::_BuildDTMFromTriangles()
    {
    int status = bcdtmObject_triangulateStmTrianglesDtmObject(m_dtm->GetTinHandle());
    if (status)
        return DTM_ERROR;
    else
        return DTM_SUCCESS;
    }
