//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRATiledRaster.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <Imagepp/all/h/HRATiledRaster.h>
#include <Imagepp/all/h/HRATiledRasterIterator.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRFMessages.h>
#include <Imagepp/all/h/HGF2DGrid.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HRABitmapRLE.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HGSMemoryBaseSurfaceDescriptor.h>
#include <Imagepp/all/h/HRAHistogramProgressIndicator.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPQuantizedPalette.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDPacketRLE.h>
#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HMDContext.h>
#include <Imagepp/all/h/HGSRegion.h>
#include <Imagepp/all/h/HFCGrid.h>
#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HRATransaction.h>

#include <Imagepp/all/h/HRARasterEditor.h>
#include <Imagepp/all/h/HRABitmapEditor.h>
#include <Imagepp/all/h/HRAEditor.h>

#include <Imagepp/all/h/HRAClearOptions.h>
#include <Imagepp/all/h/HRAMessages.h>
#include <Imagepp/all/h/HRPMessages.h>
#include <Imagepp/all/h/HGFTileIDDescriptor.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HGFMappedSurface.h>

#include <Imagepp/all/h/HRSObjectStore.h>
#include <ImagePPInternal/gra/HRAImageNode.h>
#include <ImagePPInternal/gra/DownSampling.h>
#include <ImagePPInternal/gra/HRACopyToOptions.h>
#include <ImagePPInternal/gra/ImageAllocator.h>


#define MANAGEABLE_RASTER_PHYSICAL_SIZE (1024)

// We did not see a tangible gain so we decided to disable it until for now.
// tested: block image, loaded memory by strip, nearest/average sampling.
//#define ENABLE_STRIP_CACHING




///////////////////////////
// HRATileStatus class
///////////////////////////


/** -----------------------------------------------------------------------------
    Constructor
    -----------------------------------------------------------------------------
*/
HRATileStatus::HRATileStatus ()
    {
    m_NumberOfTiles = 0;
    m_pFlags.pData = 0;
    }


/** -----------------------------------------------------------------------------
    Destructor
    -----------------------------------------------------------------------------
*/
HRATileStatus::~HRATileStatus ()
    {
    delete[] m_pFlags.pData;
    }

/** -----------------------------------------------------------------------------
    Copy constructor
    -----------------------------------------------------------------------------
*/
HRATileStatus::HRATileStatus(const HRATileStatus& pi_rObj)
    {
    m_NumberOfTiles = pi_rObj.m_NumberOfTiles;

    if (m_NumberOfTiles > 0)
        {
        uint64_t ByteCount = GetByteCount();

        HPRECONDITION(ByteCount <= SIZE_MAX);
        m_pFlags.BufSize = (size_t)ByteCount;
        m_pFlags.pData = new Byte[m_pFlags.BufSize];

        memcpy(m_pFlags.pData, pi_rObj.m_pFlags.pData, m_pFlags.BufSize);
        }
    else
        {
        m_pFlags.BufSize = 0;
        m_pFlags.pData = 0;
        }
    }


/** -----------------------------------------------------------------------------
    Assignment
    -----------------------------------------------------------------------------
*/
HRATileStatus& HRATileStatus::operator=(const HRATileStatus& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_NumberOfTiles = pi_rObj.m_NumberOfTiles;

        delete[] m_pFlags.pData;
        m_pFlags.pData = 0;
        m_pFlags.BufSize = 0;

        if (m_NumberOfTiles > 0)
            {
            uint64_t ByteCount = GetByteCount();

            HPRECONDITION(ByteCount <= SIZE_MAX);
            m_pFlags.BufSize = (size_t)ByteCount;
            m_pFlags.pData = new Byte[m_pFlags.BufSize];

            memcpy(m_pFlags.pData, pi_rObj.m_pFlags.pData, m_pFlags.BufSize);
            }
        }

    return *this;
    }


/** -----------------------------------------------------------------------------
    Allocate space for tile flags.
    -----------------------------------------------------------------------------
*/
void HRATileStatus::Allocate(uint64_t pi_NumberOfTiles)
    {
    m_NumberOfTiles = pi_NumberOfTiles;

    delete m_pFlags.pData;
    m_pFlags.pData = 0;
    m_pFlags.BufSize = 0;

    if (m_NumberOfTiles > 0)
        {
        uint64_t ByteCount = GetByteCount();

        HPRECONDITION(ByteCount <= HUINTX_MAX);
        m_pFlags.BufSize = (size_t)ByteCount;
        m_pFlags.pData = new Byte[m_pFlags.BufSize];

        memset(m_pFlags.pData, 0, m_pFlags.BufSize);
        }
    }


/** -----------------------------------------------------------------------------
    Retrieve the value of the clear flag of a tile.
    -----------------------------------------------------------------------------
*/
bool HRATileStatus::GetClearFlag(uint64_t pi_TileID)
    {
    HASSERT(pi_TileID <= m_NumberOfTiles);

    // Clear flag is the first one, no adjustment of the bitmask

    return (m_pFlags.pData[ComputeByteContainingTile(pi_TileID)] & GetBitmaskForFlagsOfTile(pi_TileID)) != 0;
    }


/** -----------------------------------------------------------------------------
    Set the value of the clear flag of a tile.
    -----------------------------------------------------------------------------
*/
void HRATileStatus::SetClearFlag(uint64_t pi_TileID, bool pi_Value)
    {
    HASSERT(pi_TileID <= m_NumberOfTiles);

    // Clear flag is the first one, no adjustment of the bitmask

    if (pi_Value)  // true = set bit to 1
        {
        m_pFlags.pData[ComputeByteContainingTile(pi_TileID)] |= GetBitmaskForFlagsOfTile(pi_TileID);
        }
    else
        {
        m_pFlags.pData[ComputeByteContainingTile(pi_TileID)] &= ~GetBitmaskForFlagsOfTile(pi_TileID);
        }
    }


/** -----------------------------------------------------------------------------
    Retrieve the value of the "dirty for subres" flag of a tile.
    -----------------------------------------------------------------------------
*/
bool HRATileStatus::GetDirtyForSubResFlag(uint64_t pi_TileID)
    {
    HASSERT(pi_TileID <= m_NumberOfTiles);

    // Second flag of tile, adjust the bitmask by one position

    return (m_pFlags.pData[ComputeByteContainingTile(pi_TileID)] & (GetBitmaskForFlagsOfTile(pi_TileID) >> 1)) != 0;
    }


/** -----------------------------------------------------------------------------
    Set the value of the "dirty for subres" flag of a tile.
    -----------------------------------------------------------------------------
*/
void HRATileStatus::SetDirtyForSubResFlag(uint64_t pi_TileID, bool pi_Value)
    {
    HASSERT(pi_TileID <= m_NumberOfTiles);

    // Second flag of tile, adjust the bitmask by one position

    if (pi_Value)  // true = set bit to 1
        {
        m_pFlags.pData[ComputeByteContainingTile(pi_TileID)] |= (GetBitmaskForFlagsOfTile(pi_TileID) >> 1);
        }
    else
        {
        m_pFlags.pData[ComputeByteContainingTile(pi_TileID)] &= ~(GetBitmaskForFlagsOfTile(pi_TileID) >> 1);
        }
    }



//-----------------------------------------------------------------------------
// protected
// UpdateCachedSize
//
// this method is called by
//-----------------------------------------------------------------------------
void HRATiledRaster::HRATile::UpdateCachedSize()
    {
    // m_ObjectSize is a protected member of HPMPoolItem
    if (m_pTile != 0)
        {
        //  the tile is in memory and a IncrementRef was made by GetTile
        if (m_ObjectSize == 1)
            DecrementRef();
        m_ObjectSize = m_pTile->GetObjectSize();
        }
    else
        m_ObjectSize = 0;
    }

