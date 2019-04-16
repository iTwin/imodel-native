//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFOGCServiceFile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRFOGCService.h>
#include "HRFOGCServiceEditor.h"

#include <ImagePP/all/h/HFCException.h>

#include <ImagePP/all/h/HGF2DStretch.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HGF2DProjective.h>
#include <ImagePP/all/h/HGF2DTranslation.h>
#include <ImagePP/all/h/HGF2DSimilitude.h>
#include <ImagePP/all/h/HGF2DAffine.h>

#include <ImagePP/all/h/HRFUtility.h>
#include <ImagePP/all/h/HRFException.h>
#include <ImagePP/all/h/HRFRasterFileCapabilities.h>

#include <ImagePP/all/h/ImagePPMessages.xliff.h>

#include <ImagePP/all/h/HFCURLMemFile.h>
#include <ImagePP/all/h/HFCMemoryBinStream.h>
#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>

#include <ImagePP/all/h/HFCCallbacks.h>
#include <ImagePP/all/h/HFCCallbackRegistry.h>

#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HCPGeoTiffKeys.h>
#include <ImagePP/all/h/HCPGCoordUtility.h>
#include <ImagePPInternal/HttpConnection.h>

//-----------------------------------------------------------------------------
// class OGRAuthenticationError
//
// Authentication error type specific to OGC files.
//-----------------------------------------------------------------------------
class OGRAuthenticationError : public HFCAuthenticationError
    {
public:
    explicit OGRAuthenticationError(const HFCInternetConnectionException& pi_rException)
        :   m_RelatedException(pi_rException)            
        {
        }

private:
    virtual Utf8String _ToString() const override
        {
        return m_RelatedException.GetExceptionMessage();
        }

    virtual void _Throw() const override
        {
        throw m_RelatedException;
        }

    HFCInternetConnectionException m_RelatedException;
    }; 


//-----------------------------------------------------------------------------
// Capabilities
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HRFOGCServiceCapabilities
//-----------------------------------------------------------------------------
HRFOGCServiceCapabilities::HRFOGCServiceCapabilities()
    : HRFRasterFileCapabilities()
    {
    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(GeographicType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSTypeLong);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // MultiResolution Capability
    Add(new HRFMultiResolutionCapability(HFC_READ_ONLY,
                                         true,          // SinglePixelType,
                                         true,          // SingleBlockType,
                                         false,         // ArbitaryXRatio,
                                         false,         // ArbitaryYRatio
                                         true,          // XYRatioLocked, default value
                                         16,            // smallest res width, default value
                                         16,            // smallest res height, default value
                                         UINT64_MAX,   // biggest res width
                                         UINT64_MAX,   // biggest res height
                                         true));        // UnlimitesResolution

    // Transfo model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DAffine::CLASS_ID));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Geocoding capability
    Add(new HRFGeocodingCapability(HFC_READ_ONLY));
    }

//-----------------------------------------------------------------------------
// HRFOGCServiceBlockCapabilities
//-----------------------------------------------------------------------------
HRFOGCServiceBlockCapabilities::HRFOGCServiceBlockCapabilities ()
    :HRFRasterFileCapabilities()
    {
    Add(new HRFTileCapability(HFC_READ_ONLY,    // AccessMode
                              INT32_MAX,          // MaxSizeInBytes
                              256,              // MinWidth
                              256,              // MaxWidth
                              0,                // WidthIncrement
                              256,              // MinHeight
                              256,              // MaxHeight
                              0));              // HeightIncrement
    }
//-----------------------------------------------------------------------------
// HRFOGCServiceCodecCapabilities
//-----------------------------------------------------------------------------
HRFOGCServiceCodecCapabilities::HRFOGCServiceCodecCapabilities()
    : HRFRasterFileCapabilities()
    {
    Add (new HRFCodecCapability (HFC_READ_ONLY,
                                 HCDCodecIdentity::CLASS_ID,
                                 new HRFOGCServiceBlockCapabilities()));
    }

//-----------------------------------------------------------------------------
// class HFCOGCService
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFOGCService::~HRFOGCService()
    {
    }

