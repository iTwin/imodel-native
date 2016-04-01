//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFOGCService.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Support all type of OGC files. (WMS and WCS)
//-----------------------------------------------------------------------------
#pragma once

#include "HRFRasterFile.h"

BEGIN_IMAGEPP_NAMESPACE

class HFCURL;
struct HttpRequest;
struct HttpSession;
struct OGCBlockQuery;
struct OGCTile;
struct WorkerPool;
struct ThreadLocalHttp;
    
//-----------------------------------------------------------------------------
// HRFOGCServiceCapabilities class
//-----------------------------------------------------------------------------
class HRFOGCServiceCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFOGCServiceCapabilities();
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
    friend struct OGCBlockQuery;

    virtual                                 ~HRFOGCService();

    // File information
    virtual const HGF2DWorldIdentificator   GetWorldIdentificator   () const;

    // File manipulation
    virtual bool                           AddPage                 (HFCPtr<HRFPageDescriptor>  pi_pPage);

    virtual HRFResolutionEditor*            CreateUnlimitedResolutionEditor  (uint32_t                   pi_Page,
                                                                              double                    pi_Resolution,
                                                                              HFCAccessMode              pi_AccessMode);

    virtual HRFResolutionEditor*            CreateResolutionEditor  (uint32_t                   pi_Page,
                                                                     uint16_t            pi_Resolution,
                                                                     HFCAccessMode              pi_AccessMode);

    virtual void                            Save();

    bool                                   HasLookAheadByBlock     (uint32_t                   pi_Page) const;

    virtual bool                           CanPerformLookAhead     (uint32_t                   pi_Page) const;




protected:

    HRFOGCService           (const HFCPtr<HFCURL>&      pi_rpURL,
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

    virtual Utf8String _GetValidateConnectionRequest() const = 0;

    struct BitmapDimension
        {
        uint64_t    m_Width;
        uint64_t    m_Height;
        };

    // Members
    HAutoPtr<HFCBinStream>      m_pFile;

    // OGC file tags
    Utf8String                 m_BaseRequest;
    Utf8String                 m_Format;
    double                     m_BBoxMinX;
    double                     m_BBoxMinY;
    double                     m_BBoxMaxX;
    double                     m_BBoxMaxY;
    uint64_t                   m_Width;
    uint64_t                   m_Height;
    Utf8String                    m_ServerURL;
    Utf8String                 m_Request;

    // image size
    BitmapDimension             m_MinImageSize;
    BitmapDimension             m_MaxImageSize;

    // server capabilities
    BitmapDimension             m_MaxBitmapSize;

    HFCPtr<HRPPixelType>        m_pPixelType;
    uint16_t             m_GTModelType;
    HFCPtr<HGF2DTransfoModel>   m_pModel;

    bool                       m_NeedAuthentification;

    std::unique_ptr<HttpRequest> m_requestTemplate;
    uint32_t                    m_ConnectionTimeOut;

    HFCPtr<HGF2DTransfoModel>   CreateTransfoModel(GeoCoordinates::BaseGCSP pi_pGeocoding, uint64_t pi_Width, uint64_t pi_Height);

    void ValidateConnection(bool authenticate);
    
    // Methods Disabled
    HRFOGCService                        (const HRFOGCService& pi_rObj);
    HRFOGCService&             operator= (const HRFOGCService& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
