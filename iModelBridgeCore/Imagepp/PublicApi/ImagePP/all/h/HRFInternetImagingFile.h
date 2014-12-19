//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetImagingFile.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetImagingFile
//-----------------------------------------------------------------------------
#pragma once

#include "HRFRasterFile.h"
#include "HRFTilePool.h"
#include "HRFInternetDisconnectionThread.h"
#include "HRFInternetImagingException.h"
#include "HFCInternetConnection.h"
#include "HFCMacros.h"
#include <Imagepp/all/h/HFCExclusiveKey.h>
#include "HFCEvent.h"
#include "HFCMonitor.h"
#include "HFCBuffer.h"
#include "HFCVersion.h"
#include "HGF2DExtent.h"
#include "HGF2DLiteExtent.h"
#include "HGFTileIDDescriptor.h"
#include "HRPHistogram.h"

class HRFInternetImagingThread;
class HRFInternetImagingHandler;
class HRFInternetImagingException;
class HRFInternetImagingCancelledException;
class HPMAttributeSet;

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------

#define HRF_INTERNET_MAXIMUM_RESOLUTION     256


//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

// Handler List Type Declaration
typedef list<HFCPtr<HRFInternetImagingHandler> >
HandlerList;

// Jpeg Compression Table
class HRFInternetJpegTable : public HFCShareableObject<HRFInternetJpegTable>
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetJpegTable();
    ~HRFInternetJpegTable();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Sets the data in the class
    void            SetData(const Byte* pi_pData,
                            uint32_t     pi_DataSize);

    // Returns the data size enclosed
    uint32_t        GetDataSize() const;

    // Returns the data enclosed.  DataSize must be
    // greater than 0.
    const Byte*    GetData() const;

    // Clear the tables
    void            Clear();


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    HFCBuffer       m_Buffer;
    };


