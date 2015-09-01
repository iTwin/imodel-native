//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMBlendCorridor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRAImageView.h"

BEGIN_IMAGEPP_NAMESPACE

class HVE2DPolySegment;
class HRAClearOptions;
class HIMBlendCorridorTile;
class HGFTileIDDescriptor;
class HRABitmap;

/** -----------------------------------------------------------------------------
    This is a placeholder in an HPMPool object. It gives us the possibility to
    use an HPMPool without working with persistent objects. We will use one of
    these for each tile inside the HIMBlendCorridor class, to apply memory
    management to our tile list.

    Used inside HIMBlendCorridor.

    @see HIMBlendCorridor
    -----------------------------------------------------------------------------
*/
class HIMBlendCorridorTileToken : public HPMPersistentObject, public HPMShareableObject<HIMBlendCorridorTileToken>
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HIMBlendCorridorId_TileToken)

public:

    HIMBlendCorridorTileToken();
    HIMBlendCorridorTileToken(HIMBlendCorridorTile* pi_pTile);

    virtual ~HIMBlendCorridorTileToken();

    // Overriden from HPMPersistentObject
    virtual size_t  GetAdditionalSize() const;

private:

    // Disabled operations
    HIMBlendCorridorTileToken(const HIMBlendCorridorTileToken& pi_rObj);
    HIMBlendCorridorTileToken&
    operator=(const HIMBlendCorridorTileToken& pi_rObj);

    // Pointer to the tile we're managing
    HIMBlendCorridorTile*
    m_pTile;
    };


class HIMBlendCorridor;


/** -----------------------------------------------------------------------------
    This class holds a blend tile, along with its associated token.

    Used inside HIMBlendCorridor.

    @see HIMBlendCorridor
    @see HIMBlendCorridorTileToken
    -----------------------------------------------------------------------------
*/
class HIMBlendCorridorTile : public HFCShareableObject<HIMBlendCorridorTile>
    {
public:

    // Basic

    HIMBlendCorridorTile();
    HIMBlendCorridorTile(HIMBlendCorridor*             pi_pBlendCorridor,
                         uint64_t                     pi_TileID,
                         const HFCPtr<HRARaster>&      pi_rpRaster);

    ~HIMBlendCorridorTile();

    bool           operator==(const HIMBlendCorridorTile& pi_rObj) const;
    bool           operator<(const HIMBlendCorridorTile& pi_rObj) const;


    // Information retrieval

    uint64_t       GetTileID() const;

    HFCPtr<HRARaster>
    GetRaster() const;

    bool           LogUsage() const;

    // To be called by our token
    size_t          GetAdditionalSize() const;

    // Called by the token when it is deleted
    void            TokenDeleted() const;

private:

    // Disable access to these methods
    HIMBlendCorridorTile(const HIMBlendCorridorTile& pi_rObj);
    HIMBlendCorridorTile& operator=(const HIMBlendCorridorTile& pi_rObj);

    // Pointer to container buffered image
    HIMBlendCorridor*
    m_pBlendCorridor;

    // Identifier of the tile
    uint64_t       m_TileID;

    // Pointer to the tile
    HFCPtr<HRARaster>
    m_pTile;

    // Our token (to be used in the object pool)
#ifdef _TODO_REPLACE_USAGE_OF_HPMRef
    mutable HPMRef<HIMBlendCorridorTileToken> m_pToken;
#endif

    bool           m_Deleting;
    };



/** -----------------------------------------------------------------------------
    This class creates a blend of two images that overlap, along a polysegment
    that lies inside the overlap region. The blend corridor width is variable,
    and the blend is simply a weighted sum from one side of the corridor to the
    other.

    @note This class does not link to the two source rasters, so it doesn't react
    to messages from them. Therefore, the user of the HIMBlendCorridor is
    responsible of managing the messages from the sources are reacting accordingly.
    For example, upon a ContentChanged message, the user must inform the blend
    corridor to flush the according cached tiles. For a geometry change, a new
    HIMBlendCorridor must be constructed to replace the current one.
    @end

    Used inside HIMSeamlessMosaic.

    @see HIMSeamlessMosaic
    -----------------------------------------------------------------------------
*/
class HIMBlendCorridor : public HRARaster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HIMBlendCorridorId_Base)

    friend class HIMBlendCorridorIterator;

