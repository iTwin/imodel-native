//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFOGCService.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Support all type of OGC files. (WMS and WCS)
//-----------------------------------------------------------------------------
#pragma once

#include "HFCAccessMode.h"

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFTilePool.h"

#include <Imagepp/all/h/HFCHTTPConnection.h>

BEGIN_IMAGEPP_NAMESPACE
class HFCURL;

//-----------------------------------------------------------------------------
// HRFOGCServiceCapabilities class
//-----------------------------------------------------------------------------
class HRFOGCServiceCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFOGCServiceCapabilities();
    };

//-----------------------------------------------------------------------------
// HRFOGCServiceConnection class
//-----------------------------------------------------------------------------
class HRFOGCServiceConnection : public HFCHTTPConnection
    {
public:
    HRFOGCServiceConnection(const WString& pi_rServer,
                            const WString& pi_Usr = L"",
                            const WString& pi_Pwd = L"");

    HRFOGCServiceConnection(const HRFOGCServiceConnection& pi_rObj);
    virtual ~HRFOGCServiceConnection();

    void    NewRequest();
    bool   RequestEnded() const;

protected:
    //--------------------------------------
    // methods
    //--------------------------------------

    // called when a request is finished
    virtual void    RequestHasEnded(bool pi_Success);

private:
    HFCInterlockedValue<bool>  m_RequestEnded;

    // disable methods
    HRFOGCServiceConnection& operator=(const HRFOGCServiceConnection);
    };

//-----------------------------------------------------------------------------
// HRFOGCServiceBlockCapabilities class
//-----------------------------------------------------------------------------
class HRFOGCServiceBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFOGCServiceBlockCapabilities();

    };

//-----------------------------------------------------------------------------
// HRFOGCServiceCodecCapabilities class
//-----------------------------------------------------------------------------
class HRFOGCServiceCodecCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFOGCServiceCodecCapabilities();
    };

//-----------------------------------------------------------------------------
// HRFOGCService class
//-----------------------------------------------------------------------------
class HRFOGCService : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFOGCServiceId, HRFRasterFile)

    friend class HRFOGCServiceEditor;
    friend class BlockReaderThread;

    virtual                                 ~HRFOGCService();

    // File information
    virtual const HGF2DWorldIdentificator   GetWorldIdentificator   () const;

    // File manipulation
    virtual bool                           AddPage                 (HFCPtr<HRFPageDescriptor>  pi_pPage);

    virtual HRFResolutionEditor*            CreateUnlimitedResolutionEditor  (uint32_t                   pi_Page,
                                                                              double                    pi_Resolution,
                                                                              HFCAccessMode              pi_AccessMode);

    virtual HRFResolutionEditor*            CreateResolutionEditor  (uint32_t                   pi_Page,
                                                                     unsigned short            pi_Resolution,
                                                                     HFCAccessMode              pi_AccessMode);

    virtual void                            Save();

    bool                                   HasLookAheadByBlock     (uint32_t                   pi_Page) const;

    virtual bool                           CanPerformLookAhead     (uint32_t                   pi_Page) const;




protected:

    typedef enum
        {
        WCS,
        WMS
        } OGCServiceType;

    HRFOGCService           (const HFCPtr<HFCURL>&      pi_rpURL,
                             OGCServiceType             pi_OGCServiceType,
                             HFCAccessMode              pi_AccessMode,
                             uint64_t                  pi_Offset);

    virtual void                            CreateDescriptors       (uint64_t                  pi_Width,
                                                                     uint64_t                  pi_Height) = 0;

    void                                    ComputeResolutionSize   (double                    pi_Resolution,
                                                                     uint64_t*                   po_pResWidth,
                                                                     uint64_t*                   po_pResHeight) const;

    void                                    RequestLookAhead        (uint32_t                   pi_Page,
                                                                     const HGFTileIDList&       pi_rBlocks,
                                                                     bool                      pi_Async);

    void                                    CancelLookAhead         (uint32_t                   pi_Page);


    struct BitmapDimension
        {
        uint64_t    m_Width;
        uint64_t    m_Height;
        };

    // Members
    HAutoPtr<HFCBinStream>      m_pFile;

    // OGC file tags
    string                      m_BaseRequest;
    string                      m_Format;
    double                     m_BBoxMinX;
    double                     m_BBoxMinY;
    double                     m_BBoxMaxX;
    double                     m_BBoxMaxY;
    uint64_t                   m_Width;
    uint64_t                   m_Height;
    WString                     m_ServerURL;
    WString                     m_URLSearchPart;
    string                      m_Request;
    OGCServiceType              m_OGCServiceType;



    // image size
    BitmapDimension             m_MinImageSize;
    BitmapDimension             m_MaxImageSize;

    // server capabilities
    BitmapDimension             m_MaxBitmapSize;

    HFCPtr<HRPPixelType>        m_pPixelType;
    unsigned short             m_GTModelType;
    HFCPtr<HGF2DTransfoModel>   m_pModel;

    HFCPtr<HRFOGCServiceConnection>    m_pConnection;
    bool                       m_NeedAuthentification;

    uint32_t                    m_ConnectionTimeOut;

    HRFTilePool                 m_TilePool;

    HFCPtr<HGF2DTransfoModel>   CreateTransfoModel(GeoCoordinates::BaseGCSP pi_pGeocoding, uint64_t pi_Width, uint64_t pi_Height);


    enum AUTHENTICATION_STATUS
        {
        AUTH_SUCCESS,
        AUTH_PERMISSION_DENIED,
        AUTH_USER_CANCELLED,
        AUTH_MAX_RETRY_COUNT_REACHED,
        };

    virtual  AUTHENTICATION_STATUS       AuthorizeConnection();

    AUTHENTICATION_STATUS AuthorizeConnectionSpecificRequest(string SpecificRequest);

    // Methods Disabled
    HRFOGCService                        (const HRFOGCService& pi_rObj);
    HRFOGCService&             operator= (const HRFOGCService& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
