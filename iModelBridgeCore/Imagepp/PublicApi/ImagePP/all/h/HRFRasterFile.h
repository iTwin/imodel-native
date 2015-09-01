//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRasterFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFRasterFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCAccessMode.h"
#include "HFCBinStreamLockManager.h"
#include "HFCMonitor.h"

#include "HFCInterlockedValue.h"
#include "HFCURL.h"

#include "HGF2DWorld.h"
#include "HGFTileIDDescriptor.h"

#include "HMGMacros.h"
#include "HMGMessageDuplex.h"

#include "HRFPageDescriptor.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFResolutionEditor.h"
#include "HRFSharingControl.h"
#include "HRFSisterFileSharing.h"


BEGIN_IMAGEPP_NAMESPACE
class HMDContext;
class HFCMemoryBinStream;
class HRARaster;
class HRAStoredRaster;
struct HRACopyFromOptions;

// Data type: List of related URLs
typedef bvector<HFCPtr<HFCURL>, allocator<HFCPtr<HFCURL> > > ListOfRelatedURLs;

class HRFRasterFile : public HFCShareableObject<HRFRasterFile>,
    public HMGMessageDuplex
    {
public:
    // Class ID for this class.
    HDECLARE_BASECLASS_ID(HRFRasterFileId_Base)

    friend class HRFResolutionEditor;

    IMAGEPP_EXPORT virtual                               ~HRFRasterFile();

    // File name
    const HFCPtr<HFCURL>&         GetURL                () const;

    // Additional URLs of files that a cache would depend on,
    // for example in the case of a raster file made of multiple files
    const ListOfRelatedURLs&      GetRelatedURLs        () const;

    // Returns the file exlusive key
    virtual HFCExclusiveKey&              GetKey() const;

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const = 0;

    IMAGEPP_EXPORT virtual void                     SetContext(uint32_t            pi_PageIndex,
                                                       const HFCPtr<HMDContext>& pi_rpContext);
    IMAGEPP_EXPORT virtual HFCPtr<HMDContext>       GetContext(uint32_t            pi_PageIndex) const;

    virtual const HGF2DWorldIdentificator   GetPageWorldIdentificator (uint32_t pi_Page = 0) const;
    virtual const HGF2DWorldIdentificator   GetWorldIdentificator     () const = 0;
    virtual uint32_t                       CountPages                () const;
    virtual HFCPtr<HRFPageDescriptor>       GetPageDescriptor         (uint32_t pi_Page) const;
    virtual bool                            PagesAreRasterFile        () const;
    virtual HFCPtr<HRFRasterFile>           GetPageFile               (uint32_t pi_Page) const;

    // Access mode management
    HFCAccessMode                   GetPhysicalAccessMode () const;
    HFCAccessMode                   GetLogicalAccessMode () const;
    void                            ChangeLogicalAccessMode (HFCAccessMode  pi_AccessMode);

    HFCAccessMode                   GetAccessMode () const;

    // Pass non-NULL pointer to retrieve information.
    IMAGEPP_EXPORT virtual void GetFileStatistics(time_t* pCreated, time_t* pModified, time_t* pAccessed) const;

    // File offset
    uint64_t                        GetOffset                () const;

    // File manipulation
    // You must redefine that function in your descendant = 0
    IMAGEPP_EXPORT virtual bool                  AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t      pi_Page,
                                                                 unsigned short pi_Resolution,
                                                                 HFCAccessMode pi_AccessMode) = 0;

    virtual HRFResolutionEditor*          CreateUnlimitedResolutionEditor(uint32_t      pi_Page,
                                                                          double       pi_Resolution,
                                                                          HFCAccessMode pi_AccessMode);

    IMAGEPP_EXPORT virtual bool                  ResizePage(uint32_t   pi_Page,
                                                     uint64_t  pi_Width,
                                                     uint64_t  pi_Height);


    virtual void                          Save() = 0;

    IMAGEPP_EXPORT virtual uint64_t              GetFileCurrentSize() const;

    uint64_t                     GetFileCurrentSize(HFCBinStream* pi_pBinStream) const;

    uint64_t                      GetMaxFileSizeInBytes() const;
    //--------------------------------------
    // Sharing control methods
    //--------------------------------------
    virtual void                          SharingControlCreate          ();
    virtual bool                         SharingControlNeedSynchronization();
    virtual void                          SharingControlSynchronize     ();
    virtual void                          SharingControlIncrementCount  ();
    virtual bool                         SharingControlIsLocked        ();

    virtual HRFSharingControl*            GetSharingControl             ();

    IMAGEPP_EXPORT virtual void                   SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                                        uint32_t pi_Page = 0,
                                                                        bool   pi_CheckSpecificUnitSpec = false,
                                                                        bool   pi_InterpretUnitINTGR = false);

    IMAGEPP_EXPORT virtual double                GetDefaultRatioToMeter(uint32_t pi_Page = 0) const;

    IMAGEPP_EXPORT bool                  IsUnitFoundInFile(uint32_t pi_Page = 0) const;

    virtual bool                          IsOriginalRasterDataStorage() const;

    //--------------------------------------
    // LookAhead Methods
    //--------------------------------------

    // Indicates if the file supports LookAhead optimization
    IMAGEPP_EXPORT         bool    HasLookAhead        (uint32_t               pi_Page) const;
    IMAGEPP_EXPORT virtual bool    HasLookAheadByBlock (uint32_t               pi_Page) const;
    IMAGEPP_EXPORT virtual bool    HasLookAheadByExtent(uint32_t               pi_Page) const;


    // This method is used in SetLookAhead to verify if the derived class is
    // ready to receive LookAhead request.  It returns true by default.
    IMAGEPP_EXPORT virtual bool    CanPerformLookAhead (uint32_t               pi_Page) const;

    // Sets the LookAhead for a list of blocks
    IMAGEPP_EXPORT virtual void     SetLookAhead        (uint32_t               pi_Page,
                                                 const HGFTileIDList&   pi_rBlocks,
                                                 uint32_t               pi_ConsumerID,
                                                 bool                  pi_Async);

    // Sets the LookAhead for a shape
    IMAGEPP_EXPORT virtual void    SetLookAhead        (uint32_t               pi_Page,
                                                unsigned short        pi_Resolution,
                                                const HVEShape&        pi_rShape,
                                                uint32_t               pi_ConsumerID,
                                                bool                  pi_Async);


    // Stops LookAhead for a consumer
    IMAGEPP_EXPORT virtual void     StopLookAhead       (uint32_t               pi_Page,
                                                 uint32_t               pi_ConsumerID);


    // LockManager
    virtual HFCBinStreamLockManager* GetLockManager();

    IMAGEPP_EXPORT virtual const HFCMemoryBinStream* GetMemoryFilePtr() const;

    // This tile descriptor is only used to compute ID from levels and index
    // and vice-versa.
    IMAGEPP_EXPORT static const HGFTileIDDescriptor        s_TileDescriptor;


    //--------------------------------------
    // Messagging
    //--------------------------------------

    // Notification that a block is ready.  One is called by the descendant
    // of this class and another is called when a file we're linked to sends
    // us a message that the tile is ready.
    IMAGEPP_EXPORT void            NotifyBlockReady     (uint32_t      ppi_Page,
                                          uint64_t     pi_BlockID);
    bool           NotifyBlockReady     (const HMGMessage&   pi_rMessage);


    virtual void    CancelCreate();
    bool           IsCreateCancel();


