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

class HRPPixelType;
class HRFVirtualEarthTileReaderThread;

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
    HDECLARE_CLASS_ID(1500, HRFRasterFile)

    enum MapStyle
        {
        MAPSTYLE_Aerial = 0,
        MAPSTYLE_AerialWihtLabels,
        MAPSTYLE_Road,
        };

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
    _HDLLg  static bool                  IsURLVirtualEarth(const HFCURL* pi_pURL);

        // Compose virtual earth pseudo URL. This is not a valid URL but a specially formatted URL to 
        // identify it as virtual earth URL and which map style to use.
    _HDLLg  static WString                ComposeVirtualEarthURL(MapStyle const& pi_Style);

        // Resolve map style from pseudo URL created by ComposeVirtualEarthURL.
    _HDLLg  static MapStyle               FindMapStyleFromURL(WString const& pi_pURL);

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


    void                QueryImageURI();
    HFCPtr<HFCBuffer>   SendAndReceiveRequest(WStringCR URLRequest) const;
    bool                DetectImagerySetFromURL(WStringR po_imagerySetLabel, HFCURL const& pi_URL) const;
    bool                ReplaceTagInString(WStringR str, WString const& tag, WStringCR tagValue) const;
    WString             GetTileURI(unsigned int pixelX, unsigned int pixelY, int levelOfDetail) const;

    WString             m_ImageURI;               // ex: "http://{subdomain}.tiles.virtualearth.net/tiles/r{quadkey}.jpeg?g=266&mkt={culture}"
    bvector<WString>    m_ImageURISubdomains;     // ex: "t0","t1","t2","t3"

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

    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HRFVirtualEarthCreator)

    // Disabled methodes
    HRFVirtualEarthCreator();
    };
