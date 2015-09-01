//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRATiledRaster.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRAStoredRaster.h"
#include "HGFTileIDDescriptor.h"
#include "HPMPool.h"
#include "HRABitmapBase.h"

BEGIN_IMAGEPP_NAMESPACE
class HFCExclusiveKey;
class HRATiledRasterIterator;
class HRSObjectStore;
class HRABitmapBase;
struct TiledImageSinkNode;


/** -----------------------------------------------------------------------------
    This class holds a list of status flags for each tile in the HRATiledRaster
    class that is using it.

    The flags are compressed so that each flag uses one bit, keeping flags for
    four tiles in each byte.

    This is the sole purpose of this class (save memory). Otherwise, these flags
    could be placed in bit fields.

    @see HRATiledRaster
    -----------------------------------------------------------------------------
*/
class HRATileStatus : public HFCShareableObject<HRATileStatus>
    {
    HDECLARE_SEALEDCLASS_ID(HRATileStatusId_Base)

public:

    HRATileStatus();

    ~HRATileStatus();

    HRATileStatus(const HRATileStatus& pi_rObj);

    HRATileStatus&  operator=(const HRATileStatus& pi_rObj);

    // Allocate space for flags
    void            Allocate(uint64_t pi_NumberOfTiles);

    //:> Get/Set flags
    bool            GetClearFlag(uint64_t pi_TileID);
    void            SetClearFlag(uint64_t pi_TileID, bool pi_Value);
    bool            GetDirtyForSubResFlag(uint64_t pi_TileID);
    void            SetDirtyForSubResFlag(uint64_t pi_TileID, bool pi_Value);


private:

    uint64_t        ComputeByteContainingTile(uint64_t pi_TileID) const;
    Byte            GetBitmaskForFlagsOfTile(uint64_t pi_TileID) const;
    uint64_t        GetByteCount() const;

    /** -------------------------------------------------------------------
        Array of flags. One flag occupies one bit, so there are
        currently flags for four tiles in each byte. Flags are stored in
        the order:
          CF1 DF1 CF2 DF2 ... CF4 DF4 in each byte, CF being the "Clear"
        flag and DF the "DirtyForSubRes" flag. Highest order bit contains
        CF1.
        If a new flag is needed, it is suggested to occupy 4 bits per
        tile, to make position computing easier.
        -------------------------------------------------------------------
    */
    struct FlagBuffer
        {
        FlagBuffer() : pData(0), BufSize(0) {}
        Byte*   pData;
        size_t  BufSize;
        } m_pFlags;

    // We have flags for this number of tiles
    uint64_t       m_NumberOfTiles;
    };



class HRATiledRaster : public HRAStoredRaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRATiledRasterId)

    friend class HRATiledRasterIterator;
    friend class HRAPyramidRaster;
    friend class HRAUnlimitedResolutionRaster;
    friend class HRSObjectStore;
    friend struct TiledImageSinkNode;

