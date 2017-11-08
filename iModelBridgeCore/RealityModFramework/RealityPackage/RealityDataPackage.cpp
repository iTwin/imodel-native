/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealityDataPackage.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RealityPackage/RealityDataPackage.h>
#include <Bentley/BeFileName.h>
#include "RealitySerialization.h"

USING_NAMESPACE_BENTLEY_REALITYPACKAGE

#define XML_TOSTRING_OPTIONS ((BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent))

Utf8CP ImageryData::ElementName = PACKAGE_ELEMENT_ImageryData;
Utf8CP ModelData::ElementName = PACKAGE_ELEMENT_ModelData;
Utf8CP PinnedData::ElementName = PACKAGE_ELEMENT_PinnedData;
Utf8CP TerrainData::ElementName = PACKAGE_ELEMENT_TerrainData;
Utf8CP UndefinedData::ElementName = PACKAGE_ELEMENT_UndefinedData;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                Alain.Robert      12/2016
* Indicates if two long/lat location are alsmost equal (within given tolerance in meter)
+--------------------------------------------------------------------------------------*/
bool AlmostEqual (GeoPoint2d const & dataA, GeoPoint2d const & dataB, double tolerance=0.001) 
    {
    // We can consider a lat/long coordinate to be almost equal if they diverge by less than tolerance
    // A distance on Earth is based on the ratio 1852 meters per minute latitude
    // and 1852 meters times cos of latitude per minute longitude
    double deltaNorth = fabs(dataA.latitude-dataB.latitude) * 60 * 1852;
    double deltaEast = fabs(dataA.longitude-dataB.longitude) * 60 * 1852 * cos(Angle::DegreesToRadians(dataA.latitude));

    return (tolerance > sqrt(deltaNorth * deltaNorth + deltaEast * deltaEast));
    }