protected:

    void                       SetUnitFoundInFile(bool  pi_UnitFound,
                                                  uint32_t pi_Page = 0);

    const HRFResolutionEditor* GetResolutionEditor(unsigned short pi_Resolution);

    // The key
    mutable HFCExclusiveKey             m_Key;

    // List of pages descriptor
    typedef vector<HFCPtr<HRFPageDescriptor>, allocator<HFCPtr<HRFPageDescriptor> > >
    ListOfPageDescriptor;

    // List of resolution editor
    typedef list<const HRFResolutionEditor*, allocator<const HRFResolutionEditor*> >
    ResolutionEditorRegistry;

    // Editor registry
    ResolutionEditorRegistry            m_ResolutionEditorRegistry;

    // File information
    HFCPtr<HFCURL>                      m_pURL;
    ListOfRelatedURLs                   m_ListOfRelatedURLs;
    ListOfPageDescriptor                m_ListOfPageDescriptor;

    typedef map<uint32_t, HFCPtr<HMDContext>> ContextMap;
    ContextMap                          m_Contexts;

    // Access mode management
    HFCAccessMode                       m_PhysicalAccessMode;
    HFCAccessMode                       m_LogicalAccessMode;
    uint64_t                           m_Offset;
    bool                               m_IsOpen;

    bool                                m_IsCreateCancel;

    // This is the instance of the sharing control sister file.
    HAutoPtr<HRFSharingControl>         m_pSharingControl;

    vector<double>                     m_DefaultRatioToMeter;
    vector<bool>                       m_DefaultUnitWasFound;


    // Methods
    // allow to Open an image file
    IMAGEPP_EXPORT                                     HRFRasterFile  (const HFCPtr<HFCURL>&  pi_rpURL,
                                                               HFCAccessMode          pi_AccessMode = HFC_READ_ONLY,
                                                               uint64_t              pi_Offset = 0);

    //--------------------------------------
    // LookAhead Methods
    //--------------------------------------

    // This method is used in SetLookAhead to give the list of needed tiles
    // to a derived class, since it knows how to obtain the tiles.
    IMAGEPP_EXPORT virtual void    RequestLookAhead     (uint32_t               pi_Page,
                                                 const HGFTileIDList&   pi_rBlocks,
                                                 bool                  pi_Async);

    // This method is used in SetLookAhead to indicate to a derived class that
    // the current LookAhead has been cancelled.
    IMAGEPP_EXPORT virtual void    CancelLookAhead      (uint32_t               pi_Page);


    // This method is called by a derived-class when the link with the
    // source of data blocks has been re-established
    IMAGEPP_EXPORT virtual void     ResetLookAhead      (uint32_t               pi_Page,
                                                 bool                  pi_Async);

    // Removes a block from all the consumers that have it
    void                    RemoveBlockFromConsumers
    (uint32_t               pi_Page,
     uint64_t              pi_BlockID);

    // this methods converts a region into a list of tiles
    void                    SnapRegionToGrid    (uint32_t               pi_Page,
                                                 unsigned short        pi_Resolution,
                                                 const HVEShape&        pi_rShape,
                                                 HGFTileIDList*         po_pBlocks) const;


    // Resolution Editor register/unregister
    void RegisterResolutionEditor(const HRFResolutionEditor* pi_pResolutionEditor);
    void UnregisterResolutionEditor(const HRFResolutionEditor* pi_pResolutionEditor);