public:

    class HRATile;
    friend class HRATile;

    typedef map<HPMObjectID, HRATile*>          TileMap;
    typedef TileMap::iterator                   TileMapItr;
    typedef TileMap::const_iterator             const_TileMapItr;

    class HRATile : public HPMPoolItem

        {
    public :

        HRATile(const HFCPtr<HRABitmapBase>&  pi_rpTile,
                HPMPool*                      pi_pPool);
        virtual                     ~HRATile();

        void                        Invalidate(bool pi_Invalidate);
        bool                        IsInvalidate() const;

        void                        Discartable(bool pi_Discartable);
        bool                        IsDiscartable() const;


        IMAGEPP_EXPORT /*IppImaging_Needs*/ HFCPtr<HRABitmapBase>&    GetTile();

//        virtual byte*               AllocMemory(size_t pi_MemorySize) override;
//        virtual void                FreeMemory(byte* pi_MemPtr, size_t pi_MemorySize) override;
//        virtual byte*               AllocMemoryExt(size_t pi_MemorySize, size_t& po_ActualMemory) override;


    protected :

        void                        UpdateCachedSize();
        HFCExclusiveKey&            GetExclusiveKey();


    private :
        friend class HRATiledRaster;    // HRATiledRaster will set m_Itr

        // TO BE REMOVED Only called by HRATiledRaster through friendship
        // The HRA Tile should make the loading itself through a call to Inflate()
        void                        SetTile(HFCPtr<HRABitmapBase>& pi_rpTile);


        // this member is necessary to remove the tile from HRATiledRaster tile map on HRATile destructor
        mutable HRATiledRaster*             m_pTiledRaster;
        mutable HRATiledRaster::TileMapItr  m_Itr;

        mutable HFCPtr<HRABitmapBase>       m_pTile;
        uint64_t                            m_TileIndex;
        static HFCExclusiveKey              s_Key;

        bool                                m_Discartable;
        bool                                m_Invalidate;
//#ifdef __HMR_DEBUG_MEMBER
        uint32_t m_Resolution;
//#endif
        };



    // Primary methods

    HRATiledRaster ();
    IMAGEPP_EXPORT /*IppImaging_Needs*/ HRATiledRaster (const HFCPtr<HRABitmapBase>&  pi_pRasterModel,
                                                uint64_t                        pi_TileSizeX,
                                                uint64_t                        pi_TileSizeY,
                                                uint64_t                        pi_WidthPixels,
                                                uint64_t                        pi_HeightPixels,
                                                HPMObjectStore*                 pi_pStore=0,
                                                HPMPool*                        pi_pPool=0,
                                                bool                            pi_DisableTileStatus = false);

    HRATiledRaster(const HRATiledRaster& pi_rObj);

    virtual ~HRATiledRaster();

    HRATiledRaster&             operator=(const HRATiledRaster& pi_rObj);

    HPMPool*                    GetPool();
    // Overriden from HRAStoredRaster

    virtual HRARasterEditor*    CreateEditor(HFCAccessMode pi_Mode);
    virtual HRARasterEditor*    CreateEditor(const HVEShape& pi_rShape,
                                             HFCAccessMode   pi_Mode);
    virtual HRARasterEditor*    CreateEditorUnShaped (HFCAccessMode pi_Mode);

    virtual HRARasterIterator*  CreateIterator  (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const;

    // Special cas
    virtual void                InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels);

    // Catch it, and call the parent
    virtual void                SetTransfoModel (const HGF2DTransfoModel& pi_rModelCSp_CSl);

    virtual HPMPersistentObject*    
                                Clone () const override;

    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const override;

    virtual void    InvalidateRaster();

    virtual uint64_t   GetTileSizeX        () const;
    virtual uint64_t   GetTileSizeY        () const;

    const HFCPtr<HRATile> GetTileByIndex(uint64_t pi_Index, bool pi_NotInPool = false) const;

    // (Used by the HRSObjectStore)
    IMAGEPP_EXPORT /*IppImaging_Needs*/ const HFCPtr<HRATile> GetTile(uint64_t pi_PosX, uint64_t pi_PosY) const;

    virtual unsigned short GetRepresentativePalette(
        HRARepPalParms* pio_pRepPalParms);

    virtual void    ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                     bool                pi_ForceRecompute = false);

    void            EngageTileHistogramComputing();

    virtual void    Clear() override;
    virtual void    Clear(const HRAClearOptions& pi_rOptions) override;


    // LookAhead Methods
    virtual bool   HasLookAhead() const;

    virtual void    SetLookAhead(const HVEShape& pi_rShape,
                                 uint32_t        pi_ConsumerID,
                                 bool           pi_Async = false);

    void            GetMissingTilesInRegion(const HFCPtr<HVEShape>& pi_rpRegion,
                                            HGFTileIDList&          pi_rList,
                                            bool*                  po_pAllTilesMissing) const;

    // Message Handler...
    bool           NotifyContentChanged (const HMGMessage& pi_rMessage);
    bool           NotifyPaletteChanged (const HMGMessage& pi_rMessage);
    bool           NotifyProgressImageChanged (const HMGMessage& pi_rMessage);
    bool           NotifyGeometryChanged (const HMGMessage& pi_rMessage);

    void            SaveAndFlushTiles ();

    void            ApplyTransaction(HFCPtr<HRATransaction>& pi_rpTransaction);

    uint64_t       GetNumberOfTileX() const;
    uint64_t       GetNumberOfTileY() const;

