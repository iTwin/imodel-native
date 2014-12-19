//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRasterFile.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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


class HMDContext;
class HFCMemoryBinStream;

// Data type: List of related URLs
typedef vector<HFCPtr<HFCURL>, allocator<HFCPtr<HFCURL> > >
ListOfRelatedURLs;

class HRFRasterFile : public HFCShareableObject<HRFRasterFile>,
    public HMGMessageDuplex
    {
public:
    // Class ID for this class.
    HDECLARE_BASECLASS_ID(1400)

    friend class HRFResolutionEditor;

    _HDLLg virtual                               ~HRFRasterFile();

    // File name
    const HFCPtr<HFCURL>&         GetURL                () const;

    // Additional URLs of files that a cache would depend on,
    // for example in the case of a raster file made of multiple files
    const ListOfRelatedURLs&      GetRelatedURLs        () const;

    // Returns the file exlusive key
    virtual HFCExclusiveKey&              GetKey() const;

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const = 0;

    _HDLLg virtual void                     SetContext(uint32_t            pi_PageIndex,
                                                       const HFCPtr<HMDContext>& pi_rpContext);
    _HDLLg virtual HFCPtr<HMDContext>       GetContext(uint32_t            pi_PageIndex) const;

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

    // File offset
    uint64_t                        GetOffset                () const;

    // File manipulation
    // You must redefine that function in your descendant = 0
    _HDLLg virtual bool                  AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t      pi_Page,
                                                                 unsigned short pi_Resolution,
                                                                 HFCAccessMode pi_AccessMode) = 0;

    virtual HRFResolutionEditor*          CreateUnlimitedResolutionEditor(uint32_t      pi_Page,
                                                                          double       pi_Resolution,
                                                                          HFCAccessMode pi_AccessMode);

    _HDLLg virtual bool                  ResizePage(uint32_t   pi_Page,
                                                     uint64_t  pi_Width,
                                                     uint64_t  pi_Height);


    virtual void                          Save() = 0;

    _HDLLg virtual uint64_t              GetFileCurrentSize() const;

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

    _HDLLg virtual void                   SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                                 uint32_t pi_Page = 0,
                                                                 bool   pi_CheckSpecificUnitSpec = false,
                                                                 bool   pi_GeoModelDefaultUnit = true,
                                                                 bool   pi_InterpretUnitINTGR = false);

    _HDLLg virtual double                GetDefaultRatioToMeter(uint32_t pi_Page = 0) const;

    _HDLLg bool                  IsUnitFoundInFile(uint32_t pi_Page = 0) const;

    virtual bool                          IsOriginalRasterDataStorage() const;

    //--------------------------------------
    // LookAhead Methods
    //--------------------------------------

    // Indicates if the file supports LookAhead optimization
    _HDLLg         bool    HasLookAhead        (uint32_t               pi_Page) const;
    _HDLLg virtual bool    HasLookAheadByBlock (uint32_t               pi_Page) const;
    _HDLLg virtual bool    HasLookAheadByExtent(uint32_t               pi_Page) const;


    // This method is used in SetLookAhead to verify if the derived class is
    // ready to receive LookAhead request.  It returns true by default.
    _HDLLg virtual bool    CanPerformLookAhead (uint32_t               pi_Page) const;

    // Sets the LookAhead for a list of blocks
    _HDLLg virtual void     SetLookAhead        (uint32_t               pi_Page,
                                                 const HGFTileIDList&   pi_rBlocks,
                                                 uint32_t               pi_ConsumerID,
                                                 bool                  pi_Async);

    // Sets the LookAhead for a shape
    _HDLLg virtual void    SetLookAhead        (uint32_t               pi_Page,
                                                unsigned short        pi_Resolution,
                                                const HVEShape&        pi_rShape,
                                                uint32_t               pi_ConsumerID,
                                                bool                  pi_Async);


    // Stops LookAhead for a consumer
    _HDLLg virtual void     StopLookAhead       (uint32_t               pi_Page,
                                                 uint32_t               pi_ConsumerID);


    // LockManager
    virtual HFCBinStreamLockManager* GetLockManager();

    _HDLLg virtual const HFCMemoryBinStream* GetMemoryFilePtr() const;

    // This tile descriptor is only used to compute ID from levels and index
    // and vice-versa.
    static const HGFTileIDDescriptor        s_TileDescriptor;


    //--------------------------------------
    // Messagging
    //--------------------------------------

    // Notification that a block is ready.  One is called by the descendant
    // of this class and another is called when a file we're linked to sends
    // us a message that the tile is ready.
    void            NotifyBlockReady     (uint32_t      ppi_Page,
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
    _HDLLg                                     HRFRasterFile  (const HFCPtr<HFCURL>&  pi_rpURL,
                                                               HFCAccessMode          pi_AccessMode = HFC_READ_ONLY,
                                                               uint64_t              pi_Offset = 0);

    //--------------------------------------
    // LookAhead Methods
    //--------------------------------------

    // This method is used in SetLookAhead to give the list of needed tiles
    // to a derived class, since it knows how to obtain the tiles.
    _HDLLg virtual void    RequestLookAhead     (uint32_t               pi_Page,
                                                 const HGFTileIDList&   pi_rBlocks,
                                                 bool                  pi_Async);

    // This method is used in SetLookAhead to indicate to a derived class that
    // the current LookAhead has been cancelled.
    _HDLLg virtual void    CancelLookAhead      (uint32_t               pi_Page);


    // This method is called by a derived-class when the link with the
    // source of data blocks has been re-established
    _HDLLg virtual void     ResetLookAhead      (uint32_t               pi_Page,
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

    HMG_DECLARE_MESSAGE_MAP_DLL(_HDLLg);
    };

// This is a utility class.  There will be a class that derives from this one
// for each RasterFile class. It is used by the raster file factory and the creators registry.
struct HRFRasterFileCreator
    {
    HDECLARE_BASECLASS_ID(1475);

    // constructor init access mode to none
    _HDLLg HRFRasterFileCreator(HCLASS_ID pi_ClassID);

    _HDLLg virtual ~HRFRasterFileCreator();

    _HDLLg virtual bool                CanRegister() const;

    // Opens the file and verifies if it is the right type
    virtual bool                       IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                     uint64_t                pi_Offset = 0) const = 0;

    _HDLLg virtual bool                NeedRasterDllDirectory() const;
    _HDLLg virtual void                SetRasterDllDirectory(const WString& pi_rDllDirectory);
    _HDLLg const WString&              GetRasterDllDirectory() const;


    // Returns the class ID of the raster file associated with the create
    _HDLLg virtual HCLASS_ID             GetRasterFileClassID() const;

    // Identification information
    _HDLLg virtual WString               GetLabel() const = 0;
    _HDLLg virtual WString               GetSchemes() const = 0;
    _HDLLg virtual WString               GetDefaultExtension() const;
    _HDLLg virtual WString               GetExtensions() const = 0;

    // Get the supported access mode for this file - logic const function
    _HDLLg virtual HFCAccessMode         GetSupportedAccessMode() const;

    // Capability - The Generic SupportsURL function can be overwrite
    _HDLLg virtual bool                 SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const;
    _HDLLg virtual bool                 SupportsScheme(const WString& pi_rScheme) const;
    _HDLLg virtual bool                 SupportsExtension(const WString& pi_rExtension) const;

    // Raster file made of multiple files - Get the list of related files from a given URL
    _HDLLg virtual bool                 GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                                        ListOfRelatedURLs&    pio_rRelatedURLs) const;
    //--------------------------------------
    // Sharing control methods
    //--------------------------------------
    _HDLLg virtual void                  SharingControlCreate          (const HFCPtr<HFCURL>& pi_pURL);
    virtual HRFSharingControl*           GetSharingControl             () const;
    virtual HFCBinStreamLockManager*     GetLockManager             () const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() = 0;

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile> Create(const HFCPtr<HFCURL>&          pi_rpURL,
                                         HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                                         uint64_t                      pi_Offset = 0) const = 0;
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

#include "HRFRasterFile.hpp"

