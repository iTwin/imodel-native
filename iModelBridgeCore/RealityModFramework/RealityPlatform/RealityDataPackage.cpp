/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataPackage.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RealityPlatform/RealityDataPackage.h>
#include <Bentley/BeFileName.h>
#include "RealitySerialization.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

#define XML_TOSTRING_OPTIONS ((BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent))

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
// @bsimethod                                   Spencer.Mason           	   11/2017
//-------------------------------------------------------------------------------------
bvector<GeoPoint2d> BoundingPolygon::GetFootprint() const
    {
    return m_points;
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

bvector<PackageRealityDataPtr> const&  RealityDataPackage::GetImageryGroup() const   {return (m_entities.find(RealityDataBase::Classification::IMAGERY))->second;}
bvector<PackageRealityDataPtr>&        RealityDataPackage::GetImageryGroupR()        {return (m_entities.find(RealityDataBase::Classification::IMAGERY))->second;}
bvector<PackageRealityDataPtr> const&  RealityDataPackage::GetModelGroup() const     {return (m_entities.find(RealityDataBase::Classification::MODEL))->second;}
bvector<PackageRealityDataPtr>&        RealityDataPackage::GetModelGroupR()          {return (m_entities.find(RealityDataBase::Classification::MODEL))->second;}
bvector<PackageRealityDataPtr> const&  RealityDataPackage::GetPinnedGroup() const    {return (m_entities.find(RealityDataBase::Classification::PINNED))->second;}
bvector<PackageRealityDataPtr>&        RealityDataPackage::GetPinnedGroupR()         {return (m_entities.find(RealityDataBase::Classification::PINNED))->second;}
bvector<PackageRealityDataPtr> const&  RealityDataPackage::GetTerrainGroup() const   {return (m_entities.find(RealityDataBase::Classification::TERRAIN))->second;}
bvector<PackageRealityDataPtr>&        RealityDataPackage::GetTerrainGroupR()        {return (m_entities.find(RealityDataBase::Classification::TERRAIN))->second;}
bvector<PackageRealityDataPtr> const&  RealityDataPackage::GetUndefinedGroup() const {return (m_entities.find(RealityDataBase::Classification::UNDEFINED_CLASSIF))->second;}
bvector<PackageRealityDataPtr>&        RealityDataPackage::GetUndefinedGroupR()      {return (m_entities.find(RealityDataBase::Classification::UNDEFINED_CLASSIF))->second;}

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
    m_entities = bmap<RealityDataBase::Classification, bvector<PackageRealityDataPtr>>();
    m_entities.Insert(RealityDataBase::Classification::UNDEFINED_CLASSIF, bvector<PackageRealityDataPtr>());
    m_entities.Insert(RealityDataBase::Classification::IMAGERY, bvector<PackageRealityDataPtr>());
    m_entities.Insert(RealityDataBase::Classification::TERRAIN, bvector<PackageRealityDataPtr>());
    m_entities.Insert(RealityDataBase::Classification::MODEL, bvector<PackageRealityDataPtr>());
    m_entities.Insert(RealityDataBase::Classification::PINNED, bvector<PackageRealityDataPtr>());
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
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
PackageRealityData::PackageRealityData(SpatialEntityDataSourceR dataSource)  { m_DataSources.push_back(SpatialEntityDataSourcePtr(&dataSource));}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
PackageRealityData::~PackageRealityData(){}


//=======================================================================================
//                              ImageryData
//=======================================================================================
PackageRealityData::PackageRealityData(SpatialEntityDataSourceR dataSource, bvector<GeoPoint2d> pCorners):
    PackageRealityData(dataSource)
    {
    SetFootprint(pCorners);
    SetClassification(Classification::IMAGERY);
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
PackageRealityDataPtr PackageRealityData::CreateImagery(SpatialEntityDataSourceR dataSource, bvector<GeoPoint2d> pCorners)
    {
    // If corners are given they must be valid.
    if (!pCorners.empty() && !HasValidCorners(pCorners))
        return NULL;

    return new PackageRealityData(dataSource, pCorners);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
PackageRealityDataPtr PackageRealityData::CreateModel(SpatialEntityDataSourceR dataSource)
    {
    PackageRealityDataPtr ptr = new PackageRealityData(dataSource);
    ptr->SetClassification(Classification::MODEL);
    return ptr;
    }

//=======================================================================================
//                              PinnedData
//=======================================================================================
PackageRealityData::PackageRealityData(SpatialEntityDataSourceR dataSource, double longitude, double latitude):PackageRealityData(dataSource)
    {
    m_location.Init(longitude, latitude);
    SetClassification(Classification::PINNED);
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
PackageRealityDataPtr PackageRealityData::CreatePinned(SpatialEntityDataSourceR dataSource, double longitude, double latitude)
    {
    if (!RealityDataSerializer::IsValidLongLat(longitude, latitude))
        return NULL;

    return new PackageRealityData(dataSource, longitude, latitude);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
GeoPoint2dCR PackageRealityData::GetLocation() const{return m_location;}

bool PackageRealityData::SetLocation(GeoPoint2dCR location)
    {
    if(!RealityDataSerializer::IsValidLongLat(location.longitude, location.latitude))
        return false;

    m_location = location;
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
PackageRealityDataPtr PackageRealityData::CreateTerrain(SpatialEntityDataSourceR dataSource)
    {
    PackageRealityDataPtr ptr = new PackageRealityData(dataSource);
    ptr->SetClassification(Classification::TERRAIN);
    return ptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Alain.Robert  04/2017
//----------------------------------------------------------------------------------------
PackageRealityDataPtr PackageRealityData::CreateUndefined(SpatialEntityDataSourceR dataSource)
    {
    PackageRealityDataPtr ptr = new PackageRealityData(dataSource);
    ptr->SetClassification(Classification::UNDEFINED_CLASSIF);
    return ptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                  Spencer.Mason  11/2017
//----------------------------------------------------------------------------------------
void PackageRealityData::SetFootprint(bvector<GeoPoint2d> const& footprint, Utf8String coordSys)
    {
    m_footprint = footprint;
    m_footprintString = "";
    m_footprintExtentComputed = false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                  Spencer.Mason  11/2017
//----------------------------------------------------------------------------------------
Utf8String PackageRealityData::GetFootprintString() const
    {
    if(m_footprintExtentComputed)
        return m_footprintString;

    // convert to a space separated string.
    WString result;
    for (auto point : m_footprint)
        {
        WPrintfString pointStr(LATLONG_PRINT_FORMAT L" ", point.longitude, point.latitude);
        result.append(pointStr);
        }

    // Remove extra whitespace
    if (result.size() > 1)
        result.resize(result.size() - 1);

    m_footprintString = Utf8String(result.c_str());
    m_footprintExtentComputed = true;

    return m_footprintString;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                  Spencer.Mason  11/2017
//----------------------------------------------------------------------------------------
bool PackageRealityData::HasArea() const
    {
    return !m_footprint.empty();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
bool PackageRealityData::HasValidCorners(const bvector<GeoPoint2d>& pCorners)
    {
    if (pCorners.empty())
        return false;

    for (size_t i = 0; i < 4; ++i)
        {
        if (!RealityDataSerializer::IsValidLongLat(pCorners[i].longitude, pCorners[i].latitude))
            {
            BeDataAssert(!"Invalid long/lat");
            return false;
            }
        }

    // &&AR TO DO ... Make sure the path defined by corners does not autocross.
    return true;
    }