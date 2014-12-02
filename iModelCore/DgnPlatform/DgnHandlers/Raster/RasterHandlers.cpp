/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Raster/RasterHandlers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#include <libjpeg-turbo/jpeglib.h>
#include <setjmp.h> // for jpeglib error handling.

static const double GLOBAL_EPSILON         = (0.0000001);
inline double DOUBLE_EQUAL_EPSILON(double v1, double v2)        {return ((v1 <= (v2+GLOBAL_EPSILON)) && (v1 >= (v2-GLOBAL_EPSILON)));}


DGNPLATFORM_TYPEDEFS(MultiResolutionRaster)
DGNPLATFORM_TYPEDEFS(RasterTile)
DGNPLATFORM_TYPEDEFS(RasterTileProvider)
DGNPLATFORM_REF_COUNTED_PTR(MultiResolutionRaster)
DGNPLATFORM_REF_COUNTED_PTR(RasterTile)
DGNPLATFORM_REF_COUNTED_PTR(RasterTileProvider)

#define IsDesignPlane(displayOrder) (0==displayOrder)
#define IsFrontPlane(displayOrder)  (displayOrder>0)
#define IsBackPlane(displayOrder)   (displayOrder<0)

enum
    {
    QV_RGBA_FORMAT              = 0,
    QV_BGRA_FORMAT              = 1,
    QV_RGB_FORMAT               = 2,
    QV_MAX_LAYER                = 31,
    QV_CI_BINRLE                = 2,
    };

// warning C4324: 'my_jpeg_error_mgr' : structure was padded due to __declspec(align())
// __declspec(align()) is coming from jmp_buf definition.
#if defined (_MSC_VER)
    #pragma warning (push)
    #pragma warning (disable:4324)  
#endif // defined (_MSC_VER)
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct my_jpeg_error_mgr : public jpeg_error_mgr
    {
    jmp_buf setjmp_buffer;	
    };
#if defined (_MSC_VER)
    #pragma warning (pop)
#endif // defined (_MSC_VER)