protected:

    // Members

    // No persistent
    HPMPool*                m_pPool;

    // Copy of the Model with an Extent (1,1)
    HFCPtr<HRABitmapBase> m_pBitmapModel;

    // Methods

    virtual void _Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const override;

    // From HGFGraphicObject
    virtual void    SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys);


    void            ReplaceObject   (HFCPtr<HRATiledRaster>& pio_pRaster);
    void            DeepCopy        (const HRATiledRaster&   pi_rTiledRaster,
                                     HPMObjectStore*         pi_pStore=0,
                                     HPMPool*                pi_pLog=0);
    void            DeepDelete      ();
    void            CopyMembers     (const HRATiledRaster& pi_rObj);

    // Used by HRSObjectStore to enable the LookAhead
    // mechanism for this raster. Should only be called
    // when the underlying RasterFile supports the
    // LookAhead mechanism.
    void            EnableLookAhead(bool pi_ByExtent = false);

    const HFCPtr<HRATile>
    GetTileFromLoaded(uint64_t pi_Index) const;

    const HFCPtr<HRABitmapBase> GetStoredRaster(uint64_t pi_Index, IHPMMemoryManager* memoryManager) const;

    virtual ImageSinkNodePtr _GetSinkNode(ImagePPStatus& status, HVEShape const& sinkShape, HFCPtr<HRPPixelType>& pReplacingPixelType) override;

    virtual ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) override;

private:


    // Members

    // Tile informations
    uint64_t       m_TileSizeX;            // Tile Dimension
    uint64_t       m_TileSizeY;
    mutable uint64_t m_TileSize;             // Memory manager stuff
    mutable uint64_t m_TileObjectSize;

    uint64_t       m_NumberOfTileX;        // Number of tile to cover the extent
    uint64_t       m_NumberOfTileY;
    uint64_t       m_NumberOfTiles;

    // List of tiles
    // The tiles are number from Left-Right and Top-Down
    //      0  1  2  3
    //      4  5  6  7
    //          ...
    mutable TileMap                 m_TileMap;
    mutable HFCExclusiveKey         m_TileMapKey;

    // Converter ID to TilePosition
    HGFTileIDDescriptor*            m_pTileDescriptor;

    mutable HRATileStatus           m_TileStatus;
    bool                           m_TileStatusDisabled;

    // Note if our internal RasterFile supports the mechanism.
    // Set by the EnableLookAhead method, default to false.
    bool                           m_LookAheadEnabled;
    bool                           m_LookAheadByExtent;

    bool                           m_ComputeTileHistograms;

    // Methods

    void            Constructor    (const HFCPtr<HRABitmapBase>&  pi_pRasterModel,
                                    uint64_t                       pi_WidthPixels,
                                    uint64_t                       pi_HeightPixels);


    void            SetTileCoordSys (const const_TileMapItr&  pi_rItr);

    // Added methods (only used by Iterator and Editor)


    HGFTileIDDescriptor*        GetPtrTileDescription      () const;
    HRATileStatus&              GetInternalTileStatusList  (uint64_t* po_pNumberOfTile=0);

    // use by HRAUnlimtedResolutionRaster
    void                        SetInternalTileStatusSupported(bool pi_TileStatusSupported);

    uint64_t                    ComputeIndexFromTile    (uint64_t  pi_PosX, 
                                                         uint64_t  pi_PosY) const;

    void                        ComputeTileFromIndex    (uint64_t  pi_Index,
                                                         uint64_t*   po_PosX,
                                                         uint64_t*   po_PosY) const;


    uint64_t                   ComputeTileID(uint32_t  pi_RasterID,
                                              uint64_t pi_TileIndex) const;

    void                        TileMapCleanUp();

    void                        ClearWithRLEMask    (const HRAClearOptions& pi_rOptions);
    void                        ClearWithScanlines  (const HRAClearOptions& pi_rOptions);

    // called by HRSObjectStore
    void                        SaveTiles();

    void                        RemoveTile(HRATiledRaster::TileMapItr&  pi_rItr,
                                           bool                        pi_WillBeNotSaved);

    void                        SaveTile(HRATiledRaster::TileMapItr&  pi_rItr);


    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT)
    };
END_IMAGEPP_NAMESPACE

#include "HRATiledRaster.hpp"

