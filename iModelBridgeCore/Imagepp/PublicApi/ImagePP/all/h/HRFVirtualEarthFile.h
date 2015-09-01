//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFVirtualEarthFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFVirtualEarthFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include <ImagePP/h/HArrayAutoPtr.h>
#include <Imagepp/all/h/HFCHTTPConnection.h>
#include "HFCMacros.h"
#include "HFCSemaphore.h"
#include "HRFCacheFileSharing.h"
#include "HRFException.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFTilePool.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelType;
class HRFVirtualEarthTileReaderThread;

// The most common ImagerySet that can be used with HRFVirtualEarthFile::ComposeURL. 
// See Bing Maps REST API(http://msdn.microsoft.com/en-us/library/ff701721.aspx/)
#define BINGMAPS_IMAGERYSET_ROAD                L"Road"
#define BINGMAPS_IMAGERYSET_AERIAL              L"Aerial"
#define BINGMAPS_IMAGERYSET_AERIALWITHLABELS    L"AerialWithLabels"
// Less common:
//#define BINGMAPS_IMAGERYSET_COLLINSBART      L"CollinsBart"       // This imagery is visible only for the London area.
//#define BINGMAPS_IMAGERYSET_ORDNANCESURVEY   L"OrdnanceSurvey"    // This imagery is visible only for the London area.

//-----------------------------------------------------------------------------
// HRFVirtualEarthCapabilities class
//-----------------------------------------------------------------------------
class HRFVirtualEarthCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFVirtualEarthCapabilities();
    };

//-----------------------------------------------------------------------------
// HRFVirtualEarthConnection class
//-----------------------------------------------------------------------------
class HRFVirtualEarthConnection : public HFCHTTPConnection
    {
public:
    HRFVirtualEarthConnection(const WString& pi_rServer,
                              const WString& pi_Usr = L"",
                              const WString& pi_Pwd = L"");

    HRFVirtualEarthConnection(const HRFVirtualEarthConnection& pi_rObj);
    virtual ~HRFVirtualEarthConnection();

    void  NewRequest();
    bool RequestEnded() const;

protected:
    //--------------------------------------
    // methods
    //--------------------------------------

    //Called when a request is finished
    virtual void RequestHasEnded(bool pi_Success);

private:
    HFCInterlockedValue<bool> m_RequestEnded;

    //Disable method
    HRFVirtualEarthConnection& operator=(const HRFVirtualEarthConnection);
    };

//-----------------------------------------------------------------------------
// HRFVirtualEarthTileRequest struct
//-----------------------------------------------------------------------------
struct HRFVirtualEarthTileRequest
{
    public:
        uint64_t              m_TileID;
        uint64_t              m_PosX;
        uint64_t              m_PosY;
        int                    m_LevelOfDetail;
        uint32_t               m_BlockWidth;
        uint32_t               m_BlockHeight;
        uint32_t               m_BlockSizeInBytes;
        uint32_t               m_BytesPerBlockWidth;
        uint32_t               m_Page;
        HFCPtr<HRPPixelType>   m_PixelType;
};


//-----------------------------------------------------------------------------
// HRFVirtualEarthFile class
//-----------------------------------------------------------------------------
class HRFVirtualEarthFile : public HRFRasterFile
    {
public:
    friend class HRFVirtualEarthEditor;
    friend class HRFVirtualEarthTileReaderThread;

    //Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_VirtualEarth, HRFRasterFile)

    struct CoverageArea
        {
        int     levelOfDetailMin;   // from 1 (lowest detail)
        int     levelOfDetailMax;   // from 1 (lowest detail)
        double   southLatitude;
        double   westLongitude;
        double   northLatitude;
        double   eastLongitude;
        };

    struct ImageryProvider
        {
        WString attribution;
        BentleyApi::bvector<CoverageArea> coverage;
        };        

    typedef BentleyApi::bvector<ImageryProvider> ImageryProviders;

    HRFVirtualEarthFile(const HFCPtr<HFCURL>& pi_rpURL,
                        HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                        uint64_t             pi_Offset = 0);

    virtual     ~HRFVirtualEarthFile();

    //File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() const;

    //File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator() const;

    //File manipulation
    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t      pi_Page,
                                                                 unsigned short pi_Resolution,
                                                                 HFCAccessMode pi_AccessMode);

    virtual void                          Save();

    //Private utility method
    virtual uint64_t                     GetFileCurrentSize() const;
    virtual uint64_t                     GetFileCurrentSize(HFCBinStream* pi_pBinStream) const;

    //Look ahead method.
    virtual bool                         CanPerformLookAhead(uint32_t pi_Page) const;

    //This static method indicates if a plain URL can be represented
    //by this specific URL
    IMAGEPP_EXPORT static bool IsURLVirtualEarth(HFCURL const& pi_pURL);

    // Compose virtual earth pseudo URL. This is not a valid URL but a specially formatted URL to 
    // identify it as virtual earth URL and which imagery set to use.
    IMAGEPP_EXPORT static WString ComposeURL(WStringCR imagerySet);

    // Resolve map style from pseudo URL created by ComposeVirtualEarthURL.
    IMAGEPP_EXPORT static bool FindImagerySetFromURL(WStringR imagerySet, HFCURL const& pi_pURL);

    // Get the logo URL
    IMAGEPP_EXPORT WStringCR GetBrandLogoURI() const;

    // Get the lists of provider for the specified extent and zoom
    IMAGEPP_EXPORT ImageryProviders const& GetProviders() const;

