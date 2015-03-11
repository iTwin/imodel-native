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
void       RealityDataPackage::SetCreationDate(DateTimeCR date) {m_dateTime=date;}

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
RealityDataPackagePtr RealityDataPackage::Create(RealityPackageStatus& status, BeFileNameCR filename)
    {
    status = RealityPackageStatus::UnknownError;

    RealityDataPackagePtr pPackage = new RealityDataPackage(filename.GetFileNameWithoutExtension().c_str());

    status = pPackage->Read(filename);

    if(RealityPackageStatus::Success != status)
        pPackage = NULL;

    return pPackage;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataPackage::Read(BeFileNameCR filename)
    {
    //&&MM todo
    return RealityPackageStatus::UnknownError;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataPackage::Write(BeFileNameCR filename)
    {
    //&&MM todo
    return RealityPackageStatus::UnknownError;
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