public:

    //:> Primary methods
    HIMBlendCorridor();

    HIMBlendCorridor(const HFCPtr<HGF2DCoordSys>&    pi_rpCoordSys,
                     const HFCPtr<HRARaster>&        pi_rpSource1,
                     const HFCPtr<HRARaster>&        pi_rpSource2,
                     const HFCPtr<HVE2DPolySegment>& pi_rpFeatherLine,
                     HPMPool*                        pi_pPool,
                     double                          pi_rCorridorWidth);

    HIMBlendCorridor(const HIMBlendCorridor& pi_rDitheredImage);

    virtual         ~HIMBlendCorridor();


    //:> Overriden methods

    virtual bool   ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                              Byte                      pi_Id = 0) const;

    virtual void    CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster);

    virtual void    CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions);

    virtual void    Clear() override;
    virtual void    Clear(const HRAClearOptions& pi_rOptions) override;


    virtual HRARasterEditor*
    CreateEditor   (HFCAccessMode pi_Mode);

    virtual HRARasterEditor*
    CreateEditor   (const HVEShape& pi_rShape,
                    HFCAccessMode   pi_Mode);

    virtual HRARasterEditor*
    CreateEditorUnShaped (HFCAccessMode pi_Mode);

    virtual HRARasterIterator*
    CreateIterator  (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const;

    virtual HPMPersistentObject* Clone () const;
    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const override;


    virtual HGF2DExtent
    GetAveragePixelSize () const;
    virtual void    GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const;

    virtual HFCPtr<HVEShape>
    GetEffectiveShape () const;

    virtual HFCPtr<HRPPixelType>
    GetPixelType() const;

    virtual bool   IsStoredRaster  () const;

    virtual bool   HasSinglePixelType() const;

    virtual void    Move(const HGF2DDisplacement& pi_rDisplacement);

    virtual void    Rotate(double               pi_Angle,
                           const HGF2DLocation& pi_rOrigin);

    virtual void    Scale(double pi_ScaleFactorX,
                          double pi_ScaleFactorY,
                          const HGF2DLocation& pi_rOrigin);

    virtual HGF2DExtent
    GetExtent() const;

    //:> Sources
    const HFCPtr<HRARaster>&
    GetSource1() const;
    const HFCPtr<HRARaster>&
    GetSource2() const;

    double         GetCorridorWidth() const;
    void           SetCorridorWidth(double pi_CorridorWidth);

    // Ask if the blend corridor uses the two specified sources.
    bool            Blends(const HFCPtr<HRARaster>& pi_rpImage1,
                           const HFCPtr<HRARaster>& pi_rpImage2) const;

    // Ask if the specified source is used for this corridor
    bool            Uses(const HFCPtr<HRARaster>& pi_rpImage) const;

    HPMPool*        GetTilePool() const;

    // Remove a tile, given its ID. Used when the pooltells
    // a tile to remove itself.
    void            RemoveTile(uint64_t pi_TileID);

    //:> LookAhead Methods
    virtual bool    HasLookAhead() const;
    virtual void    SetLookAhead(const HVEShape& pi_rShape,
                                 uint32_t        pi_ConsumerID,
                                 bool           pi_Async = false);

    void            Invalidate();
    void            Invalidate(HFCPtr<HVEShape>& pi_rpShape);

    //Context Methods
    virtual void                      SetContext(const HFCPtr<HMDContext>& pi_rpContext);
    virtual HFCPtr<HMDContext>        GetContext();

    virtual void                      InvalidateRaster();

protected:

    HGFTileIDDescriptor*     GetTileDescriptor() const;

    HFCPtr<HGF2DCoordSys>    GetPhysicalCoordSys() const;

    const HFCPtr<HRARaster>  GetTile(uint64_t pi_Index) const;


private:

    void            MoveOneImage(HFCPtr<HRARaster>& pi_rpRaster,
                                 const HGF2DDisplacement& pi_rDisplacement);
    void            RotateOneImage(HFCPtr<HRARaster>&   pi_rpRaster,
                                   double               pi_Angle,
                                   const HGF2DLocation& pi_rOrigin);
    void            ScaleOneImage(HFCPtr<HRARaster>&   pi_rpRaster,
                                  double               pi_ScaleFactorX,
                                  double               pi_ScaleFactorY,
                                  const HGF2DLocation& pi_rOrigin);

    void            PrecomputeBlendCorridorParameters();

    HFCPtr<HRARaster>     CreateTile(uint64_t pi_Index) const;

    // The two source images
    HFCPtr<HRARaster>     m_pSource1;
    HFCPtr<HRARaster>     m_pSource2;

    // The pool to use to manage our tiles
    HPMPool*        m_pPool;

    // The example tile.
    HFCPtr<HRABitmap>     m_pBaseTile;

    // The physical CoordSys of the tiles.
    HFCPtr<HGF2DCoordSys>    m_pPhysicalCS;

    //:> Polysegments. Featherline + different borders
    //:> used when blending pixels
    HFCPtr<HVE2DPolySegment>    m_pFeatherLine;
    HFCPtr<HVE2DPolySegment>    m_pRaster1Border;
    HFCPtr<HVE2DPolySegment>    m_pRaster2Border;
    HFCPtr<HVE2DPolySegment>    m_pRaster2FartherBorder;

    // The blend corridor
    HFCPtr<HVEShape>            m_pBlendCorridorShape;

    // The width of the blend corridor, in the corridor's logical CS.
    double     m_CorridorWidth;

    // Manages the tile organization for the corridor.
    HGFTileIDDescriptor*    m_pTileDescriptor;

    // The array of tiles. An empty position (0) means
    // the tile isn't created.
    HFCPtr<HIMBlendCorridorTile>*    m_pTiles;

    // Only one at a time can change the tile array contents.
    mutable HFCExclusiveKey    m_TilesKey;
    };

END_IMAGEPP_NAMESPACE