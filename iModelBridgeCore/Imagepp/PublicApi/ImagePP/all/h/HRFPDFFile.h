//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPDFFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#if !defined(_DEBUG) // We do not support this format when using the debug version of the C run-time library
    #define IPP_HAVE_PDF_SUPPORT
#endif

#if defined(IPP_HAVE_PDF_SUPPORT) 


#include "HFCMacros.h"

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HFCURLFile.h"

#include "HMDLayers.h"
#include "HMDAnnotations.h"

BEGIN_IMAGEPP_NAMESPACE
class PDFWrapper;

class HMDVolatileLayers;

class HRFPDFCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFPDFCapabilities();

    };

class HRFPDFFile : public HRFRasterFile
    {
public:
    friend class HRFPDFEditor;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_PDF, HRFRasterFile)

    static double s_dpiConvertScaleFactor;

    // allow to Open an image file
    HRFPDFFile          (const HFCPtr<HFCURL>&          pi_rpURL,
                         HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                         uint64_t                      pi_Offset = 0);

    virtual                                 ~HRFPDFFile         ();

    static bool                            CanLoadPDFWrapper();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities     () const;

    //Set the context
    virtual void                            SetContext(uint32_t                 pi_PageIndex,
                                                       const HFCPtr<HMDContext>& pi_rpContext);

    // File information
    virtual const HGF2DWorldIdentificator   GetWorldIdentificator () const;

    virtual const HGF2DWorldIdentificator   GetPageWorldIdentificator(uint32_t pi_Page = 0) const;
        
    IMAGEPP_EXPORT void                      GetDimensionForDWGUnderlay(uint32_t pi_Page,                                                                                                          
                                                                       double& po_xDimension, 
                                                                       double& po_yDimension) const;

    // File manipulation
    virtual bool                        AddPage             (HFCPtr<HRFPageDescriptor> pi_pPage);

    uint32_t                           CountPages          () const;
    HFCPtr<HRFPageDescriptor>           GetPageDescriptor   (uint32_t pi_Page) const;

    virtual HRFResolutionEditor*        CreateResolutionEditor  (uint32_t                   pi_Page,
                                                                 unsigned short            pi_Resolution,
                                                                 HFCAccessMode              pi_AccessMode);

    virtual HRFResolutionEditor*        CreateUnlimitedResolutionEditor  (uint32_t                   pi_Page,
                                                                          double                    pi_Resolution,
                                                                          HFCAccessMode              pi_AccessMode);

    virtual void                        Save();

    bool                               HasLookAheadByExtent    (uint32_t                   pi_Page) const;

    virtual bool                       CanPerformLookAhead     (uint32_t                   pi_Page) const;

    // Sets the LookAhead for a list of blocks
    virtual void                        SetLookAhead            (uint32_t                   pi_Page,
                                                                 const HGFTileIDList&       pi_rBlocks,
                                                                 uint32_t                   pi_ConsumerID,
                                                                 bool                      pi_Async);

    // Sets the LookAhead for a shape
    virtual void                        SetLookAhead            (uint32_t                   pi_Page,
                                                                 unsigned short            pi_Resolution,
                                                                 const HVEShape&            pi_rShape,
                                                                 uint32_t                   pi_ConsumerID,
                                                                 bool                      pi_Async);

    // Stops LookAhead for a consumer
    virtual void                        StopLookAhead           (uint32_t                   pi_Page,
                                                                 uint32_t                   pi_ConsumerID);

    void                                GetLayers               (uint32_t pi_Page, HFCPtr<HMDLayers>& pi_rpLayers);

    void                                SetLayerVisibility      (uint32_t                   pi_Page,
                                                                 HFCPtr<HMDVolatileLayers>& pi_rpVolatileLayers);

    void                                SetLayerVisibility      (uint32_t                   pi_Page);


    //Function used to initialize the PDF library
    IMAGEPP_EXPORT static int                   InitializePDFLibraryInThread();

    IMAGEPP_EXPORT static void                  TerminatePDFLibraryInThread();

    uint32_t                            GetMainThreadId() const;
    void*                               GetDocument(uint32_t pi_Page) const;

protected:

    // Methods

    // Constructor use only to create a child
    //
    HRFPDFFile            (const HFCPtr<HFCURL>&      pi_rpURL,
                           HFCAccessMode              pi_AccessMode,
                           uint64_t                  pi_Offset,
                           bool                      pi_DontOpenFile);
    virtual void                         CreateDescriptors     ();

private:
    friend struct HRFPDFCreator;

    typedef map<uint32_t, HFCPtr<HRFPageDescriptor> > Pages;

    mutable Pages              m_Pages;
    uint32_t                   m_NumPages;
    HAutoPtr<PDFWrapper>       m_pPDFWrapper;

    uint32_t                   m_MainThreadId;

    typedef map<uint64_t, Byte*>       TilePool;
    typedef map<uint32_t,  TilePool>     PageTilePool;
    typedef map<uint64_t, PageTilePool> ContextPageTilePool;

    HFCExclusiveKey            m_TilePoolKey;
    ContextPageTilePool        m_ContextPageTilePool;

    // called by HRFPDFEditor destructor
    void                    RemoveLookAhead(uint32_t pi_Page,
                                            unsigned short pi_Resolution);

    // Methods Disabled
    HRFPDFFile(const HRFPDFFile& pi_rObj);
    HRFPDFFile& operator=(const HRFPDFFile& pi_rObj);
    };

// PDF Creator.
struct HRFPDFCreator : public HRFRasterFileCreator
    {
    virtual bool                       CanRegister() const;

    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual WString                     GetLabel() const;
    virtual WString                     GetSchemes() const;
    virtual WString                     GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();


    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFPDFCreator)

    // Disabled methodes
    HRFPDFCreator();
    };

//-----------------------------------------------------------------------------
// PDFWrapper
//-----------------------------------------------------------------------------

class HNOVTABLEINIT PDFWrapper
    {
public:
    virtual ~PDFWrapper() {};
    virtual void*       GetDocument() = 0;

    virtual uint32_t    CountPages() const = 0;
    virtual void         GetMaxResolutionSize(uint32_t   pi_Page, double dpiConvertScaleFactor, uint32_t& po_maxResSize)=0;

    virtual void        PageSize                    (uint32_t                       pi_Page,
                                                     uint32_t*                        po_pWidth,
                                                     uint32_t*                        po_pHeight,
                                                     double*                         po_rDPI) = 0;
    virtual void        GetLayers                   (uint32_t                       pi_Page,
                                                     HFCPtr<HMDLayers>&              po_rpLayers) = 0;
    virtual void        SetLayerVisibility          (uint32_t                       pi_Page,
                                                     HFCPtr<HMDVolatileLayers>&      pi_rpVolatileLayers) = 0;
    virtual void        GetAnnotations              (uint32_t                       pi_Page,
                                                     HFCPtr<HMDAnnotations>&         po_rpAnnotations) = 0;
    virtual void        GetGeocodingAndReferenceInfo(uint32_t                       pi_Page,
                                                     uint32_t                       pi_RasterizePageWidth,
                                                     uint32_t                       pi_RasterizePageHeight,
                                                     GeoCoordinates::BaseGCSPtr&    po_rpGeocoding,
                                                     HFCPtr<HGF2DTransfoModel>&      po_rpGeoreference) = 0;
    virtual void        GetDimensionForDWGUnderlay( uint32_t                  pi_Page,
                                                    double&                    po_xDimension, 
                                                    double&                    po_yDimension) const =0;     

    };
END_IMAGEPP_NAMESPACE

#endif // IPP_HAVE_PDF_SUPPORT
