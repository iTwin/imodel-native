/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshAdmin.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <HIEStdcpp.h>
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_RASTER
USING_NAMESPACE_DGNPLATFORM_DGNHISTORY
using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;

#define ALMOST_WHITE(colorRGB) (colorRGB.red >= 250 && colorRGB.green >= 250 && colorRGB.blue >= 250)

//Uncomment to debug raster frame element range
#if defined (__HMR_DEBUG)
    //#define   DEBUG_DISPLAY
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterCoreAttachmentAdmin::_LoadRasters(DgnModelRefR modelRef)
    {
    // Load rasters in modelRef. If any raster elements exist they have been collected during cache loading(_OnElementLoaded)
    // We use GetDataP instead of PeekDataP because we need to create an object that will keep track of future attachment that may occur.
    RasterModelRefDataP modelRefDataP = RasterModelRefData::GetDataP(&modelRef);
    if(NULL != modelRefDataP)
        modelRefDataP->LoadRasters();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterCoreAttachmentAdmin::_OnRasterOverridesModified(RasterOverridesCollectionCR rasterOvrCollection, DgnAttachmentR ref)
    {
    DgnRasterCollectionR rasterCollection = DgnRasterCollection::GetRastersR(&ref);

    for(DgnRasterCollection::const_iterator itr(rasterCollection.begin()); itr != rasterCollection.end(); ++itr)
        {
        DgnRasterP rasterP = *itr;
        ElementHandle eh(rasterP->GetElementRef(), rasterP->GetModelRefP());

        // Resynch to element including overrides.
        rasterP->FromElement(eh, 0x00/*don't do Xattr*/);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Chantal.Poulin  02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformLib::Host::RasterAttachmentAdmin::PublishingAction RasterCoreAttachmentAdmin::_GetPublishingAction(ElementHandleCR rasterEh)
    {
    // if raster DEM
    if(RasterDEMFilters::HasDEMFilters(rasterEh))
        {
        // We don't publish Raster DEM if descartes is not loaded
        IModelRasterDEMOption rasterDEMOptions = RasterCoreLib::GetHost().GetRasterCoreAdmin()._GetIModelPublishingRasterDEMOption();
        if (rasterDEMOptions == RasterDEMOption_Exclude)
            return Publish_Exclude;
        else if (rasterDEMOptions == RasterDEMOption_FullSize)
            return Publish_KeepOriginal;
        }
    // other raster
    else
        {
        IModelRasterOption rasterOption = RasterCoreLib::GetHost().GetRasterCoreAdmin()._GetIModelPublishingRasterOption();

        if (rasterOption == RasterOption_Exclude)
            return Publish_Exclude;
        else if (rasterOption == RasterOption_KeepOriginal)
            return Publish_KeepOriginal; 
        }    
    
    return Publish_Embed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthentication::AuthenticationType IRasterAuthentication::GetAuthenticationType () const
    {
    return _GetAuthenticationType();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthenticationInternetP IRasterAuthentication::GetRasterAuthenticationInternetP()
    {
    return _GetRasterAuthenticationInternetP(); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthenticationProxyP IRasterAuthentication::GetRasterAuthenticationProxyP()
    {
    return _GetRasterAuthenticationProxyP(); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthenticationOracleP IRasterAuthentication::GetRasterAuthenticationOracleP()
    {
    return _GetRasterAuthenticationOracleP(); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthenticationPDFP IRasterAuthentication::GetRasterAuthenticationPDFP()
    {
    return _GetRasterAuthenticationPDFP(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HFCAuthentication* IRasterAuthentication::GetInternalP()
    {
    return _GetInternalP();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const HFCAuthentication* IRasterAuthentication::GetInternalCP() const
    {
    return _GetInternalCP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IRasterAuthentication::SetByString            (WCharCP pi_rAuthenticationString)
    {
    return _SetByString(pi_rAuthenticationString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool IRasterAuthentication::Failed () const
    {
    return _Failed();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IRasterAuthentication::GetErrorsQty () const
    {
    return _GetErrorsQty();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString IRasterAuthentication::GetLastError () const
    {
    return _GetLastError();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IRasterAuthentication::PopLastError () 
    {
    return _PopLastError();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IRasterAuthentication::ResetAllErrors () 
    {
    return _ResetAllErrors();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned short IRasterAuthentication::IncrementRetryCount ()
    {
    return _IncrementRetryCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned short IRasterAuthentication::GetRetryCount () const
    {
    return _GetRetryCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            IRasterAuthentication::_SetByString         (WCharCP pi_rAuthenticationString) {GetInternalP()->SetByString(pi_rAuthenticationString);}
bool            IRasterAuthentication::_Failed              () const    {return GetInternalCP()->Failed();}
size_t          IRasterAuthentication::_GetErrorsQty        () const    {return GetInternalCP()->GetErrorsQty();}
WString         IRasterAuthentication::_GetLastError        () const    {return WString(GetInternalCP()->GetLastError().ToString().c_str());}
void            IRasterAuthentication::_PopLastError        ()          { GetInternalP()->PopLastError();}
void            IRasterAuthentication::_ResetAllErrors      ()          {return GetInternalP()->ResetAllErrors();}
unsigned short  IRasterAuthentication::_IncrementRetryCount ()          {return GetInternalP()->IncrementRetryCount();}
unsigned short  IRasterAuthentication::_GetRetryCount       () const    {return GetInternalCP()->GetRetryCount();}
IRasterAuthenticationInternetP  IRasterAuthentication::_GetRasterAuthenticationInternetP()  { return NULL; }
IRasterAuthenticationProxyP     IRasterAuthentication::_GetRasterAuthenticationProxyP()     { return NULL; }
IRasterAuthenticationOracleP    IRasterAuthentication::_GetRasterAuthenticationOracleP()    { return NULL; }
IRasterAuthenticationPDFP       IRasterAuthentication::_GetRasterAuthenticationPDFP()       { return NULL; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthentication::AuthenticationType IRasterAuthenticationInternet::_GetAuthenticationType () const 
    {
    return IRasterAuthentication::Authentication_Internet; 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthenticationInternetP IRasterAuthenticationInternet::_GetRasterAuthenticationInternetP() { return this; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool        IRasterAuthenticationInternet::IsBingMapServer() const                      {return _IsBingMapServer();}
void        IRasterAuthenticationInternet::SetUser     (WCharCP          pi_User)       {_SetUser(pi_User);}
void        IRasterAuthenticationInternet::SetPassword (WCharCP          pi_Password)   {_SetPassword(pi_Password); }
WString     IRasterAuthenticationInternet::GetUser     () const                         { return _GetUser(); }
WString     IRasterAuthenticationInternet::GetPassword () const                         { return _GetPassword(); }
WString     IRasterAuthenticationInternet::GetServer   () const                         { return _GetServer();   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthentication::AuthenticationType IRasterAuthenticationProxy::_GetAuthenticationType () const 
    {
    return IRasterAuthentication::Authentication_Proxy; 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthenticationProxyP IRasterAuthenticationProxy::_GetRasterAuthenticationProxyP() { return this; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString     IRasterAuthenticationProxy::GetUser     () const                { return _GetUser();}
WString     IRasterAuthenticationProxy::GetPassword () const                { return _GetPassword();}
void        IRasterAuthenticationProxy::SetUser     (WCharCP pi_User)       { _SetUser(pi_User); }
void        IRasterAuthenticationProxy::SetPassword (WCharCP pi_Password)   { _SetPassword(pi_Password);}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthentication::AuthenticationType IRasterAuthenticationOracle::_GetAuthenticationType () const 
    {
    return IRasterAuthentication::Authentication_Oracle; 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthenticationOracleP IRasterAuthenticationOracle::_GetRasterAuthenticationOracleP() { return this; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString         IRasterAuthenticationOracle::GetUser                    () const    { return _GetUser();}
WString         IRasterAuthenticationOracle::GetPassword                () const    { return _GetPassword();}
WString         IRasterAuthenticationOracle::GetDatabaseName            () const    { return _GetDatabaseName();}
WString         IRasterAuthenticationOracle::GetConnectionString        () const    { return _GetConnectionString();}
void            IRasterAuthenticationOracle::SetUser     (WCharCP pi_User)          { _SetUser(pi_User); }
void            IRasterAuthenticationOracle::SetPassword (WCharCP pi_Password)      { _SetPassword(pi_Password);}
void            IRasterAuthenticationOracle::SetDatabaseName (WCharCP pi_DbName)    { _SetDatabaseName(pi_DbName);}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthentication::AuthenticationType IRasterAuthenticationPDF::_GetAuthenticationType () const 
    {
    return IRasterAuthentication::Authentication_PDF; 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterAuthenticationPDFP IRasterAuthenticationPDF::_GetRasterAuthenticationPDFP() { return this; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WString         IRasterAuthenticationPDF::GetPassword   () const                { return _GetPassword();}
void            IRasterAuthenticationPDF::SetPassword   (WCharCP pi_Password)   { _SetPassword(pi_Password);}
WString         IRasterAuthenticationPDF::GetFileName   () const                { return _GetFileName(); }
IRasterAuthenticationPDF::PDFPasswordType IRasterAuthenticationPDF::GetPasswordType() const               { return _GetPasswordType(); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ElementECExtension* RasterCoreAttachmentAdmin::_CreateElementECExtensions() const
    {
    return DgnRasterECExtension::Create ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DrapeSceneParamPtr  DrapeSceneParam::Create(ViewportCR viewport, MaterialIdCR materialId)    
    {
    DrapeSceneParamPtr paramP = new DrapeSceneParam(viewport,materialId);
    RasterCoreLib::GetHost().GetRasterCoreAdmin()._QueryDrapeSceneParam(*paramP);
    return paramP;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DrapeSceneParam::DrapeSceneParam(ViewportCR viewport, MaterialIdCR materialId)
:m_viewNumber(viewport.GetViewNumber()), m_rootModel(viewport.GetRootModel()), m_materialId(materialId), m_contrast(0), m_brightness(0), m_isGrayscale(false), m_drapeRasterState(true), m_resolutionQuality(100)   {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DrapeSceneParam::~DrapeSceneParam()  {}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int                 DrapeSceneParam::GetViewNumber() const          { return m_viewNumber; }
DgnModelP           DrapeSceneParam::GetRootModelP() const          {return m_rootModel; }
MaterialIdCR        DrapeSceneParam::GetMaterialId() const          {return m_materialId;}
Int8                DrapeSceneParam::GetContrast() const            {return m_contrast;}
void                DrapeSceneParam::SetContrast(Int8 value)        {m_contrast=value; }
Int8                DrapeSceneParam::GetBrightness() const          {return m_brightness;}
void                DrapeSceneParam::SetBrightness(Int8 value)      {m_brightness=value; }
bool                DrapeSceneParam::IsGrayscale() const            {return m_isGrayscale;}
void                DrapeSceneParam::SetIsGrayscale(bool value)     {m_isGrayscale=value; }
bool                DrapeSceneParam::GetDrapeRasterState()  const   {return m_drapeRasterState;}
void                DrapeSceneParam::SetDrapeRasterState(bool value){m_drapeRasterState=value; }
double              DrapeSceneParam::GetResolutionQuality() const   {return m_resolutionQuality;}
void                DrapeSceneParam::SetResolutionQuality(double value)   {m_resolutionQuality=value; }