/*---------------------------------------------------------------------------------**//**
* JPEG library ERROR HANDLING:
*
* The default behavior is to exit(1) when an errors occurs. Of course this is
* not what we want. Based on jpeglib\example.c we will used setjmp/longjmp to 
* return control to the caller.  It has been tested on windows, android and iOS.
*
* Another possibility is to use throw in the error callback but our exception
* model assumed that C calls do not throw so it does not work unless we change the
* /EH switch. We decided not to do that. What about iOS and Android exception model?
*
* @bsimethod                                                   Mathieu.Marchand  06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
METHODDEF(void) my_error_exit_no_throw (j_common_ptr cinfo)
    {
    /* cinfo->err really points to a my_jpeg_error_mgr struct, so coerce pointer */
    my_jpeg_error_mgr* myerr = static_cast<my_jpeg_error_mgr*>(cinfo->err);

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message) (cinfo);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
    }

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct JpegDecompressor
    {
    JpegDecompressor()
        {
        // Install error manager.
        m_cInfo.err = jpeg_std_error(&m_cErrMgr);

        // Override default behavior of exit(1) when an error occurs.
        m_cErrMgr.error_exit = my_error_exit_no_throw;
        
        jpeg_create_decompress(&m_cInfo);
        }

    ~JpegDecompressor()
        {
        jpeg_destroy_decompress(&m_cInfo);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  05/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus Decompress(byte* pOutData, size_t outDataSize, byte const* pJpegData, size_t jpegDataSize)
        {
        // Establish the setjmp return context for my_error_exit to use. 
        if (setjmp(m_cErrMgr.setjmp_buffer)) 
            return BSIERROR;

        // Specify the data source.
        jpeg_mem_src(&m_cInfo, const_cast<byte*>(pJpegData), (UInt32)jpegDataSize);

        // Read JPEG Header to extract image information.
        if(JPEG_HEADER_OK != jpeg_read_header(&m_cInfo, true))
            return BSIERROR;        // Could be JPEG_HEADER_TABLES_ONLY, we do not support that.

        // Set Target Image Format. Here we store the decoded image in JCS_RGB format.
        m_cInfo.out_color_space = JCS_RGB;
    
        // Initiate decompress procedure.
        jpeg_start_decompress(&m_cInfo);

        int iRowStride = m_cInfo.output_width * m_cInfo.output_components;
        if(outDataSize < iRowStride*m_cInfo.output_height)
            {
            BeAssert(!"JpegDecompressor: outbuffer too small");
            jpeg_finish_decompress(&m_cInfo);
            return BSIERROR;
            }

        // Decoding loop
        BentleyStatus decodingStatus = BSISUCCESS;
        while(m_cInfo.output_scanline < m_cInfo.output_height)
            {
            JSAMPROW oneRow[1];
            oneRow[0] = (JSAMPROW)(pOutData + m_cInfo.output_scanline*iRowStride);
 
            if (jpeg_read_scanlines(&m_cInfo, oneRow, 1) != 1)
                {
                decodingStatus = BSIERROR;
                break;
                }

            // N.B. m_cInfo.output_scanline is incremented by jpeg_read_scanlines.
            }

        jpeg_finish_decompress(&m_cInfo);

        return decodingStatus;
        }
            

    private:
        struct jpeg_decompress_struct m_cInfo;
        struct my_jpeg_error_mgr      m_cErrMgr;
    };

/*=================================================================================**//**
* struct TileId 
* Use to call qv_getVisibleTiles. 
* layer the highest resolution image.
* row, colum define the upper left tile of an image layer
+===============+===============+===============+===============+===============+======*/
struct TileId
    {
    typedef int IdType;

    IdType layer;      
    IdType row;        
    IdType column;     

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool operator < (const TileId& _Right) const
        {
        if (layer != _Right.layer)
            return layer < _Right.layer;

        if (row != _Right.row)
            return row < _Right.row;

        return column < _Right.column;
        }
    };

/*=================================================================================**//**
* struct RasterTileProvider 
+===============+===============+===============+===============+===============+======*/
struct      RasterTileProvider : public RefCountedBase
{
    //! Load pixels into RasterTile object.
    virtual BentleyStatus _LoadTile(RasterTileR tile) = 0;

    //! Tile size. Must be power of 2.
    virtual UInt32 _GetTileWidth() const = 0;
    virtual UInt32 _GetTileHeight() const = 0;

    virtual UInt32 _GetResolutionCount() const = 0;
    virtual UInt32 _GetWidth(UInt32 resolution) const = 0;
    virtual UInt32 _GetHeight(UInt32 resolution) const = 0;

    // TODO: Will eventually need to abstract this pixel type if we want to support more then Db provider. HRPPixelType would do the job.
    virtual DgnRasterTable::PixelType _GetPixelType() const = 0;
};

/*=================================================================================**//**
* @bsiclass                                                    Mathieu.Marchand  11/2012
+===============+===============+===============+===============+===============+======*/
struct RasterTile : public RefCountedBase, NonCopyableClass
{
private:
    TileId                  m_tileId;
    QvElemP                 m_qvElem;
    MultiResolutionRasterR  m_raster;       // A ref to our parent. MultiResolutionRasterR holds raster tiles.

    RasterTile(TileId const& tileId, MultiResolutionRasterR raster);
        
    virtual ~RasterTile();

    uintptr_t GetQvId() const {return (uintptr_t)this;}

public:

    static RasterTilePtr  CreateRasterTile(TileId const& tileId, MultiResolutionRasterR raster)
        {
        return new RasterTile(tileId, raster);
        }

    //! Load texels into this object.
    BentleyStatus LoadTile(byte const* pTexels, size_t bytesPerLines, UInt32 width, UInt32 height);        

    bool IsLoaded() const {return NULL != m_qvElem;}
    
    TileId const& GetTileId() const {return m_tileId;}

    MultiResolutionRasterCR GetRaster() const {return m_raster;}

    void Draw(ViewContextR viewContext);       
};

/*=================================================================================**//**
* struct MultiResolutionRaster               
+===============+===============+===============+===============+===============+======*/
struct MultiResolutionRaster : public RefCountedBase, NonCopyableClass
{
public:
    struct LayerDefinition
        {
        UInt32 width;
        UInt32 height;
        UInt32 tileCountWidth;
        UInt32 tileCountHeight;
        };

    struct ComposeOptions
        {
        bool         invert;
        RgbaColorDef backgroundColor;
        RgbaColorDef foregroundColor;
        };

private:
    typedef bmap<TileId, RasterTilePtr>     RasterTiles;
    typedef bvector<TileId>                 VisibleTiles;

    QvMRImageP              m_qvMRImageP;
    VisibleTiles            m_visibleTiles;
    RasterTiles             m_tiles;
    RasterTileProviderPtr   m_tileProvider;
    ComposeOptions          m_composeOptions;
    UInt32                  m_CountQvInitCalls;

    MultiResolutionRaster(RasterTileProviderR tileProvider, ComposeOptions const& composeOpts);
    
    virtual ~MultiResolutionRaster();
        
    int QueryVisibleTiles(ViewContextR viewContext);
        
    // eval to one of the QV_*_FORMAT. return true is success.
    bool EvaluateQvPixelFormat(int& qvFormat, bool& enableAlpha) const;
public:
    static MultiResolutionRasterPtr CreateRaster(RasterTileProviderR tileProvider, ComposeOptions const& composeOpts);        

    QvMRImageP GetMRImageP() {return m_qvMRImageP;}

    QvCacheP GetQVCacheP();

    UInt32 GetLayerCount() const {return m_tileProvider->_GetResolutionCount();}
    UInt32 GetLayerWidth(UInt32 layer) const {return m_tileProvider->_GetWidth(layer);}
    UInt32 GetLayerHeight(UInt32 layer) const {return m_tileProvider->_GetHeight(layer);}
        
    UInt32 GetWidth() const { return m_tileProvider->_GetWidth(0);}
    UInt32 GetHeight() const { return m_tileProvider->_GetHeight(0);}
    
    UInt32 GetTileWidth() const {return m_tileProvider->_GetTileWidth();}
    UInt32 GetTileHeight() const {return m_tileProvider->_GetTileHeight();}

    ComposeOptions const& GetComposeOptions() const {return m_composeOptions;}
    
    void Draw(ViewContextR viewContext);

    bool IsBinaryPixel() const {return m_tileProvider->_GetPixelType() == DgnRasterTable::PIXELTYPE_V1Gray;}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE


/*=================================================================================**//**
* @bsiclass                                                    
+===============+===============+===============+===============+===============+======*/
struct RasterHandlerHost : DgnHost::IHostObject
    {
    virtual void _OnHostTermination (bool isProcessShutdown) override {delete this;}

    vector<byte>  m_readBlockIntermediatedBuffer;
    QvCacheP      m_qvCacheP;

    RasterHandlerHost():m_qvCacheP(NULL){}

    virtual ~RasterHandlerHost()
        {
//        if(NULL != m_qvCacheP) 
//            T_HOST.GetGraphicsAdmin()._DeleteQvCache(m_qvCacheP);         // T_HOST.GetGraphicsAdmin() is NULL at this time!
        }                                                                   // From QvTeam: On exit, all the memory is reclaimed on both the CPU and GPU anyway

    // A intermediate buffer use only during data block decompression.
    byte* GetReadBlockIntermediatedBuffer(size_t requestSize)
        {
        if(m_readBlockIntermediatedBuffer.size() < requestSize)
            m_readBlockIntermediatedBuffer.resize(requestSize);

        BeAssert(m_readBlockIntermediatedBuffer.size() >= requestSize);

        return &m_readBlockIntermediatedBuffer[0];
        }

    QvCacheP GetQvCacheP() 
        {
        if(NULL == m_qvCacheP) 
            m_qvCacheP = T_HOST.GetGraphicsAdmin()._CreateQvCache();

        return m_qvCacheP;
        }  
    
    static DgnHost::Key& GetKey() {static DgnHost::Key s_key; return s_key;}
    static RasterHandlerHost* Peek() {return (RasterHandlerHost*) T_HOST.GetHostObject(GetKey());}
    static RasterHandlerHost& Get() 
        {
        RasterHandlerHost* val = Peek();
        if (NULL == val)
            T_HOST.SetHostObject (GetKey(), val = new RasterHandlerHost());
        return  *val;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MultiResolutionRasterPtr MultiResolutionRaster::CreateRaster(RasterTileProviderR tileProvider, ComposeOptions const& composeOpts)
    {
    MultiResolutionRasterPtr mriPtr = new MultiResolutionRaster(tileProvider, composeOpts);
    if(mriPtr->GetMRImageP() == NULL)
        return NULL;

    return mriPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MultiResolutionRaster::MultiResolutionRaster(RasterTileProviderR tileProvider, ComposeOptions const& composeOpts)
    :m_qvMRImageP(NULL),
     m_visibleTiles(16),
     m_tileProvider(&tileProvider),
     m_composeOptions(composeOpts),
     m_CountQvInitCalls(0)
    {
    Point2d imageSize;
    imageSize.x = tileProvider._GetWidth(0);
    imageSize.y = tileProvider._GetHeight(0);
    
    // Induce a flip in Y, because our pixel data is upper-right oriented and we push lower-left transforms.
    DPoint3d corners[4];
    corners[0].x = corners[2].x = 0.0;
    corners[1].x = corners[3].x = imageSize.x;
    corners[2].y = corners[3].y = 0.0;
    corners[0].y = corners[1].y = imageSize.y;
    corners[0].z = corners[1].z = corners[2].z = corners[3].z = 0.0;

    Point2d tileSize;
    tileSize.x = tileProvider._GetTileWidth();
    tileSize.y = tileProvider._GetTileHeight();

    bool enableAlpha;
    int qvPixelFormat;
    if(EvaluateQvPixelFormat(qvPixelFormat, enableAlpha))
        m_qvMRImageP = T_HOST.GetGraphicsAdmin()._CreateQvMRImage(corners, imageSize, tileSize, enableAlpha, qvPixelFormat, 0x00/*tileFlags*/, GetLayerCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MultiResolutionRaster::~MultiResolutionRaster()
    {
    m_tiles.clear();    // Free Qv tiles before we delete the cache.
            
    if(NULL != m_qvMRImageP) 
        T_HOST.GetGraphicsAdmin()._DeleteQvMRImage(m_qvMRImageP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiResolutionRaster::EvaluateQvPixelFormat(int& qvFormat, bool& enableAlpha) const
    {
    bool isValid = false;

    switch(m_tileProvider->_GetPixelType())
        {
        case DgnRasterTable::PIXELTYPE_V32Rgba:
            {
            enableAlpha = true;
            qvFormat = QV_RGBA_FORMAT;
            isValid = true;
            break;
            }

        case DgnRasterTable::PIXELTYPE_V24Rgb:
            {
            enableAlpha = false;
            qvFormat = QV_RGB_FORMAT;
            isValid = true;
            break;
            }
            
        case DgnRasterTable::PIXELTYPE_V1Gray:
            {
            RgbaColorDef const& color0 = m_composeOptions.invert ? m_composeOptions.foregroundColor : m_composeOptions.backgroundColor;
            RgbaColorDef const& color1 = m_composeOptions.invert ? m_composeOptions.backgroundColor : m_composeOptions.foregroundColor;
                        
            unsigned long paletteTBGR[2];
            paletteTBGR[0] = ((((((255-color0.alpha) << 8) + color0.blue) << 8) + color0.green) << 8) + color0.red;
            paletteTBGR[1] = ((((((255-color1.alpha) << 8) + color1.blue) << 8) + color1.green) << 8) + color1.red;

            enableAlpha = color0.alpha != 255 || color1.alpha != 255;

            qvFormat = T_HOST.GetGraphicsAdmin()._DefineCIFormat(QV_CI_BINRLE, 2, paletteTBGR);
            BeAssert(0 != qvFormat);
            isValid = 0 != qvFormat;
            break;
            }

        default:
            qvFormat = -1;
            enableAlpha = false;
            isValid = false;
            break;
        }
   
    return isValid;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int MultiResolutionRaster::QueryVisibleTiles(ViewContextR viewContext)
    {
    IViewOutputP pViewOutput = viewContext.GetViewport()->GetIViewOutput();
    if(NULL == pViewOutput)
        {
        m_visibleTiles.resize(0);
        return 0;
        }

#if 0 //&&MM optimization. Display at lower resolution when raster appears as a line or close to. From observation, even for 10-20 pixels wide the 
    //       lowest resolution is enough.
    DPoint3d corners[4], viewCorners[4];
    corners[0].x = corners[2].x = 0.0;
    corners[1].x = corners[3].x = GetWidth();
    corners[0].y = corners[1].y = 0.0;
    corners[2].y = corners[3].y = GetHeight();
    corners[0].z = corners[1].z = corners[2].z = corners[3].z = 0.0;
    viewContext.LocalToView(viewCorners, corners, 4);

    viewCorners[0].z = viewCorners[1].z = viewCorners[2].z = viewCorners[3].z = 0;
    DVec3d uVecView = DVec3d::FromStartEnd(viewCorners[0], viewCorners[1]);
    DVec3d vVecView = DVec3d::FromStartEnd(viewCorners[0], viewCorners[2]);

    double magnitudeU = uVecView.Magnitude(); magnitudeU;
    double magnitudeV = vVecView.Magnitude(); magnitudeV;

    static bool s_drawTopOnly = false;

    if(s_drawTopOnly)
        {
        //if(uVecView.Magnitude() < 1.0 || vVecView.Magnitude() < 1.0)
        // Handle the case where the raster appears as a line. Qv will return that all tiles(1:1) are visible
        // In this case we only want the last res. 
    //     if(fabs(viewCorners[0].x - viewCorners[1].x) < 1 &&
    //        fabs(viewCorners[0].x - viewCorners[2].x) < 1 && 
    //        fabs(viewCorners[0].x - viewCorners[3].x) < 1 &&
    //        fabs(viewCorners[0].y - viewCorners[1].y) < 1 &&
    //        fabs(viewCorners[0].y - viewCorners[2].y) < 1 &&
    //        fabs(viewCorners[0].y - viewCorners[3].y) < 1)
            {
            m_visibleTiles.resize(1);
            m_visibleTiles[0].layer =static_cast<TileId::IdType>(m_layerDefinition.size()-1);
            m_visibleTiles[0].column = 0;
            m_visibleTiles[0].row = 0;
            return 1;
            }
        }
#endif
        
    int numTiles = pViewOutput->GetVisibleTiles(m_qvMRImageP, m_visibleTiles.size()*3, (VisibleTiles::value_type::IdType*)&m_visibleTiles[0]);

    if (numTiles > (int)m_visibleTiles.size())
        {
        m_visibleTiles.resize(numTiles);
        numTiles = pViewOutput->GetVisibleTiles(m_qvMRImageP, m_visibleTiles.size()*3, (VisibleTiles::value_type::IdType*)&m_visibleTiles[0]);
        }
    else
        {
        m_visibleTiles.resize(numTiles);
        }

    return numTiles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
QvCacheP MultiResolutionRaster::GetQVCacheP()
    {
    return RasterHandlerHost::Get().GetQvCacheP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiResolutionRaster::Draw(ViewContextR viewContext)
    {
    if (m_CountQvInitCalls != ViewContext::GetCountQvInitCalls())
        {   
        m_CountQvInitCalls = ViewContext::GetCountQvInitCalls();

        m_tiles.clear();    // Cleanup old tiles?

            //{
            //char Msg[128];
            //BeStringUtilities::Snprintf (Msg, _countof(Msg), "*****  Clear tiles - GetCountQvInitCalls:%u", m_CountQvInitCalls);
            //BeDebugLog(Msg);
            //}
        }

#if !defined (BENTLEY_WIN32)
    if(DrawPurpose::UpdateDynamic == viewContext.GetDrawPurpose())
        {
         // Draw only the current loaded tiles.
        for(RasterTiles::iterator itr(m_tiles.begin()); itr != m_tiles.end(); ++itr)
            {
             if(itr->second->IsLoaded())
                 itr->second->Draw(viewContext);
            }

        //BeDebugLog("****** Dynamic-Draw");
        return;
        }
#endif

    // -1 means the drawn call is deferred(for printing?). We won't handle that in DgnDB, I think.
    if(QueryVisibleTiles(viewContext) <= 0)
        {    
        m_tiles.clear();    // Cleanup old tiles?
        //BeDebugLog("****** QueryVisibleTiles(viewContext) <= 0");
        return;
        }     

    RasterTiles oldTiles = m_tiles;

    m_tiles.clear();

    for(size_t tile=0; tile < m_visibleTiles.size(); ++ tile)
        {
        RasterTiles::iterator itr = oldTiles.find(m_visibleTiles[tile]);

        if(itr != oldTiles.end())
            {
            m_tiles.insert(*itr);
            }
        else
            {
            // Create a new tile but do not load it yet. Wait until we flush the old ones.
            RasterTilePtr newTilePtr = RasterTile::CreateRasterTile(m_visibleTiles[tile], *this);
            m_tiles.insert(RasterTiles::value_type(m_visibleTiles[tile], newTilePtr));
            }
        }

    // Free memory of unused tiles
    oldTiles.clear();

     //{
     //char Msg[128];
     //BeStringUtilities::Snprintf (Msg, _countof(Msg), "***** NormalDraw m_tiles.size():%u", m_tiles.size());
     //BeDebugLog(Msg);
     //}


    // Load and draw visible tiles.
    for(RasterTiles::iterator itr(m_tiles.begin()); itr != m_tiles.end(); ++itr)
        {
#if 0   //&&MM needswork Tablets are more responsive but we sometimes loose all the tiles.
        if(viewContext._CheckStop())
            break;      // Should we draw tiles that are loaded?
#endif
        
        if(!itr->second->IsLoaded())
            m_tileProvider->_LoadTile(*itr->second);

        itr->second->Draw(viewContext);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RasterTile::RasterTile(TileId const& tileId, MultiResolutionRasterR raster)
    :m_tileId(tileId),
     m_raster(raster),
     m_qvElem(NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RasterTile::~RasterTile()
    {
    if(NULL != m_qvElem) 
        T_HOST.GetGraphicsAdmin()._DeleteQvElem(m_qvElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RasterTile::LoadTile(byte const* pTexels, size_t bytesPerLine, UInt32 width, UInt32 height)
    {
    BeAssert(m_qvElem == NULL);

    if(m_raster.GetQVCacheP() == NULL)
        return BSIERROR;

    Point2d tilesize;
    tilesize.x = width; 
    tilesize.y = height;
    
    m_qvElem = T_HOST.GetGraphicsAdmin()._CreateQvTile(true/*is3d*/, m_raster.GetQVCacheP(), m_raster.GetMRImageP(), GetQvId(), 
                                                       m_tileId.layer, m_tileId.row, m_tileId.column, 
                                                       height, (int)bytesPerLine, tilesize, pTexels);

    return m_qvElem ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterTile::Draw(ViewContextR viewContext)
    {
    if(IsLoaded())
        {
        // Need to call on IViewDraw to avoid by-passing context override. Calling viewport directly will assert in QV if called in pick context.
        viewContext.GetIViewDraw().DrawQvElem3d(m_qvElem, 0);
        //viewContext.GetViewport()->GetIViewOutput()->DrawQvElem3d(m_qvElem, 0);
        }
    }

/*=================================================================================**//**
* @bsiclass                                                    Mathieu.Marchand  11/2012
+===============+===============+===============+===============+===============+======*/
struct DgnDbTileProvider : public RasterTileProvider
{
private:
    DgnRasterTable::Row     m_rasterHeader;    
    DgnRasterDataTable      m_rasterData;   
    std::auto_ptr<JpegDecompressor> m_pJpegDecompresor;

    DgnDbTileProvider(DgnProjectR project, DgnRasterTable::Row const& rasterHeader)
        :m_rasterData(project),
         m_rasterHeader(rasterHeader)
        {
        };

    virtual ~DgnDbTileProvider() {};

    virtual UInt32 _GetTileWidth() const override {return m_rasterHeader.GetBlockWidth();}
    virtual UInt32 _GetTileHeight() const override {return m_rasterHeader.GetBlockHeight();}

    virtual UInt32 _GetResolutionCount() const override {return (UInt32)m_rasterHeader.GetResolutions().size();}
    virtual UInt32 _GetWidth(UInt32 resolution) const override {return m_rasterHeader.GetResolutions()[resolution].width;}
    virtual UInt32 _GetHeight(UInt32 resolution) const override {return m_rasterHeader.GetResolutions()[resolution].height;}
    virtual DgnRasterTable::PixelType _GetPixelType() const {return m_rasterHeader.GetPixelType();}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  05/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    JpegDecompressor& GetJpegDecompresor()
        {
        BeAssert(DgnRasterTable::COMPRESS_Jpeg == m_rasterHeader.GetCompressionType());
        if(m_pJpegDecompresor.get() == NULL)
            m_pJpegDecompresor.reset(new JpegDecompressor);

        return *m_pJpegDecompresor;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    UInt32 CountBytesPerPixel(DgnRasterTable::PixelType const& dbPixelType)
        {
        switch(dbPixelType)
            {
            case DgnRasterTable::PIXELTYPE_V32Rgba:    return 4;
            case DgnRasterTable::PIXELTYPE_V24Rgb:     return 3;
            case DgnRasterTable::PIXELTYPE_V1Gray:     return 0;   // Compress data.
            default:
                break;
            }

        return 0;
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  12/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus ClipRLETile(UInt16* pDestBuffer, size_t destBufferSize, UInt32 destWidth, UInt32 destHeight,
                              UInt16 const* pSrcBuffer, size_t srcBufferSize, UInt32 srcWidth, UInt32 srcHeight) const
        {
        BeAssert(destWidth <= srcWidth);
        BeAssert(destHeight <= srcHeight);

        UInt16 const* pSrcLineBuffer = pSrcBuffer;

        UInt16* pDestLineBuffer = pDestBuffer;

        for(UInt32 line=0; line < destHeight; ++line)
            {
            UInt32 pixelCount = 0;
            UInt32 srcIndex = 0;
            UInt32 destIndex = 0;

            // Copy until we reach clip limit.
            while(pixelCount + pSrcLineBuffer[srcIndex] < destWidth)
                {
                pixelCount+=pSrcLineBuffer[srcIndex];
                ++srcIndex;
                }
            memcpy(pDestLineBuffer, pSrcLineBuffer, srcIndex*sizeof(UInt16));
            destIndex = srcIndex;

            // Clip destination line
            pDestLineBuffer[destIndex] = static_cast <UInt16> (destWidth - pixelCount);
            ++destIndex;

            // Dest Line must end on OFF state
            if(!(destIndex & 0x1))
                {
                pDestLineBuffer[destIndex] = 0;
                ++destIndex;
                }

            // skip src until we read all the pixels of the current line.
            pixelCount += pSrcLineBuffer[srcIndex];
            ++srcIndex;
            while(pixelCount < srcWidth)
                {
                pixelCount += pSrcLineBuffer[srcIndex];
                ++srcIndex;
                }

            // Src line must end on OFF state.
            if(!(srcIndex & 0x1))
                {
                BeAssert(0 == pSrcLineBuffer[srcIndex]);   // Any extra entry to end a line with OFF state should be equal to '0'
                ++srcIndex;
                }

            BeAssert(pSrcLineBuffer + (srcIndex-1) < pSrcBuffer + (srcBufferSize)/2);
            BeAssert(pDestLineBuffer + (destIndex-1) < pDestBuffer + (destBufferSize/2));

            // offset to start of next line.
            pDestLineBuffer +=destIndex;
            pSrcLineBuffer +=srcIndex;
            }

        return SUCCESS;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    static RasterTileProviderPtr CreateDgnDbTileProvider(DgnProjectR project, DgnRasterFileId rasterId)
        {
        DgnRasterTable rasterTable(project);
        
        DgnRasterTable::Row rasterHeader = rasterTable.QueryRasterById(rasterId);
        if(!rasterHeader.IsValid())
            return NULL;

        if(rasterHeader.GetBlockWidth() != 256 || rasterHeader.GetBlockWidth() != 256)
            return NULL;

        bool isSupported = false;
        switch(rasterHeader.GetPixelType())
            {
            case DgnRasterTable::PIXELTYPE_V1Gray:
                isSupported = (rasterHeader.GetCompressionType() == DgnRasterTable::COMPRESS_RLE);
                break;
            case DgnRasterTable::PIXELTYPE_V24Rgb:
                isSupported = (rasterHeader.GetCompressionType() == DgnRasterTable::COMPRESS_None || rasterHeader.GetCompressionType() == DgnRasterTable::COMPRESS_Jpeg || rasterHeader.GetCompressionType() == DgnRasterTable::COMPRESS_Deflate);
                break;
            case DgnRasterTable::PIXELTYPE_V32Rgba:
                isSupported = (rasterHeader.GetCompressionType() == DgnRasterTable::COMPRESS_None || rasterHeader.GetCompressionType() == DgnRasterTable::COMPRESS_Deflate);
                break;
            default:
                isSupported = false;
                break;
            }

        if(isSupported)
            return new DgnDbTileProvider(project, rasterHeader);

        return NULL;        
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual BentleyStatus _LoadTile(RasterTileR tile) override
        {
        DgnRasterDataTable::RasterBlockDataPtr pData;
        {
        //  TODO remove this!!!
        //  This is here solely to prevent an assert from triggerring in HighPriorityOperationSequencer::CheckSQLiteOperationAllowed
        //  Remove this once the tile handling logic has been changed to stop loading tiles during update dynamics.
        BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;
        BeSQLite::DbResult result;
        pData = m_rasterData.ReadDataPtr(result, m_rasterHeader.GetId(), tile.GetTileId().layer, tile.GetTileId().row, tile.GetTileId().column);
        }

        if(pData.IsNull())
            return BSIERROR;

        UInt32      dataSize = 0; 
        byte const* pPixels  = (byte const*)pData->GetData(dataSize);
        
        size_t bytesPerLine = _GetTileWidth()*CountBytesPerPixel(_GetPixelType());

        // Adjust how many 'valid' pixels we have for border tiles.
        UInt32 effectiveWidth  = MIN(_GetWidth(tile.GetTileId().layer)  - (tile.GetTileId().column*_GetTileWidth()), _GetTileWidth());
        UInt32 effectiveHeight = MIN(_GetHeight(tile.GetTileId().layer) - (tile.GetTileId().row*_GetTileHeight()), _GetTileHeight());

        BentleyStatus status = BSIERROR;

        switch(m_rasterHeader.GetCompressionType())
            {
            case DgnRasterTable::COMPRESS_Deflate:
                {
                size_t uncompressBufferSize = bytesPerLine*_GetTileHeight();
                byte*  pUncompressBuffer = RasterHandlerHost::Get().GetReadBlockIntermediatedBuffer(uncompressBufferSize);
            
                //&&MM  use DgnZLib::UnZipper also for compress.

                // Prior to uncompress 'dataSize' must be of 'pUncompressBuffer' size. After the call, it is set to the uncompress data size.
                ULong32 uncompressSize = (ULong32)uncompressBufferSize;
                if (Z_OK == uncompress(pUncompressBuffer, &uncompressSize, pPixels, dataSize) && uncompressSize == uncompressBufferSize)
                    status = tile.LoadTile(pUncompressBuffer, bytesPerLine, effectiveWidth, effectiveHeight);
                } 
                break;

            case DgnRasterTable::COMPRESS_Jpeg:
                {
                size_t uncompressBufferSize = bytesPerLine*_GetTileHeight();
                byte*  pUncompressBuffer = RasterHandlerHost::Get().GetReadBlockIntermediatedBuffer(uncompressBufferSize);

                if(SUCCESS == (status = GetJpegDecompresor().Decompress(pUncompressBuffer, uncompressBufferSize, pPixels, dataSize)))
                    status = tile.LoadTile(pUncompressBuffer, bytesPerLine, effectiveWidth, effectiveHeight);
                }
                break;

            case DgnRasterTable::COMPRESS_RLE:
                {
                // Need to 'clip' right border tiles to the effective width. Qv can't handle line padding for RLE data.
                if(_GetTileWidth() != effectiveWidth)
                    {       
                    size_t  destBufferSize = (((effectiveWidth * 2) + 2)* sizeof(UInt16)) * effectiveHeight;
                    UInt16* pDestBuffer = (UInt16*)RasterHandlerHost::Get().GetReadBlockIntermediatedBuffer(destBufferSize);
            
                    if(SUCCESS == (status = ClipRLETile(pDestBuffer, destBufferSize, effectiveWidth, effectiveHeight, (UInt16 const*)pPixels, dataSize, _GetTileWidth(), _GetTileHeight())))
                        status = tile.LoadTile((byte const*)pDestBuffer, 0, effectiveWidth, effectiveHeight);
                    }
                else
                    {
                    status = tile.LoadTile(pPixels, bytesPerLine, effectiveWidth, effectiveHeight);
                    }
                }
                break;

            case DgnRasterTable::COMPRESS_None:
                //&&MM make sure dataSize is what we expect.
                status = tile.LoadTile(pPixels, bytesPerLine, effectiveWidth, effectiveHeight);
                break;

            default:
                BeDataAssert(!"DgnDbTileProvider: Unsupported compression format");
                status = BSIERROR;  //&&MM We probably should flag the tile as bad so we stop trying to load it.
                break;                
            }

        return status;
        }
};


/*=================================================================================**//**
* @bsiclass                                                    Mathieu.Marchand  11/2012
+===============+===============+===============+===============+===============+======*/
struct          RasterAppData : ElementRefAppData
{
//! Member variables
private:
    static Key          s_key;

    MultiResolutionRasterPtr    m_raster;

    //! For debugging
    virtual WCharCP   _GetName () override {return L"RasterAppData";}

    /*---------------------------------------------------------------------------------**//**
    * Called to clean up owned resources and delete the app data.
    * @bsimethod                                                   Mathieu.Marchand  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void        _OnCleanup (ElementRefP host, bool unloadingCache, HeapZone& zone) override
        {
        m_raster = NULL;

        if (!unloadingCache)
            zone.Free (this, sizeof *this);
        }

    /*---------------------------------------------------------------------------------**//**
    * Called to allow app data to react to changes to the persistent element it was added to.
    * @return true to drop this app data entry from the element.
    * @bsimethod                                                   Mathieu.Marchand  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool        _OnElemChanged (ElementRefP host, bool qvCacheDeleted, ElemRefChangeReason) override
        {
        // Do not care for now. Our raster tiles are not affected by element changes.
        return false;
        }

    RasterAppData () {}
    virtual ~RasterAppData () {}

    MultiResolutionRasterP GetMultiResolutionRasterP() {return m_raster.get();}
    void SetMultiResolutionRasterP(MultiResolutionRasterP pNewRaster) {m_raster = pNewRaster;}

public:

    static RasterAppData* GetAppData (ElementRefP ref)
        {
        return ref ? (RasterAppData*) ref->FindAppData (s_key) : NULL;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    static MultiResolutionRasterP QueryMultiResolutionRaster(ElementRefP ref)
        {
        RasterAppData* appData = GetAppData (ref);

        if (NULL == appData)
            return NULL;

        return appData->GetMultiResolutionRasterP();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void StoreMultiResolutionRaster(MultiResolutionRasterP pNewRaster, ElementRefP ref)
        {
        if (NULL == ref)
            return;     

        HeapZone& zone = ref->GetHeapZone();

        RasterAppData* appData = GetAppData (ref);
        if (NULL == appData)
            {
            void* mem = zone.Alloc (sizeof(RasterAppData));
            appData = new (mem) RasterAppData ();
            ref->AddAppData (s_key, appData, zone);
            }

        appData->SetMultiResolutionRasterP(pNewRaster);
        }
    
}; // RasterAppData

ElementRefAppData::Key RasterAppData::s_key;

#if defined(WIP_V10_RASTER)//Do we really want to prevent raster to drape if drape is on; draping is not functional
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_IsRenderModeDrapable(ViewContextP viewContextP)
    {
    // Check context render mode
    switch (viewContextP->GetViewFlags()->renderMode)
        {
        case MSRenderMode::ConstantShade:
        case MSRenderMode::SmoothShade:
        case MSRenderMode::Phong:
        case MSRenderMode::RayTrace:
        case MSRenderMode::Radiosity:
        case MSRenderMode::ParticleTrace:
        case MSRenderMode::RenderLuxology:
            return true;   // Draping mode.

        case MSRenderMode::Wireframe:
        case MSRenderMode::CrossSection:
        case MSRenderMode::Wiremesh:
        case MSRenderMode::HiddenLine:
        case MSRenderMode::SolidFill:
        case MSRenderMode::RenderWireframe:
        default:
            break;
        }
    return false;
    }
#endif

/*---------------------------------------------------------------------------------**//**

/*=================================================================================**//**
* @bsiclass                                     		Marc.Bedard     11/2012
+===============+===============+===============+===============+===============+======*/
class SetAndRestoreProjectionDepth
    {
    private:
        ViewportP m_vp;

        SetAndRestoreProjectionDepth (SetAndRestoreProjectionDepth const&);
        SetAndRestoreProjectionDepth& operator= (SetAndRestoreProjectionDepth const&);

    public:
        SetAndRestoreProjectionDepth(ViewportP vp, double newDepth)
            {
            m_vp = vp;

            if (m_vp && m_vp->GetIViewOutput())
                m_vp->GetIViewOutput()->SetProjectDepth(newDepth);
            }
        ~SetAndRestoreProjectionDepth()
            {
            if (m_vp &&  m_vp->GetIViewOutput())
                m_vp->GetIViewOutput()->SetProjectDepth(0.0);
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class PushPopBackFrontClipPlanes
    {
    private:
        ViewContextP    m_contextP;
        bool            m_depthPlanesPushed;

    public:

        PushPopBackFrontClipPlanes(ViewContextP context) : m_contextP (context), m_depthPlanesPushed(false)
            {
            if (m_contextP == NULL)
                return;

            ViewFlagsCP viewFlags = m_contextP->GetViewFlags();
            if (!viewFlags)
                return;

            // Front or back clip must be active.
            if(viewFlags->noFrontClip && viewFlags->noBackClip)
                return;

            DPoint3d localPoints[NPC_CORNER_COUNT], npcPoints[NPC_CORNER_COUNT], viewPoints[NPC_CORNER_COUNT];
            GetNpcPoints(npcPoints, m_contextP);
            m_contextP->NpcToView (viewPoints, npcPoints, 8);
            m_contextP->ViewToLocal(localPoints, viewPoints, 8);

            ConvexClipPlaneSet  depthPlanes;

            if (!viewFlags->noBackClip)
                AddPlaneFromPoints (depthPlanes, &localPoints[0], &localPoints[2], &localPoints[1], 1.0E-6); // Back

            if (!viewFlags->noFrontClip)
                AddPlaneFromPoints (depthPlanes, &localPoints[4], &localPoints[5], &localPoints[6], 1.0E-6); // Front

            if (false != (depthPlanes.size() > 0))
                {
                ClipPlaneSet    clipPlanes (depthPlanes);

                m_contextP->PushClipPlanes(clipPlanes);
                }
            }

        ~PushPopBackFrontClipPlanes()
            {
            if (m_contextP != NULL && m_depthPlanesPushed)
                m_contextP->PopTransformClip();
            }

    private:
        // Disabled
        PushPopBackFrontClipPlanes();
        PushPopBackFrontClipPlanes(PushPopBackFrontClipPlanes const&);
        PushPopBackFrontClipPlanes& operator=(PushPopBackFrontClipPlanes const&);

        void AddPlaneFromPoints
            (
            ConvexClipPlaneSetR planeSet,
            DPoint3d*           pPoint0,
            DPoint3d*           pPoint1,
            DPoint3d*           pPoint2,
            double              expandPlaneDistance
            )
            {
            ClipPlane           plane;

            plane.SetFlags (/*invisible*/false, /*interior*/false);
            bsiDPoint3d_crossProduct3DPoint3d (&plane.m_normal, pPoint2, pPoint1, pPoint0);
            if (0.0 != bsiDPoint3d_normalizeInPlace (&plane.m_normal))
                {
                plane.m_distance = bsiDPoint3d_dotProduct (&plane.m_normal, pPoint0) - expandPlaneDistance;
                planeSet.push_back (plane);
                }
            }

        void GetNpcPoints (DPoint3d npcPoints[8], ViewContextP context)
            {
            Frustum frustum = context->GetFrustum ();

            memcpy (npcPoints, frustum.GetPts (), 8*sizeof(DPoint3d));
            }
    };

/*---------------------------------------------------------------------------------------
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/_/_/_/_/_/_/_/_/_/ RasterFrameHandler  _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       RasterFrameHandler::_OnTransform
(
EditElementHandleR editElmHandle,
TransformInfoCR transform
)
    {
    StatusInt status(SUCCESS);

#ifdef WIP_V10_GEOCOORD
    if (RasterDgnGCSFacility::IsReprojected (editElmHandle))
        return ERROR;
#endif

    if ((status = T_Super::_OnTransform (editElmHandle, transform)) != SUCCESS)
        return status;

    //Nothing to do if identity transform
    if (transform.GetTransform()->isIdentity())
        return SUCCESS;

    //Transform raster frame element object
    Transform frameT;
    RasterTransformFacility::SetUV(frameT,GetU(editElmHandle),GetV(editElmHandle));
    RasterTransformFacility::SetTranslation(frameT,GetOrigin(editElmHandle));

    Transform    resultMatrix(Transform::FromProduct(*transform.GetTransform(), frameT));

    //Get new u and v bvector
    DVec3d uNewVect(RasterTransformFacility::GetU(resultMatrix));
    DVec3d vNewVect(RasterTransformFacility::GetV(resultMatrix));

    //Do some validation, We don't support to have one column set to 0, change it to identity
    //This case occurred when transform from 3D to 2D by a flatten transform
    if (!Is3dElem(editElmHandle.GetElementCP()))
        {
        //Check 2D because matrix might contains a 3D component when it cames from a snap point which appears to
        //include display priority of the snap point in its z coordinate.
        //Since snap point are always drawn on top of everything in 2D, we got translation z=maxDisplayPriority.
        //This is not appropriate and we don't want to change  z origin in 2D.

        // This is a 2D frame, wipe 3D information from matrix.
        resultMatrix.form3d[0][2] = 0.0;
        resultMatrix.form3d[1][2] = 0.0;
        resultMatrix.form3d[2][0] = 0.0;
        resultMatrix.form3d[2][1] = 0.0;
        resultMatrix.form3d[2][2] = 1.0;
        resultMatrix.form3d[2][3] = 0.0;

        // the frame must have an area in the xy plane.
        // Compute the normal to UV
        DVec3d nVect;
        nVect.crossProduct(&uNewVect, &vNewVect);

        // if the normal of the frame do not have a z component, then it is not visible in the xy plane.
        if (DOUBLE_EQUAL_EPSILON(nVect.z, 0.0))
            return ERROR;
        }
    else
        {
        // 3D frame, get UV
        // Compute the normal to UV
        DVec3d nVect;
        nVect.crossProduct(&uNewVect, &vNewVect);

        // Third column of the matrix must be the normal of UV
        nVect.normalize();
        resultMatrix.form3d[0][2] = nVect.x;
        resultMatrix.form3d[1][2] = nVect.y;
        resultMatrix.form3d[2][2] = nVect.z;

        // UV must define an area
        if (DOUBLE_EQUAL_EPSILON(nVect.magnitude(), 0.0))
            return ERROR;
        }

    //Calculate new extent and set in input editElemHandle
    SetU(editElmHandle,RasterTransformFacility::GetU(resultMatrix),false);
    SetV(editElmHandle,RasterTransformFacility::GetV(resultMatrix),false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    // Flags it as 3D 
    eeh.GetElementP ()->SetIs3d(true);

    //Change elevation 
    DPoint3d origin(GetOrigin(eeh));
    origin.z = elevation;
    SetOrigin(eeh,origin,false);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RasterPropertiesFilter : public ProcessPropertiesFilter
{
public:

RasterPropertiesFilter (IQueryProperties* queryObj) : ProcessPropertiesFilter (queryObj) {}
RasterPropertiesFilter (IEditProperties*  editObj)  : ProcessPropertiesFilter (editObj)  {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            _EachLevelCallback (EachLevelArg& arg) override
    {
    // Fix TR #155094
    // Raster Frame element with level 0 are valid since they came from an upgrade of
    // pre-XM design file (< 8.9) on which raster doesn't have a level.
    // We want them to always be displayed.
    if (0 != (arg.GetPropertyFlags() & PROPSCALLBACK_FLAGS_IsBaseID) && !arg.GetStoredValue().IsValid())
        {
        IEditProperties*    editObj = arg.GetPropertyContext ().GetIEditPropertiesP ();

        // Allow 0 level raster to be changed to a real level w/change attribute type tools...
        if (!editObj || EditPropertyPurpose::Change != editObj->_GetEditPropertiesPurpose ())
            arg.SetPropertyFlags ((PropsCallbackFlags) (PROPSCALLBACK_FLAGS_ElementIgnoresID | arg.GetPropertyFlags ()));
        }

    m_callbackObj->_EachLevelCallback (arg);
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            RasterFrameHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    // NOTE: Need to set level "ignore" flags....
    IQueryProperties*       queryObj = context.GetIQueryPropertiesP ();
    RasterPropertiesFilter  filterObj (queryObj);

    context.SetIQueryPropertiesP (&filterObj);
    T_Super::_QueryProperties (eh, context);
    context.SetIQueryPropertiesP (queryObj);

    if (0 == (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
        return;

    DgnElementCP elmCP = eh.GetElementCP ();

    context.DoColorCallback (NULL, EachColorArg (elmCP->ToRasterFrameElm().backgroundColor, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
    context.DoColorCallback (NULL, EachColorArg (elmCP->ToRasterFrameElm().foregroundColor, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            RasterFrameHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    // NOTE: Need to set level "ignore" flags....
    IEditProperties*        editObj = context.GetIEditPropertiesP ();
    RasterPropertiesFilter  filterObj (editObj);

    context.SetIEditPropertiesP (&filterObj);
    T_Super::_EditProperties (eeh, context);
    context.SetIEditPropertiesP (editObj);

    if (0 == (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
        return;

    DgnElementP  elmP = eeh.GetElementP ();

    context.DoColorCallback (&elmP->ToRasterFrameElmR().backgroundColor, EachColorArg (elmP->ToRasterFrameElm().backgroundColor, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
    context.DoColorCallback (&elmP->ToRasterFrameElmR().foregroundColor, EachColorArg (elmP->ToRasterFrameElm().foregroundColor, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MarcBedard  10/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RasterFrameHandler::_OnPreprocessCopy
(
EditElementHandle &    editElmHandle,
CopyContextP        context
)
    {
    if (SUCCESS != T_Super::_OnPreprocessCopy (editElmHandle, context))
        return ERROR;

    // When an attachment is duplicated, we want the new attachment to have a the Georeference Priority set to attachment,
    // and the raster file opened in ReadOnly.
    // We don't know why we're copying the attachment, it could be from a MstnTool with Copy toggled on or from a merge into master
    // either way, we can't assume that we will be able to keep a PRIORITY_RASTERFILE and a RW mode.
    SetOpenReadWrite(editElmHandle,false);

    // We intentionally create duplicate the layer.

#ifdef WIP_V10_GEOCOORD
    //Processing Reprojection settings
    DgnModelP    sourceDgnModel;
    DgnModelP    destinationDgnModel;
    bool            sameCache;
    bool            sameFile;

    context->GetModels (&sourceDgnModel, &destinationDgnModel, &sameCache, &sameFile, &editElmHandle);

    IGeoCoordinateServicesP gcsServices;
    if (NULL == (gcsServices = GeoCoordinationManager::GetServices()))
        return ERROR;

    DgnGCSP         rasterGCS           = NULL;
    DgnGCSP         sourceDgnModelGCS   = NULL;
    DgnGCSP         destDgnModelGCS     = NULL;
    ElementHandle   geoCodingEh = pRasterFrameHandler->GetGeocoding(editElmHandle);
    if (geoCodingEh.IsValid())
        rasterGCS = gcsServices->CreateGCSFromElement (geoCodingEh);

    bool isSourceModelGeoReprojected = sourceDgnModel->GetDgnModelP()->IsGeographicReprojected();

    // If the source model is reprojected and raster inherit from its GCS
    if (isSourceModelGeoReprojected && (rasterGCS==NULL || pRasterFrameHandler->GetGCSInheritedFromModelState(editElmHandle)))
        {
        //We will set sourceDgnModel GCS to raster because we don't want raster to move relative to source elements
        sourceDgnModelGCS = gcsServices->CreateGCSFromModel (sourceDgnModel);
        pRasterFrameHandler->SetGeocoding (editElmHandle, RasterDgnGCSFacility::GetGeocodingElemHandle (sourceDgnModelGCS, destinationDgnModel),false);

        //TR #289140; We have to take into account the UOR scale between model
        //TRICKY: We need to apply uor scale between models to our raster frame element.
        //        BUT, we cannot extract this scale from model info since it was changed
        //        at open when the cache was reprojected. (see mstn\geocoord\mstn\transformCache.cpp 
        //        in method Cachereproject::DoReproject; refModelInfo->m_uorPerStorage  = rootModelInfo->m_uorPerStorage;).
        //        Thus, we extract it from the MstnGCS unit info.
        double uorScaleBetweenModel(1.0);
        destDgnModelGCS  = gcsServices->CreateGCSFromModel (destinationDgnModel);
        if (sourceDgnModelGCS!=NULL && destDgnModelGCS!=NULL)
            {
            UnitDefinition unitTo;
            UnitDefinition unitFrom;
            if (RasterDgnGCSFacility::GetUnitDefinition(sourceDgnModelGCS,unitFrom) && RasterDgnGCSFacility::GetUnitDefinition(destDgnModelGCS,unitTo))
                {
                if (SUCCESS != unitTo.GetConversionFactorFrom (uorScaleBetweenModel, unitFrom))
                    uorScaleBetweenModel = 1.0;

                if (uorScaleBetweenModel != 1.0)
                    {
                    //Apply UOR scale between model to our raster element
                    Transform UorRefToUorActive;
                    UorRefToUorActive.InitIdentity();
                    UorRefToUorActive.form3d[0][0] = uorScaleBetweenModel;
                    UorRefToUorActive.form3d[1][1] = uorScaleBetweenModel;

                    //Apply uor scale between models to our raster element
                    Transform rasterMatrix;
                    GetTransform(editElmHandle, rasterMatrix);
                    Transform newrasterMatrix4d;
                    newrasterMatrix4d.InitProduct(UorRefToUorActive,rasterMatrix);
                    DPoint2d extent(GetExtent(editElmHandle));
                    extent.x *= uorScaleBetweenModel;
                    extent.y *= uorScaleBetweenModel;
                    SetTransform(editElmHandle, newrasterMatrix4d,&extent,false);
                    }
                }
            }
        }
    if (sourceDgnModel != destinationDgnModel)
        pRasterFrameHandler->SetGCSInheritedFromModelState (editElmHandle, !isSourceModelGeoReprojected,false);

    gcsServices->FreeGCS (&rasterGCS);
    gcsServices->FreeGCS (&sourceDgnModelGCS);
    gcsServices->FreeGCS (&destDgnModelGCS);

#endif
    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            RasterFrameHandler::_GetTypeName (WStringR descr, UInt32 desiredLength)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_RASTER_FRAME_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RasterFrameHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    // N.B. If cell is supported, we must also change {dgnfileio\lib\history\utils.cpp} isNeverComplexComponent()
    if (SupportOperation::CellGroup == stype)
        return false;

    return T_Super::_IsSupportedOperation (eh, stype);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            RasterFrameHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    TransformInfo   tInfo (flattenTrans);

    // Flags it has 2D first so we correctly process transform in RasterFrameHandler->ApplyTransform
    eeh.GetElementP()->SetIs3d(false);

    if (eeh.GetElementCP()->IsViewIndependent()) // View independent
        {
        //Transform raster frame element object
        Transform frameT;
        RasterTransformFacility::SetUV(frameT,GetU(eeh),GetV(eeh));
        RasterTransformFacility::SetTranslation(frameT,GetOrigin(eeh));

        // backup raster frame transform matrix and origin
        DPoint3d        origin(GetOrigin(eeh));

        // Create a new transform matrix that only contain translation
        Transform   newRasterFrameTransform;

        newRasterFrameTransform.InitIdentity ();
        newRasterFrameTransform.SetTranslation (origin);

        // Set it in our raster frame element
        SetU(eeh,RasterTransformFacility::GetU(newRasterFrameTransform),false);
        SetV(eeh,RasterTransformFacility::GetV(newRasterFrameTransform),false);

        // Call normal case transform -> only origin will be change...
        eeh.GetHandler().ApplyTransform (eeh, tInfo);

        //DPoint3d    newOrigin(GetOrigin(eeh));

        // Restore old transform matrix
        SetU(eeh,RasterTransformFacility::GetU(frameT),false);
        SetV(eeh,RasterTransformFacility::GetV(frameT),false);
        }
    else
        {
        // Use standard case
        eeh.GetHandler().ApplyTransform (eeh, tInfo);
        }

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MarcBedard  5/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::_IsTransformGraphics
(
ElementHandle const &   element,
TransformInfoCR transform
)
    {
    // When a raster is view independent, we must return false so we can perform the special transformation.
    if(GetViewIndependentState(element))
        return false;

    // There is no problem transforming a 3d frame with any transform
    if (Is3dElem(element.GetElementCP()))
        return true;

    // If the frame is 2d, then the input transform must be 2d.
    // The only means of this test is to avoid the transform to put the raster out of the visible frustum.
    if (DOUBLE_EQUAL_EPSILON(transform.GetTransform()->form3d[0][2], 0.0) &&
        DOUBLE_EQUAL_EPSILON(transform.GetTransform()->form3d[1][2], 0.0) &&
        DOUBLE_EQUAL_EPSILON(transform.GetTransform()->form3d[2][0], 0.0) && 
        DOUBLE_EQUAL_EPSILON(transform.GetTransform()->form3d[2][1], 0.0) && 
        DOUBLE_EQUAL_EPSILON(transform.GetTransform()->form3d[2][2], 1.0) && 
        DOUBLE_EQUAL_EPSILON(transform.GetTransform()->form3d[2][3], 0.0))
        return true;


    // frame is 2d, transform is 3D.
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MarcBedard  5/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength)
    {
    if(BSISUCCESS == T_HOST.GetRasterAttachmentAdmin()._GetDescription(el, descr, desiredLength))
        return;   

    // Default to what is stored in the element.
    WString sourceUrl(GetSourceUrl(el));

    // Description : "Raster Attachment [CompactPath]"
    GetTypeName (descr, desiredLength);  // Get type name part.

    WString rasterEmbeddedName(BeFileName::GetFileNameWithoutExtension(sourceUrl.c_str()));


    descr += L" [";
    descr += rasterEmbeddedName;
    descr += L"]";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_GetTransformOrigin (ElementHandleCR elHandle, DPoint3dR origin)
    {
    GetRangeCenter (elHandle, origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_GetSnapOrigin (ElementHandleCR elHandle, DPoint3dR origin)
    {
    GetRangeCenter (elHandle, origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_GetOrientation (ElementHandleCR elHandle, RotMatrixR orientation)
    {
    orientation = RotMatrix::From2Vectors(GetU(elHandle),GetV(elHandle));
    orientation.squareAndNormalizeColumns (&orientation, 0, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::_IsPlanar (ElementHandleCR thisElm, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal)
    {
    if (point)
        GetTransformOrigin (thisElm, *point);
    if (normal)
        {
        *normal = DVec3d::FromNormalizedCrossProduct(GetU(thisElm), GetV(thisElm));
        }

    //A raster frame is always planar!
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  06/2007
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus RasterFrameHandler::_OnGeoCoordinateReprojection (EditElementHandleR source, IGeoCoordinateReprojectionHelper& reprojectionHelper, bool inChain)
    {
#ifdef WIP_V10_GEOCOORD
    DgnModelP    sourceDgnModel = source.GetDgnModel();
    DgnAttachmentCP sourceDgnAttCP = sourceDgnModel->AsDgnAttachmentCP();

    RefAttachMethod attachMethod = ATTACHMETHOD_Unknown;
    if (sourceDgnAttCP!= NULL)
        attachMethod = sourceDgnAttCP->GetAttachMethod();
#endif

    // We will validate range here if required or on open if required
    //TRICKY: force element descriptor allocation because of ValidateElementRange optimization
    //        that will not recalculate range for persistent element not changed
    source.GetElementDescrCP();
    // Update range. NOTE: will be rewritten by caller
    ValidateElementRange (source);

    return REPROJECT_DontValidateRange; //This means SUCCESS but caller will not call validate range.
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Marc.Bedard  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameElement::IsKindOf (DgnElementCP el)
    {
    return  NULL != el &&  RASTER_FRAME_ELM == el->GetLegacyType();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFrameElementIterator::RasterFrameElementIterator ()
:m_modelRefP(NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFrameElementIterator::RasterFrameElementIterator (PersistentElementRefList* l,DgnModelP modelRefP)
:m_modelRefP(modelRefP)
    {
    SetElmList(l);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameElementIterator::IsValid() const
    {
    return m_it.GetCurrentElementRef() != NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandleCR RasterFrameElementIterator::operator*() const
    {
    return m_eh;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameElementIterator::operator==(RasterFrameElementIterator const& rhs) const
    {
    return m_eh.GetElementRef() == rhs.m_eh.GetElementRef();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameElementIterator::operator!=(RasterFrameElementIterator const& rhs) const
    {
    return !(*this == rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFrameElementIterator& RasterFrameElementIterator::operator++() 
    {
    ++m_it;
    ToNext(); 
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     		Marc.Bedard      08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameElementIterator::ToNext ()
    {
    for (ElementRefP ref = m_it.GetCurrentElementRef(); NULL != ref; ref = m_it.GetNextElementRef())
        {
        if (RasterFrameElement::IsKindOf (ref->GetUnstableMSElementCP()))
            {
            m_eh = ElementHandle (ref);
            return;
            }
        }
    m_eh = ElementHandle ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameElementIterator::SetElmList (PersistentElementRefList* l) 
    {
    m_eh = ElementHandle();
    if (NULL != l)
        {    
        m_it = l->begin();
        ToNext();
        }
    else
        {
        m_it.SetCurrentElementRef (NULL);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void s_RgbaFromColorIndex(RgbaColorDef& rgba, ViewContextP viewContextP, UInt32 colorIdex)
{
    // Continue to support wacky behavior of passing -1 to return background color...
    if (-1 == colorIdex || DgnColorMap::INDEX_Background == colorIdex)
        {
        // NOTE: Color map background color doesn't account for viewController/modelRef override...
        IntColorDef viewBackColor(viewContextP->GetViewport()->GetBackgroundColor());
        rgba.red = viewBackColor.m_rgb.red;
        rgba.green = viewBackColor.m_rgb.green;
        rgba.blue = viewBackColor.m_rgb.blue;
        rgba.alpha = 255;
        }
    else
        {
        RgbColorDef tempRgb;
        RasterFrameHandler::RgbFromColorIndexInModel(tempRgb, viewContextP->GetDgnProject (), colorIdex);
        rgba.red    = tempRgb.red;
        rgba.green  = tempRgb.green;
        rgba.blue   = tempRgb.blue;
        rgba.alpha  = 255;
        }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand 09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::DrawBitmap(ElementHandleCR rasterEh, ViewContextR context)
    {
    MultiResolutionRasterPtr rasterP = RasterAppData::QueryMultiResolutionRaster(rasterEh.GetElementRef());
 
    if(rasterP.IsNull())
        {
        DgnRasterFileId rasterId = RasterIdFromDgnDbUrl(GetSourceUrl(rasterEh).c_str());

        DgnModelP modelP = rasterEh.GetDgnModelP();

        RasterTileProviderPtr providerPtr;
        if(rasterId.IsValid())
            providerPtr = DgnDbTileProvider::CreateDgnDbTileProvider(modelP->GetDgnProject(), rasterId);

        if(providerPtr.IsValid())
            {
            MultiResolutionRaster::ComposeOptions opts;
            opts.invert = GetInvertState(rasterEh);

            // By default background black and foreground white.
            opts.backgroundColor.red = opts.backgroundColor.green = opts.backgroundColor.blue = 0;
            opts.backgroundColor.alpha = 255;
            opts.foregroundColor.red = opts.foregroundColor.green = opts.foregroundColor.blue = opts.foregroundColor.alpha = 255;
            
            s_RgbaFromColorIndex(opts.backgroundColor, &context, GetBackgroundColor(rasterEh));
            s_RgbaFromColorIndex(opts.foregroundColor, &context, GetForegroundColor(rasterEh));

            // image fg == image bg, invert image fg.
            if (opts.backgroundColor.red == opts.foregroundColor.red && 
                opts.backgroundColor.green == opts.foregroundColor.green && 
                opts.backgroundColor.blue == opts.foregroundColor.blue &&
                opts.backgroundColor.alpha == opts.foregroundColor.alpha) 
                {
                opts.foregroundColor = IntColorDef(context.GetViewport()->GetContrastToBackgroundColor()).m_rgba;
                opts.foregroundColor.alpha = 255;
                }

            if(GetTransparencyState(rasterEh))
                {
                opts.backgroundColor.alpha  = 255 - GetBackgroundTransparencyLevel(rasterEh);
                opts.foregroundColor.alpha  = 255 - GetForegroundTransparencyLevel(rasterEh);
                }

            //&&MM if in the end only one color is visible and it is equal to background we should use GetContrastToBackgroundColor
            //&&MM we should be testing if background color has change since last draw. We probably could skip all these when no binary. 
            
            rasterP = MultiResolutionRaster::CreateRaster(*providerPtr, opts);
            RasterAppData::StoreMultiResolutionRaster(rasterP.get(), rasterEh.GetElementRef());
            }        
        }

    if(rasterP.IsValid())
        {       
        DVec3d uVec = GetU(rasterEh);
        DVec3d vVec = GetV(rasterEh);
        DVec3d zVec = DVec3d::FromNormalizedCrossProduct(uVec, vVec);

        // Adjust magnitude to the length of one pixel.
        uVec.scaleToLength(uVec.Magnitude() / rasterP->GetWidth());
        vVec.scaleToLength(vVec.Magnitude() / rasterP->GetHeight());

        Transform effectiveLocalToPixel = Transform::FromOriginAndVectors(GetOrigin(rasterEh), uVec, vVec, zVec);

        context.PushTransform(effectiveLocalToPixel);
        rasterP->Draw(context);
        context.PopTransformClip();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand 09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void  RasterFrameHandler::DrawBorder(ElementHandleCR rasterEh, ViewContextR context)
    {
#ifdef WIP_V10_GEOCOORD
    // if (RasterDgnGCSFacility::IsReprojected (rasterEh))
    //     return BSIERROR;
#endif

    DPoint3d  origin = GetOrigin(rasterEh); //In UOR
    DVec3d    uVec = GetU(rasterEh);
    DVec3d    vVec = GetV(rasterEh);

    // Compute the four corners
    DPoint3d shape[5];
    shape[0] = shape[4] = origin;
    shape[1].sumOf(&origin, &uVec);
    shape[2].sumOf(&shape[1], &vVec);
    shape[3].sumOf(&origin, &vVec);

    ViewContext::ContextMark mark (&context);

    bool needDrawFill = NeedDrawFill(context);

    if (DisplayHandler::Is3dElem (rasterEh.GetElementCP()))
        {
        context.GetIDrawGeom().DrawShape3d (5, shape, needDrawFill, NULL);
        }
    else
        {
        DPoint2d shape2d[5];
        DataConvert::Points3dTo2d(shape2d, shape, 5);
        context.GetIDrawGeom().DrawShape2d(5, shape2d, needDrawFill, context.GetDisplayPriority(), NULL);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RasterFrameHandler::DrawRange(ElementHandleCR rasterEh, ViewContextR context)
    {
    // Draw a shape that represent raster range.  
    // Clipping status can be modified by an override so the range must represent the raster corners without clipping. 

#ifdef WIP_V10_GEOCOORD
   // if (RasterDgnGCSFacility::IsReprojected (rasterEh))
   //     return BSIERROR;
#endif

    DPoint3d  origin = GetOrigin(rasterEh); //In UOR
    DVec3d    uVec = GetU(rasterEh);
    DVec3d    vVec = GetV(rasterEh);

    // Compute the four corners
    DPoint3d shape[5];
    shape[0] = shape[4] = origin;
    shape[1].sumOf(&origin, &uVec);
    shape[2].sumOf(&shape[1], &vVec);
    shape[3].sumOf(&origin, &vVec);

    ViewContext::ContextMark    mark (&context);

    if(GetViewIndependentState(rasterEh))
        context.PushViewIndependentOrigin(&origin);

    if (DisplayHandler::Is3dElem (rasterEh.GetElementCP()))
        {
        context.GetIDrawGeom().DrawShape3d (5, shape, false/*fill*/, NULL);
        }
    else
        {
        DPoint2d shape2d[5];
        DataConvert::Points3dTo2d(shape2d, shape, 5);
        context.GetIDrawGeom().DrawShape2d(5, shape2d, false/*fill*/, context.GetDisplayPriority(), NULL);
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_Draw (ElementHandleCR eh, ViewContextR context)
    {
    //We don't support pre V10 version in this handler!
    if(V10_RASTER_FRAME_FIRSTVERSION > GetVersion(eh))
        return;

    // Always draw in range purpose no matter if the raster is currently visible or not.
    if (DrawPurpose::RangeCalculation == context.GetDrawPurpose())
        {
        if(BSISUCCESS != T_HOST.GetRasterAttachmentAdmin()._OutputForRangeCalculation(eh, context))
            DrawRange(eh, context);

        return;
        }

    if(BSISUCCESS == T_HOST.GetRasterAttachmentAdmin()._OutputRaster(eh, context))
        return; // Handled

    if (context.GetViewport() == NULL || context.GetViewport()->GetIViewOutput() == NULL)
        return;

    if(!ShouldDrawInContext(eh, context))
        return;

    ViewContext::ContextMark mark (&context);

    if (GetViewIndependentState(eh))
        {
        DPoint3d origin = GetOrigin(eh);
        context.PushViewIndependentOrigin(&origin);
        }

    // It is not illegal to set the depth for a 2D view. It's just going to be ignored, but it will ensure 
    // that 2D refs to 3D master will display as expected. (Refs are inserted in the rasters bg/fg planes).
    double depth = 0.0;
    bool pushClipPlanes = false;
    if (context.GetViewport() != NULL && context.Is3dView())
        {
        depth = CalcDisplayDepth(GetDisplayOrder(eh));
        if (depth != 0.0)
            pushClipPlanes = true;
        }

    SetAndRestoreProjectionDepth  __setAndRestoreProjectionDepth (context.GetViewport(), depth);

    // Push bg/fg view clip planes. TR 191461, 275667, 292711, 293575, 294869
    PushPopBackFrontClipPlanes    __pushPopBackFrontClipPlanes (pushClipPlanes ? &context : NULL);

    if(ShouldDrawBitmap(context))
        DrawBitmap(eh, context);

    if(ShouldDrawBorder(eh, context))
        DrawBorder(eh, context);
    }

#if 0    // Remove in Graphite
/*---------------------------------------------------------------------------------**//**
 We have the same range, of z-buffer values, available before and after any bvector z-buffer value.
 The z-buffer is a signed integer, the same range of values is available in positive and in negative.

  displayPriorityMaxTotalValue  --> maximum z-buffer value for bvector elements (bvector plane).
 -displayPriorityMaxTotalValue  --> minimum z-buffer value for vetor elements (bvector plane)
  maxDisplayPriority            --> Maximum valid z-buffer value in QVision
 -maxDisplayPriority            --> Minimum valid z-buffer value in QVision
  base                          --> Minimum z-buffer value of the plane we are currently computing.
  rangeAvailable                --> Range of values available for each of the back and front plane.
                                    = maxDisplayPriority - displayPriorityMaxTotalValue;

* @bsimethod                                       MarcBedard and YvesBoivin 10/2004
+---------------+---------------+---------------+---------------+---------------+------*/
Int32 RasterFrameHandler::CalcDisplayPriority (ElementHandleCR eh, ElemDisplayParamsCP elParams)
    {
    // This occurs during creation of a new rasterFrame element. Using the activeModel is bad since it may not be initialize.
    if(eh.GetDgnModel() == NULL)
        {
#ifdef WIP_VANCOUVER_MERGE // raster
        return T_Super::_CalcDisplayPriority(eh, elParams);
#endif
        return 0;
        }

    Int32 displayOrder(GetDisplayOrder(eh));
      
    // If designPlane use default implementation.
    if(IsDesignPlane(displayOrder))
        {
#ifdef WIP_VANCOUVER_MERGE // raster
        return T_Super::_CalcDisplayPriority(eh, elParams);
#endif
        return 0;
        }
      
    Int32 maxDisplayPriority = Viewport::GetMaxDisplayPriority();
    Int32 displayPriorityMaxTotalValue = Viewport::GetDisplayPriorityFrontPlane();
    Int32 base = IsFrontPlane(displayOrder) ? displayPriorityMaxTotalValue : (maxDisplayPriority-1) * -1;

    Int32 rangeAvailable = maxDisplayPriority - displayPriorityMaxTotalValue;

    // Make sure we limit the rasterDisplayOrder to the model range otherwise it can overflow in others models.
    Int32 displayPriority = base + min(abs(displayOrder), rangeAvailable-1);

    LIMIT_RANGE(-maxDisplayPriority,maxDisplayPriority,displayPriority);

    return displayPriority;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
double RasterFrameHandler::CalcDisplayDepth(long displayOrder)
    {
    if (IsDesignPlane(displayOrder))
        return 0.0;

    static const double rasterStep = 1.0/250/*maxRasterLayer*/;      //we assume we will have a maximum of 250 raster

    //Valid values are ]-1.0,1.0[ 

    if (IsBackPlane(displayOrder))
        {
        displayOrder = abs(displayOrder); // remove sign.

        // back plane starts a 1.
        double depth = -1.0 + (displayOrder * rasterStep);

        //In case we have more than maxRasterLayer, limit depth to be smaller than 0.0
        LIMIT_RANGE(-1.0+(0.5*rasterStep), -0.5*rasterStep, depth);

        return depth;
        }
    else 
        {
        BeAssert(IsFrontPlane(displayOrder));

        // front plane starts a 1.
        double depth = displayOrder * rasterStep;

        //In case we have more than maxRasterLayer, limit depth to be greater than 0.0
        LIMIT_RANGE(0.5*rasterStep, 1.0-(0.5*rasterStep), depth);

        return depth;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand 09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool        RasterFrameHandler::NeedDrawFill(ViewContextR context) const
    {
    switch (context.GetDrawPurpose())
        {
        case DrawPurpose::Pick:
        case DrawPurpose::CaptureGeometry:
        case DrawPurpose::RangeCalculation:
        case DrawPurpose::FenceAccept:
            return !DgnPlatformLib::GetHost().GetRasterAttachmentAdmin()._IsIgnoreInterior();

        case DrawPurpose::ChangedPre:       // erase
        case DrawPurpose::RestoredPre:      
            return true;

        default:
            break;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::RasterPlane RasterFrameHandler::DisplayOrderToRasterPlane(int displayOrder)
    {
    if(IsFrontPlane(displayOrder))
        return ViewContext::RasterPlane_Foreground;

    if(IsDesignPlane(displayOrder))
        return ViewContext::RasterPlane_Design;

    return ViewContext::RasterPlane_Background;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterFrameHandler::ShouldDrawInContext (ElementHandleCR rasterEh, ViewContextR context)
    {
    BeAssert(DrawPurpose::RangeCalculation != context.GetDrawPurpose());  // Range should be handle by RasterAttachmentAdmin::_OutputForRangeCalculation

#ifdef WIP_V10_GEOCOORD
    // Never draw raster marked as reprojected if reprojection is not available because range is probably invalid.
    if (NULL==GeoCoordinationManager::GetServices() && !raster.GetGCSInheritedFromModelState())
        return false;
#endif

    // Always draw in erase mode. do we really need to test for IsErase()? Try without it.
    //if(IsErase() || DrawPurpose::ChangedPre == context.GetDrawPurpose() || DrawPurpose::RestoredPre == context.GetDrawPurpose(
    //    return true;

    // Make sure raster is visible in view or plot.
    if (context.GetViewport() != NULL && !GetViewState(rasterEh, context.GetViewport()->GetViewNumber()) ||
        (DrawPurpose::Plot == context.GetDrawPurpose() && !GetPrintState(rasterEh)))
        return false;

    // Never draw if config(rastmgr pref) display state is OFF.
    if (!DgnPlatformLib::GetHost().GetRasterAttachmentAdmin()._IsDisplayEnable())
        return false;

#ifdef WIP_V10_GEOCOORD
    // Make sure raster is not drawn by the draping. 
    if (GetDrapeState(rasterEh) && s_IsRenderModeDrapable(viewContextP))
        return false;
#endif

    UInt32 displayPlane = DisplayOrderToRasterPlane(GetDisplayOrder(rasterEh));

    // Draw only on display plane from context. 
    if (!(displayPlane & context.GetRasterPlane())) 
        return false;

    // Finally we are done....
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool    RasterFrameHandler::ShouldDrawBitmap(ViewContextR context)
    {
    switch (context.GetDrawPurpose())
        {
        case DrawPurpose::Hilite:
        case DrawPurpose::Unhilite:
        case DrawPurpose::ChangedPre:       // Erase, rely on Healing.
        case DrawPurpose::RestoredPre:      // Erase, rely on Healing.
        case DrawPurpose::RangeCalculation:
        case DrawPurpose::Pick:
        case DrawPurpose::Flash:
        case DrawPurpose::CaptureGeometry:
        case DrawPurpose::FenceAccept:
        case DrawPurpose::RegionFlood:
        case DrawPurpose::FitView:
        case DrawPurpose::ExportVisibleEdges:
        case DrawPurpose::ModelFacet:
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool    RasterFrameHandler::ShouldDrawBorder(ElementHandleCR rasterEh, ViewContextR context)
    {
    if(context.GetDrawPurpose() == DrawPurpose::ExportVisibleEdges || 
       context.GetDrawPurpose() == DrawPurpose::ModelFacet)
        return false;

    // Preference wins over element state.
    if(GetDisplayBorderState(rasterEh))
        return true;

    // Draw boundary when selected/hilited...
    if(HILITED_None != context.GetCurrHiliteState())
        return true;      

    return !ShouldDrawBitmap(context); // Draw border if we are not drawing the bitmap.
    }

#if defined ELEMENT_LOADING_REWORK
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MarcBedard  5/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_OnAdded (ElementHandleP element)
    {
    T_HOST.GetRasterAttachmentAdmin()._OnAdded(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MarcBedard  5/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_OnDeleted (ElementHandleP element)
    {
    T_HOST.GetRasterAttachmentAdmin()._OnDeleted(element);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MarcBedard  5/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_OnUndoRedo
(
ElementHandleP          afterUndoRedo,         // the current state (after undo or redo has happened)
ElementHandleP          beforeUndoRedo,        // the state immediately previous to undo/redo
ChangeTrackAction     action,                // the action that happened
bool                    isUndo                // if true -> this is an undo, if false -> this is a redo
)
    {
    T_HOST.GetRasterAttachmentAdmin()._OnUndoRedo(afterUndoRedo,beforeUndoRedo,action,isUndo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MarcBedard  10/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_OnModified
(
ElementHandleP          newElement,          // the current state of the element
ElementHandleP          oldElement,          // the previous state of the element
ChangeTrackAction     action               // the action that happened
)
    {
    T_HOST.GetRasterAttachmentAdmin()._OnModified(newElement,oldElement,action);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MathieuMarchand  08/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_OnXAttributeChanged (XAttributeHandleCR xAttr, ChangeTrackAction action)
    {
    T_HOST.GetRasterAttachmentAdmin()._OnXAttributeChanged(xAttr, action, cantBeUndoneFlag);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MarcBedard  3/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_OnUndoRedoXAttributeChange
(
XAttributeHandleCR      xAttr,
ChangeTrackAction     action,
bool                    isUndo
)
    {
    T_HOST.GetRasterAttachmentAdmin()._OnUndoRedoXAttributeChange(xAttr,action,isUndo);
   }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MarcBedard  11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RasterFrameHandler::_OnFenceClip
    (
    ElementAgendaP      inside,
    ElementAgendaP      outside,
    ElementHandleCR     elmHandle,
    FenceParamsP        fenceParam,
    FenceClipFlags      flag
    )
    {
    return T_HOST.GetRasterAttachmentAdmin()._OnFenceClip(inside, outside, elmHandle, fenceParam, flag);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString RasterFrameHandler::ComposeDgnDbUrl(WCharCP filename, DgnRasterFileId rasterId)
    {
    // syntax: dgndb://{DgnRasterFileId}/filename
    WPrintfString dgnDbURL(L"dgndb://{%lld}/", rasterId.GetValue());

    dgnDbURL += BeFileName::GetFileNameAndExtension(filename);
   
    return dgnDbURL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRasterFileId RasterFrameHandler::RasterIdFromDgnDbUrl(WCharCP url)
    {
    static WCharCP DGNDB_START_TAG = L"dgndb://{";
    static WCharCP DGNDB_END_TAG = L"}/";

    // syntax: dgndb://{DgnRasterFileId}/filename
    WString urlStr(url);

    WString::size_type startDelim = urlStr.find(DGNDB_START_TAG);
    if(WString::npos == startDelim || startDelim != 0)  // Should be start of string.
        return DgnRasterFileId();       // not a dgndb url

    startDelim = wcslen(DGNDB_START_TAG);

    WString::size_type endDelim = urlStr.find(DGNDB_END_TAG);
    if(WString::npos == endDelim || endDelim <= startDelim)
        return DgnRasterFileId();

    WString idStr = urlStr.substr(startDelim, endDelim-startDelim);

    UInt64 id = -1;
    if(0 == BE_STRING_UTILITIES_SWSCANF(idStr.c_str(), L"%lld", &id))
        return DgnRasterFileId();

    return DgnRasterFileId(id);
    }

/*=================================================================================**//**
* @bsiclass                                     Marc.Bedard                     05/2011
+===============+===============+===============+===============+===============+======*/
class RasterAttachmentFixRange : public DerivedElementRange
    {
    virtual void        _OnRemoveDerivedRange (PersistentElementRef const& host, bool deletedCache, HeapZone& zone) override
        {
        if (!deletedCache)
            zone.Free (this, sizeof (*this));
        }

    virtual void        _UpdateDerivedRange (PersistentElementRef& host) override
        {
        //TRICKY: use DerivedElementRange helper class to remove old element from range tree and add new one
        //        even if this element does not really have a derived element range.
        // Thus this methods do nothing: see RasterAttachmentFixRange::Set

        //  Note: do NOT update the range of my components. The derived range on each component will get its own callback
        }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
    static void RemoveFromRangeTree (PersistentElementRef& host)    {OnDerivedRangeChangePre (host);}
    static void AddToRangeTree (PersistentElementRef& host)         {OnDerivedRangeChangePost(host);}
    };


#if defined ELEMENT_LOADING_REWORK
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterFrameHandler::_OnElementLoaded (ElementHandleCR eh) 
    {
    T_HOST.GetRasterAttachmentAdmin()._OnElementLoaded(eh);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Transform RasterTransformFacility::FromMatrixParameters(DPoint3d const& translation, double scalingX, double scalingY, double rotation, double anorthogonality)
    {
    DVec3d u;
    u.init(scalingX, 0.0, 0.0);
    DVec3d v;
    v.init(0.0, scalingY, 0.0);

    RotMatrix rotationMatrix;
    rotationMatrix.initFromAxisAndRotationAngle(2/*Z*/, rotation);
    RotMatrix affinityMatrix;
    affinityMatrix.initFromAxisAndRotationAngle(2/*Z*/, anorthogonality);

    // Rotate UV
    rotationMatrix.multiply(&u, &u);
    rotationMatrix.multiply(&v, &v);
    affinityMatrix.multiply(&v, &v);

    Transform trn;
    SetUV(trn, u, v);
    SetTranslation(trn, translation);

    return trn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void  RasterTransformFacility::RemoveRotationAndAffinity (TransformR trn)
    {
    // Get u, v
    DVec3d u(GetU(trn));
    DVec3d v(GetV(trn));

    double xSize = u.magnitude();
    double ySize = v.magnitude();

    // Set columns
    trn.form3d[0][0] = xSize; trn.form3d[1][0] = 0.0;   trn.form3d[2][0] = 0.0;
    trn.form3d[0][1] = 0.0;   trn.form3d[1][1] = ySize; trn.form3d[2][1] = 0.0;
    trn.form3d[0][2] = 0.0;   trn.form3d[1][2] = 0.0  ; trn.form3d[2][2] = 1.0;
    trn.form3d[2][2] = 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d RasterTransformFacility::GetU(TransformCR trn)
    {
    DVec3d column;
    trn.GetMatrixColumn(column, 0);

    return column;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d RasterTransformFacility::GetV(TransformCR trn)
    {
    DVec3d column;
    trn.GetMatrixColumn(column, 1);
    
    return column;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d RasterTransformFacility::GetTranslation(TransformCR trn)
    {
    DVec3d trans;
    trn.GetTranslation(trans);
    
    return trans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d RasterTransformFacility::GetNormal(TransformCR trn)
    {
    DVec3d normal; normal.NormalizedCrossProduct(GetU(trn), GetV(trn));
    BeAssert (normal.magnitude() > 0.0);

    return normal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
double RasterTransformFacility::GetScalingX(TransformCR trn)
    {
    return GetU(trn).Magnitude();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
double RasterTransformFacility::GetScalingY(TransformCR trn)
    {
    return GetV(trn).Magnitude();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
double RasterTransformFacility::GetRotationXY(TransformCR trn)
    {
    DVec3d u(GetU(trn));
    DVec3d u2d(DVec3d::From(u.x, u.y, 0.0));

    if (u2d.Magnitude() <= 1E-13)
        return 0.0;

    DVec3d xAxis(DVec3d::From(1.0, 0.0, 0.0));
    double angle(ComputeAngle(xAxis, u2d));

    if (!LegacyMath::Vec::AreParallel(&u2d, &xAxis) && u2d.y < 0.0)
        angle = msGeomConst_2pi - angle;

    return angle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
double RasterTransformFacility::GetAffinity(TransformCR trn)
    {
    DVec3d u(GetU(trn));
    DVec3d v(GetV(trn));
    double angle(ComputeAngle(u, v));

    DVec3d normal; normal.CrossProduct(u, v);
    DVec3d zAxis; zAxis.Init(0.0, 0.0, 1.0);
    if (!LegacyMath::Vec::ArePerpendicular(&normal, &zAxis) && normal.z < 0.0)
        angle = msGeomConst_2pi - angle;    // Reverse angle

    angle -= msGeomConst_piOver2;

    // convert to a positive angle
    if (angle < 0.0)
        angle = angle + msGeomConst_2pi;

    return angle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterTransformFacility::Has3dRotation(TransformCR trn)
    {
    DVec3d zAxis;
    zAxis.init(0.0, 0.0, 1.0);

    DVec3d normal(GetNormal(trn));

    // Check if the normal is parallel to the axis
    if (LegacyMath::Vec::AreParallel(&normal, &zAxis))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool RasterTransformFacility::IsValidRasterTransform(TransformCR trn)
    {
    DVec3d u(GetU(trn));
    DVec3d v(GetV(trn));

    if (LegacyMath::Vec::AreParallel(&u, &v))
        return false;

    return (u.magnitude() > 0.0 && v.magnitude() > 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterTransformFacility::SetUV(TransformR trn, DVec3dCR u, DVec3dCR v)
    {
    BeAssert (u.Magnitude() > 0.0);
    BeAssert (v.Magnitude() > 0.0);

    // Set U
    trn.form3d[0][0] = u.x;
    trn.form3d[1][0] = u.y;
    trn.form3d[2][0] = u.z;

    // Set V
    trn.form3d[0][1] = v.x;
    trn.form3d[1][1] = v.y;
    trn.form3d[2][1] = v.z;

    // Set 3rd column as the normalized cross product of U and V
    DVec3d n; n.NormalizedCrossProduct(u, v);
    BeAssert (n.Magnitude() > 0.0);
    trn.form3d[0][2] = n.x;
    trn.form3d[1][2] = n.y;
    trn.form3d[2][2] = n.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterTransformFacility::SetAffinity(TransformR trn, double affinity)
    {
    DVec3d u (GetU(trn));
    DVec3d v (GetV(trn));
    DVec3d normal (GetNormal(trn));
    DVec3d zAxis(DVec3d::From(0.0, 0.0, 1.0));

    if (!LegacyMath::Vec::ArePerpendicular(&normal, &zAxis) && normal.z < 0.0)
        normal.Scale(normal, -1.0); // Reverse Normal vector

    double uMag = u.magnitude();
    double vMag = v.magnitude();
    BeAssert(uMag > 0.0);

    double scale = vMag / uMag;

    DVec3d vNew(u);
    vNew.Scale(vNew, scale);

    RotMatrix rMatrix;
    rMatrix.InitFromVectorAndRotationAngle(normal, -(affinity + msGeomConst_piOver2));
    rMatrix.MultiplyTranspose(vNew);

    // Set V
    trn.form3d[0][1] = vNew.x;
    trn.form3d[1][1] = vNew.y;
    trn.form3d[2][1] = vNew.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterTransformFacility::SetScalingX(TransformR trn, double scale)
    {
    DVec3d u (GetU(trn));
    u.ScaleToLength(scale);

    trn.form3d[0][0] = u.x;
    trn.form3d[1][0] = u.y;
    trn.form3d[2][0] = u.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterTransformFacility::SetScalingY(TransformR trn, double scale)
    {
    DVec3d v (GetV(trn));
    v.ScaleToLength(scale);

    trn.form3d[0][1] = v.x;
    trn.form3d[1][1] = v.y;
    trn.form3d[2][1] = v.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterTransformFacility::SetTranslation(TransformR trn, DPoint3dCR translation)
    {
    trn.form3d[0][3] = translation.x;   
    trn.form3d[1][3] = translation.y; 
    trn.form3d[2][3] = translation.z;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  7/2002
+---------------+---------------+---------------+---------------+---------------+------*/
double RasterTransformFacility::ComputeAngle (DVec3dCR u, DVec3dCR v)
    {
    double uMag (u.magnitude());
    double vMag (v.magnitude());

    if (uMag <= 0.0 || vMag <= 0.0)
        return 0.0;

    // compute the angle between u and v
    double angleCosine = u.DotProduct(v) / (uMag * vMag);
    double angle = acos(angleCosine);   // angle range is [0.0, PI]

    return angle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterTransformFacility::ComputeBitmapSize (UInt64& bitmapWidth, UInt64& bitmapHeight, TransformCR pixelToUORs, DPoint2d extentInUORs)
    {
    // Compute scale
    double scaleX = RasterTransformFacility::GetScalingX(pixelToUORs);
    double scaleY = RasterTransformFacility::GetScalingY(pixelToUORs);
    BeAssert (scaleX > 0.0);
    BeAssert (scaleY > 0.0);

    // Compute sixze
    bitmapWidth     = (UInt64)ceil(extentInUORs.x / scaleX);
    bitmapHeight    = (UInt64)ceil(extentInUORs.y / scaleY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d  RasterTransformFacility::ComputeExtent (TransformCR pixelToUORs, UInt64 bitmapWidth, UInt64 bitmapHeight)
    {
    // Compute scale
    double scaleX = RasterTransformFacility::GetScalingX(pixelToUORs);
    double scaleY = RasterTransformFacility::GetScalingY(pixelToUORs);
    BeAssert (scaleX > 0.0);
    BeAssert (scaleY > 0.0);

    // Compute extent
    DPoint2d  extent = {scaleX * bitmapWidth, scaleY * bitmapHeight};

    return extent;
    }