//=======================================================================================
//                              BoundingPolygon
//=======================================================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
GeoPoint2dCP BoundingPolygon::GetPointCP() const {return &m_points[0];}
bool BoundingPolygon::IsValid() const { return GetPointCount() > 3;}
size_t BoundingPolygon::GetPointCount() const {return m_points.size();}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
BoundingPolygonPtr BoundingPolygon::Create()
    {
    return new BoundingPolygon();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
BoundingPolygonPtr BoundingPolygon::Create(GeoPoint2dCP pPoints, size_t count)
    {
    if(count < 3)
        return NULL;

    for(size_t i=0; i < count; ++i)
        {
        if(!RealityDataSerializer::IsValidLongLat(pPoints[i].longitude, pPoints[i].latitude))
            {
            BeDataAssert(!"Invalid polygon long/lat");
            return NULL;
            }
        }

    // &&AR TO DO We should definitely validate to check if the polygon autocrosses or is autocontiguous.


    return new BoundingPolygon(pPoints, count);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
BoundingPolygon::BoundingPolygon(GeoPoint2dCP pPoints, size_t count)
    {
    if(count > 2)
        {
        if(AlmostEqual(pPoints[0], pPoints[count-1]))
            {
            m_points.assign(pPoints, pPoints + (count-1));
            }
        else
            {
            m_points.assign(pPoints, pPoints + count);
            }

        m_points.push_back(pPoints[0]);  // Explicitly assign the last for bitwise equality.
        }

    BeDataAssert(IsValid());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
BoundingPolygon::~BoundingPolygon(){};

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WString BoundingPolygon::ToString() const
    {
    // convert to a space separated string.
    WString result;
    for(auto point : m_points)
        {
        WPrintfString pointStr(LATLONG_PRINT_FORMAT L" ", point.longitude, point.latitude);
        result.append(pointStr);
        }

    // Remove extra whitespace
    if(result.size() > 1)
        result.resize(result.size()-1);

    return result;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
BoundingPolygonPtr BoundingPolygon::FromString(Utf8StringCR polygonStr)
    {
    bvector<GeoPoint2d> points;

    Utf8StringTokenizer tokenizer(polygonStr, SPACE_DELIMITER);

    while (tokenizer.HasValue())
        {
        GeoPoint2d point;
        if (!tokenizer.Get(point.longitude) || !tokenizer.Get(point.latitude) || !RealityDataSerializer::IsValidLongLat(point.longitude, point.latitude))
            {
            BeDataAssert(!"Invalid polygon data");
            points.clear(); // incomplete x-y sequence.
            break;
            }

        points.push_back(point);
        }

    if (points.size() > 2)
        {
        if (AlmostEqual(points[0], points[points.size() - 1]))
            {
            points[points.size() - 1] = points[0];  // Explicitly assign the last for bitwise equality.
            }
        else
            {
            points.push_back(points[0]);    // add closure point.
            }

        // &&AR TO DO We should definitely validate to check if the polygon autocrosses or is autocontiguous.

        return new BoundingPolygon(points); // >>> Use std::move on 'points'.
        }

    return nullptr;    // invalid polygon string.
    }

//=======================================================================================
//                              RealityDataPackage
//=======================================================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   
//----------------------------------------------------------------------------------------
Utf8StringCR RealityDataPackage::GetOrigin() const {return m_origin;}
void         RealityDataPackage::SetOrigin(Utf8CP origin) {m_origin = origin; }

Utf8StringCR RealityDataPackage::GetRequestingApplication() const {return m_requestingApplication;}
void         RealityDataPackage::SetRequestingApplication(Utf8CP requestingApplication) {m_requestingApplication = requestingApplication; }

Utf8StringCR RealityDataPackage::GetName() const {return m_name;}
void         RealityDataPackage::SetName(Utf8CP name) { BeAssert(!Utf8String::IsNullOrEmpty(name)); m_name = name; }

Utf8StringCR RealityDataPackage::GetDescription() const { return m_description; }
void         RealityDataPackage::SetDescription(Utf8CP description) { m_description = description; }

Utf8StringCR RealityDataPackage::GetCopyright() const { return m_copyright; }
void         RealityDataPackage::SetCopyright(Utf8CP copyright) { m_copyright = copyright; }

Utf8StringCR RealityDataPackage::GetId() const { return m_id; }
void         RealityDataPackage::SetId(Utf8CP id) { m_id = id; }

DateTimeCR RealityDataPackage::GetCreationDate() const {return m_creationDate;}
void       RealityDataPackage::SetCreationDate(DateTimeCR date) {m_creationDate = date;}

BoundingPolygonCR RealityDataPackage::GetBoundingPolygon() const {return *m_pBoundingPolygon;}
void              RealityDataPackage::SetBoundingPolygon(BoundingPolygonR polygon) {m_pBoundingPolygon = &polygon;}

uint32_t RealityDataPackage::GetMajorVersion() const {return m_majorVersion;} 
void RealityDataPackage::SetMajorVersion(uint32_t major) { m_majorVersion = major; }
uint32_t RealityDataPackage::GetMinorVersion() const {return m_minorVersion;}
void RealityDataPackage::SetMinorVersion(uint32_t minor) { m_minorVersion = minor; }

bool RealityDataPackage::HasUnknownElements() const {return m_hasUnknownElements;}
void RealityDataPackage::SetUnknownElements(bool hasUnknownElements) { m_hasUnknownElements = hasUnknownElements; }

RealityDataPackage::ImageryGroup const&   RealityDataPackage::GetImageryGroup() const   {return m_imagery;}    
RealityDataPackage::ImageryGroup&         RealityDataPackage::GetImageryGroupR()        {return m_imagery;}   
RealityDataPackage::ModelGroup const&     RealityDataPackage::GetModelGroup() const     {return m_model;}    
RealityDataPackage::ModelGroup&           RealityDataPackage::GetModelGroupR()          {return m_model;}    
RealityDataPackage::PinnedGroup const&    RealityDataPackage::GetPinnedGroup() const    {return m_pinned;}    
RealityDataPackage::PinnedGroup&          RealityDataPackage::GetPinnedGroupR()         {return m_pinned;}    
RealityDataPackage::TerrainGroup const&   RealityDataPackage::GetTerrainGroup() const   {return m_terrain;}    
RealityDataPackage::TerrainGroup&         RealityDataPackage::GetTerrainGroupR()        {return m_terrain;}   
RealityDataPackage::UndefinedGroup const& RealityDataPackage::GetUndefinedGroup() const {return m_undefined;}    
RealityDataPackage::UndefinedGroup&       RealityDataPackage::GetUndefinedGroupR()      {return m_undefined;}   

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackage::RealityDataPackage(Utf8CP name)
:m_majorVersion(PACKAGE_CURRENT_MAJOR_VERSION),
 m_minorVersion(PACKAGE_CURRENT_MINOR_VERSION),
 m_creationDate(),  // create empty in case package doesn't have a creation date. In that case, we want to report an invalid DataTime
 m_pBoundingPolygon(BoundingPolygon::Create()), //empty invalid polygon
 m_hasUnknownElements(false)
    {
    BeAssert(!Utf8String::IsNullOrEmpty(name));
    m_name = name;    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackage::~RealityDataPackage(){}


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackagePtr RealityDataPackage::Create(Utf8CP name)
    {
    if (Utf8String::IsNullOrEmpty(name))
        return NULL;

    return new RealityDataPackage(name);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackagePtr RealityDataPackage::CreateFromString(RealityPackageStatus& status, Utf8CP pSource, WStringP pParseError)
    {
    status = RealityPackageStatus::UnknownError;

    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, pSource, 0, pParseError);
    if(BEXML_Success != xmlStatus)    
        {
        status = RealityPackageStatus::XmlReadError;
        return NULL;
        }

    return RealityDataPackage::CreateFromDom(status, *pXmlDom, "?"/*defaultName*/, pParseError);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackagePtr RealityDataPackage::CreateFromFile(RealityPackageStatus& status, BeFileNameCR filename, WStringP pParseError)
    {
    status = RealityPackageStatus::UnknownError;

    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, filename.c_str(), pParseError);
    if(BEXML_Success != xmlStatus)    
        {
        status = RealityPackageStatus::XmlReadError;
        return NULL;
        }

    Utf8String filenameUtf8;
    BeStringUtilities::WCharToUtf8(filenameUtf8, filename.GetFileNameWithoutExtension().c_str());

    return RealityDataPackage::CreateFromDom(status, *pXmlDom, filenameUtf8.c_str(), pParseError);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackagePtr RealityDataPackage::CreateFromDom(RealityPackageStatus& status, BeXmlDomR xmlDom, Utf8CP defaultName, WStringP pParseError)
    {
    status = RealityPackageStatus::UnknownError;

    // Instantiate the appropriated serializer/deserializer based on the major version number.
    RealityDataSerializerPtr pSerializer = RealityDataSerializerFactory::CreateSerializer(xmlDom);
    if (pSerializer == NULL)
        {
        status = RealityPackageStatus::UnsupportedVersion;
        return NULL;
        }

    RealityDataPackagePtr pPackage = new RealityDataPackage(defaultName);
    status = pSerializer->Read(*pPackage, xmlDom);
    if (RealityPackageStatus::Success != status)
        return NULL;

    return pPackage;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataPackage::Write(BeFileNameCR filename)
    {
    // Instantiate the appropriated serializer/deserializer based on the major version number.
    RealityDataSerializerPtr pSerializer = RealityDataSerializerFactory::CreateSerializer(GetMajorVersion());
    if (pSerializer == NULL)
        return RealityPackageStatus::UnsupportedVersion;

    // Create XmlDom.
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateEmpty();
    pSerializer->Write(*pXmlDom, *this);

    // Write to file.
    BeXmlStatus xmlStatus = pXmlDom->ToFile(filename, XML_TOSTRING_OPTIONS, BeXmlDom::FILE_ENCODING_Utf8);
    if (BEXML_Success != xmlStatus)
        return RealityPackageStatus::WriteToFileError;

    return RealityPackageStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
Utf8String RealityDataPackage::BuildCreationDateUTC()
    {
    if(GetCreationDate().IsValid() && GetCreationDate().GetInfo().GetKind() != DateTime::Kind::Utc)
        return GetCreationDate().ToString();

    DateTime utcDateTime;
    if(SUCCESS == GetCreationDate().ToUtc(utcDateTime))
        return utcDateTime.ToString();

    // By default use current time and update our internal member.
    m_creationDate = DateTime::GetCurrentTimeUtc ();
    return m_creationDate.ToString();
    }

//=======================================================================================
//                              PackageRealityData
//=======================================================================================

RealityDataSourceR PackageRealityData::GetSourceR(size_t index) 
    {
    BeAssert(index < m_Sources.size());
        
    return *(m_Sources[index]);
    }
RealityDataSourceCR PackageRealityData::GetSource(size_t index) const 
    {
    BeAssert(index < m_Sources.size());

    return *(m_Sources[index]);
    }


void PackageRealityData::AddSource(RealityDataSourceR dataSource)
    {
    m_Sources.push_back(RealityDataSourcePtr(&dataSource));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Alain.Robert  3/2016
//----------------------------------------------------------------------------------------
Utf8StringCR PackageRealityData::GetDataId() const {return m_id;}
void PackageRealityData::SetDataId(Utf8CP dataId) {m_id = dataId;}
Utf8StringCR PackageRealityData::GetDataName() const {return m_name;}
void PackageRealityData::SetDataName(Utf8CP dataName) {m_name = dataName;}
Utf8StringCR PackageRealityData::GetDataset() const {return m_dataset;}
void PackageRealityData::SetDataset(Utf8CP dataset) {m_dataset = dataset;}
size_t PackageRealityData::GetNumSources() const {return m_Sources.size();}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
PackageRealityData::PackageRealityData(RealityDataSourceR dataSource)  {m_Sources.push_back(RealityDataSourcePtr(&dataSource));}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
PackageRealityData::~PackageRealityData(){}


//=======================================================================================
//                              ImageryData
//=======================================================================================
ImageryData::ImageryData(RealityDataSourceR dataSource, GeoPoint2dCP pCorners):PackageRealityData(dataSource){SetCorners(pCorners);}

ImageryData::~ImageryData(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
ImageryDataPtr ImageryData::Create(RealityDataSourceR dataSource, GeoPoint2dCP pCorners)
    {
    // If corners are given they must be valid.
    if(NULL != pCorners && !HasValidCorners(pCorners))
        return NULL;

    return new ImageryData(dataSource, pCorners);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
bool ImageryData::HasValidCorners(GeoPoint2dCP pCorners)
    {
    if(NULL == pCorners)
        return false;

    for(size_t i=0; i < 4; ++i)
        {
        if(!RealityDataSerializer::IsValidLongLat(pCorners[i].longitude, pCorners[i].latitude))
            {
            BeDataAssert(!"Invalid long/lat");
            return false;
            }
        }

    // &&AR TO DO ... Make sure the path defined by corners does not autocross.
    return true;
    } 

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
GeoPoint2dCP ImageryData::GetCornersCP() const
    {
    if(!HasValidCorners(m_corners))
        return NULL;

    return m_corners;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
void ImageryData::SetCorners(GeoPoint2dCP pCorners)
    {
    if(NULL == pCorners || !HasValidCorners(pCorners))
        {
        InvalidateCorners();
        return;
        }

    memcpy(m_corners, pCorners, sizeof(m_corners));
    }

//=======================================================================================
//                              ModelData
//=======================================================================================
ModelData::ModelData(RealityDataSourceR dataSource):PackageRealityData(dataSource){}
ModelData::~ModelData(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
ModelDataPtr ModelData::Create(RealityDataSourceR dataSource)
    {
    return new ModelData(dataSource);
    }


//=======================================================================================
//                              PinnedData
//=======================================================================================
PinnedData::PinnedData(RealityDataSourceR dataSource, double longitude, double latitude):PackageRealityData(dataSource){m_location.Init(longitude, latitude);}
PinnedData::~PinnedData(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
PinnedDataPtr PinnedData::Create(RealityDataSourceR dataSource, double longitude, double latitude)
    {
    if(!RealityDataSerializer::IsValidLongLat(longitude, latitude))
        return NULL;
    
    return new PinnedData(dataSource, longitude, latitude);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
GeoPoint2dCR PinnedData::GetLocation() const{return m_location;}
bool PinnedData::SetLocation(GeoPoint2dCR location)
    {
    if(!RealityDataSerializer::IsValidLongLat(location.longitude, location.latitude))
        return false;

    m_location = location;
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Alain.Robert  5/2016
//----------------------------------------------------------------------------------------
bool PinnedData::HasArea() const {return !m_area.IsNull();}
BoundingPolygonCP PinnedData::GetAreaCP() const{return m_area.get();}
bool PinnedData::SetArea(BoundingPolygonR area)
    {
    m_area = &area;
    return true;
    }

//=======================================================================================
//                              TerrainData
//=======================================================================================
TerrainData::TerrainData(RealityDataSourceR dataSource):PackageRealityData(dataSource){}
TerrainData::~TerrainData(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TerrainDataPtr TerrainData::Create(RealityDataSourceR dataSource)
    {
    return new TerrainData(dataSource);
    }

//=======================================================================================
//                              UndefinedData
//=======================================================================================
UndefinedData::UndefinedData(RealityDataSourceR dataSource):PackageRealityData(dataSource){}
UndefinedData::~UndefinedData(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Alain.Robert  04/2017
//----------------------------------------------------------------------------------------
UndefinedDataPtr UndefinedData::Create(RealityDataSourceR dataSource)
    {
    return new UndefinedData(dataSource);
    }