//------------------------------------------------------------------------------
// Class HRFInternetImagingFile
//------------------------------------------------------------------------------
class HRFInternetImagingFile : public HRFRasterFile
    {
    //--------------------------------------
    // Macros & Friends
    //--------------------------------------

    HDECLARE_CLASS_ID(1433, HRFRasterFile)

    // Receving Thread & Tile Editor
    friend class HRFInternetImagingThread;
    friend class HRFInternetImagingTileEditor;

    // Response Handlers
    friend class HRFInternetTileHandler;
    friend class HRFInternetHIPTileHandler;
    friend class HRFInternetErrorHandler;
    friend class HRFInternetROIHandler;
    friend class HRFInternetResolutionHandler;
    friend class HRFInternetCompGroupHandler;
    friend class HRFInternetTransformHandler;
    friend class HRFInternetColorspaceHandler;
    friend class HRFInternetMaxSizeHandler;
    friend class HRFInternetIIPVersionHandler;
    friend class HRFInternetHIPVersionHandler;
    friend class HRFInternetHIPTileSizeHandler;
    friend class HRFInternetHIPPixelTypeHandler;
    friend class HRFInternetHIPHistogramHandler;
    friend class HRFInternetHIPPaletteHandler;
    friend class HRFInternetHIPImageSizeHandler;
    friend class HRFInternetHIPFileInfoHandler;
    friend class HRFInternetHIPWorldIDHandler;
    friend class HRFInternetHIPLogicalShapeHandler;
    friend class HRFInternetHIPFileHandler;
    friend class HRFInternetHIPRedirectHandler;
    friend class HRFInternetHIPFileCompressionHandler;
    friend class HRFInternetHIPBackgroundHandler;
    friend class HRFInternetHIPFileTypeHandler;
    friend class HRFInternetAttributesHandler;
    friend class HRFInternetAttributeNamesHandler;
    friend class HRFInternetHIPPageNumberHandler;


    //--------------------------------------
    // Types
    //--------------------------------------

    typedef enum
        {
        ESTABLISH_PROTOCOL,
        DISCONNECTED,       // when a disconnection occured
        CONNECTING,         // when we are connecting and protocol is still unknown
        CONNECTING_HIP,     // when we are still receiving basic information (HIP)
        CONNECTING_IIP,     // when we are still receiving basic information (IIP)
        RECONNECTING,       // when we are trying to reconnect to the server
        CONNECTED           // when most of the time tile data is coming through
        } HRFInternetImagingState;


    //--------------------------------------
    // Constants
    //--------------------------------------

    static const uint32_t s_IIP_Protocol;
    static const uint32_t s_HIP_Protocol;
    static const uint32_t s_HIP_UTF8_Protocol;

public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetImagingFile(const HFCPtr<HFCURL>&  pi_rpURL,
                           HFCAccessMode          pi_AccessMode = HFC_READ_ONLY,
                           uint64_t              pi_Offset = 0,
                           const WString&         pi_rOpeningPrefix = WString());
    virtual         ~HRFInternetImagingFile();

    // Returns the file exlusive key
    virtual HFCExclusiveKey&              GetKey() const;


    //--------------------------------------
    // Methods
    //--------------------------------------

    // returns the type of the original file on the server
    virtual const WString&                      RequestOriginalFileType(uint32_t pi_TimeOut = 30000);

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities () const;

    // World attching the imaging
    virtual const HGF2DWorldIdentificator       GetWorldIdentificator () const;

    // File manipulation
    virtual bool                               PagesAreRasterFile() const;
    virtual HFCPtr<HRFRasterFile>               GetPageFile(uint32_t pi_Page) const;
    uint32_t                                    CountPages            () const;
    HFCPtr<HRFPageDescriptor>                   GetPageDescriptor     (uint32_t pi_Page) const;
    virtual bool                               AddPage(HFCPtr<HRFPageDescriptor> pi_pPage);

    // Editing
    virtual HRFResolutionEditor*                CreateResolutionEditor(uint32_t         pi_Page,
                                                                       unsigned short  pi_Resolution,
                                                                       HFCAccessMode    pi_AccessMode);

    virtual HRFResolutionEditor*                CreateUnlimitedResolutionEditor(uint32_t         pi_Page,
                                                                                double          pi_Resolution,
                                                                                HFCAccessMode    pi_AccessMode);

    virtual void                                Save();

    //--------------------------------------
    // LookAhead Methods
    //--------------------------------------

    // Indicates if the file supports LookAhead optimization
    virtual bool   HasLookAheadByBlock (uint32_t   pi_Page) const;

    // This method is used in SetLookAhead to verify if the derived class is
    // ready to receive LookAhead request.  It returns true by default.
    virtual bool   CanPerformLookAhead (uint32_t   pi_Page) const;

    // This method is used in SetLookAhead to give the list of needed tiles
    // to a derived class, since it knows how to obtain the tiles.
    virtual void    RequestLookAhead    (uint32_t               pi_Page,
                                         const HGFTileIDList&   pi_rBlocks,
                                         bool                  pi_Async = false);

    // This method is used in SetLookAhead to indicate to a derived class that
    // the current LookAhead has been cancelled.
    virtual void    CancelLookAhead     (uint32_t               pi_Page);

    virtual void    ResetLookAhead      (uint32_t               pi_Page,
                                         bool                  pi_Async);

    virtual void    StopLookAhead       (uint32_t               pi_Page,
                                         uint32_t               pi_ConsumerID);


    virtual void    SetLookAhead        (uint32_t               pi_Page,
                                         const HGFTileIDList&   pi_rBlocks,
                                         uint32_t               pi_ConsumerID,
                                         bool                  pi_Async);

    virtual void    SetLookAhead        (uint32_t               pi_Page,
                                         unsigned short        pi_Resolution,
                                         const HVEShape&        pi_rShape,
                                         uint32_t               pi_ConsumerID,
                                         bool                  pi_Async);


    //--------------------------------------
    // The Contrary to The Police Last album (Synchronicity)
    // Asynchronicity
    //--------------------------------------

    // Indicates if the file is asynchronous or not
    bool           IsAsynchronous() const;
    void            SetAsynchronous(bool pi_Async);


    //--------------------------------------
    // Progression Info
    //--------------------------------------

    static bool    HasTilePending();


    //--------------------------------------
    // HIP Small file caching
    //--------------------------------------


    // Indicates if the InternetImagingFile is caching a local file
    _HDLLg bool             HasLocalCachedFile() const;


    //--------------------------------------
    // Reconnection Settings
    //--------------------------------------

    virtual bool   Reconnect();


    //--------------------------------------
    // Utility methods
    //--------------------------------------

    // Stops blocked requests in other threads
    void            StopRequests();

    // Get the time stamp of the file
    time_t          GetCreationTime() const;
    time_t          GetLastAccessTime() const;
    time_t          GetModificationTime() const;

    // returns the protocol information for the current file
    const HFCVersion&
    GetProtocolVersion() const;

    // Returns the file size has returned by the server
    uint64_t       GetFileSize() const;

    // If the image does not have a histogram, this method request
    // a generated histogram from the server.
    bool           GenerateHistogram();

    // Force the downloading of HPMAttributes from the server
    _HDLLg bool    DownloadAttributes();
    bool           DownloadAttributes(const list<string>& pi_rAttributeNames);


    // Checks if a server is Online
    static bool    IsServerOnline(const HFCURL* pi_pURL,
                                   uint32_t      pi_TimeOut = 30000);

    // Checks if a server is HIP compatible
    static bool    IsServerHIPCompatible(const HFCURL* pi_pURL,
                                          uint32_t      pi_TimeOut = 30000);

    // Get the list of file formats supported by a HIP server
    static bool    GetSupportedFormats(const HFCURL*   pi_pURL,
                                        list<WString>*  po_pList,
                                        bool           pi_OnlyFileBased = false,
                                        uint32_t        pi_TimeOut = 30000);

    // Get the security token from a HIP server
    static bool    GetSecurityToken(const HFCURL* pi_pURL,
                                     string*       po_pToken,
                                     uint32_t      pi_TimeOut = 30000);

    void            SetWaitBlockOnRead(bool pi_WaitBlockOnRead);

    // Filter the setting of the default ratio to meter.
    virtual void    SetDefaultRatioToMeter(double pi_RatioToMeter,
                                           uint32_t pi_Page = 0,
                                           bool   pi_CheckSpecificUnitSpec = false,
                                           bool   pi_GeoModelDefaultUnit = true,
                                           bool   pi_InterpretUnitINTGR = false);
private:
    //--------------------------------------
    // Methods
    //--------------------------------------

    // special constructor
    // this constructor is use to support multi page
    HRFInternetImagingFile(const HRFInternetImagingFile*    pi_pFile,
                           uint32_t                         pi_Page);

    void            Constructor();

    // This methods waits until a value becomes available or
    // an error is signaled by the thread
    void            WaitForEvent(HFCEvent& pi_rEvent);
    bool           WaitForEvent(HFCEvent& pi_rEvent, int32_t pi_TimeOut);

    // Method to set the last exception that occurred in the thread
    void            SetThreadException(const HFCException& pi_rException);
    void            SetThreadException(const HRFInternetImagingException& pi_rException);
    void            SetThreadException(const HRFInternetImagingCancelledException& pi_rException);
    void            ClearThreadException();
    void            HandleThreadException();

    // Connects to a server.  The first time parameter indicates if
    // we want to receive the basic information or not
    void            ConnectToHost       (const HFCPtr<HFCURL>& pi_rpURL);

    bool           FindBestSupportingProtocol(bool pi_TryMultibyte);

    bool           TryHIPProtocol(const HFCVersion& pi_rVersion);

    bool           RequestFileWithHIPProtocol(const HFCPtr<HFCURL>& pi_rpURL);
    bool           RequestFileWithIIPProtocol(const HFCPtr<HFCURL>& pi_rpURL);

    void            ConstructEncodedInfoForRequest(const WString& pi_rPWCookie,
                                                   const string&  pi_rPWCookieUTF8Escaped);

    void            ExtractPWCookie(WString* po_PWCookie,
                                    string*  po_PWCookieUTF8Escaped);

    // Changes and inquires the state of the internet imaging file
    void            InitializeState(HRFInternetImagingState pi_State);
    void            SetState  (HRFInternetImagingState pi_State);
    HRFInternetImagingState
    GetState() const;

    // Setup oject for a protocol
    void            SetupForHIP(bool pi_UseUTF8Protocol);
    void            SetupForIIP();

    bool           IsHMRImagingProtocol(const HFCVersion& pi_rVersion) const;
    bool           IsInternetImagingProtocol(const HFCVersion& pi_rVersion) const;

    // Verify if the data is valid and default unecessary params
    bool           VerifyHIPBasicInfo();
    bool           VerifyIIPBasicInfo();
    bool           VerifyProtocolVersion() const;

    // Resets the image information
    void            ResetValues();

    // Tries to obtain a local cached file
    bool           GetLocalCachedFile();

    // Called when the page descriptor is to be built.  This
    // method will wait for data to arrive from the thread to
    // build the page and its resolutions
    virtual void    BuildDescriptors();

    // Called when a lone marker ("\r\n") is found so
    // that the file may changes its state.  Especially
    // useful when waiting for the basic information and
    // some may not be needed.
    virtual void    EndOfCurrentRequest();

    // Notification from the receive thread that a
    // tile has arrived
    void            TileHasArrived(HFCPtr<HRFTile>& pi_rpTile);


    // Methods that sends an attribute request and waits for the data
    bool           DownloadAttributes(const string& pi_rRequest);

    HFCPtr<HRFInternetImagingFile> GetPage(uint32_t pi_Page) const;


    //--------------------------------------
    // Attributes
    //--------------------------------------

    //
    // Thread Exception Handling
    //
    HFCEvent                    m_ThreadExceptionEvent;
    mutable HFCExclusiveKey     m_ThreadExceptionKey;
    bool                       m_IsInternetImagingException;
    HAutoPtr<HFCException>      m_pThreadException;

    // for IM4MSI: Indicates if the process must be stopped.
    HFCInterlockedValue<bool>  m_RequestStopped;


    //
    // State of the internet image
    //
    HFCEvent                    m_StateEvent;
    HFCInterlockedValue<HRFInternetImagingState>
    m_State;

    //
    // The connection to the server and its key
    //
    mutable HFCExclusiveKey     m_ConnectionKey;
    HAutoPtr<HFCInternetConnection>
    m_pConnection;
    bool                       m_HTTPConnection;


    //
    // The image name as understood by the server
    //
    WString                     m_ImageName;
    WString                     m_LocalCachedFileName;

    string                      m_EncodedPWCookieForRequest;
    string                      m_EncodedImageNameForRequest;

    //
    // The Receiving thread
    //
    HAutoPtr<HRFInternetImagingThread>
    m_pThread;


    //
    // Universal values for an image that will be setup and
    // validated by the InternetImaging Thread.  The page and
    // resolution descriptor will be built when all these values
    // and the descendant's value are all signaled.
    //

    HFCEvent                    m_PageDescriptorAvailable;
    mutable HFCExclusiveKey     m_DataKey;

    unsigned short             m_ResolutionCount;
    bool                       m_IsUnlimitedResolution;

    // Resolution specific information
    typedef map<string, uint32_t> UnlimitedResolutionMap;
    UnlimitedResolutionMap      m_UnlimitedResolutionMap;
    list<uint32_t>                m_UsedResIndexList;

    UnlimitedResolutionMap      m_ResolutionMap;    // this member is used when the raster file is an unlimited resolution
    uint32_t                    m_TileWidth     [HRF_INTERNET_MAXIMUM_RESOLUTION];
    uint32_t                    m_TileHeight    [HRF_INTERNET_MAXIMUM_RESOLUTION];
    uint32_t                    m_ImageWidth    [HRF_INTERNET_MAXIMUM_RESOLUTION];
    uint32_t                    m_ImageHeight   [HRF_INTERNET_MAXIMUM_RESOLUTION];
    HFCPtr<HRPPixelType>        m_pPixelType    [HRF_INTERNET_MAXIMUM_RESOLUTION];


    // Image specific information
    HFCEvent                    m_HistogramEvent;
    HFCPtr<HRPHistogram>        m_pHistogram;
    HFCPtr<HGF2DTransfoModel>   m_pTransfoModel;
    bool                       m_HasBackground;
    uint32_t                    m_Background;
    HGF2DWorldIdentificator     m_WorldID;
    HFCVersion                  m_Version;
    HGF2DLiteExtent             m_ROI;
    HArrayAutoPtr<double>      m_pClipShape;
    uint32_t                    m_ClipShapeSize;

    // multi page support
    bool                       m_MultiPageSupported;
    typedef map<uint32_t, HFCPtr<HRFInternetImagingFile> > PageList;
    uint32_t                    m_PageCount;
    mutable PageList            m_Pages;    // this member is set only in the first page (m_PageID == 0)
    uint32_t                    m_PageID;

    // connection information
    HFCVersion                  m_CurrentProtocol;
    // Time informations
    string                      m_TimeStamp;
    time_t                      m_CreationTime;
    time_t                      m_ModifyTime;
    time_t                      m_AccessTime;
    uint64_t                    m_FileSize;

    // Compression information
    HFCPtr<HCDCodec>            m_pCodec;
    typedef vector<HFCPtr<HRFInternetJpegTable> >
    JpegTableVector;
    JpegTableVector             m_JpegTables;

    // Original file type on server
    HFCEvent                    m_OriginalFileTypeEvent;
    WString                     m_OriginalFileType;

    // File that is small enough to be transfered locally
    HFCEvent                    m_LocalCacheFileEvent;
    HFCPtr<HRFRasterFile>       m_pLocalCachedFile;

    // Tag information from the server
    HAutoPtr<HPMAttributeSet>        m_pAttributes;
    HFCEvent                         m_AttributesEvent;
    HFCPtr<HMDMetaDataContainerList> m_pListOfMetaDataContainers;

    //
    // Tile Pool
    //
    HRFTilePool                 m_TilePool;
    bool                       m_WaitTileOnRead;


    //
    // Progress Information
    //

    static HFCExclusiveKey      s_ProgressKey;
    static uint32_t             s_ProgressTotalTiles;
    static uint32_t             s_ProgressCurrentTiles;
    uint32_t                    m_ProgressCurrentTiles;


    //
    // LookAhead, asynchronous, reconnection, small file caching
    //

    // LookAhead Information
    int32_t                    m_TileWaitTime;

    // Disconnector Thread
    static HFCPtr<HRFInternetDisconnectionThread>
    s_pDisconnectorThread;
    HFCPtr<HRFInternetDisconnectionThread>
    m_pDisconnectorThread;

    // One tile descriptor for each resolution of the image
    HArrayAutoPtr<HGFTileIDDescriptor>
    m_pTileDescriptors;


    // Indicates the state of the object
    HFCInterlockedValue<bool>  m_IsConstructing;

    // Indicates if a redirection has occurred.  When reconnecting,
    // we will wait for an event to be set a dispatcher was encountered
    bool                       m_Cancelled;
    bool                       m_Redirected;
    HFCEvent                    m_RedirectEvent;


    HFCEvent                    m_TileSizeHandlerEvent;
    HFCEvent                    m_ImageSizeHandlerEvent;

    //--------------------------------------
    // Others
    //--------------------------------------

    // Methods Disabled
    HRFInternetImagingFile(const HRFRasterFile& pi_rObj);
    HRFInternetImagingFile& operator=(const HRFRasterFile& pi_rObj);
    };


