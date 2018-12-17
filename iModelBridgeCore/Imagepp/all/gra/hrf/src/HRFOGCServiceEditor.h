//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFOGCServiceEditor.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HRFResolutionEditor.h>
#include <ImagePPInternal/gra/Task.h>

BEGIN_IMAGEPP_NAMESPACE
class HMDVolatileLayers;
class BlockReaderThread;
class HRFOGCService;
struct HttpSession;
struct OGCTile;
struct OGCBlockQuery;

//----------------------------------------------------------------------------------------
// @bsiclass
//----------------------------------------------------------------------------------------
class HRFOGCServiceEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFWMSFile;
    friend class HRFOGCService;

    virtual         ~HRFOGCServiceEditor  ();


    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData) override;

    HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData) override;

    HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }



protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFOGCServiceEditor    (HFCPtr<HRFRasterFile>      pi_rpRasterFile,
                            uint32_t                   pi_Page,
                            double                    pi_Resolution,
                            HFCAccessMode              pi_AccessMode);

    virtual void    RequestLookAhead(       const HGFTileIDList&       pi_rTileIDList);
    virtual void    CancelLookAhead ();

    virtual void    ContextChanged  ();


private:

    friend struct OGCBlockQuery;
    
    typedef struct
        {
        uint64_t m_MinX;
        uint64_t m_MinY;
        uint64_t m_MaxX;
        uint64_t m_MaxY;
        } BlocksExtent;

    enum ImageType
        {
        JPEG,
        PNG,
        BMP,
        GIF,
        GEOTIFF
        };
       
    RefCountedPtr<OGCBlockQuery>  CreateBlockQuery(BlocksExtent const& blockExtent);

    RefCountedPtr<OGCTile> GetTile(uint64_t tileId);

    std::unique_ptr<WorkerPool>                      m_pWorkerPool;
    std::unique_ptr<ThreadLocalStorage<HttpSession>> m_threadLocalHttp;

    std::mutex                                  m_tileMapMutex;
    std::map<uint64_t, RefCountedPtr<OGCTile>>  m_tileMap;
    std::list<RefCountedPtr<OGCBlockQuery>>     m_blockQueryList;


    HFCPtr<HGF2DTransfoModel>              m_pTransfoModel;
    HGFTileIDDescriptor                    m_TileIDDescriptor;
    ImageType                              m_ImageType;

    // optimization for LookAhead
    uint64_t                       m_MaxTilesPerBlockWidth;
    uint64_t                       m_MaxTilesPerBlockHeight;

    HFCExclusiveKey                 m_ExceptionKey;
    HAutoPtr<HFCException>          m_pException;

    Utf8String                         m_grayPixelType;

    static size_t                   s_UncompressedInvalidTileBitmapSize;
    static size_t                   s_CompressedInvalidTileBitmapSize;
    static Byte                    s_CompressedInvalidTileBitmap[];
    HFCPtr<HCDPacket>               m_pInvalidTileBitmap;

    // Methods Disabled
    HRFOGCServiceEditor(const HRFOGCServiceEditor& pi_rObj);
    HRFOGCServiceEditor& operator=(const HRFOGCServiceEditor& pi_rObj);
    };

//----------------------------------------------------------------------------------------
// @bsiclass
//----------------------------------------------------------------------------------------
struct OGCTile : public WorkerPool::Task
{
    OGCTile() {m_hasData=false;}

    virtual ~OGCTile(){};
    
    virtual void _Run() override {/*do nothing we inherit from task to get notify and wait feature*/};

    void SetData(std::vector<Byte>& data)
        {
        m_tileData = std::move(data);
        m_hasData = true;
        }

    void SetData(Byte const* pData, size_t dataSize)
        {
        m_tileData.assign (pData, pData+dataSize);
        m_hasData = true;
        }

    bool HasData() const {return m_hasData;}

    std::atomic<bool> m_hasData;
    std::vector<Byte> m_tileData;
};

//----------------------------------------------------------------------------------------
// @bsiclass
//----------------------------------------------------------------------------------------
struct OGCBlockQuery : public WorkerPool::Task
{
    OGCBlockQuery(HRFOGCServiceEditor::BlocksExtent const& extent, DRange2dCR bbox, HRFOGCServiceEditor& editor)
    :m_blockExtent(extent), m_bbox(bbox), m_editor(editor)
        {}

    virtual ~OGCBlockQuery(){};
    
    virtual void _Run() override;

    bool        ReadBlocksFromServer(uint64_t pi_MinX,
                                     uint64_t pi_YMin,
                                     uint64_t pi_XMax,
                                     uint64_t pi_YMax);

    void        InvalidateTiles (uint64_t                  pi_MinX,
                                 uint64_t                  pi_MinY,
                                 uint64_t                  pi_MaxX,
                                 uint64_t                  pi_MaxY);


    bool       UncompressBuffer(HFCPtr<HFCBuffer>&         pi_rpBuffer,
                                 uint32_t                   pi_Width,
                                 uint32_t                   pi_Height,
                                 Byte*                     po_pUncompressedData,
                                 size_t                    pi_UncompressedDataSize) const;

    HRFOGCServiceEditor::BlocksExtent   m_blockExtent;
    DRange2d                            m_bbox;
    HRFOGCServiceEditor&                m_editor;
};

END_IMAGEPP_NAMESPACE
