//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRSObjectStore.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRSObjectStore
//-----------------------------------------------------------------------------
// This class is used to define Raster object stores.
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HPMObjectStore.h>
#include "HRFRasterFile.h"
#include "HRFResolutionEditor.h"
#include "HRFRasterFileFactory.h"
#include "HRFRasterFileCapabilities.h"
#include "HFCURLFile.h"
#include "HFCAccessMode.h"
#include "HRABitmap.h"

#include "HRAStoredRaster.h"
#include "HGFTileIDDescriptor.h"

#include <ImagePP/all/h/HGSTypes.h>

BEGIN_IMAGEPP_NAMESPACE
class HCDPacketRLE;
class HRATiledRaster;
class HRAPyramidRaster;
class HGFTileIDDescriptor;
class HRABitmapRLE;
class HRAUnlimitedResolutionRaster;

class HRSObjectStore : public HPMObjectStore, public HMGMessageReceiver
    {
public:

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRSObjectStoreId, HPMObjectStore)

    // Constants
    static const uint64_t   ID_TiledRaster;
    static const uint64_t   ID_PyramidRaster;
    static const uint64_t   ID_UnlimitedResolutionRaster;
    static const uint64_t   ID_ResizableRaster;
    static const uint32_t  CONFIGDATA_FixeLenght;
    static const uint32_t  CONFIGDATA_PageNumberLenght;
    static const uint32_t  CONFIGDATA_Filename;


    // Primary methods
    IMAGEPP_EXPORT                  HRSObjectStore(HPMPool*                     pi_pLog,
                                           HFCPtr<HRFRasterFile>&       pi_pRasterFile,
                                           uint32_t                    pi_Page,
                                           const HFCPtr<HGF2DCoordSys>& pi_rRefCoordSys);

    IMAGEPP_EXPORT virtual          ~HRSObjectStore();

    // Default is true. If false the native clip shape won't be read and nor
    // modified by a save operation.
    IMAGEPP_EXPORT void             SetUseClipShape(bool pi_UseClipShape);

    // Change the throw attribute of the class. If it is set to true, the class
    // will throw upon a HRF error in the Load and Save methods of the class
    IMAGEPP_EXPORT void                    SetThrowable(bool pi_Throwable);

    IMAGEPP_EXPORT HFCPtr<HRAStoredRaster> LoadRaster();

    // From HPMObjectStore
    virtual void            Save(HPMPersistentObject* pi_pObj) override;
    virtual bool            IsReadOnly() const override;
    virtual void            ForceReadOnly(bool pi_ReadOnly) override;
    

        //TR 322491 : The GetContext method is not virtual as the SetContext method because we don't want 
        //            to break the binary compatibility.
    const HFCPtr<HMDContext> GetContext();
    void                     SetContext(const HFCPtr<HMDContext>& pi_rContext);

    // Method for the HRAUnlimitedResolutionRaster
    uint32_t       CreateResolution        (double pi_Scale);
    void            RemoveResolution        (unsigned short pi_Resolution);
    void            ChangeResolution        (unsigned short pi_ResIndex,
                                             double pi_Resolution,
                                             uint64_t* po_pResWidth,
                                             uint64_t* po_pResHeight);

    // Keeps the 1 bit background color for future references
    bool                    Has1BitBackgroundColor() const;
    uint32_t               Get1BitBackgroundColor() const;

        
    // Message Handler...
    bool                    NotifyHRALookAhead            (const HMGMessage& pi_rMessage);
    bool                    NotifyHRAPyramidRasterClosing (const HMGMessage& pi_rMessage);

    // CoordSys gestion...
    HFCPtr<HGF2DCoordSys>   GetPhysicalCoordSys() const;
    HFCPtr<HGF2DCoordSys>   GetLogicalCoordSys() const;


    // re sizable
    // this method must be call before Load()
    bool                    CanBeResizable() const;
    bool                    SetResizableRasterSize  (uint64_t pi_RasterWidth,
                                                     uint64_t pi_RasterHeight);
    void                    RasterSizeChanged       (int64_t pi_Top,
                                                     int64_t pi_Bottom,
                                                     int64_t pi_Left,
                                                     int64_t pi_Right);

    HFCPtr<HRABitmapBase>  LoadTile(uint64_t           pi_TileIndex,
                                    uint32_t          pi_Resolution,
                                    uint64_t*            po_pPosX,
                                    uint64_t*            po_pPosY,
                                    HPMPool*           pi_pPool=0);

    IMAGEPP_EXPORT HFCPtr<HRFRasterFile> const& GetRasterFile() const;

    IMAGEPP_EXPORT uint32_t GetPageIndex() const;

    
