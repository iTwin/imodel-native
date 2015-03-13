/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealityDataPackage.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RealityPackage/RealityDataPackage.h>
#include <Bentley/BeFileName.h>
#include <BeXml/BeXml.h>

USING_BENTLEY_NAMESPACE_REALITYPACKAGE


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WString BoundingPolygon::ToString() const
    {
    //&&MM TODO. It would be nice to reuse something here.  CurveVector maybe? but that seems overkill and its 3d.
    return WString();   //&&MM TODO 
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackageStatus BoundingPolygon::FromString(WCharCP polygon)
    {
    return RealityDataPackageStatus::UnknownError; //&&MM todo
    }

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

DateTimeCR RealityDataPackage::GetCreationDate() const {return m_dateTime;}
void       RealityDataPackage::SetCreationDate(DateTimeCR date) {m_dateTime = date;}

BoundingPolygonCR RealityDataPackage::GetBoundingPolygon() const {return m_boundingPolygon;}
void              RealityDataPackage::SetBoundingPolygon(BoundingPolygonCR polygon) {BeAssert(polygon.IsValid()); m_boundingPolygon = polygon;}

RealityDataPackage::DataSources const&  RealityDataPackage::GetImageryGroup() const {return m_imagery;}    
RealityDataPackage::DataSources&        RealityDataPackage::GetImageryGroupR()      {return m_imagery;}   
RealityDataPackage::DataSources const&  RealityDataPackage::GetTerrainGroup() const {return m_terrain;}    
RealityDataPackage::DataSources&        RealityDataPackage::GetTerrainGroupR()      {return m_terrain;}   
RealityDataPackage::DataSources const&  RealityDataPackage::GetModelGroup() const   {return m_model;}    
RealityDataPackage::DataSources&        RealityDataPackage::GetModelGroupR()        {return m_model;}    
RealityDataPackage::DataSources const&  RealityDataPackage::GetPinnedGroup() const  {return m_pinned;}    
RealityDataPackage::DataSources&        RealityDataPackage::GetPinnedGroupR()       {return m_pinned;}    

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackage::RealityDataPackage(WCharCP name)
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
RealityDataPackagePtr RealityDataPackage::Create(RealityDataPackageStatus& status, BeFileNameCR filename)
    {
    status = RealityDataPackageStatus::UnknownError;

    RealityDataPackagePtr pPackage = new RealityDataPackage(filename.GetFileNameWithoutExtension().c_str());

    status = pPackage->Read(filename);

    if(RealityDataPackageStatus::Success != status)
        pPackage = NULL;

    return pPackage;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackageStatus RealityDataPackage::Read(BeFileNameCR filename)
    {
    //&&MM todo
    return RealityDataPackageStatus::UnknownError;
    }

#define TOSTRING_FORMATTED_INDENTED ((BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent))

#define W3SCHEMA_PREFIX     "xsi"
#define W3SCHEMA_URI        "http://www.w3.org/2001/XMLSchema-instance"

#define PACKAGE_NAMESPACE_1_0       "http://www.bentley.com/RealityDataServer/1.0"
#define PACKAGE_VERSION_1_0         L"1.0"

#define PACKAGE_ROOT_ELEMENT                "RealityDataPackage"
#define PACKAGE_ROOT_ATTRIBUTE_Version      "Version"

#define PACKAGE_ELEMENT_Name                "Name"
#define PACKAGE_ELEMENT_Description         "Description"
#define PACKAGE_ELEMENT_CreationDate        "CreationDate"
#define PACKAGE_ELEMENT_Copyright           "Copyright"
#define PACKAGE_ELEMENT_PackageId           "PackageId"
#define PACKAGE_ELEMENT_BoundingPolygon     "BoundingPolygon"




//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataPackageStatus RealityDataPackage::Write(BeFileNameCR filename)
    {
    //&&MM Todo validate data before writing the file
    // 

    BeXmlDomPtr pXmlDom = BeXmlDom::CreateEmpty();

    BeXmlNodeP pRootNode = pXmlDom->AddNewElement(PACKAGE_ROOT_ELEMENT, NULL, NULL);

    pRootNode->AddAttributeStringValue(PACKAGE_ROOT_ATTRIBUTE_Version, PACKAGE_VERSION_1_0);
    
    // Namespaces
    pRootNode->AddNamespace(NULL, PACKAGE_NAMESPACE_1_0);       // Default namespace.
    pRootNode->AddNamespace(W3SCHEMA_PREFIX, W3SCHEMA_URI);

    // Root children
    pRootNode->AddElementStringValue(PACKAGE_ELEMENT_Name, GetName().c_str());
    pRootNode->AddElementStringValue(PACKAGE_ELEMENT_Description, GetDescription().c_str());
    pRootNode->AddElementStringValue(PACKAGE_ELEMENT_CreationDate, GetCreationDateUTC().c_str());
    pRootNode->AddElementStringValue(PACKAGE_ELEMENT_Copyright, GetCopyright().c_str());
    pRootNode->AddElementStringValue(PACKAGE_ELEMENT_PackageId, GetPackageId().c_str());
    pRootNode->AddElementStringValue(PACKAGE_ELEMENT_BoundingPolygon, GetBoundingPolygon().ToString().c_str());


           
    BeXmlStatus status = pXmlDom->ToFile(filename, TOSTRING_FORMATTED_INDENTED, BeXmlDom::FILE_ENCODING_Utf8);
    if(BEXML_Success != status)
        return RealityDataPackageStatus::UnknownError;

    return RealityDataPackageStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WString RealityDataPackage::GetCreationDateUTC() const
    {
    if(GetCreationDate().IsValid() && GetCreationDate().GetInfo().GetKind() != DateTime::Kind::Utc)
        return GetCreationDate().ToString();

    DateTime utcDateTime;
    if(SUCCESS == GetCreationDate().ToUtc(utcDateTime))
        return utcDateTime.ToString();

    // By default use current time
    return DateTime::GetCurrentTimeUtc ().ToString();
    }

//=======================================================================================
//                              RealityDataSource
//=======================================================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   
//----------------------------------------------------------------------------------------
WStringCR RealityDataSource::GetUri() const {return m_uri;} 
void RealityDataSource::SetUri(WCharCP uri) {m_uri=uri;}

WStringCR RealityDataSource::GetType() const  {return m_type;} 
void RealityDataSource::SetType(WCharCP type) {m_type=type;}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSource::RealityDataSource(WCharCP uri, WCharCP type)
    {
    BeAssert(!WString::IsNullOrEmpty(uri) && !WString::IsNullOrEmpty(type));
    m_uri = uri;
    m_type = type;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSource::~RealityDataSource(){}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSourcePtr RealityDataSource::Create(WCharCP uri, WCharCP type)
    {
    if(WString::IsNullOrEmpty(uri) || WString::IsNullOrEmpty(type))
        return NULL;
        
    return new RealityDataSource(uri, type);
    }