//------------------------------------------------------------------------------
// Main Creator
//------------------------------------------------------------------------------
struct HRFInternetImagingFileCreator : public HRFRasterFileCreator
    {
    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HRFInternetImagingFileCreator);


public:

    // Opens the file and verifies if it is the right type
    virtual bool       IsKindOfFile       (const HFCPtr<HFCURL>& pi_rpURL,
                                            uint64_t             pi_Offset = 0) const;

    // Capability
    virtual bool       SupportsURL        (const HFCPtr<HFCURL>& pi_rpURL) const;


    // Identification information
    virtual WString     GetExtensions      () const;
    virtual WString     GetLabel           () const;
    virtual WString     GetSchemes         () const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities    ();



    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>
    Create             (const HFCPtr<HFCURL>& pi_rpURL,
                        HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                        uint64_t             pi_Offset = 0) const;

protected:
    HRFInternetImagingFileCreator();

    // capabilities instance member definition
    HFCPtr<HRFRasterFileCapabilities>
    m_pCapabilities;

    // Label & schemes
    WString             m_Label;
    WString             m_Schemes;
    WString             m_Extensions;
    };


//------------------------------------------------------------------------------
// InternetImagingFile Capabilities
//------------------------------------------------------------------------------
class HRFInternetImagingFileCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFInternetImagingFileCapabilities();

    };