private:
    // Methods Disabled
    HRFRasterFile(const HRFRasterFile& pi_rObj);
    HRFRasterFile& operator=(const HRFRasterFile& pi_rObj);

    //--------------------------------------
    // LookAhead Methods
    //--------------------------------------
    // Adds new blocks to a consumer
    void            AddBlocksToConsumer         (uint32_t               pi_Page,
                                                 uint32_t               pi_ConsumerID,
                                                 const HGFTileIDList&   pi_rBlocks,
                                                 bool                  pi_Cancelled);

    // Verify if a cancellation is required.  It also removes the blocks
    // that were already requested.
    bool           CanCancelLookAhead          (uint32_t               pi_Page,
                                                 HGFTileIDList&         pi_rRequest,
                                                 uint32_t               pi_ConsumerID) const;

    void            InitializeDefaultRatioToMeter();

    // The map of consumers to block list
    typedef map<uint64_t, HGFTileIDList*>    ConsumerMap;
    typedef map<uint32_t,  ConsumerMap>       PageConsumerMap;


    PageConsumerMap             m_PageConsumers;
    mutable HFCExclusiveKey     m_ConsumersKey;

    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT);
    };

// This is a utility class.  There will be a class that derives from this one
// for each RasterFile class. It is used by the raster file factory and the creators registry.
struct HRFRasterFileCreator
    {
    HDECLARE_BASECLASS_ID(HRFRasterFileId_Creator);

    // constructor init access mode to none
    IMAGEPP_EXPORT HRFRasterFileCreator(HCLASS_ID pi_ClassID);

    IMAGEPP_EXPORT virtual ~HRFRasterFileCreator();

    IMAGEPP_EXPORT virtual bool                CanRegister() const;

    // Opens the file and verifies if it is the right type
    virtual bool                       IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                     uint64_t                pi_Offset = 0) const = 0;

    IMAGEPP_EXPORT virtual bool                NeedRasterDllDirectory() const;
    IMAGEPP_EXPORT virtual void                SetRasterDllDirectory(const WString& pi_rDllDirectory);
    IMAGEPP_EXPORT const WString&              GetRasterDllDirectory() const;


    // Returns the class ID of the raster file associated with the create
    IMAGEPP_EXPORT virtual HCLASS_ID             GetRasterFileClassID() const;

    // Identification information
    IMAGEPP_EXPORT virtual WString               GetLabel() const = 0;
    IMAGEPP_EXPORT virtual WString               GetSchemes() const = 0;
    IMAGEPP_EXPORT virtual WString               GetDefaultExtension() const;
    IMAGEPP_EXPORT virtual WString               GetExtensions() const = 0;

    // Get the supported access mode for this file - logic const function
    IMAGEPP_EXPORT virtual HFCAccessMode         GetSupportedAccessMode() const;

    // Capability - The Generic SupportsURL function can be overwrite
    IMAGEPP_EXPORT virtual bool                 SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const;
    IMAGEPP_EXPORT virtual bool                 SupportsScheme(const WString& pi_rScheme) const;
    IMAGEPP_EXPORT virtual bool                 SupportsExtension(const WString& pi_rExtension) const;

    // Raster file made of multiple files - Get the list of related files from a given URL
    IMAGEPP_EXPORT virtual bool                 GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                                        ListOfRelatedURLs&    pio_rRelatedURLs) const;
    //--------------------------------------
    // Sharing control methods
    //--------------------------------------
    IMAGEPP_EXPORT virtual void                  SharingControlCreate          (const HFCPtr<HFCURL>& pi_pURL);
    virtual HRFSharingControl*           GetSharingControl             () const;
    virtual HFCBinStreamLockManager*     GetLockManager             () const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() = 0;

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile> Create(const HFCPtr<HFCURL>&          pi_rpURL,
                                         HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                                         uint64_t                      pi_Offset = 0) const = 0;

    virtual bool _HandleExportToFile(HFCPtr<HRFRasterFile>& pDestinationFile, HFCPtr<HRAStoredRaster>& pDestinationRaster, 
                                     HFCPtr<HRFRasterFile>& pSourceFile, HFCPtr<HRARaster>& pSourceRaster, 
                                     HRACopyFromOptions const& copyFromOpts) const {return /*not handle*/false;}
protected:
    // The class ID of the associated rasterfile
    HCLASS_ID                         m_ClassID;

    // capabilities instance member definition
    HFCPtr<HRFRasterFileCapabilities> m_pCapabilities;

    // Keep the computed access mode for the file format
    HFCAccessMode                      m_AccessMode;

    // This is the instance of the sharing control sister file.
    HAutoPtr<HRFSharingControl>       m_pSharingControl;


    WString                           m_DllDirectory;
    };
END_IMAGEPP_NAMESPACE

#include "HRFRasterFile.hpp"

