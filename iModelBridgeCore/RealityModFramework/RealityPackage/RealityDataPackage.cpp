/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealityDataPackage.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

//=======================================================================================
//                              BoundingPolygon
//=======================================================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
DPoint2dCP BoundingPolygon::GetPointCP() const {return &m_points[0];}
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
BoundingPolygonPtr BoundingPolygon::Create(DPoint2dCP pPoints, size_t count)
    {
    if(count < 3)
        return NULL;

    for(size_t i=0; i < count; ++i)
        {
        if(!RealityDataSerializer::IsValidLongLat(pPoints[i].x, pPoints[i].y))
            {
            BeDataAssert(!"Invalid polygon long/lat");
            return NULL;
            }
        }

    return new BoundingPolygon(pPoints, count);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
BoundingPolygon::BoundingPolygon(DPoint2dCP pPoints, size_t count)
    {
    if(count > 2)
        {
        if(pPoints[0].AlmostEqual(pPoints[count-1]))
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
        WPrintfString pointStr(LATLONG_PRINT_FORMAT L" ", point.x, point.y);
        result.append(pointStr);
        }

    // Remove extra whitespace
    if(result.size() > 1)
        result.resize(result.size()-1);

    return result;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
BoundingPolygonPtr BoundingPolygon::FromString(WStringCR polygonStr) 
    {
    bvector<DPoint2d> points;

    WStringTokenizer tokenizer(polygonStr, SPACE_DELIMITER_U);
    
    while(tokenizer.HasValue())
        {
        DPoint2d point;
        if(!tokenizer.Get(point.x) || !tokenizer.Get(point.y) || !RealityDataSerializer::IsValidLongLat(point.x, point.y))
            {
            BeDataAssert(!"Invalid polygon data");
            points.clear(); // incomplete x-y sequence.
            break;
            }
        
        points.push_back(point);
        }

    if(points.size() > 2)
        {
        if(points[0].AlmostEqual(points[points.size()-1]))
            {
            points[points.size()-1] = points[0];  // Explicitly assign the last for bitwise equality.
            }
        else
            {
            points.push_back(points[0]);    // add closure point.
            }    

        return new BoundingPolygon(points); // >>> Use std::move on 'points'.
        }
    
    return NULL;    // invalid polygon string.
    }

//=======================================================================================
//                              RealityDataPackage
//=======================================================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   
//----------------------------------------------------------------------------------------
WStringCR RealityDataPackage::GetName() const {return m_name;}
void      RealityDataPackage::SetName(WCharCP name) {BeAssert(!WString::IsNullOrEmpty(name)); m_name = name;}

WStringCR RealityDataPackage::GetDescription() const {return m_description;}
void      RealityDataPackage::SetDescription(WCharCP description) {m_description = description;}

WStringCR RealityDataPackage::GetCopyright() const {return m_copyright;}
void      RealityDataPackage::SetCopyright(WCharCP copyright) {m_copyright = copyright;}

WStringCR RealityDataPackage::GetPackageId() const {return m_packageId;}
void      RealityDataPackage::SetPackageId(WCharCP packageId) {m_packageId = packageId;}

DateTimeCR RealityDataPackage::GetCreationDate() const {return m_creationDate;}
void       RealityDataPackage::SetCreationDate(DateTimeCR date) {m_creationDate = date;}

BoundingPolygonCR RealityDataPackage::GetBoundingPolygon() const {return *m_pBoundingPolygon;}
void              RealityDataPackage::SetBoundingPolygon(BoundingPolygonR polygon) {BeAssert(polygon.IsValid()); m_pBoundingPolygon = &polygon;}

uint32_t RealityDataPackage::GetMajorVersion() const {return m_majorVersion;} 
uint32_t RealityDataPackage::GetMinorVersion() const {return m_minorVersion;}

bool RealityDataPackage::HasUnknownElements() const {return m_hasUnknownElements;}

RealityDataPackage::ImageryGroup const& RealityDataPackage::GetImageryGroup() const {return m_imagery;}    
RealityDataPackage::ImageryGroup&       RealityDataPackage::GetImageryGroupR()      {return m_imagery;}   
RealityDataPackage::ModelGroup const&   RealityDataPackage::GetModelGroup() const   {return m_model;}    
RealityDataPackage::ModelGroup&         RealityDataPackage::GetModelGroupR()        {return m_model;}    
RealityDataPackage::PinnedGroup const&  RealityDataPackage::GetPinnedGroup() const  {return m_pinned;}    
RealityDataPackage::PinnedGroup&        RealityDataPackage::GetPinnedGroupR()       {return m_pinned;}    
RealityDataPackage::TerrainGroup const& RealityDataPackage::GetTerrainGroup() const {return m_terrain;}    
RealityDataPackage::TerrainGroup&       RealityDataPackage::GetTerrainGroupR()      {return m_terrain;}   

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackage::RealityDataPackage(WCharCP name)
:m_majorVersion(PACKAGE_CURRENT_MAJOR_VERSION),
 m_minorVersion(PACKAGE_CURRENT_MINOR_VERSION),
 m_creationDate(),  // create empty in case package doesn't have a creation date. In that case, we want to report an invalid DataTime
 m_pBoundingPolygon(BoundingPolygon::Create()), //empty invalid polygon
 m_hasUnknownElements(false)
    {
    BeAssert(!WString::IsNullOrEmpty(name));
    m_name = name;    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackage::~RealityDataPackage(){}


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackagePtr RealityDataPackage::Create(WCharCP name)
    {
    if(WString::IsNullOrEmpty(name))
        return NULL;

    return new RealityDataPackage(name);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataPackage::ReadVersion(BeXmlNodeR rootNode)
    {
    Utf8String version;
    if(BEXML_Success == rootNode.GetAttributeStringValue(version, PACKAGE_ATTRIBUTE_Version) && 
       2 == BE_STRING_UTILITIES_UTF8_SSCANF(version.c_str(), "%u.%u", &m_majorVersion, &m_minorVersion))
        {
        return RealityPackageStatus::Success;
        }
        
    BeDataAssert(!"missing package version or badly formatted");

    return RealityPackageStatus::UnknownError;
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

    return RealityDataPackage::CreateFromDom(status, *pXmlDom, L"?"/*defaultName*/, pParseError);
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

    return RealityDataPackage::CreateFromDom(status, *pXmlDom, filename.GetFileNameWithoutExtension().c_str(), pParseError);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackagePtr RealityDataPackage::CreateFromDom(RealityPackageStatus& status, BeXmlDomR xmlDom, WCharCP defaultName, WStringP pParseError)
    {
    status = RealityPackageStatus::UnknownError;

    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if(NULL == pRootNode)
        return NULL;

    RealityDataPackagePtr pPackage = new RealityDataPackage(defaultName);
    
    if(RealityPackageStatus::Success != pPackage->ReadVersion(*pRootNode) || pPackage->GetMajorVersion() > PACKAGE_CURRENT_MAJOR_VERSION)
        {
        status = RealityPackageStatus::UnsupportedVersion;
        return NULL;
        }

    xmlDom.RegisterNamespace(PACKAGE_PREFIX, PACKAGE_CURRENT_NAMESPACE);

    xmlXPathContextPtr pRootContext = xmlDom.AcquireXPathContext (pRootNode);

    // Things we do not validate.[Optional] 
    xmlDom.SelectNodeContent(pPackage->m_name, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Name, pRootContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pPackage->m_description, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Description, pRootContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pPackage->m_copyright, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Copyright, pRootContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pPackage->m_packageId, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_PackageId, pRootContext, BeXmlDom::NODE_BIAS_First);

    // Creation date. [Optional] 
    WString creationDateUTC;
    xmlDom.SelectNodeContent(creationDateUTC, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_CreationDate, pRootContext, BeXmlDom::NODE_BIAS_First);
    if(!creationDateUTC.empty() && BentleyStatus::SUCCESS != DateTime::FromString(pPackage->m_creationDate, creationDateUTC.c_str()))
        {
        status = RealityPackageStatus::InvalidDateFormat;   // If present format must be valid
        return NULL;
        } 

    // Bounding polygon. [Optional] 
    WString polygonString;
    xmlDom.SelectNodeContent(polygonString, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_BoundingPolygon, pRootContext, BeXmlDom::NODE_BIAS_First);
    if(!polygonString.empty() && (pPackage->m_pBoundingPolygon = BoundingPolygon::FromString(polygonString.c_str())).IsNull())
        {
        status = RealityPackageStatus::PolygonParsingError; // If present format must be valid
        return NULL;
        }
    BeAssert(pPackage->m_pBoundingPolygon.IsValid()); // an instance is required
    
    status = RealityPackageStatus::Success;

    // Data sources [Optional]
    if(RealityPackageStatus::Success != (status = pPackage->ReadDataGroup_T(pPackage->GetImageryGroupR(), PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ImageryGroup, *pRootNode)) ||
       RealityPackageStatus::Success != (status = pPackage->ReadDataGroup_T(pPackage->GetModelGroupR(), PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ModelGroup, *pRootNode)) ||
       RealityPackageStatus::Success != (status = pPackage->ReadDataGroup_T(pPackage->GetPinnedGroupR(), PACKAGE_PREFIX ":" PACKAGE_ELEMENT_PinnedGroup, *pRootNode)) ||
       RealityPackageStatus::Success != (status = pPackage->ReadDataGroup_T(pPackage->GetTerrainGroupR(), PACKAGE_PREFIX ":" PACKAGE_ELEMENT_TerrainGroup, *pRootNode)))
        return NULL;

    return pPackage;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataPackage::Write(BeFileNameCR filename)
    {
    RealityPackageStatus status = RealityPackageStatus::UnknownError;

    BeXmlDomPtr pXmlDom = BeXmlDom::CreateEmpty();

    BeXmlNodeP pRootNode = pXmlDom->AddNewElement(PACKAGE_ELEMENT_Root, NULL, NULL);
    if(NULL == pRootNode)
        return RealityPackageStatus::UnknownError;

    // Update internal version to match the version we are persisting.
    m_majorVersion = PACKAGE_CURRENT_MAJOR_VERSION;
    m_minorVersion = PACKAGE_CURRENT_MINOR_VERSION;
    pRootNode->AddAttributeStringValue(PACKAGE_ATTRIBUTE_Version, PACKAGE_CURRENT_VERSION);
    
    // Namespaces
    pRootNode->AddNamespace(NULL, PACKAGE_CURRENT_NAMESPACE);       // Set as default namespace.
    pRootNode->AddNamespace(W3SCHEMA_PREFIX, W3SCHEMA_URI);

    // Root children
    pRootNode->AddElementStringValue(PACKAGE_ELEMENT_Name, GetName().c_str());
    pRootNode->AddElementStringValue(PACKAGE_ELEMENT_Description, GetDescription().c_str());
    pRootNode->AddElementStringValue(PACKAGE_ELEMENT_CreationDate, BuildCreationDateUTC().c_str());
    pRootNode->AddElementStringValue(PACKAGE_ELEMENT_Copyright, GetCopyright().c_str());
    pRootNode->AddElementStringValue(PACKAGE_ELEMENT_PackageId, GetPackageId().c_str());
    pRootNode->AddElementStringValue(PACKAGE_ELEMENT_BoundingPolygon, GetBoundingPolygon().ToString().c_str());

    // Data sources
    if(RealityPackageStatus::Success != (status = WriteDataGroup_T(GetImageryGroup(), PACKAGE_ELEMENT_ImageryGroup, *pRootNode)) ||
       RealityPackageStatus::Success != (status = WriteDataGroup_T(GetModelGroup(), PACKAGE_ELEMENT_ModelGroup, *pRootNode)) ||
       RealityPackageStatus::Success != (status = WriteDataGroup_T(GetPinnedGroup(), PACKAGE_ELEMENT_PinnedGroup, *pRootNode)) ||
       RealityPackageStatus::Success != (status = WriteDataGroup_T(GetTerrainGroup(), PACKAGE_ELEMENT_TerrainGroup, *pRootNode)))
        return status;
    
    BeXmlStatus xmlStatus = pXmlDom->ToFile(filename, XML_TOSTRING_OPTIONS, BeXmlDom::FILE_ENCODING_Utf8);
    if(BEXML_Success != xmlStatus)
        return RealityPackageStatus::WriteToFileError;

    return RealityPackageStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
template<class Group_T>
RealityPackageStatus RealityDataPackage::ReadDataGroup_T(Group_T& group, Utf8CP nodePath, BeXmlNodeR parent)
    {
    group.clear();

    BeXmlNodeP pSourceGroup = parent.SelectSingleNode (nodePath);
    if(NULL == pSourceGroup)
        return RealityPackageStatus::Success;       // Source groups are optional.
    
    RealityPackageStatus status = RealityPackageStatus::Success;    // Empty group is valid.

    for (BeXmlNodeP childElement = pSourceGroup->GetFirstChild(); NULL != childElement; childElement = childElement->GetNextSibling())
        {
        Group_T::value_type pData = RealityDataSerializer::TryLoad<Group_T::value_type>(status, *childElement);
        if(RealityPackageStatus::UnknownElementType == status ||    // unknown group
           RealityPackageStatus::MissingDataSource == status)       // unknown source.
            {
            // if this is the last iteration we do not want to return UnknownElementType or MissingDataSource
            status = RealityPackageStatus::Success; 
            m_hasUnknownElements = true;
            continue;   // skip things that we don't know. ex: Future element
            }
        
        // We found the type but could not load it.  This is an error.
        if(RealityPackageStatus::Success != status)
            break;

        group.push_back(pData);        
        }

    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
template<class Group_T>
RealityPackageStatus RealityDataPackage::WriteDataGroup_T(Group_T const& group, Utf8CP nodeName, BeXmlNodeR parent)
    {
    if(group.empty())
        return RealityPackageStatus::Success;

    BeXmlNodeP pGroupNode = parent.AddEmptyElement(nodeName);
    if(NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    RealityPackageStatus status = RealityPackageStatus::UnknownError;

    for (auto pData : group)
        {
        if(!pData.IsValid())
            continue;

        status = RealityDataSerializer::Store<Group_T::value_type::element_type>(*pData, *pGroupNode);
        if(RealityPackageStatus::Success != status)
            break;
        }
    
    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WString RealityDataPackage::BuildCreationDateUTC()
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
//                              RealityData
//=======================================================================================
RealityDataSourceR RealityData::GetSourceR() {return *m_pSource;}
RealityDataSourceCR RealityData::GetSource() const {return *m_pSource;}
Utf8StringCR RealityData::GetCopyright() const { return m_copyright; }
void RealityData::SetCopyright(Utf8CP dataCopyright) { m_copyright = dataCopyright; }
double RealityData::GetFilesize() const { return m_size; }
void RealityData::SetFilesize(double size) { m_size = size; }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityData::RealityData(RealityDataSourceR dataSource)
    : m_pSource(&dataSource),
      m_copyright(""),          // Default.
      m_size(0)                 // Default.
    {}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityData::~RealityData(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityData::_Read(BeXmlNodeR dataNode)
    {
    RealityPackageStatus status = RealityPackageStatus::MissingDataSource;

    for (BeXmlNodeP pChildElement = dataNode.GetFirstChild(); NULL != pChildElement; pChildElement = pChildElement->GetNextSibling())
        {
        if (pChildElement->IsName(PACKAGE_ELEMENT_Copyright))
            pChildElement->GetContent(m_copyright);
        else if (pChildElement->IsName(PACKAGE_ELEMENT_Filesize))
            pChildElement->GetContentDoubleValue(m_size);
        else
            {
            m_pSource = RealityDataSourceSerializer::Get().Load(status, *pChildElement);
            if(RealityPackageStatus::UnknownElementType != status)
                break;  // either we loaded the source or we had an error loading it.
            }
        
        status = RealityPackageStatus::MissingDataSource;   // reset status we do not want return UnknownElementType if it's the last iteration.
        }

    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityData::_Write(BeXmlNodeR dataNode) const
    {
    if(!m_pSource.IsValid())
        return RealityPackageStatus::MissingDataSource;

    // Optional data copyright.
    if (!m_copyright.empty())
        {
        BeXmlNodeP pCopyrightNode = dataNode.AddElementStringValue(PACKAGE_ELEMENT_Copyright, m_copyright.c_str());
        if (NULL == pCopyrightNode)
            return RealityPackageStatus::UnknownError;
        }

    // Optional data size.
    if (0 != m_size)
        {
        BeXmlNodeP pFilesizeNode = dataNode.AddElementDoubleValue(PACKAGE_ELEMENT_Filesize, m_size);
        if (NULL == pFilesizeNode)
            return RealityPackageStatus::UnknownError;
        }

    return RealityDataSourceSerializer::Get().Store(*m_pSource, dataNode);
    }

//=======================================================================================
//                              ImageryData
//=======================================================================================
ImageryData::ImageryData(RealityDataSourceR dataSource, DPoint2dCP pCorners):RealityData(dataSource){SetCorners(pCorners);}

ImageryData::~ImageryData(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
ImageryDataPtr ImageryData::Create(RealityDataSourceR dataSource, DPoint2dCP pCorners)
    {
    if(NULL != NULL && !HasValidCorners(pCorners))
        return NULL;

    return new ImageryData(dataSource, pCorners);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus ImageryData::_Read(BeXmlNodeR dataNode)
    {
    // Write base first
    RealityPackageStatus status = T_Super::_Read(dataNode);
    if(RealityPackageStatus::Success != status)
        return status;

    BeXmlNodeP pCornerNode = dataNode.SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Corners);
    if(NULL == pCornerNode)
        {
        InvalidateCorners();
        return RealityPackageStatus::Success;   // Corners are optional
        }

    if (RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(m_corners[LowerLeft].x, m_corners[LowerLeft].y, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_LowerLeft)) ||
        RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(m_corners[LowerRight].x, m_corners[LowerRight].y, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_LowerRight)) ||
        RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(m_corners[UpperLeft].x, m_corners[UpperLeft].y, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_UpperLeft)) ||
        RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(m_corners[UpperRight].x, m_corners[UpperRight].y, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_UpperRight)) ||
       !HasValidCorners(m_corners))
        {
        InvalidateCorners();
        return status;
        }

    return status;    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus ImageryData::_Write(BeXmlNodeR dataNode) const
    {
    // Write base first
    RealityPackageStatus status = T_Super::_Write(dataNode);
    if(RealityPackageStatus::Success != status)
        return status;

    if(!HasValidCorners(m_corners))
        return RealityPackageStatus::Success;  // Corners are optional.
        
    BeXmlNodeP pCornerNode = dataNode.AddEmptyElement(PACKAGE_ELEMENT_Corners);
    if(NULL == pCornerNode)
        return RealityPackageStatus::UnknownError;

    if (RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_LowerLeft, m_corners[LowerLeft].x, m_corners[LowerLeft].y)) ||
        RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_LowerRight, m_corners[LowerRight].x, m_corners[LowerRight].y)) ||
        RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_UpperLeft, m_corners[UpperLeft].x, m_corners[UpperLeft].y)) ||
        RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_UpperRight, m_corners[UpperRight].x, m_corners[UpperRight].y)))
        {
        return status;
        }

    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
bool ImageryData::HasValidCorners(DPoint2dCP pCorners)
    {
    if(NULL == pCorners || pCorners[0].IsEqual(pCorners[1]))
        return false;

    for(size_t i=0; i < 4; ++i)
        {
        if(!RealityDataSerializer::IsValidLongLat(pCorners[i].x, pCorners[i].y))
            {
            BeDataAssert(!"Invalid long/lat");
            return false;
            }
        }

    return true;
    } 

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
DPoint2dCP ImageryData::GetCornersCP() const
    {
    if(!HasValidCorners(m_corners))
        return NULL;

    return m_corners;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
void ImageryData::SetCorners(DPoint2dCP pCorners)
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
ModelData::ModelData(RealityDataSourceR dataSource):RealityData(dataSource){}
ModelData::~ModelData(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
ModelDataPtr ModelData::Create(RealityDataSourceR dataSource)
    {
    return new ModelData(dataSource);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus ModelData::_Read(BeXmlNodeR dataNode)
    {
    // Read base first
    RealityPackageStatus status = T_Super::_Read(dataNode);
    if(RealityPackageStatus::Success != status)
        return status;

    // Read ModelData specific here...

    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus ModelData::_Write(BeXmlNodeR dataNode) const
    {
    // Write base first
    RealityPackageStatus status = T_Super::_Write(dataNode);
    if(RealityPackageStatus::Success != status)
        return status;

    // Write TerrainObject specific here...
    
    return status;
    }

//=======================================================================================
//                              PinnedData
//=======================================================================================
PinnedData::PinnedData(RealityDataSourceR dataSource, double longitude, double latitude):RealityData(dataSource){m_location.Init(longitude, latitude);}
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
RealityPackageStatus PinnedData::_Read(BeXmlNodeR dataNode)
    {
    // Read base first
    RealityPackageStatus status = T_Super::_Read(dataNode);
    if(RealityPackageStatus::Success != status)
        return status;
    
    if(RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(m_location.x, m_location.y, dataNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Position))) 
        return status;  // location is mandatory for pinned data.

    return RealityDataSerializer::IsValidLongLat(m_location.x, m_location.y) ? RealityPackageStatus::Success : RealityPackageStatus::InvalidLongitudeLatitude;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus PinnedData::_Write(BeXmlNodeR dataNode) const
    {
    BeAssert(RealityDataSerializer::IsValidLongLat(m_location.x, m_location.y));

    // Write base first
    RealityPackageStatus status = T_Super::_Write(dataNode);
    if(RealityPackageStatus::Success != status)
        return status;

    status = RealityDataSerializer::WriteLongLat(dataNode, PACKAGE_ELEMENT_Position, GetLocation().x, GetLocation().y);
       
    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
DPoint2dCR PinnedData::GetLocation() const{return m_location;}
bool PinnedData::SetLocation(DPoint2dCR location)
    {
    if(!RealityDataSerializer::IsValidLongLat(location.x, location.y))
        return false;

    m_location = location;
    return true;
    }

//=======================================================================================
//                              TerrainData
//=======================================================================================
TerrainData::TerrainData(RealityDataSourceR dataSource):RealityData(dataSource){}
TerrainData::~TerrainData(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TerrainDataPtr TerrainData::Create(RealityDataSourceR dataSource)
    {
    return new TerrainData(dataSource);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus TerrainData::_Read(BeXmlNodeR dataNode)
    {
    // Read base first
    RealityPackageStatus status = T_Super::_Read(dataNode);
    if(RealityPackageStatus::Success != status)
        return status;

    // Read TerrainData specific here...

    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus TerrainData::_Write(BeXmlNodeR dataNode) const
    {
    // Write base first
    RealityPackageStatus status = T_Super::_Write(dataNode);
    if(RealityPackageStatus::Success != status)
        return status;

    // Write TerrainObject specific here...
    
    return status;
    }