#if 0
/*
   #TR 250337, Raymond Gauthier, 23/07/2008:
   Merged the 3 Creators in in single one (HRFInternetImagingFileCreator) so that
   there is only one Creator associated with a class ID.
*/
//------------------------------------------------------------------------------
// Socket creator
//------------------------------------------------------------------------------
struct HRFInternetFileSocketCreator : public HRFInternetFileCreator
    {
    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HRFInternetFileSocketCreator);

public:
    virtual bool   IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                 uint64_t             pi_Offset = 0) const;

    virtual bool   SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const;

protected:
    HRFInternetFileSocketCreator();
    };


//------------------------------------------------------------------------------
// HTTP creator
//------------------------------------------------------------------------------
struct HRFInternetFileHTTPCreator : public HRFInternetFileCreator
    {
    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HRFInternetFileHTTPCreator);

public:
    virtual bool   IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                 uint64_t             pi_Offset = 0) const;

    virtual bool   SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const;


protected:
    HRFInternetFileHTTPCreator();
    };


//------------------------------------------------------------------------------
// HTTPS creator
//------------------------------------------------------------------------------
struct HRFInternetFileHTTPSCreator : public HRFInternetFileCreator
    {
    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HRFInternetFileHTTPSCreator);

public:
    virtual bool   IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                 uint64_t             pi_Offset = 0) const;

    virtual bool   SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const;


protected:
    HRFInternetFileHTTPSCreator();
    };
#endif