protected:

    //Constructor use only to create a child
    HRFVirtualEarthFile (const HFCPtr<HFCURL>& pi_rpURL,
                         HFCAccessMode         pi_AccessMode,
                         uint64_t             pi_Offset,
                         bool                 pi_DontOpenFile);
    virtual void                        CreateDescriptors   ();

    //Use by the File and the Editor

    //Thumbnail interface
    HFCPtr<HRFThumbnail>                ReadThumbnailFromFile (uint32_t pi_Page);
    void                                AddThumbnailToFile    (uint32_t pi_Page);

    void                                AddResolutionToFile   (uint32_t pi_Page,
                                                               unsigned short pi_Resolution);

    void                                WritePixelTypeAndCodecToFile(
        uint32_t                    pi_Page,
        const HFCPtr<HRPPixelType>& pi_rpPixelType,
        const HFCPtr<HCDCodec>&     pi_rpCodec,
        uint32_t                    pi_BlockWidth,
        uint32_t                    pi_BlockHeight);

    void                                WritePaletteToFile    (uint32_t                 pi_Page,
                                                               const HRPPixelPalette&   pi_rPalette);

    HFCPtr<HGF2DTransfoModel>           CreateTransfoModelFromTiffMatrix() const;
    
    //--------------------------------------
    // LookAhead Methods
    //--------------------------------------
    //This method is used to determine if the file format supports look ahead
    //by block.
    virtual bool HasLookAheadByBlock(uint32_t pi_Page) const;

    //This method is used in SetLookAhead to give the list of needed tiles
    //to a derived class, since it knows how to obtain the tiles.
    virtual void RequestLookAhead(uint32_t             pi_Page,
                                  const HGFTileIDList& pi_rBlocks,
                                  bool                pi_Async);

    //This method is used in SetLookAhead to indicate to a derived class that
    //the current LookAhead has been cancelled.
    virtual void CancelLookAhead(uint32_t              pi_Page);

private:

    friend struct HRFVirtualEarthCreator;

    //Threading look ahead related member
    void StartReadingThreads();

    HFCExclusiveKey   m_HRFKey;
    HFCExclusiveKey   m_RequestKey;
        list<HRFVirtualEarthTileRequest> m_RequestList;
    HFCSemaphore      m_RequestEvent;

    void                QueryImageURI(WStringCR bingMapKey);
    HFCPtr<HFCBuffer>   SendAndReceiveRequest(WStringCR URLRequest) const;
    WString             GetTileURI(unsigned int pixelX, unsigned int pixelY, int levelOfDetail) const;

    WString             m_ImageURI;               // ex: "http://{subdomain}.tiles.virtualearth.net/tiles/r{quadkey}.jpeg?g=266&mkt={culture}"
    BentleyApi::bvector<WString>    m_ImageURISubdomains;     // ex: "t0","t1","t2","t3"

    ImageryProviders    m_Providers;
    WString             m_LogoURI;                // ex: "http://dev.virtualearth.net/Branding/logo_powered_by.png"

    HArrayAutoPtr<HAutoPtr<HRFVirtualEarthTileReaderThread>> m_ppBlocksReadersThread;
    HRFTilePool                                              m_TilePool;

    //Disabled methods
    HRFVirtualEarthFile(const HRFVirtualEarthFile& pi_rObj);
    HRFVirtualEarthFile& operator=(const HRFVirtualEarthFile& pi_rObj);
    };


//-----------------------------------------------------------------------------
// HRFVirtualEarthCreator class
//-----------------------------------------------------------------------------
struct HRFVirtualEarthCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                   uint64_t             pi_Offset = 0) const;
    virtual bool                     SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const;

    // Identification information
    virtual WString                   GetLabel() const;
    virtual WString                   GetSchemes() const;
    virtual WString                   GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;

private:

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFVirtualEarthCreator)

    // Disabled methodes
    HRFVirtualEarthCreator();
    };
END_IMAGEPP_NAMESPACE