//-----------------------------------------------------------------------------
// class HRATile
//-----------------------------------------------------------------------------
HFCExclusiveKey HRATiledRaster::HRATile::s_Key;

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HRATiledRaster::HRATile::HRATile(const HFCPtr<HRABitmapBase>& pi_rpTile,
    HPMPool*                       pi_pPool)
    : HPMPoolItem(pi_pPool),
    m_pTile(pi_rpTile),
    m_Discartable(true),
    m_Invalidate(false)

    {
    HPRECONDITION(pi_rpTile != 0);
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HRATiledRaster::HRATile::~HRATile()
    {
    HPRECONDITION(m_pTiledRaster != 0);

    // remove tile from the HRATiledRaster list
    m_pTiledRaster->RemoveTile(m_Itr, !m_Discartable && m_pTile->ToBeSaved());

    // the tile can be save only if is discarded
    if (!m_Discartable)
        m_pTile->SetModificationState(false);
    }


// methods for this class are in HRATiledRaster.hpp

///////////////////////////
// HRATiledRaster class
///////////////////////////

HPM_REGISTER_CLASS(HRATiledRaster, HRAStoredRaster)


HMG_BEGIN_DUPLEX_MESSAGE_MAP(HRATiledRaster, HRAStoredRaster, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRATiledRaster, HRAContentChangedMsg, NotifyContentChanged)
HMG_REGISTER_MESSAGE(HRATiledRaster, HRPPaletteChangedMsg, NotifyPaletteChanged)
HMG_REGISTER_MESSAGE(HRATiledRaster, HRFProgressImageChangedMsg, NotifyProgressImageChanged)
HMG_REGISTER_MESSAGE(HRATiledRaster, HGFGeometryChangedMsg, NotifyGeometryChanged)
HMG_END_MESSAGE_MAP()

// Constants
//

#define TEMPORARY_SURFACE_MAX_MEMORY_SIZE 33554432 // 32 meg

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HRATiledRaster::HRATiledRaster ()
    : HRAStoredRaster (),
      m_TileSizeX           (0),
      m_TileSizeY           (0),
      m_TileSize            (0),
      m_TileObjectSize      (0),
      m_NumberOfTileX       (0),
      m_NumberOfTileY       (0),
      m_NumberOfTiles       (0),
      m_TileStatusDisabled  (false)
    {
    m_pPool                 = 0;
    m_pTileDescriptor       = 0;
    m_LookAheadEnabled      = false;
    m_LookAheadByExtent     = false;
    m_ComputeTileHistograms = false;

    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRATiledRaster::HRATiledRaster(const HFCPtr<HRABitmapBase>&   pi_pRasterModel,
                               uint64_t                  pi_TileSizeX,
                               uint64_t                  pi_TileSizeY,
                               uint64_t                  pi_WidthPixels,
                               uint64_t                  pi_HeightPixels,
                               HPMObjectStore*            pi_pStore,
                               HPMPool*                   pi_pPool,
                               bool                      pi_DisableTileStatus)

    : HRAStoredRaster   (pi_WidthPixels, pi_HeightPixels,
                         pi_pRasterModel->GetTransfoModel (),
                         pi_pRasterModel->GetCoordSys (),
                         pi_pRasterModel->GetPixelType ()),
    m_TileSizeX       (MAX(1, pi_TileSizeX)),
    m_TileSizeY       (MAX(1, pi_TileSizeY)),
    m_TileSize        (0),
    m_TileObjectSize  (0),
    m_NumberOfTileX   ((pi_WidthPixels  + (m_TileSizeX-1L)) / m_TileSizeX),
    m_NumberOfTileY   ((pi_HeightPixels + (m_TileSizeY-1L)) / m_TileSizeY),
    m_NumberOfTiles   (m_NumberOfTileX* m_NumberOfTileY),
    m_TileStatusDisabled(pi_DisableTileStatus)

    {
    m_pPool         = pi_pPool;
    SetStore(pi_pStore);

    // Exception-Safe
    m_pTileDescriptor        = 0;
    m_LookAheadEnabled        = false;
    m_LookAheadByExtent     = false;
    m_ComputeTileHistograms = false;

    Constructor (pi_pRasterModel,
                 pi_WidthPixels,
                 pi_HeightPixels);
    }


//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRATiledRaster::HRATiledRaster(const HRATiledRaster& pi_rObj)
    : HRAStoredRaster (pi_rObj),
      m_TileSizeX       (pi_rObj.m_TileSizeX),
      m_TileSizeY       (pi_rObj.m_TileSizeY),
      m_TileSize        (0),
      m_TileObjectSize  (0),
      m_NumberOfTileX   (pi_rObj.m_NumberOfTileX),
      m_NumberOfTileY   (pi_rObj.m_NumberOfTileY),
      m_NumberOfTiles   (m_NumberOfTileX* m_NumberOfTileY),
      m_TileStatusDisabled(pi_rObj.m_TileStatusDisabled)
    {
    // Exception-Safe
    m_pTileDescriptor       = 0;
    m_LookAheadEnabled      = false;
    m_LookAheadByExtent     = false;
    m_ComputeTileHistograms = false;

    // Perform initialization of the object
    DeepCopy    (pi_rObj);
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRATiledRaster::~HRATiledRaster()
    {
    // Artificially increment our reference count because
    // during the deep delete it is possible that the message
    // HRAModifiedTileNotSavedMsg is sent to HRAPyramidRaster.
    // This might lead to the inclusion of this tile raster in an HFCPtr
    // which will a second attempt to delete this tile raster.
    IncrementRef();

    DeepDelete ();

    DecrementRef();
    }


//-----------------------------------------------------------------------------
// public
// Assignment operation
//-----------------------------------------------------------------------------
HRATiledRaster& HRATiledRaster::operator=(const HRATiledRaster& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        HRAStoredRaster::operator=(pi_rObj);

        // Delete currently allocated memory for the object
        DeepDelete ();

        CopyMembers(pi_rObj);

        // Perform initialization of the object
        DeepCopy (pi_rObj);
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// Public
// Passed the request to the source
//-----------------------------------------------------------------------------
bool HRATiledRaster::HasLookAhead() const
    {
    return m_LookAheadEnabled;
    }


//-----------------------------------------------------------------------------
// Public
// Pass the request to the source
//-----------------------------------------------------------------------------
void HRATiledRaster::SetLookAhead(const HVEShape& pi_rShape,
                                  uint32_t        pi_ConsumerID,
                                  bool           pi_Async)
    {
    HPRECONDITION(HasLookAhead());

    // Bring the shape to the found resolution's coord sys
    HFCPtr<HVEShape> pShape(new HVEShape(pi_rShape));
    pShape->ChangeCoordSys(GetPhysicalCoordSys());

    //TR 270043
    //Currently, a raster is invalidated when the context has changed
    //(layer display state, annotation icon display state), so set the
    //new context.
	HRSObjectStore* pStore = (HRSObjectStore*)GetStore();

    if ((m_pContext != 0) && (m_pContext != pStore->GetContext()))
    {
    	pStore->SetContext(m_pContext);        
    }
			
    if (m_LookAheadByExtent)
        {
        Propagate(HRALookAheadMsg(*pShape,
                                  (unsigned short)(GetID() - HRSObjectStore::ID_TiledRaster),
                                  pi_ConsumerID,
                                  pi_Async));
        }
    else
        {
        // Calculate the list of IDs not already loaded
        HGFTileIDList TileList;
        bool AllTilesMissing;
        GetMissingTilesInRegion(pShape, TileList, &AllTilesMissing);

        if (!TileList.empty())
            {
            // send the LookAhead
            if (AllTilesMissing)  // all tiles are missing, use the shape
                Propagate(HRALookAheadMsg(*pShape,
                                          (unsigned short)(GetID() - HRSObjectStore::ID_TiledRaster),
                                          pi_ConsumerID,
                                          pi_Async));
            else
                Propagate(HRALookAheadMsg(TileList,
                                          (unsigned short)(GetID() - HRSObjectStore::ID_TiledRaster),
                                          pi_ConsumerID,
                                          pi_Async));
            }
        }
    }



//-----------------------------------------------------------------------------
// Notification for palette change
//-----------------------------------------------------------------------------
bool HRATiledRaster::NotifyPaletteChanged(const HMGMessage& pi_rMessage)
    {
    // we only propagate the "palette changed" messages coming from
    // the pixeltype of the tiled raster, not from the tiles.

    // compare the address of the sender with the one of the pixel type
    // if they are not equal, do not propagate the current message
    if(pi_rMessage.GetSender() != GetPixelType().GetPtr())
        return false;
        
    SetModificationState();

    // otherwise, propagate the message
    return true;
    }


//-----------------------------------------------------------------------------
// public
// InitPhysicalShape - Init the physical shape, set a new shape and flush
//                     the previous Data.
//-----------------------------------------------------------------------------
void HRATiledRaster::InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels)
    {
    // Don't call HRAStoredRaster::InitPhysicalShape because we create a new TiledRaster
    //
    HFCPtr<HRATiledRaster> pTmpRaster = new HRATiledRaster (m_pBitmapModel,
                                                            m_TileSizeX, m_TileSizeY,
                                                            pi_WidthPixels,
                                                            pi_HeightPixels,
                                                            GetStore(),
                                                            m_pPool,
                                                            m_TileStatusDisabled);
    // Keep the ID
    pTmpRaster->SetID (GetID());

    // Replace the object
    ReplaceObject (pTmpRaster);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
void HRATiledRaster::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys)
    {
    // Ancestor before...
    HRAStoredRaster::SetCoordSysImplementation(pi_rOldCoordSys);

    // Set model == to the TiledRaster
    m_pBitmapModel->SetCoordSys (GetCoordSys());

    // Change all CoordSys of tiles in memory
    HFCMonitor Monitor(m_TileMapKey);
    TileMapItr Itr(m_TileMap.begin());
    while (Itr != m_TileMap.end())
        {
        SetTileCoordSys(Itr);
        Itr++;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
void HRATiledRaster::SetTransfoModel(const HGF2DTransfoModel& pi_rModelCSp_CSl)
    {
    // Call the parent
    HRAStoredRaster::SetTransfoModel (pi_rModelCSp_CSl);

    // Set model == to the TiledRaster
    m_pBitmapModel->SetTransfoModel (pi_rModelCSp_CSl);

    // Save in the Store if present
    if (GetStore())
        {
        // Change all coordSys of tiles in memory
        HFCMonitor Monitor(m_TileMapKey);
        TileMapItr Itr(m_TileMap.begin());
        while (Itr != m_TileMap.end())
            {
            SetTileCoordSys(Itr);
            Itr++;
            }
        }
    else
        {
        // Change all Model in each Tile
        // Compose my current model with the new model between the new PhysicalCoordSys.
        HFCMonitor Monitor(m_TileMapKey);
        TileMapItr Itr(m_TileMap.begin());
        while (Itr != m_TileMap.end())
            {
            SetTileCoordSys(Itr);
            Itr++;
            }
        }
    }


//-----------------------------------------------------------------------------
// public
// GetRepresentativePalette
//-----------------------------------------------------------------------------
unsigned short HRATiledRaster::GetRepresentativePalette(HRARepPalParms* pio_pRepPalParms)
    {
    HPRECONDITION(pio_pRepPalParms != 0);

    unsigned short CountUsed = HRAStoredRaster::GetRepresentativePalette(pio_pRepPalParms);

    // if no operation has been done at the parent level or if the cache is not
    // updated
    if (CountUsed == 0)
        {
        // we copy the representative palette parameters structure and
        // its pixel type to not notify the tiled raster after each
        // unlock in the tiles
        HRARepPalParms RepPalParms(*pio_pRepPalParms);
        RepPalParms.SetPixelType((HRPPixelType*)RepPalParms.GetPixelType()->Clone());

        Byte  TilesInterval = 100 / pio_pRepPalParms->GetSamplingOptions().GetTilesToScan();
        uint64_t Tiles = m_NumberOfTileX * m_NumberOfTileY;
        uint64_t Tile = 0;
        const HRPPixelPalette& rPalette = (RepPalParms.GetPixelType())->GetPalette();

        HFCPtr<HRABitmapBase> pRaster;
        unsigned short Entries;
        uint32_t Index;

        // create a quantized palette object
        HRPQuantizedPalette* pQuantizedPalette = pio_pRepPalParms->CreateQuantizedPalette();
        // if not enough memory
        if (!pQuantizedPalette)
            goto wrap_up;

        // do a first pass for tiles in a look ahead way
        // Iterate on each tile to scan
        while (Tile < Tiles)
            {
            pRaster = GetTileByIndex(Tile)->GetTile();

            // get the representative palette of a tile
            // we create a new histogram for each tile if there is one
            if(pio_pRepPalParms->GetHistogram() != 0)
                RepPalParms.SetHistogram(new HRPHistogram(rPalette));

            Entries = pRaster->GetRepresentativePalette(&RepPalParms);

            // insert each entry of the representative palette in the
            // quantized object
            for (Index = 0; Index < Entries; Index++)
                {
                if (RepPalParms.GetHistogram() != 0)
                    {
                    if (!pQuantizedPalette->AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                              RepPalParms.GetHistogram()->GetEntryCount(Index)))
                        goto wrap_up;
                    }
                else
                    {
                    if (!pQuantizedPalette->AddCompositeValue(rPalette.GetCompositeValue(Index)))
                        goto wrap_up;
                    }
                }

            // go to next tile to scan
            Tile += TilesInterval;
            }
        // Release the last tile here, before we do anything else.
        pRaster = 0;

        // get the number of entries in the quantized palette
        CountUsed = pQuantizedPalette->GetPalette(&((pio_pRepPalParms->GetPixelType())->LockPalette()),
                                                  pio_pRepPalParms->GetHistogram());
        (pio_pRepPalParms->GetPixelType())->UnlockPalette();

        if (pio_pRepPalParms->UseCache())
            {
            // update the palette cache if required
            UpdateRepPalCache(CountUsed, (pio_pRepPalParms->GetPixelType())->GetPalette());
            }

wrap_up:
        if (pQuantizedPalette)
            delete pQuantizedPalette;
        }

    return CountUsed;
    }


//-----------------------------------------------------------------------------
// public
// ComputeHistogram
//-----------------------------------------------------------------------------
void HRATiledRaster::ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                      bool                pi_ForceRecompute)
    {
    HPRECONDITION(pio_pOptions != 0);

    if (pi_ForceRecompute || (GetHistogram() == 0) || (GetHistogram() != 0 && !GetHistogram()->CanBeUsedInPlaceOf(*pio_pOptions)))
        {
        // we copy the options structure and
        // its pixel type to not modify the original
        HRAHistogramOptions TmpOptions(*pio_pOptions);

        Byte TilesInterval = 100 / pio_pOptions->GetSamplingOptions().GetTilesToScan();

        HFCPtr<HRABitmapBase> pRaster;
        uint32_t HistogramEntries  = pio_pOptions->GetHistogram()->GetEntryFrequenciesSize();
        uint32_t HistogramChannels = pio_pOptions->GetHistogram()->GetChannelCount();
        uint32_t HistogramIndex;
        uint32_t ChannelIndex;

        if (pio_pOptions->GetSamplingOptions().GetRegionToScan() != 0)
            {
            HFCPtr<HVEShape> pPhysicalRegion(new HVEShape(*pio_pOptions->GetSamplingOptions().GetRegionToScan()));
            pPhysicalRegion->ChangeCoordSys(GetPhysicalCoordSys());

            // Work on specified region only...
            HVETileIDIterator TileIterator(m_pTileDescriptor, pPhysicalRegion);

            uint64_t Index = TileIterator.GetFirstTileIndex();
            while (Index != HGFTileIDDescriptor::INDEX_NOT_FOUND)
                {
                // get the tile
                pRaster = GetTileByIndex(Index)->GetTile();

                // compute the histogram
                if (TmpOptions.GetHistogram() != 0)
                    TmpOptions.ClearHistogram();
                else
                    TmpOptions.SetHistogram(new HRPHistogram(HistogramEntries, HistogramChannels, pio_pOptions->GetSamplingColorSpace()));

                if ((Index+1) % m_NumberOfTileX == 0 ||
                    Index >= (m_NumberOfTileY-1) * m_NumberOfTileX)
                    {
                    // Border tile: take its shape into account. GetTile shapes the border
                    // tiles but only the physical shape is used.
                    HFCPtr<HVEShape> pShapeWithTile(new HVEShape(*pPhysicalRegion));
                    pShapeWithTile->Intersect(*pRaster->GetEffectiveShape());

                    TmpOptions.GetSamplingOptions().SetRegionToScan(pShapeWithTile);
                    }
                else
                    {
                    // Reset the general region in the temp. options.
                    TmpOptions.GetSamplingOptions().SetRegionToScan(pPhysicalRegion);
                    }

                pRaster->ComputeHistogram(&TmpOptions, pi_ForceRecompute);

                // add each entry in the histogram
                for(ChannelIndex = 0; ChannelIndex < HistogramChannels; ChannelIndex++)
                    {
                    for(HistogramIndex = 0; HistogramIndex < HistogramEntries; HistogramIndex++)
                        pio_pOptions->GetHistogram()->IncrementEntryCount(HistogramIndex,
                                                                          TmpOptions.GetHistogram()->GetEntryCount(HistogramIndex, ChannelIndex),
                                                                          ChannelIndex);
                    }

                for (Byte SkipIndex = 0 ; SkipIndex < TilesInterval && Index != HGFTileIDDescriptor::INDEX_NOT_FOUND; SkipIndex++)
                    Index = TileIterator.GetNextTileIndex();
                }
            }
        else
            {
            uint64_t Tile = 0;
            uint64_t Tiles = m_NumberOfTileX * m_NumberOfTileY;

            HRAHistogramProgressIndicator::GetInstance()->Restart(Tiles);

            // Iterate on each tile to scan
            while (Tile < Tiles && HRAHistogramProgressIndicator::GetInstance()->ContinueIteration())
                {
                // get the tile
                pRaster = GetTileByIndex(Tile)->GetTile();

                // compute the histogram
                if (TmpOptions.GetHistogram() != 0)
                    TmpOptions.ClearHistogram();
                else
                    TmpOptions.SetHistogram(new HRPHistogram(HistogramEntries, HistogramChannels, pio_pOptions->GetSamplingColorSpace()));

                if ((Tile+1) % m_NumberOfTileX == 0 ||
                    Tile >= (m_NumberOfTileY-1) * m_NumberOfTileX)
                    {
                    // Border tile: take its shape into account. GetTile shapes the border
                    // tiles but only the physical shape is used.
                    TmpOptions.GetSamplingOptions().SetRegionToScan(pRaster->GetEffectiveShape());
                    }
                else
                    {
                    // Remove the region from the temp. options.
                    TmpOptions.GetSamplingOptions().SetRegionToScan(0);
                    }

                pRaster->ComputeHistogram(&TmpOptions, pi_ForceRecompute);

                // add each entry in the histogram
                for(ChannelIndex = 0; ChannelIndex < HistogramChannels; ChannelIndex++)
                    {
                    for(HistogramIndex = 0; HistogramIndex < HistogramEntries; HistogramIndex++)
                        pio_pOptions->GetHistogram()->IncrementEntryCount(HistogramIndex,
                                                                          TmpOptions.GetHistogram()->GetEntryCount(HistogramIndex, ChannelIndex),
                                                                          ChannelIndex);
                    }

                // go to next tile to scan
                Tile += TilesInterval;
                }
            }
        }
    else
        {
        HPRECONDITION(GetHistogram()->GetHistogram()->GetEntryFrequenciesSize() <=
                      pio_pOptions->GetHistogram()->GetEntryFrequenciesSize());

        HFCPtr<HRPHistogram> pHisto(new HRPHistogram(*GetHistogram()->GetHistogram()));
        HFCPtr<HRPHistogram> pOutHisto(pio_pOptions->GetHistogram());

        HASSERT(pOutHisto->GetChannelCount() == pHisto->GetChannelCount());
        // add each entry in the histogram
        for(uint32_t ChannelIndex = 0; ChannelIndex < pOutHisto->GetChannelCount(); ChannelIndex++)
            {
            for (uint32_t i = 0; i < pHisto->GetEntryFrequenciesSize(); i++)
                pOutHisto->IncrementEntryCount(i, pHisto->GetEntryCount(i,ChannelIndex),ChannelIndex);
            }
        }
    }


//-----------------------------------------------------------------------------
// public
// GetTile - From Index
// pi_NotInPool : When we don't want the pool to flush a tile because of
// getting this requested one.  Used only in UpdateNextRes.
//-----------------------------------------------------------------------------
const HFCPtr<HRATiledRaster::HRATile> HRATiledRaster::GetTileByIndex (uint64_t pi_Index, bool pi_NotInPool) const
    {
    HPRECONDITION (pi_Index < m_NumberOfTiles);

    HFCPtr<HRABitmapBase>         pTile;
    HFCPtr<HRATiledRaster::HRATile> pResult;
    bool                           InvalidatedTile = false;

    HFCMonitor PoolMonitor;

    // Lock pool
    // m_TileMap contain all HRATile in memory. Because HPMPool can be flush a tile
    // that we currently try to load, we must lock the pool to be sure that
    // m_TileMap is not modified during the process
    if (m_pPool != 0)
        PoolMonitor.Assign(*m_pPool);

    HFCMonitor TileMapMonitor(m_TileMapKey);

    // check if the map is on the tile map
    const_TileMapItr Itr(m_TileMap.find(pi_Index));

    if (Itr != m_TileMap.end() && !Itr->second->IsInvalidate())
        {
        pResult = Itr->second;
        if (pi_NotInPool)
            {
            if (pResult->HasInPool())
                {
                // the tile is in the pool
                pResult->Discard();
                }
            }
        }
    else
        {
        InvalidatedTile = (Itr != m_TileMap.end());

        // the tile isn't in the map, load it from the store
        HPMObjectStore* pStore = GetStore();

        if (pStore && pStore->IsCompatibleWith(HRSObjectStore::CLASS_ID))
            {
            uint64_t PosX;
            uint64_t PosY;

            if (m_TileSize != 0 && m_pPool != 0)
                m_pPool->NeedMemory((uint32_t)m_TileSize, (uint32_t)m_TileObjectSize);

            //TR 270043
            //Currently, a raster is invalidated when the context has changed
            //(layer display state, annotation icon display state), so set the
            //new context.            
            if ((m_pContext != 0) && (m_pContext != static_cast<HRSObjectStore*>(pStore)->GetContext()))
                {
                static_cast<HRSObjectStore*>(pStore)->SetContext(m_pContext);
                }

            pTile = static_cast<HRSObjectStore*>(pStore)->LoadTile(pi_Index,
                                                                   (uint32_t)(GetID() - HRSObjectStore::ID_TiledRaster),
                                                                   &PosX,
                                                                   &PosY,
                                                                   m_pPool);

        
            HFCPtr<HGF2DTransfoModel> pTileModel = GetTransfoModel();
            if (PosX != 0 || PosY != 0)
                {
                CHECK_HUINT64_TO_HDOUBLE_CONV(PosX)
                CHECK_HUINT64_TO_HDOUBLE_CONV(PosY)

                pTile->SetPosInRaster((HUINTX)PosX, (HUINTX)PosY);
            
                HGF2DTranslation TranslateModel (HGF2DDisplacement ((double)PosX, (double)PosY));

                pTileModel = TranslateModel.ComposeInverseWithDirectOf (*GetTransfoModel());
                }
            else
                pTile->SetPosInRaster(0, 0);

            if (pTileModel != 0)
                {
                pTile->SetTransfoModel(*pTileModel, GetCoordSys());
                }

            pTile->SetModificationState(false);

            if (m_TileSize == 0)
                {
                m_TileObjectSize = m_TileSize = (uint32_t)pTile->GetObjectSize();
                
                if (pTile->IsCompatibleWith(HRABitmap::CLASS_ID))
                    m_TileSize = static_cast<HRABitmap*>(pTile.GetPtr())->GetPacket()->GetBufferSize();
                else if(pTile->IsCompatibleWith(HRABitmapRLE::CLASS_ID))
                    m_TileSize = static_cast<HRABitmapRLE*>(pTile.GetPtr())->GetPacket()->GetBufferSize();
                }

            // clear the entire tile if requested previously
            if (!m_TileStatusDisabled && m_TileStatus.GetClearFlag(pi_Index))
                {
                pTile->Clear();
                m_TileStatus.SetClearFlag(pi_Index, false);
                }

            // Link ourselves to the tiles raster, to receive notifications
            LinkTo((HRARaster*)pTile);

            // We shape the tiles at the end of each line and at the bottom...
            if ((pi_Index + 1) % m_NumberOfTileX == 0 ||
                pi_Index >= (m_NumberOfTileY - 1) * m_NumberOfTileX)
                {
                uint64_t Width;
                uint64_t Height;

                GetSize(&Width, &Height);

                CHECK_HUINT64_TO_HDOUBLE_CONV(Width)
                CHECK_HUINT64_TO_HDOUBLE_CONV(Height)
                CHECK_HUINT64_TO_HDOUBLE_CONV(PosX)
                CHECK_HUINT64_TO_HDOUBLE_CONV(PosY)

                pTile->SetShape(HVEShape((double)PosX,
                                         (double)PosY,
                                         MIN((double)Width, (double)(PosX + m_TileSizeX)),
                                         MIN((double)Height, (double)(PosY + m_TileSizeY)),
                                         GetPhysicalCoordSys ()));

                pTile->SetModificationState(false);
                }

            if (m_ComputeTileHistograms)
                {
                // Compute tile histogram (will be used by pyramid to update main histogram after tile edition)
                HRAHistogramOptions HistoOptions(pTile->GetPixelType());

                // Set quality to 100%
                HRASamplingOptions SamplingOpt;
                SamplingOpt.SetPixelsToScan(100);
                SamplingOpt.SetTilesToScan(100);
                SamplingOpt.SetPyramidImageSize(100);
                HistoOptions.SetSamplingOptions(SamplingOpt);

                // Compute it!
                pTile->ComputeHistogram(&HistoOptions);
                }
            pTile->SetStore(pStore);

            if (InvalidatedTile == false)
                {
                // insert the new tile in the map
                pResult = new HRATile(pTile, m_pPool);
                pair<TileMapItr, bool> InsertResult;
                InsertResult = m_TileMap.insert(TileMap::value_type(pi_Index, pResult));
                HASSERT(InsertResult.second);
                // these members are use on the HRATile destructor.
                // before deleting HRATiledRaster, all tiles must be deleted
                pResult->m_Itr = InsertResult.first;
                pResult->m_pTiledRaster = const_cast<HRATiledRaster*>(this);
#ifdef __HMR_DEBUG_MEMBER
                pResult->m_TileIndex = pi_Index;
                pResult->m_Resolution = (uint32_t)(GetID() - HRSObjectStore::ID_TiledRaster);
#endif
                // it's a new tile, increment ref on it
                pResult->m_ObjectSize = 1;  // New tile, never in the pool
                pResult->IncrementRef();    // when the tile will be added in the pool,
                // DecrementRef will be called, see HRATile::UpdateCachedSize()
                }
            else
                {
                Itr->second->SetTile(pTile);
                Itr->second->Invalidate(false);
                pResult = Itr->second;
                }
            }
        }

    if (!pi_NotInPool && pResult->IsDiscartable())
        pResult->NotifyPool();
    else
        pResult->Discartable(false);

    HASSERT (pResult != 0);

    return (pResult);
    }

//-----------------------------------------------------------------------------
// public
// EngageTileHistogramComputing
//-----------------------------------------------------------------------------
void HRATiledRaster::EngageTileHistogramComputing()
    {
    // Compute histogram of all tiles already loaded...

    // Set histogram quality to 100%
    HRASamplingOptions SamplingOpt;
    SamplingOpt.SetPixelsToScan(100);
    SamplingOpt.SetTilesToScan(100);
    SamplingOpt.SetPyramidImageSize(100);

    HFCPtr<HRABitmapBase> pCurrentTile;

    HFCMonitor Monitor(m_TileMapKey);
    TileMapItr Itr(m_TileMap.begin());
    // scan tiles
    while (Itr != m_TileMap.end())
        {
        pCurrentTile = Itr->second->GetTile();
        HRAHistogramOptions HistoOptions(pCurrentTile->GetPixelType());
        HistoOptions.SetSamplingOptions(SamplingOpt);

        pCurrentTile->ComputeHistogram(&HistoOptions);
        Itr++;
        }

    // (Will compute histogram of all future loaded tiles)
    m_ComputeTileHistograms = true;
    }

//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// protected
// GetTileFromLoaded - From Index
//
// this method is used by HRAPyramidRaster
//-----------------------------------------------------------------------------
const HFCPtr<HRATiledRaster::HRATile> HRATiledRaster::GetTileFromLoaded(uint64_t pi_Index) const
    {
    HPRECONDITION (pi_Index < m_NumberOfTiles);

    HFCPtr<HRATiledRaster::HRATile> pResult;

    HFCMonitor Monitor(m_TileMapKey);

    // check if the map is on the tile map
    const_TileMapItr Itr(m_TileMap.find(pi_Index));

    if (Itr != m_TileMap.end())
        {
        pResult = Itr->second;
        }

    return pResult;
    }

//-----------------------------------------------------------------------------
// private
// Constructor
//-----------------------------------------------------------------------------
void HRATiledRaster::Constructor (const HFCPtr<HRABitmapBase>& pi_pRasterModel,
                                  uint64_t                      pi_WidthPixels,
                                  uint64_t                      pi_HeightPixels)
    {
    HPRECONDITION (pi_pRasterModel != 0);

    // Make a copy of the RasterModel.
    // Set CoordSys and Extent (TileSize)
    //
    m_pBitmapModel = (HRABitmapBase*)pi_pRasterModel->Clone();
    if (GetTransfoModel() != 0)
        {
        m_pBitmapModel->SetTransfoModel (*GetTransfoModel(), GetCoordSys());
        }

    m_pBitmapModel->InitSize (m_TileSizeX, m_TileSizeY);

    // Create the TileDescriptor
    m_pTileDescriptor = new HGFTileIDDescriptor (pi_WidthPixels, pi_HeightPixels, m_TileSizeX, m_TileSizeY);

    if (!m_TileStatusDisabled)
        m_TileStatus.Allocate(m_pTileDescriptor->GetTileCount());
    }


//-----------------------------------------------------------------------------
// private
// DeepCopy
//-----------------------------------------------------------------------------
void HRATiledRaster::DeepCopy(const HRATiledRaster& pi_rTiledRaster,
                              HPMObjectStore*       pi_pStore,
                              HPMPool*              pi_pPool)

    {
    try
        {
        // Copy the model
        m_pBitmapModel = (HRABitmapBase*)pi_rTiledRaster.m_pBitmapModel->Clone();

        // Alloc List of flags associated to tiles
        //
        m_TileStatus = pi_rTiledRaster.m_TileStatus;
        m_TileStatusDisabled = pi_rTiledRaster.m_TileStatusDisabled;

        m_pPool = pi_pPool;
        SetStore(pi_pStore);

        // copy each tile
        HFCMonitor SrcMonitor(pi_rTiledRaster.m_TileMapKey);
        HFCMonitor DstMonitor(m_TileMapKey);
        TileMapItr Itr(pi_rTiledRaster.m_TileMap.begin());
        while (Itr != pi_rTiledRaster.m_TileMap.end())
            {
            HFCPtr<HRABitmapBase> pNewTile((HRABitmapBase*)Itr->second->GetTile()->Clone());

            if (pi_pStore)
                pNewTile->SetStore(pi_pStore);

            HFCPtr<HRATile> pHRATile(new HRATile(pNewTile, m_pPool));
            pair<TileMapItr, bool> InsertResult;
            InsertResult = m_TileMap.insert(TileMap::value_type(Itr->first, pHRATile));
            HASSERT(InsertResult.second);
            // these members are use on the HRATile destructor.
            // before deleting HRATiledRaster, all tiles must be deleted
            pHRATile->m_Itr = InsertResult.first;
            pHRATile->m_pTiledRaster = this;

            // Link ourselves to the tile, to receive notifications
            LinkTo(pNewTile);

            Itr++;
            }

        SrcMonitor.ReleaseKey();
        DstMonitor.ReleaseKey();

        // Copy TileDrescriptor ID
        if (pi_rTiledRaster.m_pTileDescriptor)
            m_pTileDescriptor = new HGFTileIDDescriptor (*pi_rTiledRaster.m_pTileDescriptor);

        SetModificationState();
        }
    catch(...)
        {
        DeepDelete();
        }
    }


//-----------------------------------------------------------------------------
// private
// DeepDelete
//
// This method remove all HRATile from the pool. Because m_TileMap contain
// an HRATile pointer instead an HFCPtr, we need to put the HRATile pointer in
// an HFCPtr to manipulate the object.
// All HRATile will be removed from m_TileMap by his destructor
//-----------------------------------------------------------------------------
void HRATiledRaster::DeepDelete()
    {
    HFCMonitor Monitor(m_TileMapKey);

    // remove tile from pool
    TileMapItr Itr(m_TileMap.begin());
    HFCPtr<HRATile> pTile;
    while (Itr != m_TileMap.end())
        {
        pTile = Itr->second;
        // 2 case,
        //  the tile is in the pool
        //  the tile is in memory and a IncrementRef was made by GetTile, if never put in the pool.
        if (pTile->m_ObjectSize == 1)
            pTile->DecrementRef();
        else
            pTile->Discard();

        if (pTile->GetRefCount() != 1)
            {
            pTile->m_pTiledRaster = 0;
            }
        Itr++;
        }

    pTile = 0;  // ~HRATIle must be called before clearing the map
    m_TileMap.clear();

    // Delete TileDescriptor ID
    delete m_pTileDescriptor;
    m_pTileDescriptor = 0;
    }


//-----------------------------------------------------------------------------
// private
// ReplaceObject - Replace the object by the object in parameter and delete it
//-----------------------------------------------------------------------------
void HRATiledRaster::ReplaceObject (HFCPtr<HRATiledRaster>& pio_pRaster)
    {
    // For the performance do not call HRATiledRaster::Operator=
    //
    HRAStoredRaster::operator=(*pio_pRaster);

    // Delete currently allocated memory for the object
    DeepDelete ();

    // Replace value and pointer
    CopyMembers(*pio_pRaster);

    SetStore(pio_pRaster->GetStore());
    m_pPool           = pio_pRaster->m_pPool;

    m_TileStatus      = pio_pRaster->m_TileStatus;
    m_TileStatusDisabled = pio_pRaster->m_TileStatusDisabled;
    m_pBitmapModel    = pio_pRaster->m_pBitmapModel;
    m_pTileDescriptor = pio_pRaster->m_pTileDescriptor;

    // Link the list of TiledRaster with the object and
    // Unlink the list from the pio_pRaster
    HFCMonitor SrcMonitor(pio_pRaster->m_TileMapKey);
    HFCMonitor MyMonitor(m_TileMapKey);
    TileMapItr Itr(pio_pRaster->m_TileMap.begin());
    while (Itr != pio_pRaster->m_TileMap.begin())
        {
        HFCPtr<HRATile> pTile(Itr->second);
        pio_pRaster->UnlinkFrom(pTile->GetTile());
        LinkTo(pTile->GetTile());
        Itr++;
        }

    // Set to 0 --> Destructor must not delete these members.
    pio_pRaster->m_pBitmapModel             = 0;
    pio_pRaster->m_pTileDescriptor          = 0;
    }


//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
void HRATiledRaster::Clear()
    {
    HRAClearOptions ClearOptions;
    Clear(ClearOptions);
    }


//-----------------------------------------------------------------------------
// public
// Clear - Clear an area of the tiled raster
//-----------------------------------------------------------------------------
void HRATiledRaster::Clear(const HRAClearOptions& pi_rOptions)
    {
    HPRECONDITION(!m_TileStatusDisabled);

    // don't support the clear if tile status are disabled
    if (m_TileStatusDisabled)
        return;

    if (pi_rOptions.HasRLEMask())
        {
        ClearWithRLEMask(pi_rOptions);
        }
    else if (pi_rOptions.HasScanlines())
        {
        ClearWithScanlines(pi_rOptions);
        }
    else
        {
        const HFCPtr<HGF2DCoordSys>& PhysCoordSys = GetPhysicalCoordSys();

        HFCPtr<HVEShape> pShape;
        const HVEShape*  pClearShape;

        if (pi_rOptions.HasShape())
            {
            if (pi_rOptions.HasApplyRasterClipping())
                {
                pShape = (HVEShape*)GetEffectiveShape()->Clone();
                pShape->Intersect(*pi_rOptions.GetShape());
                pClearShape = pShape;
                }
            else
                pClearShape = pi_rOptions.GetShape();
            }
        else
            pClearShape = GetEffectiveShape();


        HRAClearOptions ClearOptions(pi_rOptions);
        ClearOptions.SetApplyRasterClipping(false);

        HGF2DExtent Extent = pClearShape->GetExtent();
        Extent.ChangeCoordSys(PhysCoordSys);
        uint64_t TileIndex = m_pTileDescriptor->GetFirstTileIndex(Extent);

        uint64_t PosX;
        uint64_t PosY;
        HVEShape ShapeIntersected;

        HFCPtr<HRATiledRaster::HRATile> pTile;
        TileMapItr Itr;

        // parse all the tiles partially or totally included in the shape
        while (TileIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
            {
            ComputeTileFromIndex(TileIndex, &PosX, &PosY);

            CHECK_HUINT64_TO_HDOUBLE_CONV(PosX)
            CHECK_HUINT64_TO_HDOUBLE_CONV(PosY)

            HVEShape Shape((double)PosX,
                           (double)PosY,
                           (double)(PosX + m_TileSizeX),
                           (double)(PosY + m_TileSizeY),
                           PhysCoordSys);

            ShapeIntersected = Shape;
            ShapeIntersected.Intersect(*pClearShape);

            // verify if the shape is not empty
            if (!ShapeIntersected.IsEmpty())
                {
                HFCMonitor Monitor(m_TileMapKey);

                Itr = m_TileMap.find(TileIndex);
                if (Itr != m_TileMap.end())
                    {
                    // The tile is loaded, clear the entire tile if in memory
                    // Get the tile ptr and clear it
                    pTile = Itr->second;

                    // set the tiled raster transaction on the tile
                    HFCPtr<HRATransaction> pCurTrans(pTile->GetTile()->GetCurrentTransaction());
                    pTile->GetTile()->SetCurrentTransaction(GetCurrentTransaction());
                    ClearOptions.SetShape(&ShapeIntersected);
                    pTile->GetTile()->Clear(ClearOptions);
                    pTile->GetTile()->SetCurrentTransaction(pCurTrans);
                    }
                else if (pi_rOptions.HasLoadingData() || GetCurrentTransaction() != 0)
                    {
                    // need to load the tile
                    pTile = GetTileByIndex(TileIndex);

                    // set the tiled raster transaction on the tile
                    HFCPtr<HRATransaction> pCurTrans(pTile->GetTile()->GetCurrentTransaction());
                    pTile->GetTile()->SetCurrentTransaction(GetCurrentTransaction());
                    ClearOptions.SetShape(&ShapeIntersected);
                    pTile->GetTile()->Clear(ClearOptions);
                    pTile->GetTile()->SetCurrentTransaction(pCurTrans);
                    }
                else
                    {
                    // This is a trap, if the tile is not loaded tiledraster will do nothing! Instead it will clear the whole tile 
                    // with a default color the first time it is loaded. If we had a shape and/or a specific color this is wrong! 
                    // When do we require this behavior?  
                    // Shape is somethimes equal to physical. BeAssert(!pi_rOptions.HasShape() && pi_rOptions.GetRawDataValue() == NULL);
                    
                    // The tile is not yet loaded, set a flag to be cleared later
                    m_TileStatus.SetClearFlag(TileIndex, true);
                    }
                }
            TileIndex = m_pTileDescriptor->GetNextTileIndex();
            }
        }
    }


//-----------------------------------------------------------------------------
// public
// NotifyContentChanged -
//-----------------------------------------------------------------------------
bool HRATiledRaster::NotifyContentChanged (const HMGMessage& pi_rMessage)
    {
    HPRECONDITION(!m_TileStatusDisabled);

    // don't support edition if tile status are disabled
    if (m_TileStatusDisabled)
        return false;

    HVEShape Shape(((HRAStoredRaster*)pi_rMessage.GetSender())->GetPhysicalExtent());
    Shape.ChangeCoordSys(GetPhysicalCoordSys());
    HGF2DExtent Extent(Shape.GetExtent());

    // The extent must be defined
    HASSERT(Extent.IsDefined());

    HFCGrid Grid(Extent.GetXMin(),
                    Extent.GetYMin(),
                    Extent.GetXMax(),
                    Extent.GetYMax());

    m_TileStatus.SetDirtyForSubResFlag(ComputeIndexFromTile (MAX(0, Grid.GetXMin()),
                                                                MAX(0, Grid.GetYMin())),
                                        true);
    
    SetModificationState();

    return true;
    }


//-----------------------------------------------------------------------------
// public
// NotifyProgressImageChanged -
//-----------------------------------------------------------------------------
bool HRATiledRaster::NotifyProgressImageChanged (const HMGMessage& pi_rMessage)
    {
    HPRECONDITION(pi_rMessage.GetClassID() == HRFProgressImageChangedMsg::CLASS_ID);

    uint64_t PosX  = ((HRFProgressImageChangedMsg&)pi_rMessage).GetPosX();
    uint64_t PosY  = ((HRFProgressImageChangedMsg&)pi_rMessage).GetPosY();
    bool  Ended = ((HRFProgressImageChangedMsg&)pi_rMessage).IsEnded();

    // Discard Tile, the will be reload
    uint64_t Index = ComputeIndexFromTile (PosX, PosY);
    HFCMonitor Monitor(m_TileMapKey);
    const_TileMapItr Itr;

    if ((Itr = m_TileMap.find(Index)) != m_TileMap.end())
        Itr->second->Discard();

    Monitor.ReleaseKey();

    CHECK_HUINT64_TO_HDOUBLE_CONV(PosX)
    CHECK_HUINT64_TO_HDOUBLE_CONV(PosY)

    // Propagate a new message HRA...
    Propagate (HRAProgressImageChangedMsg(HVEShape ((double)PosX,
                                                    (double)PosY,
                                                    (double)(PosX + m_TileSizeX),
                                                    (double)(PosY + m_TileSizeY),
                                                    GetPhysicalCoordSys ()),
                                          Ended));

    return false;
    }


//-----------------------------------------------------------------------------
// Receive a coordinate system changed notification
//-----------------------------------------------------------------------------
bool HRATiledRaster::NotifyGeometryChanged (const HMGMessage& pi_rMessage)
    {
    // Stop Message...
    return false;
    }


//-----------------------------------------------------------------------------
// Return the list of tile IDs that are not already loaded (in the region)
//-----------------------------------------------------------------------------
void HRATiledRaster::GetMissingTilesInRegion(const HFCPtr<HVEShape>& pi_rpRegion,
                                             HGFTileIDList&          pi_rList,
                                             bool*                  po_pAllTilesMissing) const
    {
    HVETileIDIterator TileIterator(m_pTileDescriptor, pi_rpRegion);
    *po_pAllTilesMissing = true;
    uint64_t Index = TileIterator.GetFirstTileIndex();
    HASSERT_X64(GetID() - HRSObjectStore::ID_TiledRaster <= ULONG_MAX);
    uint32_t Resolution = (uint32_t)(GetID() - HRSObjectStore::ID_TiledRaster);
    HFCMonitor Monitor(m_TileMapKey);
    TileMapItr Itr;
    while (Index != HGFTileIDDescriptor::INDEX_NOT_FOUND)
        {
        if ((Itr = m_TileMap.find(Index)) == m_TileMap.end() || Itr->second->IsInvalidate())
            pi_rList.push_back(m_pTileDescriptor->ComputeIDFromIndex(Index, Resolution));
        else
            *po_pAllTilesMissing = false;

        Index = TileIterator.GetNextTileIndex();
        }
    }

//-----------------------------------------------------------------------------
// public
// Draw
//-----------------------------------------------------------------------------
void HRATiledRaster::_Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    HASSERT(m_pBitmapModel->IsCompatibleWith(HRABitmapBase::CLASS_ID));

    HRADrawOptions Options(pi_Options);

    // set the effective coordsys
    if (Options.GetReplacingCoordSys() == 0)
        Options.SetReplacingCoordSys(GetCoordSys());

    //
    // Compute the region to draw
    //

    HFCPtr<HVEShape> pRegionToDraw;
    if (Options.GetShape() != 0)
        pRegionToDraw = new HVEShape(*Options.GetShape());
    else
        pRegionToDraw = new HVEShape(*GetEffectiveShape());

    // test if there is a clip region in the destination
    const HFCPtr<HGSRegion>& pClipRegion(pio_destSurface.GetRegion());
    if (pClipRegion != 0)
        {
        // if yes, intersect it with the destination
        HFCPtr<HVEShape> pSurfaceShape(pClipRegion->GetShape());

        pSurfaceShape->ChangeCoordSys(Options.GetReplacingCoordSys());
        pSurfaceShape->SetCoordSys(GetCoordSys());

        pRegionToDraw->Intersect(*pSurfaceShape);
        }
    else
        {
        // Create a rectangular clip region to stay
        // inside the destination surface.
        HVEShape DestSurfaceShape(0.0, 0.0, pio_destSurface.GetSurfaceDescriptor()->GetWidth(), pio_destSurface.GetSurfaceDescriptor()->GetHeight(), pio_destSurface.GetSurfaceCoordSys());

        // Set a quarter of a pixel stroke tolerance
        double CenterX = pio_destSurface.GetSurfaceDescriptor()->GetWidth() / 2.0;
        double CenterY = pio_destSurface.GetSurfaceDescriptor()->GetHeight() / 2.0;
        HFCPtr<HGFTolerance> pTol (new HGFTolerance(CenterX - DEFAULT_PIXEL_TOLERANCE,
                                                    CenterY - DEFAULT_PIXEL_TOLERANCE,
                                                    CenterX + DEFAULT_PIXEL_TOLERANCE,
                                                    CenterY + DEFAULT_PIXEL_TOLERANCE,
                                                    pio_destSurface.GetSurfaceCoordSys()));
        DestSurfaceShape.SetStrokeTolerance(pTol);

        //NEEDS_WORK_DCHG_ImagePP; this patch below is used by Descartes - see warpToArea.cpp)
        if (0/*&&AR m_IntersectForWarpToShape == TRUE*/)
            {
            //TR 287453/290748 - Ensure that the destination surface is intersected in the
            //            destination world instead of the world of the region to
            //            draw because it is possible that both of these worlds be
            //            connected by a projective, and only the region to draw
            //            has been intersected with the valid shape by the application
            //            (i.e. : Descartes).

            //Move the region to draw to the reference world.
            pRegionToDraw->ChangeCoordSys(GetCoordSys());
            pRegionToDraw->SetCoordSys(Options.GetReplacingCoordSys());

            //Change the region to draw coordinate system to the
            //destination coordinate system.
            pRegionToDraw->ChangeCoordSys(pio_destSurface.GetSurfaceCoordSys());

            pRegionToDraw->Intersect(DestSurfaceShape);

            //Move the region to draw back to source world.
            pRegionToDraw->ChangeCoordSys(Options.GetReplacingCoordSys());
            pRegionToDraw->SetCoordSys(GetCoordSys());
            }
        else
            {
            DestSurfaceShape.ChangeCoordSys(Options.GetReplacingCoordSys());
            DestSurfaceShape.SetCoordSys(GetCoordSys());

            pRegionToDraw->Intersect(DestSurfaceShape);
            }
        }

    Options.SetShape(pRegionToDraw);

    HFCPtr<HVEShape> pRegionToDrawCs (new HVEShape (*pRegionToDraw));
    pRegionToDrawCs->ChangeCoordSys(GetPhysicalCoordSys());

    HGF2DExtent ToDrawExtent(pRegionToDrawCs->GetExtent());

    HFCPtr<HVEShape> pNeededShape;
    uint32_t Neighborhood = Options.GetResamplingMode().GetNeighborhoodSize();
    if (Neighborhood > 0)
        {
        pNeededShape = new HVEShape(ToDrawExtent.GetXMin() - Neighborhood,
                                    ToDrawExtent.GetYMin() - Neighborhood,
                                    ToDrawExtent.GetXMax() + Neighborhood,
                                    ToDrawExtent.GetYMax() + Neighborhood,
                                    GetPhysicalCoordSys());

        // Set a quarter of a pixel tolerance
        double CenterX = (ToDrawExtent.GetXMax() + ToDrawExtent.GetXMin()) / 2.0;
        double CenterY = (ToDrawExtent.GetYMax() + ToDrawExtent.GetYMin()) / 2.0;
        HFCPtr<HGFTolerance> pTol (new HGFTolerance(CenterX - DEFAULT_PIXEL_TOLERANCE,
                                                    CenterY - DEFAULT_PIXEL_TOLERANCE,
                                                    CenterX + DEFAULT_PIXEL_TOLERANCE,
                                                    CenterY + DEFAULT_PIXEL_TOLERANCE,
                                                    GetPhysicalCoordSys()));
        pNeededShape->SetStrokeTolerance(pTol);
        }
    else
        {
        pNeededShape = pRegionToDrawCs;
        }


    HVETileIDIterator TileIterator(m_pTileDescriptor, pNeededShape);
    uint64_t Index = TileIterator.GetFirstTileIndex();
    if (Index != HGFTileIDDescriptor::INDEX_NOT_FOUND)
        {
        // Optimization
        // If no alpha in the source and destination bitmap, we can skip the temporary copy.
        if (!pi_Options.ApplyAlphaBlend() ||
            (GetPixelType()->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) == HRPChannelType::FREE &&
             pio_destSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID) &&
             ((HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)(pio_destSurface.GetSurfaceDescriptor()))->GetPixelType()->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0)
             == HRPChannelType::FREE) )
            {
            HFCPtr<HRATiledRaster::HRATile> pTile;
            do
                {
                HFCPtr<HVEShape> pCurrentClip(new HVEShape(*pRegionToDraw));
                pTile = GetTileByIndex(Index);
                pCurrentClip->Intersect(*pTile->GetTile()->GetEffectiveShape());
                Options.SetShape(pCurrentClip);


                //TR 300554 : Set the size of the data in the tile, which can be less than the tile size,
                //            to ensure that the sampler doesn't take a pixel outside the valid range.
                if ((Options.GetDataDimensionFix() == true) &&
                    (pTile->GetTile()->IsCompatibleWith(HRABitmapBase::CLASS_ID)))
                    {
                    uint64_t dataWidth;
                    uint64_t dataHeight;

                    m_pTileDescriptor->GetTileDataSize(Index, dataWidth, dataHeight);

                    HASSERT((dataWidth <= ULONG_MAX) && (dataHeight <= ULONG_MAX));

                    HFCPtr<HGSSurfaceDescriptor> pSurfaceDescriptor(pTile->GetTile()->GetSurfaceDescriptor());

                    pSurfaceDescriptor->SetDataDimensions((uint32_t)dataWidth, (uint32_t)dataHeight);
                    }

                pTile->GetTile()->Draw(pio_destSurface, Options);

                Options.SetShape(0);

                Index = TileIterator.GetNextTileIndex();
                }
            while (Index != HGFTileIDDescriptor::INDEX_NOT_FOUND);
            }
        // Optimization
        // Only 1 tile to draw, no need for temporary stuff.
        else if (TileIterator.GetNextTileIndex() == HGFTileIDDescriptor::INDEX_NOT_FOUND)
            {
            HFCPtr<HVEShape> pClipShape(new HVEShape(*pRegionToDraw));
            HFCPtr<HRATiledRaster::HRATile> pTile(GetTileByIndex(Index));
            pClipShape->Intersect(*pTile->GetTile()->GetEffectiveShape());
            Options.SetShape(pClipShape);

            pTile->GetTile()->Draw(pio_destSurface, Options);
            }
        else
            // We need a temporary copy to resolve problem with the translucent pixel, by default we can set more than one
            // time the same pixel in the destination bitmap, it is not a good idea when we have pixels translucent because we
            // compose the alpha channel at each time.
            {
            uint64_t Width;
            uint64_t Height;
            GetSize(&Width, &Height);

            // ToDrawExtent is in physical
            uint64_t XMin = (uint64_t)MAX(ToDrawExtent.GetXMin() + HGLOBAL_EPSILON - Neighborhood, 0.0);
            uint64_t YMin = (uint64_t)MAX(ToDrawExtent.GetYMin() + HGLOBAL_EPSILON - Neighborhood, 0.0);
            uint64_t XMax = MIN((uint64_t)MAX(ToDrawExtent.GetXMax() - HGLOBAL_EPSILON + Neighborhood, 0.0), Width - 1);
            uint64_t YMax = MIN((uint64_t)MAX(ToDrawExtent.GetYMax() - HGLOBAL_EPSILON + Neighborhood, 0.0), Height - 1);

            Width = XMax + 1 - XMin;
            Height = YMax + 1 - YMin;

            // check how many memory we need to create the temporary surface
            uint64_t StripWidthInPixel = ((Width * GetPixelType()->CountPixelRawDataBits()) + 7) / 8;
            uint64_t StripHeight = Height;
            uint64_t StripSize = Width * Height;

            HFCPtr<HRABitmapBase> pTempBitmap((HRABitmapBase*)m_pBitmapModel->Clone());

            if (StripSize <= TEMPORARY_SURFACE_MAX_MEMORY_SIZE)
                {
                pTempBitmap->InitSize (Width, Height);
                }
            else
                {
                StripHeight = TEMPORARY_SURFACE_MAX_MEMORY_SIZE / StripWidthInPixel;
                StripHeight = MAX(StripHeight, 1);
                pTempBitmap->InitSize (Width, StripHeight);
                }

            while (YMin < YMax)
                {
                try
                    {
                    CHECK_HUINT64_TO_HDOUBLE_CONV(XMin);
                    CHECK_HUINT64_TO_HDOUBLE_CONV(YMin);
                    HGF2DTranslation NewModel(HGF2DDisplacement((double)XMin, (double)YMin));

                    HFCPtr<HGF2DTransfoModel> pModel(NewModel.ComposeInverseWithDirectOf(*GetTransfoModel()));

                    HFCPtr<HGF2DTransfoModel> pSimplifiedModel(pModel->CreateSimplifiedModel());
                    if (pSimplifiedModel == 0)
                        pSimplifiedModel = pModel;

                    // Add the translation for the tile to the model in the coordSys.
                    pTempBitmap->SetTransfoModel (*pSimplifiedModel, GetCoordSys());

                    // Clear the bitmap before copying tiles in it. Normally, the
                    // bitmap would be filled with our tiles, but this is in case
                    // one of the tiles is inaccessible (internet file for example)
                    //
                    // This will also force the allocation.
                    pTempBitmap->Clear();

                    HFCPtr<HGSSurfaceDescriptor> pDescriptor(pTempBitmap->GetSurfaceDescriptor());

                    // create a surface for the destination
                    HGFMappedSurface destSurface(pDescriptor, pTempBitmap->GetPhysicalCoordSys());

                    HFCPtr<HVEShape> pTempBitmapShape(new HVEShape(*pTempBitmap->GetEffectiveShape()));
                    pTempBitmapShape->ChangeCoordSys(GetPhysicalCoordSys());

                    HVETileIDIterator TempTileIterator(m_pTileDescriptor, pTempBitmapShape);
                    uint64_t TempIndex = TempTileIterator.GetFirstTileIndex();
                    HFCPtr<HRAStoredRaster> pTile;
                    HRADrawOptions TempOptions;
                    while (TempIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
                        {
                        HFCPtr<HVEShape> pDrawShape(new HVEShape(*pTempBitmapShape));
                        pTile = GetTileByIndex(TempIndex)->GetTile();
                        pDrawShape->Intersect(*pTile->GetEffectiveShape());

                        TempOptions.SetShape(pDrawShape);
                        pTile->Draw(destSurface, TempOptions);

                        TempIndex = TempTileIterator.GetNextTileIndex();
                        }

                    // We still need to shape the draw, as the intersection
                    // of the request and what we generated.
                    pTempBitmapShape->Intersect(*pRegionToDraw);
                    Options.SetShape(pTempBitmapShape);

                    pTempBitmap->Draw(pio_destSurface, Options);

                    YMin += StripHeight;
                    }
                catch(HFCOutOfMemoryException&)
                    {
                    // separate tiles case.
                    pTempBitmap = 0;

                    HWARNING(0, "HRATiledRaster::Draw out of memory. Drawing tiles separately");

                    uint64_t Index = TileIterator.GetFirstTileIndex();
                    HFCPtr<HRATiledRaster::HRATile> pTile;
                    while (Index != HGFTileIDDescriptor::INDEX_NOT_FOUND)
                        {
                        HFCPtr<HVEShape> pCurrentClip(new HVEShape(*pRegionToDraw));
                        pTile = GetTileByIndex(Index);
                        pCurrentClip->Intersect(*pTile->GetTile()->GetEffectiveShape());
                        Options.SetShape(pCurrentClip);

                        pTile->GetTile()->Draw(pio_destSurface, Options);

                        Options.SetShape(0);

                        Index = TileIterator.GetNextTileIndex();
                        }
                    }
                }
            }
        }
    }



//-----------------------------------------------------------------------------
// private
// SaveTiles
//
// This method remove all HRATile from the pool. Because m_TileMap contain
// an HRATile pointer instead an HFCPtr, we need to put the HRATile pointer in
// an HFCPtr to manipulate the object.
// All HRATile will be removed from m_TileMap by his destructor
//
// Note : The Itr must be increment before the HRATile destructor. The HRATile
//        destructor will remove the object from m_TileMap. The destructor
//        will be called by the HFCPtr, this mean on the pTile assignment,
//        at this point, the Itr already point on the next HRATile
//---------------------------------------------------------------------------
void HRATiledRaster::SaveAndFlushTiles ()
    {
    HFCMonitor Monitor(m_TileMapKey);
    HFCPtr<HRATile> pTile;
    while (!m_TileMap.empty())
        {
        if (m_TileMap.begin()->second->IsDiscartable())
            {
            pTile = m_TileMap.begin()->second;
            pTile->Discard();   // always use an HFCPtr to call method on HRATile
            pTile = 0;          // release immediatly the tile
            }
        else
            m_TileMap.erase(m_TileMap.begin());

        }
    HPOSTCONDITION(m_TileMap.empty());
    }

//-----------------------------------------------------------------------------
// private
// SaveTiles
//-----------------------------------------------------------------------------
void HRATiledRaster::SaveTiles()
    {
    HFCMonitor Monitor(m_TileMapKey);
    TileMapItr Itr(m_TileMap.begin());
    HFCPtr<HRATile> pTile;
    while (Itr != m_TileMap.end())
        {
        if (Itr->second->IsDiscartable())
            {
            if (Itr->second->GetTile()->ToBeSaved())
                {
                pTile = Itr->second;        // the HRATile must be used with an HRFPtr because the tile can
                pTile->GetTile()->Save();   // can be discarded by the Save()
                pTile->GetTile()->SetModificationState(false);
                }
            }
        else
            {
            // with this message, HRAPyramidRaster will set all parents of this tile to dirty for sub res
            Propagate(HRAModifiedTileNotSavedMsg((unsigned short)(GetID() - HRSObjectStore::ID_TiledRaster),
                                                 Itr->first));

            // the tile is undiscartable,
            // - this means that the tile cannot be in pool
            HPRECONDITION(Itr->second->m_ObjectSize == 1);  // the tile is in memory

            // the tile is UnDiscartable, only m_TileMap must have a reference on it
            HPRECONDITION(Itr->second->GetRefCount() == 1);
            pTile = Itr->second;
            pTile->DecrementRef();  // an IncrementRef was made by GetTile()
            }
        Itr++;
        }
    }




//-----------------------------------------------------------------------------
// private
// RemoveTile
//
// this method is called by HRATiledRaster::HRATile
//-----------------------------------------------------------------------------
void HRATiledRaster::RemoveTile(HRATiledRaster::TileMapItr& pi_rItr, bool pi_WillBeNotSaved)
    {
    if (pi_WillBeNotSaved)
        {
        Propagate(HRAModifiedTileNotSavedMsg((unsigned short)(GetID() - HRSObjectStore::ID_TiledRaster),
                                             pi_rItr->first));
        }

    HFCMonitor Monitor(m_TileMapKey);
    m_TileMap.erase(pi_rItr);
    }



//-----------------------------------------------------------------------------
// private
// Clear - Clear an area of the tiled raster
//-----------------------------------------------------------------------------
void HRATiledRaster::ClearWithRLEMask(const HRAClearOptions& pi_rOptions)
    {
    HPRECONDITION(pi_rOptions.GetRawDataValue() != 0);
    HPRECONDITION(!pi_rOptions.HasShape() && !pi_rOptions.HasApplyRasterClipping());

#ifdef __HMR_DEBUG
    HPRECONDITION(pi_rOptions.GetRLEMask()->GetCodec() != 0);
    uint64_t Width;
    uint64_t Height;
    GetSize(&Width, &Height);
    HPRECONDITION(pi_rOptions.GetRLEMask()->GetCodec()->GetWidth() == Width && pi_rOptions.GetRLEMask()->GetCodec()->GetHeight() == Height);
#endif

    void* pRawData = const_cast<void*>((pi_rOptions.GetRawDataValue() == 0 ? GetPixelType()->GetDefaultRawData() : pi_rOptions.GetRawDataValue()));
    unsigned short OutData[4];
    uint64_t ImageWidth;
    uint64_t ImageHeight;
    GetSize(&ImageWidth, &ImageHeight);

    HRAEditor* pEditor = NULL;
    HAutoPtr<HRABitmapEditor> pBitmapEditor;

    if (m_NumberOfTileX == 1)
        {
        HAutoPtr<HCDPacketRLE::RLEScanlineGenerator> pScanlineGenerator;
        HUINTX PixelCount;
        HUINTX PosX;
        HFCPtr<HRABitmapBase> pTile;
        uint64_t BlockPosX = 0;
        uint64_t CurrentTileIndex = -1;
        HUINTX TileXPosInRaster=0;
        HUINTX TileYPosInRaster=0;

        // first, find the first run to clear
        for (HUINTX PosY = 0; PosY < ImageHeight; ++PosY)
            {
            pScanlineGenerator = pi_rOptions.GetRLEMask()->GetRLEScanlineGenerator(PosY, true);  // state = 1
            PixelCount = pScanlineGenerator->GetFirstScanline(&PosX);
            while (PixelCount != 0)
                {
                // we have a line, get the corresponding strip
                uint64_t TileIndex = m_pTileDescriptor->ComputeIndex(BlockPosX, PosY);
                if (TileIndex != CurrentTileIndex)
                    {
                    if (pTile)
                        pTile->Updated();

                    pTile = GetTileByIndex(TileIndex)->GetTile();
                    pTile->GetPosInRaster(&TileXPosInRaster, &TileYPosInRaster);
                    pBitmapEditor = (HRABitmapEditor*)pTile->CreateEditor(HFC_WRITE_ONLY);
                    pEditor = pBitmapEditor->GetSurfaceEditor();
                    CurrentTileIndex = TileIndex;

                    HPRECONDITION(pEditor != 0);
                    HPRECONDITION(pEditor->GetSurface().GetSurfaceDescriptor() != 0);
                    HPRECONDITION(pEditor->GetSurface().GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));

                    HFCPtr<HRPPixelType> pPixelTypeRLE1 = ((const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)pEditor->GetSurface().GetSurfaceDescriptor())->GetPixelType();

                    // We assume the converter will return RLE run
                    if (pPixelTypeRLE1 != 0 &&
                        (pPixelTypeRLE1->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || pPixelTypeRLE1->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID)))
                        {
                        HFCPtr<HRPPixelConverter> pConverter(GetPixelType()->GetConverterTo(pPixelTypeRLE1));
                        pConverter->Convert((pi_rOptions.GetRawDataValue() != 0 ? pi_rOptions.GetRawDataValue() : GetPixelType()->GetDefaultRawData()),
                                            &OutData);
                        OutData[0] = OutData[0] ? 0 : 1; // TR 159986: must clear with state(0,1) not RLE run.
                        pRawData = OutData;
                        }
                    }

                pEditor->ClearRun(PosX,
                                  PosY - TileYPosInRaster,
                                  PixelCount,
                                  pRawData,
                                  GetCurrentTransaction());

                PixelCount = pScanlineGenerator->GetNextScanline(&PosX);
                }
            }
        pEditor = 0;
        pBitmapEditor = 0;
        if (pTile)
            pTile->Updated();
        }
    else
        {
        HArrayAutoPtr<HAutoPtr<HCDPacketRLE::RLEScanlineGenerator> > pScanlinesGenerator;
        pScanlinesGenerator = new HAutoPtr<HCDPacketRLE::RLEScanlineGenerator>[(size_t)m_TileSizeY];

        // process image by strip
        for (HUINTX Strip = 0; Strip < m_NumberOfTileY; ++Strip)
            {
            HUINTX StripPos = Strip * (HUINTX)m_TileSizeY;
            HUINTX LineCount = MIN((HUINTX)m_TileSizeY, (HUINTX)ImageHeight - StripPos);
            HUINTX NextPosX = HUINTX_MAX;
            HUINTX PosX;

            // create the scanline editor on each line
            // find the first X pos
            for (HUINTX i = 0; i < LineCount; ++i)
                {
                pScanlinesGenerator[i] = pi_rOptions.GetRLEMask()->GetRLEScanlineGenerator(StripPos + i, true); // state = 1
                if (pScanlinesGenerator[i]->GetFirstScanline(&PosX) != 0)
                    NextPosX = MIN(NextPosX, PosX);
                }

            while (NextPosX < ImageWidth)
                {
                // get the specific tile
                HUINTX TilePosX = NextPosX / (HUINTX)m_TileSizeX;
                uint64_t TileIndex = m_pTileDescriptor->ComputeIndex(TilePosX * m_TileSizeX, Strip * m_TileSizeY);

                // get the tile and process it
                HFCPtr<HRABitmapBase> pTile(GetTileByIndex(TileIndex)->GetTile());
                pBitmapEditor = (HRABitmapEditor*)pTile->CreateEditor(HFC_WRITE_ONLY);
                pEditor = pBitmapEditor->GetSurfaceEditor();

                HUINTX TilePosXInRaster;
                HUINTX TilePosYInRaster;
                pTile->GetPosInRaster(&TilePosXInRaster, &TilePosYInRaster);

                HUINTX NextTilePosX = TilePosXInRaster + (HUINTX)m_TileSizeX;

                NextPosX = HUINTX_MAX;

                // process all lines in the tile
                for (HUINTX LineInStrip = 0; LineInStrip < LineCount; ++LineInStrip)
                    {
                    HUINTX PixelCount = pScanlinesGenerator[LineInStrip]->GetCurrentScanline(&PosX);

                    // process all run in this tile line
                    while (PixelCount != 0)
                        {
                        // EndPos is one past the last pixel to clear!
                        HUINTX EndPos = PosX + PixelCount;

                        if (EndPos <= TilePosXInRaster)
                            {
                            // go to next run
                            PixelCount = pScanlinesGenerator[LineInStrip]->GetNextScanline(&PosX);
                            continue;
                            }

                        if (PosX >= NextTilePosX)
                            {
                            NextPosX = MIN(NextPosX, PosX);
                            break;  // Don't process the run yet, go to next line.
                            }

                        // If we get here, there is an intersection between the run and the tile.
                        HUINTX StartX = MAX (PosX, TilePosXInRaster);
                        HUINTX EndX   = MIN (EndPos, NextTilePosX);
                        HASSERT (EndX > StartX);

                        pEditor->ClearRun(StartX - TilePosXInRaster,
                                          LineInStrip,
                                          EndX - StartX,
                                          pRawData,
                                          GetCurrentTransaction());

                        if (EndPos <= NextTilePosX)
                            {
                            // The run do not extends the tile, get next run and continue to process this line.
                            // Maybe we reach the end of the tile, but we need to continue in order to set NextPosX properly.
                            PixelCount = pScanlinesGenerator[LineInStrip]->GetNextScanline(&PosX);
                            }
                        else
                            {
                            // The run extends the tile, set NextPosX to the beginning of the next tile.
                            if (NextPosX != NextTilePosX)
                                NextPosX = NextTilePosX;

                            // The line is finished, go to next
                            break;  // Go to next line.
                            }
                        }
                    }
                pEditor = 0;
                pBitmapEditor = 0;
                pTile->Updated();
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// private
// Clear - Clear an area of the tiled raster
//-----------------------------------------------------------------------------
void HRATiledRaster::ClearWithScanlines(const HRAClearOptions& pi_rOptions)
    {
    HASSERT(!L"Not Yet Supported");
    }

//-----------------------------------------------------------------------------
// public
// InvalidateRaster : Invalidate the tiles
//-----------------------------------------------------------------------------
void HRATiledRaster::InvalidateRaster()
    {
    HRATiledRaster::TileMapItr TileIter = m_TileMap.begin();
    HRATiledRaster::TileMapItr TileIterEnd = m_TileMap.end();

    while (TileIter != TileIterEnd)
        {
        TileIter->second->Invalidate(true);
        TileIter++;
        }

    //Currently, a raster is invalidated when the context has changed
    //(layer display state, annotation icon display state), so set the
    //new context.
    if (GetContext() != 0)
        {
        HPMObjectStore* pStore = GetStore();
        ((HRSObjectStore*)pStore)->SetContext(GetContext());
        }
    }

//-----------------------------------------------------------------------------
// public
// Clone -
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> HRATiledRaster::Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pPool) const
    {
    // Store Specify ?
    if (pi_pStore)
        {
        // Copy the Tile Raster
        //
        HRATiledRaster* pTmpRaster = new HRATiledRaster ();

        pTmpRaster->HRAStoredRaster::operator=(*this);

        pTmpRaster->DeepDelete();

        pTmpRaster->CopyMembers(*this);

        pTmpRaster->DeepCopy (*this, pi_pStore, pi_pPool);

        return (pTmpRaster);
        }
    else
        return new HRATiledRaster (*this);
    }

//-----------------------------------------------------------------------------
// private
// SetTileCoordSys
// CS parameter is Logical CS to use. TransfoModel must already be set.
//-----------------------------------------------------------------------------
void HRATiledRaster::SetTileCoordSys (const const_TileMapItr& pi_rItr)
    {
    uint64_t Column;
    uint64_t Line;

    HFCMonitor Monitor(m_TileMapKey);
    HFCPtr<HRAStoredRaster> pTile(pi_rItr->second->GetTile());
    // Create Translation model
    ComputeTileFromIndex (pi_rItr->first,
        &Column,
        &Line);

    CHECK_HUINT64_TO_HDOUBLE_CONV(Column)
        CHECK_HUINT64_TO_HDOUBLE_CONV(Line)

        HGF2DTranslation NewModel (HGF2DDisplacement ((double)Column, (double)Line));

    // Don't modify the ToBeSave status, because the Tile Models are not
    // saved in the file. (only the TiledRaster Shape)
    bool PrevSaveStatus = pTile->ToBeSaved();


    // Add the translation for the tile to the model in the coordSys.
    pTile->SetTransfoModel (*NewModel.ComposeInverseWithDirectOf (*GetTransfoModel()), GetCoordSys());

    // Reset the previous value.
    pTile->SetModificationState(PrevSaveStatus);
    }

//-----------------------------------------------------------------------------
// public
// CreateIterator    - Create an iterator.
//-----------------------------------------------------------------------------
HRARasterIterator* HRATiledRaster::CreateIterator(const HRAIteratorOptions& pi_rOptions) const
    {
    return new HRATiledRasterIterator (HFCPtr<HRATiledRaster>((HRATiledRaster*)this), pi_rOptions);
    }

//-----------------------------------------------------------------------------
// private
// ComputeIndexFromTile
//-----------------------------------------------------------------------------
inline uint64_t HRATiledRaster::ComputeIndexFromTile (uint64_t pi_PosX,
                                                    uint64_t pi_PosY) const
    {
    return (m_pTileDescriptor->ComputeIndex (pi_PosX, pi_PosY));
    }

//-----------------------------------------------------------------------------
// private
// ComputeTileFromIndex
//-----------------------------------------------------------------------------
inline void HRATiledRaster::ComputeTileFromIndex (uint64_t pi_Index,
                                                  uint64_t*  po_PosX,
                                                  uint64_t*  po_PosY) const
    {
    m_pTileDescriptor->GetPositionFromIndex (pi_Index, po_PosX, po_PosY);
    }

//-----------------------------------------------------------------------------
// private
// ComputeTileID - The tile ID is created from the tile Index and the Level from
//                 the TiledRaster ID if present.
//-----------------------------------------------------------------------------
inline uint64_t HRATiledRaster::ComputeTileID (uint32_t pi_RasterID,
                                             uint64_t pi_TileIndex) const
    {
    uint64_t TileID (pi_TileIndex);

    if (pi_RasterID != INVALID_OBJECT_ID)
        {
        TileID = m_pTileDescriptor->ComputeIDFromIndex (pi_TileIndex,
            m_pTileDescriptor->GetLevel (pi_RasterID));
        }

    return (TileID);
    }

//-----------------------------------------------------------------------------
// public
// GetTile - Get tile from Position.
//-----------------------------------------------------------------------------
const HFCPtr<HRATiledRaster::HRATile> HRATiledRaster::GetTile (uint64_t pi_PosX, uint64_t pi_PosY) const
    {
    return (GetTileByIndex(ComputeIndexFromTile (pi_PosX, pi_PosY)));
    }


//-----------------------------------------------------------------------------
// private
// CopyMembers   -
//-----------------------------------------------------------------------------
void HRATiledRaster::CopyMembers (const HRATiledRaster& pi_rObj)
    {
    m_pPool             = 0;
    m_TileSizeX         = pi_rObj.m_TileSizeX;
    m_TileSizeY         = pi_rObj.m_TileSizeY;
    m_NumberOfTileX     = pi_rObj.m_NumberOfTileX;
    m_NumberOfTileY     = pi_rObj.m_NumberOfTileY;
    m_NumberOfTiles     = pi_rObj.m_NumberOfTiles;
    m_LookAheadEnabled  = false;
    m_LookAheadByExtent = false;
    }


//-----------------------------------------------------------------------------
// Returns pointer to pool
//-----------------------------------------------------------------------------
HPMPool* HRATiledRaster::GetPool()
    {
    return m_pPool;
    }


//-----------------------------------------------------------------------------
// Return a new copy of self
//-----------------------------------------------------------------------------
HPMPersistentObject* HRATiledRaster::Clone () const
    {
    return new HRATiledRaster(*this);
    }

//-----------------------------------------------------------------------------
// public
// Create an editor
//-----------------------------------------------------------------------------
HRARasterEditor* HRATiledRaster::CreateEditor (HFCAccessMode pi_Mode)
    {
    return 0;
    }


//-----------------------------------------------------------------------------
// public
// Create a shaped editor
//-----------------------------------------------------------------------------
HRARasterEditor* HRATiledRaster::CreateEditor (const HVEShape& pi_rShape, HFCAccessMode pi_Mode)
    {
    return 0;
    }


//-----------------------------------------------------------------------------
// public
// Create an unshaped editor
//-----------------------------------------------------------------------------
HRARasterEditor* HRATiledRaster::CreateEditorUnShaped (HFCAccessMode pi_Mode)
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// private
// GetPtrTileDescription - Get the pointer on the tile descriptor object.
//-----------------------------------------------------------------------------
HGFTileIDDescriptor* HRATiledRaster::GetPtrTileDescription() const
    {
    return m_pTileDescriptor;
    }

//=======================================================================================
// @bsiclass                                                    
//=======================================================================================
struct TiledImageSourceNode : ImageSourceNode
{
protected:
    TiledImageSourceNode(HRATiledRaster& tiledRaster, HFCPtr<HGF2DCoordSys>& pPhysicalCoordSys, HFCPtr<HRPPixelType>& pixelType)
        :ImageSourceNode(pixelType),
        m_tiledRaster(tiledRaster)
        {
        // If we have a replacer, it must have same the pixelsize.
        BeAssert(pixelType->CountPixelRawDataBits() == m_tiledRaster.GetPixelType()->CountPixelRawDataBits());
        
        uint64_t width, height;
        tiledRaster.GetSize(&width, &height);
        m_physicalExtent = HGF2DExtent(0, 0, (double)width, (double)height, pPhysicalCoordSys);
        }

    virtual ~TiledImageSourceNode()
        {
        }

    virtual HGF2DExtent const& _GetPhysicalExtent() const override { return m_physicalExtent; }
    virtual HFCPtr<HGF2DCoordSys>& _GetPhysicalCoordSys() override { return const_cast<HFCPtr<HGF2DCoordSys>&>(m_physicalExtent.GetCoordSys()); }

protected:
    HRATiledRaster&       m_tiledRaster;
    HGF2DExtent           m_physicalExtent;
    };

//=======================================================================================
// @bsiclass                                                    
//=======================================================================================
struct TiledN8ImageSourceNode : TiledImageSourceNode
{
public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static RefCountedPtr<TiledImageSourceNode> Create(HRATiledRaster& tiledRaster, HFCPtr<HGF2DCoordSys>& pPhysicalCoordSys, HFCPtr<HRPPixelType>& pixelType, uint32_t scaleShift, HGSResampling const& scaleShiftMethod)
        {
        return new TiledN8ImageSourceNode(tiledRaster, pPhysicalCoordSys, pixelType, scaleShift, scaleShiftMethod);
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    TiledN8ImageSourceNode(HRATiledRaster& tiledRaster, HFCPtr<HGF2DCoordSys>& pPhysicalCoordSys, HFCPtr<HRPPixelType>& pixelType, uint32_t scaleShift, HGSResampling const& scaleShiftMethod)
        :TiledImageSourceNode(tiledRaster, pPhysicalCoordSys, pixelType), 
        m_scaleShift(scaleShift), m_scaleFactor(1)
        {
#ifdef ENABLE_STRIP_CACHING
        m_useCacheGrid = false;
#endif
        // pixel must be byte aligned, otherwise we cannot reference our internal buffer because that would mean
        // that a pixel doesn't always start at a byte boundary.
        HPRECONDITION(m_tiledRaster.GetPixelType()->CountPixelRawDataBits() % 8 == 0); 
            
        m_resWidth = (uint64_t)m_tiledRaster.GetPhysicalExtent().GetCorner().GetX();
        m_resHeight = (uint64_t)m_tiledRaster.GetPhysicalExtent().GetCorner().GetY();

        if (scaleShift > 0)
            {
            m_scaleFactor = (0x01 << m_scaleShift);
            m_stretcherN = (CreateGenericStretcherN(*pixelType, scaleShiftMethod));
             BeAssert(NULL != m_stretcherN);

            // Compute the size of the resolution we are producing
            for (uint64_t i = 0; i < m_scaleShift; ++i)
                {
                m_resWidth = (m_resWidth + 1) >> 1;
                m_resHeight = (m_resHeight + 1) >> 1;
                }
            }
        // If we ref only one tile we return it directly.
        // We cannot refernce the bitmap packet only because HRABitmap must be the last one to reference 
        // the packet when a memory manager is present. 
        // Because bitmap dpool allocated packet. see comment in TiledImageSurfaceIterator::NotifyAndInvalidate()
        else if(m_tiledRaster.GetNumberOfTileX() == 1 && m_tiledRaster.GetNumberOfTileY() == 1)
            {
            BeAssert(0 == scaleShift);

            HFCPtr<HRABitmapBase> pTile = m_tiledRaster.GetTile(0, 0)->GetTile();
            if(pTile->_AsHRABitmapP() != NULL)
                {
                m_pSingleBitmap = pTile->_AsHRABitmapP();
                m_pSingleSurface = HRAPacketSurface::Create((uint32_t)m_resWidth, (uint32_t)m_resHeight, GetPixelType(), const_cast<HFCPtr<HCDPacket>&>(m_pSingleBitmap->GetPacket()), m_pSingleBitmap->ComputeBytesPerWidth());
                }
            }

        EvaluateCopyClampPixelsMethods(m_CopyPixelMethod, m_ClampPixelMethod, tiledRaster.GetPixelType()->CountPixelRawDataBits() / 8);
        }

    ~TiledN8ImageSourceNode()
        {
        // Release the surface and sample before before we release the bitmap. No one should be holding the surface at this point.
        BeAssert(m_pSingleSurface != NULL ? m_pSingleSurface->GetRefCount() == 1 : true);
        m_pSingleSurface = NULL;
        m_pSingleBitmap = NULL;

#ifdef ENABLE_STRIP_CACHING
        // Release the surface and sample before before we delete the allocator. No one should be holding the surface at this point.
        BeAssert(m_pCachedStrip != NULL ? m_pCachedStrip->GetRefCount() == 1 && m_pCachedStrip->GetSample().GetRefCount() == 1 : true);
        m_pCachedStrip = NULL;
#endif
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  12/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
#ifdef ENABLE_STRIP_CACHING
    virtual ImagePPStatus _PrepareForStrip(HFCInclusiveGrid const& strip) override
        {

        m_pCachedStrip = NULL;
        
        if (0 == m_scaleShift)
            {
            m_useCacheGrid = false;
            m_cachedGrid.InitEmpty();
            }
        else
            {
            m_useCacheGrid = true;
            m_cachedGrid = strip;
            }

        return TiledImageSourceNode::_PrepareForStrip(strip);
        }
#endif        

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ImagePPStatus _GetRegion(HRAImageSurfacePtr& pOutSurface, PixelOffset64& outOffset, HFCInclusiveGrid const& region, IImageAllocatorR allocator) override
        {
        ImagePPStatus status = IMAGEPP_STATUS_Success;

        if(m_pSingleSurface != NULL)
            {
            outOffset.x = outOffset.y = 0;

            pOutSurface = m_pSingleSurface;
            return IMAGEPP_STATUS_Success;
            }

#ifdef ENABLE_STRIP_CACHING
        if(m_useCacheGrid)
            {
            if(m_pCachedStrip == NULL)
                {
                ImagePPStatus status;
                HRAImageSamplePtr pStripSample = HRAImageSample::CreateSample(status, m_cachedGrid.GetWidth(), m_cachedGrid.GetHeight(), GetPixelType(), m_cachedBlockAllocator);

                // If requested region is outside the physical region GenerateSampleData will clamp the missing data.
                if (IMAGEPP_STATUS_Success != (status = GenerateSampleData(*pStripSample, m_cachedGrid)))
                    return status;

                m_pCachedStrip = HRASampleSurface::Create(*pStripSample);
                }

            outOffset.x = m_cachedGrid.GetXMin();
            outOffset.y = m_cachedGrid.GetYMin();
            pOutSurface = m_pCachedStrip;
            return IMAGEPP_STATUS_Success;
            }
#endif

        HRAImageSamplePtr pSample = HRAImageSample::CreateSample(status, (uint32_t)region.GetWidth(), (uint32_t)region.GetHeight(), GetPixelType(), allocator);
        if (IMAGEPP_STATUS_Success != status)
            return status;

        // If requested region is outside the physical region GenerateSampleData will clamp the missing data.
        if (IMAGEPP_STATUS_Success != (status = GenerateSampleData(*pSample, region)))
            return status;

        outOffset.x = region.GetXMin();
        outOffset.y = region.GetYMin();
        pOutSurface = HRASampleSurface::Create(*pSample);

        return pOutSurface.get() != NULL ? IMAGEPP_STATUS_Success : IMAGEPP_STATUS_UnknownError;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 07/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImagePPStatus GenerateSampleData(HRAImageSampleR outSample, HFCInclusiveGrid const& srcGrid)
        {
        HPRECONDITION(srcGrid.GetWidth() < ULONG_MAX && srcGrid.GetHeight() < ULONG_MAX);
        HPRECONDITION(srcGrid.GetWidth() <= outSample.GetWidth() && srcGrid.GetHeight() <= outSample.GetHeight());

        if(m_CopyPixelMethod == NULL || m_ClampPixelMethod == NULL)
            return IMAGEPP_STATUS_NoImplementation;

        if (outSample.GetBufferP() == NULL)
            return IMAGEPP_STATUS_UnknownError;

        const size_t bytePerPixel = (outSample.GetPixelType().CountPixelRawDataBits() + 7) / 8;
        const uint32_t blockWidth  = (uint32_t)m_tiledRaster.GetTileSizeX();
        const uint32_t blockHeight = (uint32_t)m_tiledRaster.GetTileSizeY();

        const uint64_t tiledRasterWidth = (uint64_t)m_tiledRaster.GetPhysicalExtent().GetCorner().GetX();
        const uint64_t tiledRasterHeight = (uint64_t)m_tiledRaster.GetPhysicalExtent().GetCorner().GetY();
        const uint64_t lastTilePosX = (m_tiledRaster.GetNumberOfTileX() - 1)*blockWidth;
        const uint64_t lastTilePosY = (m_tiledRaster.GetNumberOfTileY() - 1)*blockHeight;

        HFCInclusiveGrid availableGrid;
        availableGrid.InitFromLenght(0.0, 0.0, m_resWidth, m_resHeight);

        // Compute the position in source coordinates
        const int64_t scaledXMin = MAX(MIN(srcGrid.GetXMin() * m_scaleFactor, (int64_t)tiledRasterWidth), 0);
        const int64_t scaledYMin = MAX(MIN(srcGrid.GetYMin() * m_scaleFactor, (int64_t)tiledRasterHeight), 0);
        const int64_t scaledXMax = MAX(MIN((srcGrid.GetXMax() + 1) * m_scaleFactor, (int64_t)tiledRasterWidth), 0);
        const int64_t scaledYMax = MAX(MIN((srcGrid.GetYMax() + 1) * m_scaleFactor, (int64_t)tiledRasterHeight), 0);

        HFCInclusiveGrid scaledSrcGrid((double)scaledXMin, (double)scaledYMin, (double)scaledXMax, (double)scaledYMax);

        size_t outPitch;
        Byte* pOutBuffer = outSample.GetBufferP()->GetDataP(outPitch);

        HRAImageBufferPtr pDownSampledSrcBufPtr;
        SquareTessellation srcTessellation(scaledSrcGrid, blockWidth, blockHeight);

        for (auto srcTileItr : srcTessellation)
            {
            // Must be within source physical. Clamp occurs in a later step.
            BeAssert(IN_RANGE(srcTileItr.GetXMin(), 0, (int64_t)lastTilePosX) && IN_RANGE(srcTileItr.GetYMin(), 0, (int64_t)lastTilePosY));
            const uint64_t tilePosX = srcTileItr.GetXMin();
            const uint64_t tilePosY = srcTileItr.GetYMin();

            HFCPtr<HRABitmapBase> pSrcBaseTile = m_tiledRaster.GetTile(tilePosX, tilePosY)->GetTile();
            BeAssert(pSrcBaseTile->IsCompatibleWith(HRABitmap::CLASS_ID));
            HFCPtr<HRABitmap> pSrcTile = static_cast<HRABitmap*>(pSrcBaseTile.GetPtr());

            uint64_t tileSizeX, tileSizeY;
            pSrcTile->GetSize(&tileSizeX, &tileSizeY);
            Byte const* pSrcBuffer = pSrcTile->GetPacket()->GetBufferAddress();
            size_t srcPitch = pSrcTile->ComputeBytesPerWidth();

            // DownSample source tile.
            if (m_scaleShift > 0)
                {
                HFCInclusiveGrid tileGrid, effectiveTileGrid;
                tileGrid.InitFromLenght((double)srcTileItr.GetXMin(), (double)srcTileItr.GetYMin(), tileSizeX, tileSizeY);
                effectiveTileGrid.InitFromIntersectionOf(tileGrid, scaledSrcGrid);
                double xmin = effectiveTileGrid.GetXMin() / (double)m_scaleFactor;
                double ymin = effectiveTileGrid.GetYMin() / (double)m_scaleFactor;
                double xmax = (effectiveTileGrid.GetXMax()+1) / (double)m_scaleFactor;
                double ymax = (effectiveTileGrid.GetYMax()+1) / (double)m_scaleFactor;

                HFCInclusiveGrid effectiveDstGrid(xmin, ymin, xmax, ymax);

                const uint64_t tileOffsetX = effectiveTileGrid.GetXMin() - tilePosX;
                const uint64_t tileOffsetY = effectiveTileGrid.GetYMin() - tilePosY;
                
                const uint64_t dstOffsetX = effectiveDstGrid.GetXMin() - srcGrid.GetXMin();
                const uint64_t dstOffsetY = effectiveDstGrid.GetYMin() - srcGrid.GetYMin();

                uint32_t outWidth = (uint32_t)effectiveDstGrid.GetWidth();
                uint32_t outHeight = (uint32_t)effectiveDstGrid.GetHeight();
                uint32_t inWidth = (uint32_t)effectiveTileGrid.GetWidth();
                uint32_t inHeight = (uint32_t)effectiveTileGrid.GetHeight();

                Byte const* pInData = pSrcBuffer + tileOffsetY*srcPitch + tileOffsetX*bytePerPixel;
                Byte*       pOutData = pOutBuffer + dstOffsetY*outPitch + dstOffsetX*bytePerPixel;

                (*m_stretcherN)(outWidth, outHeight, pOutData, outPitch, inWidth, inHeight, pInData, srcPitch, 1 << m_scaleShift);
                }
            else
                {
                // Compute the position of the src into the destination
                const int64_t srcPosX = srcTileItr.GetXMin() / m_scaleFactor;
                const int64_t srcPosY = srcTileItr.GetYMin() / m_scaleFactor;

                // Compute the first valid src pixel to copy in the dst
                const int64_t firstSrcX = srcPosX > srcGrid.GetXMin() ? 0 : srcGrid.GetXMin() - srcPosX;
                const int64_t firstSrcY = srcPosY > srcGrid.GetYMin() ? 0 : srcGrid.GetYMin() - srcPosY;

                // Compute the last valid pixel of the src
                const int64_t lastSrcX = MIN(srcPosX + (int64_t)tileSizeX - 1, srcGrid.GetXMax());
                const int64_t lastSrcY = MIN(srcPosY + (int64_t)tileSizeY - 1, srcGrid.GetYMax());

                const uint32_t width = (uint32_t)(lastSrcX - (srcPosX + firstSrcX) + 1);
                const uint32_t height = (uint32_t)(lastSrcY - (srcPosY + firstSrcY) + 1);

                // Compute destination positions
                const int64_t dstPosX = MAX(srcPosX, srcGrid.GetXMin());
                const int64_t dstPosY = MAX(srcPosY, srcGrid.GetYMin());
                const int64_t dstOffsetX = dstPosX - srcGrid.GetXMin();
                const int64_t dstOffsetY = dstPosY - srcGrid.GetYMin();

                Byte const* pSrc = pSrcBuffer + firstSrcY*srcPitch + firstSrcX*bytePerPixel;
                Byte*       pDst = pOutBuffer + dstOffsetY*outPitch + dstOffsetX*bytePerPixel;

                PixelOffset64 srcOffsetX(0, 0);
                (*m_CopyPixelMethod)(width, height, pDst, outPitch, width, height, pSrc, srcPitch, srcOffsetX);
                }
            }

        (*m_ClampPixelMethod)(availableGrid, srcGrid, outSample.GetWidth(), outSample.GetHeight(), pOutBuffer, outPitch);
        return IMAGEPP_STATUS_Success;
        }

protected:
    uint64_t                  m_resWidth;
    uint64_t                  m_resHeight;
    uint32_t                  m_scaleFactor;
    uint32_t                  m_scaleShift;
    Stretch_1N_FunctionP      m_stretcherN;

    // For case where there is only one block in the tiledRaster:
    // We must hold the bitmap since surface will reference its packet and HRABitmap does a nasty packet allocation/desallocation trick 
    // when GetPool()->IsMemoryMgrEnabled(). See HRABitmap::~HRABitmap()
    HRAImageSurfacePtr        m_pSingleSurface;
    HFCPtr<HRABitmap>         m_pSingleBitmap; 
    
#ifdef ENABLE_STRIP_CACHING
    bool                      m_useCacheGrid;
    HFCInclusiveGrid          m_cachedGrid;
    HRASampleSurfacePtr       m_pCachedStrip;
    KeepLastBlockAllocator    m_cachedBlockAllocator;
#endif

    CopyPixelsMethodN8_T      m_CopyPixelMethod;
    ClampPixelsMethodN8_T     m_ClampPixelMethod;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct TiledRLEImageSourceNode : TiledImageSourceNode
{
public:
    static RefCountedPtr<TiledImageSourceNode> Create(HRATiledRaster& tiledRaster, HFCPtr<HGF2DCoordSys>& pPhysicalCoordSys, HFCPtr<HRPPixelType>& pixelType)
        {
        // Only strip is supported.
        if (tiledRaster.GetNumberOfTileX() != 1)
            return NULL;

        return new TiledRLEImageSourceNode(tiledRaster, pPhysicalCoordSys, pixelType);
        }

protected:
    ~TiledRLEImageSourceNode(){}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    TiledRLEImageSourceNode(HRATiledRaster& tiledRaster, HFCPtr<HGF2DCoordSys>& pPhysicalCoordSys, HFCPtr<HRPPixelType>& pixelType)
        :TiledImageSourceNode(tiledRaster, pPhysicalCoordSys, pixelType)
        {
        BeAssert(GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) || GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ImagePPStatus _GetRegion(HRAImageSurfacePtr& pOutSurface, PixelOffset64& outOffset, HFCInclusiveGrid const& region, IImageAllocatorR allocator) override
        {
        // For RLE we do not care about the asked region.  It returns a ref to all the required strip, without copying memory. 
        // If extra clamping pixels are required it is assumed that the transformNode sampler will do it.

        BeAssert(m_tiledRaster.GetNumberOfTileX() == 1); // Only strip is supported.

        uint64_t width64, height64;
        m_tiledRaster.GetSize(&width64, &height64);

        uint64_t firstStrip = MAX(0, region.GetYMin() / m_tiledRaster.GetTileSizeY());
        uint64_t lastStrip = MIN(region.GetYMax() / m_tiledRaster.GetTileSizeY(), m_tiledRaster.GetNumberOfTileY()-1);

        HRAPacketRleSurfacePtr pSurfaceRle = HRAPacketRleSurface::CreateSurface((uint32_t)width64, (uint32_t)MIN((lastStrip+1)*m_tiledRaster.GetTileSizeY(), height64), GetPixelType(), (uint32_t)m_tiledRaster.GetTileSizeY());

        for (uint64_t strip = firstStrip; strip <= lastStrip; ++strip)
            {
            uint64_t stripPos = strip*m_tiledRaster.GetTileSizeY();

            HFCPtr<HRABitmapBase> pTile = m_tiledRaster.GetTile(0, stripPos)->GetTile();
            BeAssert(pTile->IsCompatibleWith(HRABitmapRLE::CLASS_ID));

            if (pTile->IsCompatibleWith(HRABitmapRLE::CLASS_ID))
                {
                HFCPtr<HRABitmapRLE> pSrcTile = static_cast<HRABitmapRLE*>(pTile.GetPtr());
                pSurfaceRle->AppendStrip(pSrcTile);
                }
            }       

        outOffset.x = 0;        // RLE is always stripped.
        outOffset.y = firstStrip * m_tiledRaster.GetTileSizeY();
        pOutSurface = pSurfaceRle;

        return IMAGEPP_STATUS_Success;
        }

private:
    HFCPtr<HCDPacketRLE> m_pRLERaster;
    list<HFCPtr<HRABitmapBase>> m_pStripsInUse;
    };

//=======================================================================================
// @bsiclass                                                    
//=======================================================================================
template<typename SinkNode_T>
struct TiledImageSurfaceIterator : public ImageSurfaceIterator
{
public:
    TiledImageSurfaceIterator(SinkNode_T& sinkNode, HFCInclusiveGrid const& grid)
    :ImageSurfaceIterator(),
     m_sinkNode(sinkNode),
     m_pCurrentTile(NULL),
     m_tessellation(grid, (uint32_t)sinkNode.GetRaster().GetTileSizeX(), (uint32_t)sinkNode.GetRaster().GetTileSizeY())
        {
        m_blockItr = m_tessellation.begin();
        LoadCurrentTile();
        }

    virtual ~TiledImageSurfaceIterator()
        {
        NotifyAndInvalidate();
        }
    
    HRATiledRaster& GetRaster() { return m_sinkNode.GetRaster(); }
    virtual bool _Next() override { ++m_blockItr; return LoadCurrentTile();}
    HFCInclusiveGrid const& GetGrid() const { return m_tessellation.GetGrid(); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void NotifyAndInvalidate()
        {
        if (m_pCurrentTile != NULL)
            {
            BeAssert(IsValid());

            HFCInclusiveGrid tileGrid;
            tileGrid.InitFromLenght(0, 0, GetRaster().GetTileSizeX(), GetRaster().GetTileSizeY());

            HFCInclusiveGrid effectiveGrid;
            effectiveGrid.InitFromIntersectionOf(m_tessellation.m_grid, tileGrid);

            HVEShape rect((double)effectiveGrid.GetXMin(), (double)effectiveGrid.GetYMin(), (double)effectiveGrid.GetXMax() + 1.0, (double)effectiveGrid.GetYMax() + 1.0, m_pCurrentTile->GetTile()->GetPhysicalCoordSys());
            m_pCurrentTile->GetTile()->Updated(&rect);
            
            }
            
        // ***  IMPORTANT ***
        // The cleanup order is important here because Bitmap hack the packet allocation by using its pool alloc/free memory(see HRABitmap destructor). 
        // So in these cases, we cannot hold a packet longer than the lifetime of the bitmap. If we do, bitmap special cleanup won't be executed and the 
        // packet will try to delete memory that was allocated by the bitmap pool(crash!). 
        // First invalidate the surface which hold the packet and then release the HRATile, that hold the bitmap that hold the packet.
        Invalidate();   // Release the surface and packet. 
        m_pCurrentTile = NULL;      // Release the tile and maybe cleanup bitmap and its packet.
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool LoadCurrentTile()
        {
        NotifyAndInvalidate();       

        if (!(m_blockItr != m_tessellation.end()))
            return false;
        
        BeAssert((*m_blockItr).GetXMin() % GetRaster().GetTileSizeX() == 0 && (*m_blockItr).GetYMin() % GetRaster().GetTileSizeY() == 0);

        uint64_t tilePosX = (*m_blockItr).GetXMin();
        uint64_t tilePosY = (*m_blockItr).GetYMin();

        m_pCurrentTile = GetRaster().GetTile(tilePosX, tilePosY);

        HFCInclusiveGrid tileGrid((double)(MAX((uint64_t)GetGrid().GetXMin(), tilePosX) - tilePosX),
                                  (double)(MAX((uint64_t)GetGrid().GetYMin(), tilePosY) - tilePosY),
                                  (double)(MIN((uint64_t)GetGrid().GetXMax() + 1, tilePosX + GetRaster().GetTileSizeX()) - tilePosX),
                                  (double)(MIN((uint64_t)GetGrid().GetYMax() + 1, tilePosY + GetRaster().GetTileSizeY()) - tilePosY));

        HRATransaction* pTransaction = m_sinkNode.GetTransaction(); 
      
        if (m_pCurrentTile->GetTile()->IsCompatibleWith(HRABitmap::CLASS_ID))
            {
            HRABitmap* pBitmap = static_cast<HRABitmap*>(m_pCurrentTile->GetTile().GetPtr());

            BeAssert(pBitmap->GetPixelType()->CountPixelRawDataBits() % 8 == 0);    // N1 is not supported, it should have 

            // Use sink node pixeltype that take replacing pixeltype into account.
            HRAImageSurfacePtr pSurface = HRAPacketSurface::Create((uint32_t)GetRaster().GetTileSizeX(), (uint32_t)GetRaster().GetTileSizeY(), m_sinkNode.GetPixelType(), const_cast<HFCPtr<HCDPacket>&>(pBitmap->GetPacket()), pBitmap->ComputeBytesPerWidth());
            
            SetCurrent(*pSurface, PixelOffset64(tilePosX, tilePosY));   // take owenership of surface.

            // Saved data for undo
            if (NULL != pTransaction)
                {
                if (tileGrid.GetWidth() == GetRaster().GetTileSizeX() && tileGrid.GetHeight() == GetRaster().GetTileSizeY())
                    {
                    // push entire tile, one shot.
                    pTransaction->PushEntry(tilePosX, tilePosY, (uint32_t)GetRaster().GetTileSizeX(), (uint32_t)GetRaster().GetTileSizeY(), pBitmap->GetPacket()->GetBufferSize(), pBitmap->GetPacket()->GetBufferAddress());
                    }
                else
                    {
                    size_t pixelSize = pBitmap->GetPixelType()->CountPixelRawDataBits() / 8;
                    size_t lineSize = ((uint32_t)tileGrid.GetWidth())*pixelSize;
                    size_t pitch = ((uint32_t)GetRaster().GetTileSizeX())*pixelSize;
                    Byte const* pBuffer = pBitmap->GetPacket()->GetBufferAddress() + (tileGrid.GetXMin()*pixelSize);
                    // push line that touches.
                    for (int64_t line = tileGrid.GetYMin(); line < tileGrid.GetYMax() + 1; ++line)
                        {
                        BeAssert(pBuffer + line*pitch + lineSize <= pBitmap->GetPacket()->GetBufferAddress() + pBitmap->GetPacket()->GetDataSize());

                        // push entire line
                        pTransaction->PushEntry(tilePosX + tileGrid.GetXMin(), tilePosY + line, (uint32_t)tileGrid.GetWidth(), 1, lineSize, pBuffer + line*pitch);
                        }
                    }
                }
            }
        else if (m_pCurrentTile->GetTile()->IsCompatibleWith(HRABitmapRLE::CLASS_ID))
            {
            BeAssert(m_sinkNode.GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || m_sinkNode.GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID));

            HRABitmapRLE* pBitmap = static_cast<HRABitmapRLE*>(m_pCurrentTile->GetTile().GetPtr());

            // Use sink node pixeltype that take replacing pixeltype into account.
            HRAPacketRleSurfacePtr pSurfaceRle = HRAPacketRleSurface::CreateSurface((uint32_t)GetRaster().GetTileSizeX(), (uint32_t)GetRaster().GetTileSizeY(), m_sinkNode.GetPixelType(), (uint32_t)GetRaster().GetTileSizeY());

            HFCPtr<HRABitmapRLE> pBitmapRle(pBitmap); // HRABitmapRLE is always an HFCPtr.
            pSurfaceRle->AppendStrip(pBitmapRle);

            SetCurrent(*pSurfaceRle, PixelOffset64(tilePosX, tilePosY));   // take owenership of surface.

            // Saved data for undo
            if (NULL != pTransaction)
                {
                // push entire line that intersect
                for (int64_t line = tileGrid.GetYMin(); line < tileGrid.GetYMax() + 1; ++line)
                    {
                    pTransaction->PushEntry(0, tilePosY + line, (uint32_t)GetRaster().GetTileSizeX(), 1, pBitmap->GetPacket()->GetLineDataSize((uint32_t)line), pBitmap->GetPacket()->GetLineBuffer((uint32_t)line));
                    }
                }
            }

        return IsValid();
        }


    SinkNode_T& m_sinkNode;
    HFCPtr<HRATiledRaster::HRATile> m_pCurrentTile;
    SquareTessellation m_tessellation;
    SquareTessellation::const_iterator m_blockItr;
};

//=======================================================================================
// @bsiclass                                                    
//=======================================================================================
struct ImagePP::TiledImageSinkNode : ImageSinkNode
{
public:
    static ImageSinkNodePtr Create(HRATiledRaster& raster, HVEShape const& sinkShape, HFCPtr<HGF2DCoordSys> pPhysicalCoordSys, HFCPtr<HRPPixelType>& pReplacingPixelType)
        {
        uint64_t width, height;
        raster.GetSize(&width, &height);

        return new TiledImageSinkNode(raster, sinkShape, pReplacingPixelType, HGF2DExtent(0, 0, (double)width, (double)height, pPhysicalCoordSys));
        }

    HRATiledRaster& GetRaster() { return m_raster; }

protected:
        
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    TiledImageSinkNode(HRATiledRaster& raster, HVEShape const& sinkShape, HFCPtr<HRPPixelType>& pReplacingPixelType, HGF2DExtent const& physicalExtent)
        :ImageSinkNode(sinkShape, pReplacingPixelType, physicalExtent), 
        m_raster(raster)
        {      
        //Must be within raster physical extent.
        HDEBUGCODE
            (
            HFCInclusiveGrid __grid(sinkShape.GetExtent().GetXMin(), sinkShape.GetExtent().GetYMin(), sinkShape.GetExtent().GetXMax(), sinkShape.GetExtent().GetYMax());
            BeAssert(__grid.GetXMin() >= 0 && __grid.GetYMin() >= 0);
            BeAssert(__grid.GetXMax() + 1 <= GetPhysicalExtent().GetXMax() && __grid.GetYMax() + 1 <= GetPhysicalExtent().GetYMax());
            )
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ~TiledImageSinkNode(){}
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ImageSurfaceIterator* _GetImageSurfaceIterator(HFCInclusiveGrid& strip) override 
        {
        BeAssert(strip.GetXMin() >= 0 && strip.GetYMin() >= 0);
        BeAssert(strip.GetXMax() + 1 <= m_raster.GetPhysicalExtent().GetXMax() && strip.GetYMax() + 1 <= m_raster.GetPhysicalExtent().GetYMax());

        return new TiledImageSurfaceIterator<TiledImageSinkNode>(*this, strip); 
        }
    
    //! Native block size of the sink.
    virtual uint32_t _GetBlockSizeX() override { return (uint32_t)m_raster.GetTileSizeX(); }
    virtual uint32_t _GetBlockSizeY() override { return (uint32_t)m_raster.GetTileSizeY(); }

private:
    HRATiledRaster& m_raster;
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSinkNodePtr HRATiledRaster::_GetSinkNode(ImagePPStatus& status, HVEShape const& sinkShape, HFCPtr<HRPPixelType>& pReplacingPixelType)
    {
    status = IMAGEPP_STATUS_Success;

    HFCPtr<HRPPixelType> pEffectivePixelType = pReplacingPixelType ? pReplacingPixelType : GetPixelType();

    if (m_pBitmapModel->IsCompatibleWith(HRABitmapRLE::CLASS_ID))
        {
        //Image Node and Image sample use the pixel type to differentiate between 1bit and Rle data. Unlike HRABitmap[RLE] that use pixeltype and codec.
        HFCPtr<HRPPixelType> pRlePixelType = ImageNode::TransformToRleEquivalent(pEffectivePixelType);
        if (pRlePixelType == NULL)
            {
            BeAssert(!"Incompatible replacing pixelType");
            status = COPYFROM_STATUS_IncompatiblePixelTypeReplacer;
            return NULL;
            }

        pEffectivePixelType = pRlePixelType;
        }
    else if (pEffectivePixelType->CountPixelRawDataBits() % 8 == 0)
        {
        // OK..
        }
    else
        {
        // We support only RLE and N8.
        BeAssert(!"HRATiledRaster unsupported pixeltype");
        status = IMAGEPP_STATUS_NoImplementation;
        return NULL;
        }

    return TiledImageSinkNode::Create(*this, sinkShape, GetPhysicalCoordSys(), pEffectivePixelType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRATiledRaster::_BuildCopyToContext(ImageTransformNodeR parentNode, HRACopyToOptionsCR options)
    {
    BeAssert(options.GetReplacingPixelType() == NULL || options.GetReplacingPixelType()->CountPixelRawDataBits() == GetPixelType()->CountPixelRawDataBits());
    HFCPtr<HRPPixelType> pEffectivePixelType = (options.GetReplacingPixelType() != NULL) ? options.GetReplacingPixelType() : GetPixelType();

    // Get the effective logical and physical coordsys
    HFCPtr<HGF2DCoordSys> pEffLogCS(GetCoordSys());
    HFCPtr<HGF2DCoordSys> pEffPhyCS(GetPhysicalCoordSys());
    if (NULL != options.GetReplacingCoordSys().GetPtr())
        {
        pEffLogCS = options.GetReplacingCoordSys();

        HFCPtr<HGF2DTransfoModel> pPhysicalToLogical(GetPhysicalCoordSys()->GetTransfoModelTo(GetCoordSys()));
        HFCPtr<HGF2DTransfoModel> pSimplModel(pPhysicalToLogical->CreateSimplifiedModel());
        if (NULL != pSimplModel)
            pPhysicalToLogical = pSimplModel;

        pEffPhyCS = new HGF2DCoordSys(*pPhysicalToLogical, pEffLogCS);
        }
    
#ifndef NDEBUG    
    // Validate that we intersect with copyRegion.

    //Use GetShape instead of GetPhysicalExtent since GetPhysicalExtent doesn't work with reprojected huge raster like BingMap 
    HVEShape myPhysicalExtent(GetShape());
    myPhysicalExtent.ChangeCoordSys(pEffPhyCS);    

    BeAssert(options.GetShape()->HasIntersect(myPhysicalExtent));    
#endif

    ImagePPStatus status = IMAGEPP_STATUS_Success;

    RefCountedPtr<TiledImageSourceNode> pSource;
    
    if (m_pBitmapModel->IsCompatibleWith(HRABitmapRLE::CLASS_ID))
        {
        //Image Node and Image sample use the pixel type to differentiate between 1bit and Rle data. Unlike HRABitmap[RLE] that use pixeltype and codec.
        HFCPtr<HRPPixelType> pRlePixelType = ImageNode::TransformToRleEquivalent(pEffectivePixelType);
        if (pRlePixelType == NULL)
            {
            BeAssert(!"Incompatible replacing pixelType");
            status = COPYFROM_STATUS_IncompatiblePixelTypeReplacer;
            }
        else
            {
            pSource = TiledRLEImageSourceNode::Create(*this, pEffPhyCS, pRlePixelType);
            }
        }
    else if (pEffectivePixelType->CountPixelRawDataBits() % 8 == 0)
        {
        uint32_t scaleShift = 0;
        uint64_t width, height;
        GetSize(&width, &height);

        // Do not downsample if the size of the tiled raster is <= MANAGEABLE_RASTER_PHYSICAL_SIZE
        // Do not scale shift if we have only one block. We will return it directly.
        if(!(GetNumberOfTileX() == 1 && GetNumberOfTileY() == 1) &&     // isSingleBlock?
            (width > MANAGEABLE_RASTER_PHYSICAL_SIZE || height > MANAGEABLE_RASTER_PHYSICAL_SIZE))
            {
            // Evaluate the scale factor between the physical coord sys of the source and the destination.
            scaleShift = EvaluateScaleFactorPowerOf2(parentNode.GetPhysicalCoordSys(), pEffPhyCS, *options.GetShape());
            if (scaleShift > 0)
                {
                const double scaleFactor = pow(2, scaleShift);
                HFCPtr<HGF2DStretch> pStretch(new HGF2DStretch(HGF2DDisplacement(), scaleFactor, scaleFactor));
                pEffPhyCS = new HGF2DCoordSys(*pStretch, pEffPhyCS);
                }
            }

        pSource = TiledN8ImageSourceNode::Create(*this, pEffPhyCS, pEffectivePixelType, scaleShift, options.GetResamplingMode());
        }
    else
        {
        // We support only RLE and N8.
        BeAssert(!"HRATiledRaster unsupported pixeltype");
        status = IMAGEPP_STATUS_NoImplementation;
        }

    if (IMAGEPP_STATUS_Success != status)
        return status;

    if (pSource == NULL)
        return IMAGEPP_STATUS_NoImplementation;
  
    return parentNode.LinkTo(*pSource);
    }