protected:

    HFCAccessMode           GetRasterFileAccessMode () const;

    // Converter ID to TilePosition
    HAutoPtr<HGFTileIDDescriptor>
    m_pLoadTileDescriptor;
    HAutoPtr<HGFTileIDDescriptor>
    m_pSaveTileDescriptor;

private:

    friend class LocalRasterReader;
    friend class HRSStoreDescriptor;

    HFCPtr<HRAStoredRaster> Load(HPMObjectID pi_ObjectID);


    // Methods
    // Not implemented
    HRSObjectStore(const HRSObjectStore& pi_rObj);
    HRSObjectStore& operator=(const HRSObjectStore& pi_rObj);

    // Members

    HPMObjectID             m_RasterObjectID;
    bool                    m_Loaded;

    // Input information
    HFCPtr<HRFRasterFile>   m_pRasterFile;

    typedef vector<HRFResolutionEditor*, allocator<HRFResolutionEditor*> >
    ListOfResolutionEditor;
    ListOfResolutionEditor  m_ResolutionEditor;
    uint32_t               m_PageIndex;
    HFCPtr<HRFPageDescriptor>
                            m_pPageDescriptor;
    HFCPtr<HRFResolutionDescriptor>
                            m_pResDescriptor;   // Res 0

    bool                   m_MultiPixelType;

    // re sizable support
    bool                   m_Resizable;
    uint64_t               m_RasterWidth;
    uint64_t               m_RasterHeight;

    // CoordSys
    HFCPtr<HGF2DCoordSys>  m_pPhysicalCoordSys;
    HFCPtr<HGF2DCoordSys>  m_pLogicalCoordSys;


    // Information
    uint32_t               m_LoadCurSubImage;  // Current subImage
    uint32_t               m_SaveCurSubImage;  // Current subImage
    HFCPtr<HRABitmapBase>   m_pTileTemplate;

    // AccesMode for the current Image
    Byte                    m_CurrentAccesMode;     // See the .cpp for define

    // Note if we can throw an exception upon a HRFRasterFile invalid result.
    bool                    m_IsThrowable;

    bool                    m_ForceReadOnly;

    // Default is true. If false the native clip shape won't be read and nor
    // modified by a save operation.
    bool                    m_UseClipShape;

    // Keeps the 1 bit background color for future references
    bool                    m_Has1BitBackgroundColor;
    uint32_t               m_BackgroundColor1Bit;

	// Converters used to hide some pixel types like I2 or I4.
    HFCPtr<HRPPixelConverter>   m_ConvertNativeToNormalize;
    HFCPtr<HRPPixelConverter>   m_ConvertNormalizeToNative;

    // Methods

    void                    Constructor             (uint32_t pi_Page,
                                                     const HFCPtr<HGF2DCoordSys>&  pi_rRefCoordSys);

    bool                    VerifyResult            (HSTATUS pi_Result, bool pi_IsThrowable) const;
    HFCPtr<HRABitmapBase>   MakeBitmap              (uint32_t                    pi_Width,
                                                     uint32_t                    pi_Height,
                                                     const HFCPtr<HRPPixelType>& pi_rpPixelType);

    HFCPtr<HRAPyramidRaster> LoadRaster (HPMObjectID*   po_pRasterID);

    HFCPtr<HRAUnlimitedResolutionRaster> LoadUnlimitedResolutionRaster (HPMObjectID*   po_pRasterID);

    HFCPtr<HRABitmapRLE>    LoadResizableRaster (HPMObjectID*   po_pRasterID);


    void                    ConstructContextForRaster(const HFCPtr<HRFRasterFile>& pi_rRasterFile,
                                                      uint32_t                     pi_PageInd,
                                                      HRARaster*                   po_rRaster);

    HCLASS_ID               CheckObjectClassKey     (HPMPersistentObject& pi_rObj);
    void                    CheckCurrentSubImageForLoad(uint32_t pi_SubImage);
    void                    CheckCurrentSubImageForSave(uint32_t pi_SubImage);

    void                    SaveTiledRaster         (HRATiledRaster*    pi_pRaster);
    HSTATUS                 SaveBitmap              (HRABitmapBase*     pi_pBitmap);

    void                    SaveModel               (HRAStoredRaster*   pi_pRaster);
    void                    SaveLogicalShape        (HRAStoredRaster*   pi_pRaster);
    void                    SavePalette             (HRAStoredRaster*   pi_pRaster,
                                                     unsigned short    pi_Resolution = 0);
    void                    SaveHistogram           (HRAPyramidRaster*  pi_pRaster);

    void                    SaveResamplingMethod    (HRAPyramidRaster*  pi_pRaster);
    void                    SaveDataFlag            (HRAPyramidRaster*  pi_pObj);
    void                    LoadDataFlag            (HRAPyramidRaster*  pi_pObj);

    void                    SetResamplingForDecimationInRaster
    (HRAPyramidRaster* pio_pRaster);

    HGSResampling::ResamplingMethod
                            ResamplingMethodChangeType(HRFDownSamplingMethod pi_DownSamplingMethod);

    HRFDownSamplingMethod   ResamplingMethodChangeType(HGSResampling::ResamplingMethod pi_ResamplingMethod);

    HSTATUS                 ExceptionsSafeReadBlock(HRFResolutionEditor* pi_pEditor,
                                                    uint64_t            pi_PosX,
                                                    uint64_t            pi_PosY,
                                                    Byte*              pi_pBuffer);

    HSTATUS                 ExceptionsSafeReadBlock(HRFResolutionEditor* pi_pEditor,
                                                    uint64_t            pi_PosX,
                                                    uint64_t            pi_PosY,
                                                    HFCPtr<HCDPacket>    pi_pPacket);

    HSTATUS                 ExceptionsSafeReadBlockRLE(HRFResolutionEditor* pi_pEditor,
                                                       uint64_t            pi_PosX,
                                                       uint64_t            pi_PosY,
                                                       HFCPtr<HCDPacketRLE> pi_pPacket);

    HSTATUS                 ExceptionsSafeWriteBlock(HRFResolutionEditor* pi_pEditor,
                                                     uint64_t            pi_PosX,
                                                     uint64_t            pi_PosY,
                                                     Byte*              pi_pBuffer);

    HSTATUS                 ExceptionsSafeWriteBlockRLE(HRFResolutionEditor* pi_pEditor,
                                                        uint64_t            pi_PosX,
                                                        uint64_t            pi_PosY,
                                                        HFCPtr<HCDPacketRLE> pi_pPacket);


    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT);
    };

//----------------------------------------------------------------------------
class HNOVTABLEINIT HRSBlockAccessListener
    {
public:
    virtual void OnReadBlockError(HSTATUS pi_Error, HFCPtr<HFCURL> const& pi_pURL) = 0;
    virtual void OnWriteBlockError(HSTATUS pi_Error, HFCPtr<HFCURL> const& pi_pURL) = 0;
    };


//----------------------------------------------------------------------------
class HRSBlockAccessIndicator
    {
public:
    IMAGEPP_EXPORT void AddListener(HRSBlockAccessListener* pi_pListener);
    IMAGEPP_EXPORT void RemoveListener(HRSBlockAccessListener* pi_pListener);

    void NotifyReadBlockError(HSTATUS pi_Error, HFCPtr<HFCURL> const& pi_pURL);
    void NotifyWriteBlockError(HSTATUS pi_Error, HFCPtr<HFCURL> const& pi_pURL);

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRSBlockAccessIndicator)

    typedef list<HRSBlockAccessListener*> ListenerList;

    ListenerList        m_Listeners;
    };
END_IMAGEPP_NAMESPACE

#include "HRSObjectStore.hpp"


