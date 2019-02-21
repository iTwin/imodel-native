//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFVirtualEarthFile.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFVirtualEarthFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE


//Application using a consumer ID greater than this define will be able to do multiple set look ahead to accumulate multiple tiles instead of 
//the default behavior, which is to cancel previous look ahead when a new one is triggered. Initialy required for 3SM.
#define BINGMAPS_MULTIPLE_SETLOOKAHEAD_MIN_CONSUMER_ID 1000000000

class HRPPixelType;
struct WorkerPool;
struct VirtualEarthTileQuery;
class HRFVirtualEarthFile;
struct ThreadLocalHttp;
struct HttpSession;

// The most common ImagerySet that can be used with HRFVirtualEarthFile::ComposeURL. 
// See Bing Maps REST API(http://msdn.microsoft.com/en-us/library/ff701721.aspx/)
#define BINGMAPS_IMAGERYSET_ROAD                "Road"
#define BINGMAPS_IMAGERYSET_AERIAL              "Aerial"
#define BINGMAPS_IMAGERYSET_AERIALWITHLABELS    "AerialWithLabels"
// Less common:
//#define BINGMAPS_IMAGERYSET_COLLINSBART      "CollinsBart"       // This imagery is visible only for the London area.
//#define BINGMAPS_IMAGERYSET_ORDNANCESURVEY   "OrdnanceSurvey"    // This imagery is visible only for the London area.

//-----------------------------------------------------------------------------
// HRFVirtualEarthCapabilities class
//-----------------------------------------------------------------------------
class HRFVirtualEarthCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFVirtualEarthCapabilities();
    };

//-----------------------------------------------------------------------------
// HRFVirtualEarthFile class
//-----------------------------------------------------------------------------
class HRFVirtualEarthFile : public HRFRasterFile
    {
public:
    friend class HRFVirtualEarthEditor;
    friend struct VirtualEarthTileQuery;

    //Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_VirtualEarth, HRFRasterFile)

    struct CoverageArea
        {
        int32_t     levelOfDetailMin;   // from 1 (lowest detail)
        int32_t     levelOfDetailMax;   // from 1 (lowest detail)
        double   southLatitude;
        double   westLongitude;
        double   northLatitude;
        double   eastLongitude;
        };

    struct ImageryProvider
        {
        Utf8String attribution;
        bvector<CoverageArea> coverage;
        };        

    typedef bvector<ImageryProvider> ImageryProviders;

    HRFVirtualEarthFile(const HFCPtr<HFCURL>& pi_rpURL,
                        HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                        uint64_t             pi_Offset = 0);

    virtual     ~HRFVirtualEarthFile();

    //File capabilities
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() const override;

    //File information
    const HGF2DWorldIdentificator GetWorldIdentificator() const override;

    //File manipulation
    HRFResolutionEditor*          CreateResolutionEditor(uint32_t      pi_Page,
                                                                 uint16_t pi_Resolution,
                                                                 HFCAccessMode pi_AccessMode) override;

    void                          Save() override;

    //Private utility method
    uint64_t                     GetFileCurrentSize() const override;
    virtual uint64_t                     GetFileCurrentSize(HFCBinStream* pi_pBinStream) const;

    //Look ahead method.
    bool                         CanPerformLookAhead(uint32_t pi_Page) const override;
    IMAGEPP_EXPORT virtual bool  ForceCancelLookAhead(uint32_t pi_Page);

    //This static method indicates if a plain URL can be represented
    //by this specific URL
    IMAGEPP_EXPORT static bool IsURLVirtualEarth(HFCURL const& pi_pURL);

    // Compose virtual earth pseudo URL. This is not a valid URL but a specially formatted URL to 
    // identify it as virtual earth URL and which imagery set to use.
    IMAGEPP_EXPORT static Utf8String ComposeURL(Utf8StringCR imagerySet);

    // Resolve map style from pseudo URL created by ComposeVirtualEarthURL.
    IMAGEPP_EXPORT static bool FindImagerySetFromURL(Utf8StringR imagerySet, HFCURL const& pi_pURL);

    // Get the logo URL
    IMAGEPP_EXPORT Utf8StringCR GetBrandLogoURI() const;

    // Get the lists of provider for the specified extent and zoom
    IMAGEPP_EXPORT ImageryProviders const& GetProviders() const;    
    
protected:

    //Constructor use only to create a child
    HRFVirtualEarthFile (const HFCPtr<HFCURL>& pi_rpURL,
                         HFCAccessMode         pi_AccessMode,
                         uint64_t             pi_Offset,
                         bool                 pi_DontOpenFile);
    virtual void                        CreateDescriptors   ();
    
    //--------------------------------------
    // LookAhead Methods
    //--------------------------------------
    //This method is used to determine if the file format supports look ahead
    //by block.
    bool HasLookAheadByBlock(uint32_t pi_Page) const override;

    //This method is used in SetLookAhead to give the list of needed tiles
    //to a derived class, since it knows how to obtain the tiles.
    void RequestLookAhead(uint32_t             pi_Page,
                                  const HGFTileIDList& pi_rBlocks,
                                  bool                pi_Async) override;

            void RequestLookAhead(uint32_t             pi_Page,         
                                  const HGFTileIDList& pi_rBlocks,                                  
                                  bool                 pi_Async, 
                                  uint32_t             pi_ConsumerID) override;


    //This method is used in SetLookAhead to indicate to a derived class that
    //the current LookAhead has been cancelled.
    void CancelLookAhead(uint32_t              pi_Page) override;

private:
    friend struct HRFVirtualEarthCreator;

    void                QueryImageURI(Utf8StringCR bingMapKey);
    Utf8String             GetTileURI(uint32_t pixelX, uint32_t pixelY, int32_t levelOfDetail) const;

    Utf8String             m_ImageURI;               // ex: "http://{subdomain}.tiles.virtualearth.net/tiles/r{quadkey}.jpeg?g=266&mkt={culture}"
    bvector<Utf8String>    m_ImageURISubdomains;     // ex: "t0","t1","t2","t3"

    ImageryProviders    m_Providers;
    Utf8String             m_LogoURI;                // ex: "http://dev.virtualearth.net/Branding/logo_powered_by.png"

    WorkerPool&         GetWorkerPool();
    std::unique_ptr<WorkerPool> m_pWorkerPool;

    HttpSession&        GetThreadLocalHttpSession();
    std::unique_ptr<ThreadLocalHttp> m_threadLocalHttp;

    std::map<uint64_t, RefCountedPtr<VirtualEarthTileQuery>> m_tileQueryMap;
    std::mutex                                               m_tileQueryMapMutex;

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
    bool                     IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                   uint64_t             pi_Offset = 0) const override;
    bool                     SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const override;

    // Identification information
    Utf8String                   GetLabel() const override;
    Utf8String                   GetSchemes() const override;
    Utf8String                   GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "BingMap"; }

    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const override;

private:

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFVirtualEarthCreator)

    // Disabled methodes
    HRFVirtualEarthCreator();
    };
END_IMAGEPP_NAMESPACE