//-----------------------------------------------------------------------------
// public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFOGCService::CreateResolutionEditor(uint32_t       pi_Page,
                                                           uint16_t pi_Resolution,
                                                           HFCAccessMode  pi_AccessMode)
    {
    HASSERT(0);
    return 0;
    }

//-----------------------------------------------------------------------------
// public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFOGCService::CreateUnlimitedResolutionEditor(uint32_t       pi_Page,
                                                                    double        pi_Resolution,
                                                                    HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);

    HRFResolutionEditor* pEditor = 0;

    //create the editor for the OGC formats
    pEditor = new HRFOGCServiceEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFOGCService::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // not supported, read only format
    HASSERT(0);
    return false;
    }

//-----------------------------------------------------------------------------
// Public GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------

const HGF2DWorldIdentificator HRFOGCService::GetWorldIdentificator () const
    {
    HPRECONDITION(CountPages() > 0);

    HGF2DWorldIdentificator World = HGF2DWorld_GEOTIFFUNKNOWN;

    switch (m_GTModelType)
        {
        case TIFFGeo_ModelTypeProjected:
            World = HGF2DWorld_HMRWORLD;
            break;

        case TIFFGeo_ModelTypeGeographic:
            World = HGF2DWorld_GEOGRAPHIC;
            break;

        case TIFFGeo_ModelTypeSpecialMSJ:
        case TIFFGeo_ModelTypeSpecialMSSE:
            World = HGF2DWorld_INTERGRAPHWORLD;
            break;
        }

    return World;
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFOGCService::Save()
    {
    HASSERT(!"HRFOGCService::Save():OGC formats are read only");
    }

//-----------------------------------------------------------------------------
// Public
// CanPerformLookAhead
//
//-----------------------------------------------------------------------------
bool HRFOGCService::CanPerformLookAhead(uint32_t pi_Page) const
    {
    return true;

    }

//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// protected
// HRFOGCService
//-----------------------------------------------------------------------------
HRFOGCService::HRFOGCService (const HFCPtr<HFCURL>& pi_rpURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset)
    : HRFRasterFile(pi_rpURL,
                    pi_AccessMode,
                    pi_Offset)
    {
    }

//-----------------------------------------------------------------------------
// protected
// ComputeResolutionSize
//-----------------------------------------------------------------------------
void HRFOGCService::ComputeResolutionSize(double  pi_Resolution,
                                          uint64_t*  po_pResWidth,
                                          uint64_t*  po_pResHeight) const
    {
    HPRECONDITION(po_pResWidth != 0);
    HPRECONDITION(po_pResHeight != 0);
    HPRECONDITION(CountPages() == 1);   // multipage not supported
    HPRECONDITION(GetPageDescriptor(0)->CountResolutions() > 0);

    HFCPtr<HRFResolutionDescriptor> pResDesc(GetPageDescriptor(0)->GetResolutionDescriptor(0));

    *po_pResWidth = (uint64_t)((double)pResDesc->GetWidth() * pi_Resolution);
    *po_pResHeight = (uint64_t)((double)pResDesc->GetHeight() * pi_Resolution);
    }

//-----------------------------------------------------------------------------
// protected
// RequestLookAhead
//
// HRFOGCService support the LookAhead by extent. This means that all blocks
// must be on the same resolution.
//-----------------------------------------------------------------------------
void HRFOGCService::RequestLookAhead(uint32_t              pi_Page,
                                     const HGFTileIDList&  pi_rBlocks,
                                     bool                 pi_Async)
    {
    HGFTileIDList::const_iterator Itr(pi_rBlocks.begin());
    if (Itr != pi_rBlocks.end())
        {
        uint16_t Resolution = (uint16_t)HRFRasterFile::s_TileDescriptor.GetLevel(*Itr);   // the maximum level is 255

        // find the resolution editor into the ResolutionEditorRegistry
        ResolutionEditorRegistry::const_iterator ResItr(m_ResolutionEditorRegistry.begin());
        HRFOGCServiceEditor* pResEditor = 0;
        while (pResEditor == 0 && ResItr != m_ResolutionEditorRegistry.end())
            {
            if ((*ResItr)->GetPage() == pi_Page && (*ResItr)->GetResolutionIndex() == Resolution)
                pResEditor = (HRFOGCServiceEditor*)*ResItr;
            else
                ResItr++;
            }
        // the editor must be exist
        HASSERT(pResEditor != 0);

        pResEditor->RequestLookAhead(pi_rBlocks);
        }
    }

//-----------------------------------------------------------------------------
// protected
// CancelLookAhead
//-----------------------------------------------------------------------------
void HRFOGCService::CancelLookAhead(uint32_t pi_Page)
    {
    HPRECONDITION(pi_Page == 0);

    // notify all resolution
    ResolutionEditorRegistry::const_iterator ResItr(m_ResolutionEditorRegistry.begin());
    while (ResItr != m_ResolutionEditorRegistry.end())
        {
        if ((*ResItr)->GetPage() == pi_Page)
            {
            ((HRFOGCServiceEditor*)(*ResItr))->CancelLookAhead();
            break;
            }

        ResItr++;
        }
    }

//-----------------------------------------------------------------------------
// private section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//This method create a HGF2DTransfoModel using the information found in the
//geo tiff file tag.
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFOGCService::CreateTransfoModel(GeoCoordinates::BaseGCSP pi_geocoding, uint64_t pi_Width, uint64_t pi_Height)
    {
    HFCPtr<HGF2DTransfoModel> pTransfoModel = new HGF2DIdentity();

    bool IsValidModel = true;
    bool NeedFlip     = true;

    double PixelScaleX = fabs((m_BBoxMaxX - m_BBoxMinX)) / (double)pi_Width;
    double PixelScaleY = fabs(m_BBoxMaxY - m_BBoxMinY) / (double)pi_Height;


    pTransfoModel = new HGF2DStretch(HGF2DDisplacement (m_BBoxMinX, m_BBoxMinY+(pi_Height*PixelScaleY)),
                                     PixelScaleX,
                                     PixelScaleY);

    // Get the simplest model possible.
    HFCPtr<HGF2DTransfoModel> pTempTransfoModel = pTransfoModel->CreateSimplifiedModel();

    if (pTempTransfoModel != 0)
        pTransfoModel = pTempTransfoModel;


    // Flip the model if needed
    if (IsValidModel && NeedFlip)
        {
        // Flip the Y Axe because the origin of ModelSpace is lower-left
        HFCPtr<HGF2DStretch> pFlipModel = new HGF2DStretch();

        pFlipModel->SetYScaling(-1.0);
        pTransfoModel = pFlipModel->ComposeInverseWithDirectOf(*pTransfoModel);
        }

    if (pi_geocoding != nullptr && pi_geocoding->IsValid())
        pTransfoModel = HCPGCoordUtility::TranslateToMeter(pTransfoModel,
                                                           1.0 / pi_geocoding->UnitsFromMeters());

    // OGC Services never specify units but rely on the EPSG code of a GCS
    SetUnitFoundInFile(false);

    return pTransfoModel;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                  
//----------------------------------------------------------------------------------------
void HRFOGCService::ValidateConnection(bool authenticate)
    {
    HttpSession session;
    HttpRequest request(*m_requestTemplate);
    request.SetUrl(m_requestTemplate->GetUrl() + _GetValidateConnectionRequest());

    if(!authenticate)
        {
        request.SetConnectOnly(true);
        HttpResponsePtr response;
        HttpRequestStatus ReqStatus = session.Request(response, request);
        if(HttpRequestStatus::Success != ReqStatus)
            throw HFCInternetConnectionException("HTTP", HFCInternetConnectionException::CANNOT_CONNECT);

        return;
        }

    // Full validation with authentication.
    const HFCAuthenticationCallback* pCallback = nullptr;
    HAutoPtr<HFCAuthentication> pAuthentication;

    bool connectionSucceed = false;
    bool TryAgain = false;
    do
        {
        TryAgain = false;

        HFCInternetConnectionException::ErrorType connectErrorType = HFCInternetConnectionException::CANNOT_CONNECT;

        HttpResponsePtr response;
        HttpRequestStatus ReqStatus = session.Request(response, request);
        if(HttpRequestStatus::Success == ReqStatus)
            {
            connectionSucceed = true;
            break;            
            }       

        if(response.IsValid() && response->GetStatus() == HttpResponseStatus::Unauthorized)
            {
            connectErrorType = HFCInternetConnectionException::PERMISSION_DENIED;
            }
        else if(response.IsValid() && response->GetStatus() == HttpResponseStatus::ProxyAuthenticationRequired)
            {
            connectErrorType = HFCInternetConnectionException::PROXY_PERMISSION_DENIED;
            }
        else
            {
            connectionSucceed = false;  // failed for whatever reasons.
            break;
            }     
                
        if (pCallback == nullptr)
            {
            if (HFCInternetConnectionException::PERMISSION_DENIED == connectErrorType)
                {
                pCallback = HFCAuthenticationCallback::GetCallbackFromRegistry(HFCInternetAuthentication::CLASS_ID);
                }
            else if (HFCInternetConnectionException::PROXY_PERMISSION_DENIED == connectErrorType)
                {
                pCallback = HFCAuthenticationCallback::GetCallbackFromRegistry(HFCProxyAuthentication::CLASS_ID);
                BeAssert(pCallback->CanAuthenticate(HFCProxyAuthentication::CLASS_ID));
                }
            else
                {
                BeAssert(0);
                }
            }       

        if (pCallback != nullptr)
            {
            const uint16_t MaxRetryCount = pCallback->RetryCount(HFCAuthenticationCallback::CLASS_ID);

            if (HFCInternetConnectionException::PERMISSION_DENIED == connectErrorType)
                {
                if (pAuthentication == nullptr || !pAuthentication->IsCompatibleWith(HFCInternetAuthentication::CLASS_ID))
                    {
                    pAuthentication = new HFCInternetAuthentication(request.GetUrl(),
                                                                    request.GetCredentials().GetUsername(),
                                                                    request.GetCredentials().GetPassword());
                    }
                }
            else
                {
                if (pAuthentication == nullptr || !pAuthentication->IsCompatibleWith(HFCProxyAuthentication::CLASS_ID))
                    pAuthentication = new HFCProxyAuthentication;
                }

            if (pAuthentication->GetRetryCount() < MaxRetryCount)
                {
                HFCPtr<HFCAuthenticationError> pAuthError(new OGRAuthenticationError(HFCInternetConnectionException("HTTP", connectErrorType)));
                pAuthentication->PushLastError(pAuthError);

                if (pCallback->GetAuthentication(pAuthentication))
                    {
                    if (pAuthentication->IsCompatibleWith(HFCInternetAuthentication::CLASS_ID))
                        {
                        Credentials newCredentials(((HFCInternetAuthentication*)pAuthentication.get())->GetUser().c_str(), 
                                                    ((HFCInternetAuthentication*)pAuthentication.get())->GetPassword().c_str());
                        request.SetCredentials(newCredentials);
                        }
                    else
                        {
                        Credentials newCredentials(((HFCProxyAuthentication*)pAuthentication.get())->GetUser().c_str(), 
                                                    ((HFCProxyAuthentication*)pAuthentication.get())->GetPassword().c_str());
                        request.SetProxyCredentials(newCredentials);
                        }

                    pAuthentication->IncrementRetryCount();
                    TryAgain = true;
                    }
                else
                    {
                    if (pCallback->IsCancelled())
                        throw HRFAuthenticationCancelledException(GetURL()->GetURL());
                    }
                }
            else
                {
                throw HRFAuthenticationMaxRetryCountReachedException(GetURL()->GetURL());
                }
            }
        }
    while (TryAgain);
    
    if(connectionSucceed)
        {
        m_requestTemplate->SetCredentials(request.GetCredentials());
        m_requestTemplate->SetProxyCredentials(request.GetProxyCredentials());
        }
    else
        {
        throw HFCInternetConnectionException("HTTP", HFCInternetConnectionException::CANNOT_CONNECT);
        }
    }

//-----------------------------------------------------------------------------
// Public
// HasLookAhead
//-----------------------------------------------------------------------------
bool HRFOGCService::HasLookAheadByBlock(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    return true;
    }
