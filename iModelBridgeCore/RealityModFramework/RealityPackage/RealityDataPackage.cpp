/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealityDataPackage.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RealityPackage/RealityDataPackage.h>
#include <Bentley/BeFileName.h>


USING_BENTLEY_NAMESPACE_REALITYPACKAGE

#define PACKAGE_TOSTRING_OPTIONS ((BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent))

#define W3SCHEMA_PREFIX     "xsi"
#define W3SCHEMA_URI        "http://www.w3.org/2001/XMLSchema-instance"

#define PACKAGE_PREFIX              "rdp"
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


#define PACKAGE_ELEMENT_ImageryGroup        "ImageryGroup"
#define PACKAGE_ELEMENT_ModelGroup          "ModelGroup"
#define PACKAGE_ELEMENT_PinnedGroup         "PinnedGroup"
#define PACKAGE_ELEMENT_TerrainGroup        "TerrainGroup"

#define PACKAGE_ELEMENT_Source              "Source"
#define PACKAGE_SOURCE_ATTRIBUTE_Uri        "Uri"
#define PACKAGE_SOURCE_ATTRIBUTE_Type       "Type"

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WString BoundingPolygon::ToString() const
    {
    //&&MM TODO. It would be nice to reuse something here.  CurveVector maybe? but that seems overkill and its 3d.
    // Use an inner and outer loop element. ?
    return WString();   //&&MM TODO 
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus BoundingPolygon::FromString(WCharCP polygon)
    {
    return RealityPackageStatus::UnknownError; //&&MM todo
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

DateTimeCR RealityDataPackage::GetCreationDate() const {return m_creationDate;}
void       RealityDataPackage::SetCreationDate(DateTimeCR date) {m_creationDate = date;}

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
RealityDataPackagePtr RealityDataPackage::CreateFromFile(RealityPackageStatus& status, BeFileNameCR filename, WStringP pParseError)
    {
    status = RealityPackageStatus::UnknownError;

    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, filename.c_str(), pParseError);
    if(BEXML_Success != xmlStatus)    
        return NULL;
      
    RealityDataPackagePtr pPackage = new RealityDataPackage(filename.GetFileNameWithoutExtension().c_str());
    
    BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
    if(NULL == pRootNode)
        return NULL;

    //&&MM do we need a member version? 
    // Should we have an internal RealityDataPackageV1?
    //      That is RealityDataPackage is a common interface and wraps RealityDataPackageV1, RealityDataPackageV2...?
    // How to handle different version?  Do we allowed to create older version? maybe we could move them in a different
    // namespace.
    WString version;
    if(BEXML_Success != pRootNode->GetAttributeStringValue(version, PACKAGE_ROOT_ATTRIBUTE_Version) || version.empty())
        {
        BeDataAssert(!"missing package version attribute");
        status = RealityPackageStatus::UnknownVersion;
        return NULL;
        }

    //&&MM version of the namespace?
    pXmlDom->RegisterNamespace(PACKAGE_PREFIX, PACKAGE_NAMESPACE_1_0);

    //&&MM todo use xmlXPathContextPtr  localeElementContext    = dom.AcquireXPathContext (localeElement);
    // see DgnFontManager::ProcessLocaleConfiguration

    //&&MM handle read errors. When an error is fatal?  ex. no boundingbox?

    pXmlDom->SelectNodeContent(pPackage->m_name, "/" PACKAGE_PREFIX ":" PACKAGE_ROOT_ELEMENT "/" PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Name, NULL, BeXmlDom::NODE_BIAS_First);
    pXmlDom->SelectNodeContent(pPackage->m_description, "/" PACKAGE_PREFIX ":" PACKAGE_ROOT_ELEMENT "/" PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Description, NULL, BeXmlDom::NODE_BIAS_First);

    WString creationDateUTC;
    pXmlDom->SelectNodeContent(creationDateUTC, "/" PACKAGE_PREFIX ":" PACKAGE_ROOT_ELEMENT "/" PACKAGE_PREFIX ":" PACKAGE_ELEMENT_CreationDate, NULL, BeXmlDom::NODE_BIAS_First);
    pPackage->m_creationDate = DateTime();
    DateTime::FromString(pPackage->m_creationDate, creationDateUTC.c_str());
    
    pXmlDom->SelectNodeContent(pPackage->m_copyright, "/" PACKAGE_PREFIX ":" PACKAGE_ROOT_ELEMENT "/" PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Copyright, NULL, BeXmlDom::NODE_BIAS_First);
    pXmlDom->SelectNodeContent(pPackage->m_packageId, "/" PACKAGE_PREFIX ":" PACKAGE_ROOT_ELEMENT "/" PACKAGE_PREFIX ":" PACKAGE_ELEMENT_PackageId, NULL, BeXmlDom::NODE_BIAS_First);

    WString polygonString;
    pXmlDom->SelectNodeContent(pPackage->m_packageId, "/" PACKAGE_PREFIX ":" PACKAGE_ROOT_ELEMENT "/" PACKAGE_PREFIX ":" PACKAGE_ELEMENT_BoundingPolygon, NULL, BeXmlDom::NODE_BIAS_First);
    pPackage->m_boundingPolygon.FromString(polygonString.c_str());
    
    //&&MM to do polygon
    status = RealityPackageStatus::Success;

    // Data sources
    if(RealityPackageStatus::Success != (status = pPackage->ReadDataSources(pPackage->GetImageryGroupR(), PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ImageryGroup, pRootNode)) ||
       RealityPackageStatus::Success != (status = pPackage->ReadDataSources(pPackage->GetModelGroupR(), PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ModelGroup, pRootNode)) ||
       RealityPackageStatus::Success != (status = pPackage->ReadDataSources(pPackage->GetPinnedGroupR(), PACKAGE_PREFIX ":" PACKAGE_ELEMENT_PinnedGroup, pRootNode)) ||
       RealityPackageStatus::Success != (status = pPackage->ReadDataSources(pPackage->GetTerrainGroupR(), PACKAGE_PREFIX ":" PACKAGE_ELEMENT_TerrainGroup, pRootNode)))
       return NULL;

    return pPackage;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataPackage::Write(BeFileNameCR filename)
    {
    RealityPackageStatus status = RealityPackageStatus::UnknownError;

    //&&MM Todo validate data before writing the file. What is mandatory?
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

    // Data sources
    if(RealityPackageStatus::Success != (status = WriteDataSources(GetImageryGroup(), PACKAGE_ELEMENT_ImageryGroup, pRootNode)) ||
       RealityPackageStatus::Success != (status = WriteDataSources(GetModelGroup(), PACKAGE_ELEMENT_ModelGroup, pRootNode)) ||
       RealityPackageStatus::Success != (status = WriteDataSources(GetPinnedGroup(), PACKAGE_ELEMENT_PinnedGroup, pRootNode)) ||
       RealityPackageStatus::Success != (status = WriteDataSources(GetTerrainGroup(), PACKAGE_ELEMENT_TerrainGroup, pRootNode)))
        return status;
    
    //&&MM map some xmlstatus to RealityPackageStatus.
    BeXmlStatus xmlStatus = pXmlDom->ToFile(filename, PACKAGE_TOSTRING_OPTIONS, BeXmlDom::FILE_ENCODING_Utf8);
    if(BEXML_Success != xmlStatus)
        return RealityPackageStatus::UnknownError;

    return RealityPackageStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataPackage::ReadDataSources(DataSources& sources, Utf8CP nodePath, BeXmlNodeP pParent)
    {
    sources.clear();

    BeXmlNodeP pSourceGroup = pParent->SelectSingleNode (nodePath);
    if(NULL == pSourceGroup)
        return RealityPackageStatus::Success;       // Source group are optional.
    
    for (BeXmlNodeP childElement = pSourceGroup->GetFirstChild(); NULL != childElement; childElement = childElement->GetNextSibling())
        {
        RealityDataSourcePtr pSource = RealityDataSource::CreateFromXml(childElement);

        // Do not return an error but warn about it. It might be a source node extension that we can't handle?
        BeDataAssert(pSource.IsValid());   
        if(pSource.IsValid())
            sources.push_back(pSource);        
        }

    return RealityPackageStatus::Success;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataPackage::WriteDataSources(DataSources const& sources, Utf8CP nodeName, BeXmlNodeP pParent)
    {
    if(sources.empty())
        return RealityPackageStatus::Success;

    RealityPackageStatus status = RealityPackageStatus::UnknownError;

    BeXmlNodeP pGroupNode = pParent->GetDom()->AddNewElement(nodeName, NULL, pParent);
    for (auto pDataSource : sources)
        {
        status = pDataSource->Write(pGroupNode);
        if(RealityPackageStatus::Success != status)
            break;
        }
    
    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
WString RealityDataPackage::GetCreationDateUTC()
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
//                              RealityDataSource
//=======================================================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   
//----------------------------------------------------------------------------------------
WStringCR RealityDataSource::GetUri() const {return m_uri;} 
void RealityDataSource::SetUri(WCharCP uri) {m_uri=uri;}

WStringCR RealityDataSource::GetType() const  {return m_type;} 
void RealityDataSource::SetType(WCharCP type) {m_type=type;}

RealityPackageStatus RealityDataSource::Write(BeXmlNodeP pParent) const {return _Write(pParent) ? RealityPackageStatus::Success : RealityPackageStatus::UnknownError;}

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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityDataSourcePtr RealityDataSource::CreateFromXml(BeXmlNodeP pSourceNode)
    {
    WString uri;
    if(BEXML_Success != pSourceNode->GetAttributeStringValue (uri, PACKAGE_SOURCE_ATTRIBUTE_Uri))
        return NULL;

    WString type;
    if(BEXML_Success != pSourceNode->GetAttributeStringValue (type, PACKAGE_SOURCE_ATTRIBUTE_Type))
        return NULL;

    return RealityDataSource::Create(uri.c_str(), type.c_str());
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
BeXmlNodeP RealityDataSource::_Write(BeXmlNodeP pParent) const
    {
    if(m_uri.empty() || m_type.empty())
        return NULL;
        
    BeXmlNodeP pSourceNode = pParent->GetDom()->AddNewElement(PACKAGE_ELEMENT_Source, NULL, pParent);

    pSourceNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Uri, m_uri.c_str());
    pSourceNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Type, m_type.c_str());

    return pSourceNode;    
    }


