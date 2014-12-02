/*----------------------------------------------------------------------+
|
|     $Source: DgnHandlers/OleCellHeaderHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)    // WIP_NONPORT
    #include <Windows.h>
    #include <objbase.h>
    #include <comdef.h>  /* used for CComPtr and CComQIPtr types such as IStoragePtr */
#elif defined (__unix__) || defined (BENTLEY_WINRT)
    // WIP_WINRT
    // NEEDS MUCH WORK
    #include <Bentley/Bentley.h>
    typedef void*   HDC;
    typedef void*   HBITMAP;
    typedef struct tagRGBQUAD { 
      UInt8 rgbBlue;
      UInt8 rgbGreen;
      UInt8 rgbRed;
      UInt8 rgbReserved;
    } RGBQUAD;
    typedef struct tagBITMAPINFOHEADER { 
      ULong32 biSize; 
      Long32  biWidth; 
      Long32  biHeight; 
      UInt16  biPlanes; 
      UInt16  biBitCount;
      ULong32 biCompression; 
      ULong32 biSizeImage; 
      Long32  biXPelsPerMeter; 
      Long32  biYPelsPerMeter; 
      ULong32 biClrUsed; 
      ULong32 biClrImportant; 
    } BITMAPINFOHEADER; 
    typedef struct tagBITMAPINFO { 
      BITMAPINFOHEADER bmiHeader; 
      RGBQUAD bmiColors[1]; 
    } BITMAPINFO; 
    #define BI_RGB        0L
    typedef void* IViewObject2Ptr;
#endif
#include "DgnPlatformInternal.h"
#include <DgnPlatform/DgnHandlers/IDgnOleDraw.h>

USING_NAMESPACE_BENTLEY_EC

#ifdef WIP_CFGVAR
DEFINE_CFGVAR_CHECKER(MS_OLE_TRANSPARENT_BACKGROUND)
#endif

enum
    {
    MAX_TILE_PIXELS     = 1024,             // maximum dimension of a single tile for offscreen bitmap
    LINKED_OBJ_STYLE    = 3,
    EMBEDDED_OBJ_STYLE  = 0,
    BOUNDARY_LINEWIDTH  = 2,
    PURE_WHITE          = 0x00ffffff,
    QV_BGRA_FORMAT      = 1,
    QV_RGB_FORMAT       = 2,
    QV_BGR_FORMAT       = 3,
    };

void limitLt(int& a,int b) {if (a<b) a=b;}
void limitGt(int& a,int b) {if (a>b) a=b;}

void limitLt(Long32& a,Long32 b) {if (a<b) a=b;}
void limitGt(Long32& a,Long32 b) {if (a>b) a=b;}

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   12/03
+===============+===============+===============+===============+===============+======*/
struct          OleObjRect : public RECTL
{
    void From (DRange2d const& in)
        {
        left   = (int) in.low.x;
        top    = (int) in.low.y;
        bottom = (int) in.high.y;
        right  = (int) in.high.x;
        }
    void Init (int l, int t, int r, int b)
        {
        left   = l;
        top    = t;
        bottom = b;
        right  = r;
        }
    void MoveBy (int x, int y)
        {
        right  -= x;
        left   -= x;
        bottom -= y;
        top    -= y;
        }
    void LimitTo (OleObjRect const& limit)
        {
        limitLt (left,   limit.left);
        limitLt (top,    limit.top);
        limitGt (right,  limit.right);
        limitGt (bottom, limit.bottom);
        }

    bool IsNull() const {return (right<=left) || (top>=bottom);}
    int  Width()  const {return right-left;}
    int  Height() const {return bottom-top;}
};

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   12/03
+===============+===============+===============+===============+===============+======*/
class           DgnOleBitmap : IDgnOleDraw
{
protected:
    ViewContextR    m_context;
    ViewportP       m_vp;
    ElementHandleCP m_elIter;
    DPoint3d        m_shapePts[5];
    Transform       m_texelToView;
    OleObjRect      m_objRect;
    OleObjRect      m_drawRect;
    bool            m_isTransparentBg;
    bool            m_isModel3d;
    //bool            m_isReference; removed in graphite
    DgnOleInfo      m_info;
    int             m_tileWidth;
    int             m_tileHeight;
    HDC             m_dc;
    HBITMAP         m_bitmap;
    byte*           m_pixels;
    IViewObject2Ptr m_iView;
    double          m_plotRasterScale;

public:
    DgnOleBitmap (ElementHandleCP elIter, ViewContextR context, DgnOleInfo const& info);
    ~DgnOleBitmap();

    StatusInt       Init();
    bool            ExtractShapePts();

    void            Draw();
    void            DrawBitmap();
    void            DrawBoundary();
    bool            ShouldDrawBoundary();
    bool            ShouldDrawBitmap();
    void            Erase();
    void            ShadeObj (OleObjRect const& bounds);

    void            ClearBackground();
    void            SetAlphaForBgTransparency();
    bool            AllocateBitmap();
    void            DrawTile (IViewObject2Ptr iView, OleObjRect const& thisTile);
    void            RenderToBitmap ();
    void            SetPlotRasterScale (double plotRasterScale) { m_plotRasterScale = plotRasterScale; }

    bool                IsTransparentBackground () const {return m_isTransparentBg;}
    RECTL const&        GetObjRect()  const         {return m_objRect;}
    RECTL const&        GetDrawRect() const         {return m_drawRect;}
    DPoint3dCP          GetShapePts() const         {return m_shapePts;}
    Transform const&    GetTexelToViewTransform() const {return m_texelToView;}
    DgnOleInfo const&   GetDgnOleInfo() const       {return m_info;}
};

#if defined (BENTLEY_WIN32)    // WIP_NONPORT

/*=================================================================================**//**
* DgnStoreLockBytes
* @bsiclass                                                     KeithBentley    09/01
+===============+===============+===============+===============+===============+======*/
struct DgnStoreLockBytes : public ILockBytes
    {
private:
    void*       m_mem;
    UInt32      m_size;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStoreLockBytes
(
void*   mem,
UInt32  size
)
    {
    m_mem  = mem;
    m_size = size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ~DgnStoreLockBytes ()
    {
    if (m_mem)
        DgnStoreHdrHandler::FreeExtractedData (m_mem);

    m_mem = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
STDMETHODIMP    QueryInterface
(
REFIID  riid,
void**  ppv
)
    {
    if (IID_IUnknown == riid || IID_ILockBytes == riid)
        *ppv = this;
    else
        *ppv = NULL;

    if (*ppv)
        {
        ((IUnknown*) *ppv)->AddRef();
        return NOERROR;
        }

    return E_NOINTERFACE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
STDMETHODIMP_ (HRESULT) ReadAt
(
ULARGE_INTEGER  offset,         // => Specifies the offset in the byte array at which to begin reading
void*           output,         // => Points to the buffer into which the data should be read.
ULONG           readSize,       // => Specifies the number of bytes to read
ULONG*          pRead           // <= Specifies the number of bytes actually read. The caller can set pcbRead to NULL, indicating this value is not of interest. The pcbRead parameter is zero if there is an error.
)
    {
    if ((readSize + offset.LowPart) > m_size)
        readSize = m_size - offset.LowPart;

    memcpy (output, ((byte*) m_mem) + offset.LowPart, readSize);

    if (pRead)
        *pRead = readSize;

    return NOERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
STDMETHODIMP_ (HRESULT) WriteAt
(
ULARGE_INTEGER  offset,         /* => Specifies the offset in the byte array at which to begin writing the data */
void const*     input,          /* => Points to the buffer containing the data to be written. */
ULONG           writeSize,      /* => Specifies the number of bytes to write. */
ULONG          *pWritten        /* <= Specifies (after the call) the number of bytes actually read. The caller can set pcbWritten to NULL, indicating this value is not of interest. The pcbWritten parameter is zero if there is an error. */
)
    {
    if ((writeSize + offset.LowPart) > m_size)
        writeSize = m_size - offset.LowPart;

    memcpy ((byte*) m_mem + offset.LowPart, input, writeSize);

    if (pWritten)
        *pWritten = writeSize;

    return NOERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT STDMETHODCALLTYPE Stat
(
STATSTG*    pstatstg,               // const OLE predefined these without const
DWORD       grfStatFlag             // const
)
    {
    memset (pstatstg, 0, sizeof *pstatstg);
    pstatstg->type           = STGTY_LOCKBYTES;
    pstatstg->cbSize.LowPart = m_size;
    pstatstg->clsid          = CLSID_NULL;

    return S_OK;
    }

STDMETHODIMP_(ULONG)        AddRef()  {return  1;}
STDMETHODIMP_(ULONG)        Release() {return  1;}
HRESULT STDMETHODCALLTYPE   Flush (void) {return NOERROR;}
HRESULT STDMETHODCALLTYPE   LockRegion (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) {return NOERROR;}
HRESULT STDMETHODCALLTYPE   UnlockRegion( ULARGE_INTEGER libOffset,ULARGE_INTEGER cb,DWORD dwLockType) {return NOERROR;}
HRESULT STDMETHODCALLTYPE   SetSize (ULARGE_INTEGER cb) {return ResultFromScode(E_NOTIMPL);}
};

#endif //defined (BENTLEY_WIN32)    // WIP_NONPORT

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     08/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     isTransparentBackground (const DgnOleInfo& oleInfo, ViewportCP vp)
    {
    // If MS_OLE_TRANSPARENT_BACKGROUND is defined, use legacy behavior where OLE object backgrounds are always transparent on a white view background.
    // This is no longer the default behavior, due to TR 252093 and many other printing complaints.
#ifdef WIP_CFGVAR
    return ((oleInfo.m_flags.transparent) || (CHECK_CFGVAR_IS_DEFINED(MS_OLE_TRANSPARENT_BACKGROUND) && (vp->GetBackgroundColor() == PURE_WHITE)));
#else
    return (oleInfo.m_flags.transparent);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   12/03
+---------------+---------------+---------------+---------------+---------------+------*/
DgnOleBitmap::DgnOleBitmap (ElementHandleCP elIter, ViewContextR context, DgnOleInfo const& info) : m_context (context)
    {
    m_vp          = context.GetViewport();
    m_elIter      = elIter;
    m_info        = info;

    DgnModelP modelRef = elIter->GetDgnModelP();
    m_isModel3d = modelRef->Is3d();
//    m_isReference = modelRef->IsDgnAttachment();  removed in graphite

    m_plotRasterScale = 1.0;

    m_dc          = NULL;
    m_bitmap      = NULL;
    m_pixels      = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   12/03
+---------------+---------------+---------------+---------------+---------------+------*/
DgnOleBitmap::~DgnOleBitmap()
    {
#if defined (BENTLEY_WIN32)    // WIP_NONPORT

    if (NULL != m_dc)
        DeleteDC (m_dc);

    if (NULL != m_bitmap)
        DeleteObject (m_bitmap);

    m_dc     = NULL;
    m_bitmap = NULL;
    m_pixels = NULL;

#elif defined (BENTLEY_WINRT)
    // WIP_WINRT
#endif
    }

/*---------------------------------------------------------------------------------**//**
* if this object is currently "active" overlay the graphics with a hatch pattern to indicate to the user that it is active.
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnOleBitmap::ShadeObj (OleObjRect const& bounds)
    {
#if defined (BENTLEY_WIN32)    // WIP_NONPORT

    if (!OleCellHeaderHandler::IsObjActive (m_elIter->GetElementRef()))
        return;

    WORD        wHatchBmp[8] = {0x11, 0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88};
    HBITMAP     hbm    = CreateBitmap (8, 8, 1, 1, wHatchBmp);
    HBRUSH      hbr    = CreatePatternBrush (hbm);
    HBRUSH      hbrOld = (HBRUSH) SelectObject (m_dc, hbr);
    COLORREF    cvText = SetTextColor(m_dc, RGB(255, 255, 255));
    COLORREF    cvBk   = SetBkColor(m_dc, RGB(0, 0, 0));

    PatBlt (m_dc, bounds.left, bounds.top, bounds.Width(), bounds.Height(),  0x00A000C9L /* DPa */ );

    SetTextColor (m_dc, cvText);
    SetBkColor   (m_dc, cvBk);

    SelectObject (m_dc, hbrOld);
    DeleteObject (hbr);
    DeleteObject (hbm);

#elif defined (BENTLEY_WINRT)
    // WIP_WINRT
#endif
    }

/*---------------------------------------------------------------------------------**//**
* set all pixels to white (the default background for OLE). Set all alpha bytes to 0xff.
* @bsimethod                                                    Keith.Bentley   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnOleBitmap::ClearBackground ()
    {
    UInt32*    pixel = (UInt32*) m_pixels;
    for (UInt32* end = pixel + (m_tileHeight*m_tileWidth); pixel<end; pixel++)
        *pixel = 0xffffffff;
    }

/*---------------------------------------------------------------------------------**//**
* If we're attempting to use transparent background, we want all of the background pixels to have their alpha channel set to 0
* (fully transparent) and all other pixels to have their alpha channel to 0xff (fully opaque.)
* Unfortunately, the way OLE works with a 32bpp bitmap is to set the alpha to 0 for every pixel to which it draws (which is good in that
* we can tell which pixels were affected by the OLE drawing, but bad in that it's the opposite value from what QV expects.)
* <p>Since we set the alpha to 0xff in the ClearBackground step, we know that any pixel with an alpha value of 0 was drawn
* from OLE. We therefore set the alpha to 0 (fully transparent) if it is now 0xff, and to 0xff (fully opaque) if it is now 0.
* @bsimethod                                                    Keith.Bentley   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnOleBitmap::SetAlphaForBgTransparency ()
    {
    RgbaColorDef* pixel = (RgbaColorDef*) m_pixels;
    RgbaColorDef* end = pixel + (m_tileHeight*m_tileWidth);

    if (!m_isTransparentBg)
        {
        // we don't want the background to be transparent, set all alpha values to fully opaque
        for (; pixel<end; pixel++)
            pixel->alpha = 0x0ff;
        }
    else
        {
        for (; pixel<end; pixel++)
            pixel->alpha = (0 == pixel->alpha) ? 0xff : 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* allocate a 32bpp bitmap and DC of size (m_tileWidth x m_tileHeight)
* @return true if successfully allocated
* @bsimethod                                                    Keith.Bentley   12/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnOleBitmap::AllocateBitmap ()
    {
#if defined (BENTLEY_WIN32)    // WIP_NONPORT

    IViewOutputP output = m_vp->GetIViewOutput();
    if (NULL == output)
        return false;

    HDC screenDC = (HDC)output->GetScreenDC();

    BITMAPINFO bmInfo;
    bmInfo.bmiHeader.biSize          = sizeof (bmInfo.bmiHeader);
    bmInfo.bmiHeader.biWidth         = m_tileWidth;
    bmInfo.bmiHeader.biHeight        = m_tileHeight;
    bmInfo.bmiHeader.biPlanes        = 1;
    bmInfo.bmiHeader.biBitCount      = 32;
    bmInfo.bmiHeader.biCompression   = BI_RGB;
    bmInfo.bmiHeader.biSizeImage     = m_tileWidth * m_tileHeight * 4;
    bmInfo.bmiHeader.biXPelsPerMeter = 1024;
    bmInfo.bmiHeader.biYPelsPerMeter = 1024;
    bmInfo.bmiHeader.biClrUsed       = 0;
    bmInfo.bmiHeader.biClrImportant  = 0;

    if (NULL == (m_dc = CreateCompatibleDC (screenDC)))
        return false;

    if (NULL == (m_bitmap = CreateDIBSection (screenDC, &bmInfo, DIB_PAL_COLORS, (void**) &m_pixels, ___, 0)))
        return false;

    SelectObject (m_dc, m_bitmap);
    return  true;

#elif defined (BENTLEY_WINRT)
    // WIP_WINRT
    return false;
#elif defined (__unix__)
    /*WIP_NONPORT*/
    return false;

#endif
    }

/*---------------------------------------------------------------------------------**//**
* verify and initialize this DgnOleBitmap, calculating the ranges for object and visible rects.
* @bsimethod                                                    Keith.Bentley   12/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnOleBitmap::Init ()
    {
    if (!m_vp || !m_context.GetIViewDraw().IsOutputQuickVision ())
        return ERROR;

    m_isTransparentBg = isTransparentBackground (m_info, m_vp);

    DPoint3d    shapePtsView[5];
    m_context.LocalToView (shapePtsView, m_shapePts, 5);
    
    Frustum viewBox = m_vp->GetFrustum(DgnCoordSystem::View, true);

    // the Z axis in view coordinates is much larger than the X/Y axes (65k vs. usually around 1k). This screws up
    // texel size calculations for any OLE shape that are not perpendicular to the view. Scale the shape so
    // that the Z extent is the same as the X extent.
    DVec3d    viewExtent;
    viewExtent.differenceOf(&viewBox.GetCorner(NPC_111), &viewBox.GetCorner(NPC_000));

    Transform scaleZ;
    scaleZ.scaleMatrixColumns (NULL, 1.0, 1.0, viewExtent.x / viewExtent.z);
    scaleZ.multiply (shapePtsView, shapePtsView, 5);
    viewBox.Multiply(scaleZ);

    // get the transform from texel coordinates to view coordinates
    DVec3d  xVec, yVec, zVec;
    xVec.normalizedDifference (shapePtsView+1, shapePtsView);
    yVec.normalizedDifference (shapePtsView,   shapePtsView+3);
    zVec.normalizedCrossProduct (&xVec, &yVec);
    m_texelToView.initFromOriginAndVectors (shapePtsView+3, &xVec, &yVec, &zVec);

    Transform   viewToTexel;
    viewToTexel.inverseOf (&m_texelToView);
    viewBox.Multiply (viewToTexel);             // view bounding box in texel coordinates
    viewToTexel.multiply (shapePtsView, shapePtsView, 5);   // shape in texel coordinates.

    Transform   scaleZinv;
    scaleZinv.inverseOf (&scaleZ);                          // back out z scaling for texel-to-view trans
    m_texelToView.productOf (&scaleZinv, &m_texelToView);

    DRange2d objRange;
    bsiDRange2d_initFromArray3d (&objRange, shapePtsView, 5);    // object range in texel coords

    // find the min/max texel values for the view box (this defines an "enclosing rectangle" for the visible region of the object)
    DRange2d viewRange; 
    bsiDRange2d_initFromArray3d (&viewRange, viewBox.GetPts(), 8);

    // and get the visible range of the object in texel coordinates.
    DRange2d drawRange;
    if (!bsiDRange2d_intersect (&drawRange, &viewRange, &objRange))
        return  ERROR;

    if (1.0 != m_plotRasterScale)
        {
        // This adjustment is here to support printer drivers such as HP RTL.
        // The output coordinates for the rasterized OLE object must be in bvector resolution (1016 dpi),
        // but we want to create the image at raster resolution (typically 300 dpi).
        objRange.low.x   *= m_plotRasterScale;
        objRange.low.y   *= m_plotRasterScale;
        objRange.high.x  *= m_plotRasterScale;
        objRange.high.y  *= m_plotRasterScale;

        drawRange.low.x  *= m_plotRasterScale;
        drawRange.low.y  *= m_plotRasterScale;
        drawRange.high.x *= m_plotRasterScale;
        drawRange.high.y *= m_plotRasterScale;
        }

    m_objRect.From (objRange);
    m_drawRect.From (drawRange);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* draw a single tile of the OLE object into bitmap and then copy to IDrawGeom output.
* @bsimethod                                                    Keith.Bentley   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnOleBitmap::DrawTile (IViewObject2Ptr iView, OleObjRect const& thisTile)
    {
#if defined (BENTLEY_WIN32)    // WIP_NONPORT

    ClearBackground ();     // start with white background

    // the "bounds" is the rectangle for the entire OLE object such that the tile is a 0,0 for upper left.
    OleObjRect bounds = m_objRect;
    bounds.MoveBy (thisTile.left, thisTile.top);

    // draw object into bitmap
    HRESULT hr = iView->Draw (m_info.m_oleAspect, -1, ___, ___, ___, m_dc, &bounds, ___, ___, ___);
    BeAssert (hr == S_OK);

    ShadeObj (bounds);          // if it's active, shade it
    GdiFlush ();                // before we can use the bitmap, we have to make sure GDI isn't buffering any output
    SetAlphaForBgTransparency (); // set the alpha channel for each pixel

    DPoint3d uvPts[4];
    memset (uvPts, 0, sizeof(uvPts));
    uvPts[0].x = uvPts[2].x = thisTile.left;        // calculate the world coordinates for this tile.
    uvPts[0].y = uvPts[1].y = thisTile.bottom;
    uvPts[1].x = uvPts[3].x = thisTile.right;
    uvPts[2].y = uvPts[3].y = thisTile.top;

    if (1.0 != m_plotRasterScale)
        {
        // This adjustment is here to support printer drivers such as HP RTL.
        // The output coordinates for the rasterized OLE object must be in bvector resolution (1016 dpi),
        // but we want to create the image at raster resolution (typically 300 dpi).
        uvPts[0].x /= m_plotRasterScale;
        uvPts[0].y /= m_plotRasterScale;
        uvPts[1].x /= m_plotRasterScale;
        uvPts[1].y /= m_plotRasterScale;
        uvPts[2].x /= m_plotRasterScale;
        uvPts[2].y /= m_plotRasterScale;
        uvPts[3].x /= m_plotRasterScale;
        uvPts[3].y /= m_plotRasterScale;
        }

    m_texelToView.multiply (uvPts, uvPts, 4);         // convert from texel to view
    m_context.ViewToLocal (uvPts, uvPts, 4);     // then to local.

    byte const* startRow = m_pixels;
    if (thisTile.Height() < m_tileHeight)
        startRow += ((m_tileHeight - thisTile.Height()) * m_tileWidth * 4);

    // we have to pass true for "enableAlpha" so that ole can be made transparent when it is in a reference with transparency
    bool enableAlpha = m_isTransparentBg;

    if (m_isModel3d)
        {
        m_context.GetIViewDraw().DrawRaster (uvPts, m_tileWidth*4, thisTile.Width(), thisTile.Height(), enableAlpha, QV_BGRA_FORMAT, startRow, NULL);
        }
    else
        {
        DPoint2d uvPts2d[4];
        uvPts2d[0].x = uvPts[0].x;
        uvPts2d[0].y = uvPts[0].y;
        uvPts2d[1].x = uvPts[1].x;
        uvPts2d[1].y = uvPts[1].y;
        uvPts2d[2].x = uvPts[2].x;
        uvPts2d[2].y = uvPts[2].y;
        uvPts2d[3].x = uvPts[3].x;
        uvPts2d[3].y = uvPts[3].y;

        m_context.GetIViewDraw().DrawRaster2d (uvPts2d, m_tileWidth*4, thisTile.Width(), thisTile.Height(), enableAlpha, QV_BGRA_FORMAT, startRow, m_context.GetDisplayPriority(), NULL);
        }

#elif defined (BENTLEY_WINRT)
    // WIP_WINRT
#endif
    }

/*---------------------------------------------------------------------------------**//**
* render this OLE IViewObject2 object to the IDrawGeom. This method allocates an offscreen bitmap of a maximum size and then
* renders the object (using m_drawRect and m_objRect) to the bitmap, in tiles as necessary, and then copies the result to the IDrawGeom
* in local coordinates.
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnOleBitmap::RenderToBitmap ()
    {
#if defined (BENTLEY_WIN32)    // WIP_NONPORT

    m_tileWidth  = m_drawRect.Width();
    m_tileHeight = m_drawRect.Height();

    limitGt (m_tileWidth,  MAX_TILE_PIXELS);
    limitGt (m_tileHeight, MAX_TILE_PIXELS);

    if (!AllocateBitmap ())
        return;

    // MicroStation leaves the polygon fill mode other than the default. Reset it to the default value to get metafiles to draw properly.
    SetPolyFillMode (m_dc, ALTERNATE);

    // create a clip region around the tile.
    HRGN clipRegion = CreateRectRgn (0, 0, m_tileWidth, m_tileHeight);
    SelectClipRgn (m_dc, clipRegion);

    OleObjRect thisTile;
    for (int row=m_drawRect.bottom; row>m_drawRect.top; row -= m_tileHeight)
        {
        for (int col=m_drawRect.left; col<m_drawRect.right; col += m_tileWidth)
            {
            thisTile.Init (col, row-m_tileHeight, col+m_tileWidth, row);
            thisTile.LimitTo (m_drawRect);
            DrawTile (m_iView, thisTile);
            }
        }

    SelectClipRgn (m_dc, NULL); // remove the clip region from the DC
    DeleteObject (clipRegion);  // and delete it.

#elif defined (BENTLEY_WINRT)
    // WIP_WINRT
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @return true if the OLE bitmap should be rendered in this context.
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnOleBitmap::ShouldDrawBitmap ()
    {
    switch (m_context.GetDrawPurpose ())
        {
        case DrawPurpose::Hilite:
        case DrawPurpose::Unhilite:
        case DrawPurpose::Flash:
        case DrawPurpose::RangeCalculation:
        case DrawPurpose::FenceAccept:
        case DrawPurpose::FitView:
            return false; // Only need to draw boundary for these modes...
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* load the OLE object from the DgnStore and render it into the IDrawGeom output device.
* @bsimethod                                                    Keith.Bentley   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnOleBitmap::DrawBitmap ()
    {
    if (!ShouldDrawBitmap() || m_drawRect.IsNull())
        return;

    // read the raw object from the dgnstore within this cell into a buffer
    void*       buf;
    UInt32      bufSize;

    if (SUCCESS != DgnStoreHdrHandler::ExtractFromCell (&buf, &bufSize, DGNSTOREID_OLE, DGNSTOREAPPID_OLE_STORAGE, *m_elIter))
        return;

#if defined (BENTLEY_WIN32)    // WIP_NONPORT

    // create an ILockBytes that points to (and assumes ownership of) the buffer
    DgnStoreLockBytes   lockBytes (buf, bufSize);
    IStoragePtr         tmpStorage;

    // Open an IStorage on the LockBytes
    if (!SUCCEEDED (StgOpenStorageOnILockBytes (&lockBytes, ___, STD_ACCESS, ___, ___, &tmpStorage)))
        return;

    // Just create a data cache rather than calling OleLoad. OleLoad actually tries to load a handler for the object, etc.
    // If the object is linked, it also attempts to connect to the link (which we don't care about just to draw it)
    // and if the link is broken over a network, there can be a long timeout. All we need is the data cache to draw
    // the cached presentation anyway.

    IPersistStoragePtr iPersist;
    if (!SUCCEEDED (CreateDataCache (___, CLSID_NULL, IID_IPersistStorage, (void**) &iPersist)))
        return;

    // load the data cache from the storage
    if (!iPersist || !SUCCEEDED (iPersist->Load (tmpStorage)))
        return;

    // get an IViewObject2 pointer, and then use that to render
    m_iView = iPersist;
    if (m_iView)
        {
        m_context.GetIViewDraw().DrawDgnOle (this);
        m_iView = (IViewObject2*) 0;
        }

#elif defined (BENTLEY_WINRT)
    // WIP_WINRT
#elif defined (__unix__)
    /*WIP_NONPORT*/
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @return true if the boundary around the OLE object should be drawn in this context.
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnOleBitmap::ShouldDrawBoundary ()
    {
    if (DrawPurpose::Plot == m_context.GetDrawPurpose ())
        return false; // Never draw boundary when plotting...

    if(m_context.GetCurrHiliteState())
        return true; // Draw boundary when selected/hilited...

    return !DgnOleBitmap::ShouldDrawBitmap (); // Draw when not drawing bitmap...
    }

/*---------------------------------------------------------------------------------**//**
* draw a box around the OLE object
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnOleBitmap::DrawBoundary ()
    {
    if (!ShouldDrawBoundary())
        return;

    OvrMatSymbP   matSymb = m_context.GetOverrideMatSymb ();
    if (matSymb)
        {
        m_context.SetIndexedLineWidth   (*matSymb, BOUNDARY_LINEWIDTH);
        m_context.SetIndexedLinePattern (*matSymb, (DGNOLE_STORAGE_Linked == m_info.m_storageType) ? LINKED_OBJ_STYLE : EMBEDDED_OBJ_STYLE);
        m_context.ActivateOverrideMatSymb ();
        }

    m_context.GetIViewDraw().DrawLineString3d (5, m_shapePts, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* erase the OLE object by drawing a filled shape of the boundary
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnOleBitmap::Erase ()
    {
    m_context.GetIViewDraw().DrawShape3d (5, m_shapePts, true, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* extract the 5 shape points from the shape element that is the first child of the cell.
* @bsimethod                                                    Keith.Bentley   12/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnOleBitmap::ExtractShapePts ()
    {
    // the first element in the element descriptor must point to a shape with 5 points. That determines where and how large
    // our object should be drawn.
    ChildElemIter    shapeIter (*m_elIter, ExposeChildrenReason::Count);
    if (!shapeIter.IsValid())
        return  false;

    DgnElementCP shapeEl = shapeIter.GetElementCP();
    if ((shapeEl->GetLegacyType() != SHAPE_ELM) || (5 != LineStringUtil::GetCount(*shapeEl)))
        return  false;

    // make sure the shape is displayed in this view
    if (m_context.FilterRangeIntersection (*m_elIter))
        return false;

    // extract the points from the shape.
    LineStringUtil::Extract (m_shapePts, NULL, *shapeEl, *shapeIter.GetDgnModelP ());

    if (!shapeEl->Is3d())
        {
        double priority = m_context.GetDisplayPriority ();
        for (int i=0; i<5; i++)
            m_shapePts[i].z = priority;
        }

    switch (m_info.m_flags.viewRotationMode)
        {
        case DGNOLE_ViewRotation_ViewIndependent:  // back out view orientation.
            DPoint3d origin;
            CellUtil::ExtractOrigin (origin, *m_elIter);

            Transform trans;
            m_context.GetViewIndTransform (&trans, &origin);
            trans.multiply (m_shapePts, m_shapePts, 5);
            break;

        case DGNOLE_ViewRotation_AutoCAD:       // mimic Autocad's way of setting range.
            DPoint3d shapePts[5];
            m_context.LocalToView (shapePts, m_shapePts, 5);

            DRange3d range;
            range.initFrom (shapePts, 5);
            range.low.z = range.high.z;

            if (DrawPurpose::Plot != m_context.GetDrawPurpose())
                {
                double tmp = range.low.y;       // y's are swapped in view coordinates
                range.low.y = range.high.y;
                range.high.y = tmp;
                }

            shapePts[0] = shapePts[1] = shapePts[4] = range.low;
            shapePts[2] = shapePts[3] = range.high;
            shapePts[1].x = range.high.x;
            shapePts[3].x = range.low.x;
            m_context.ViewToLocal (m_shapePts, shapePts, 5);
            break;
        }

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* draw the OLE object into the DrawContext
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnOleBitmap::Draw ()
    {
    // make sure the OLE linkage says to draw
    if ((m_info.m_version < DGNOLE_VERSION) || (m_info.m_flags.doNotDisplayOleObj))
        return;

    if (!ExtractShapePts ())
        return;

    if (!m_context.GetIViewDraw().IsOutputQuickVision ())
        {
        m_context.GetIViewDraw().DrawShape3d (5, m_shapePts, true, NULL); // just draw a filled box to pick

        return;
        }

    if (SUCCESS != Init())
        return;

    DrawBitmap();
    DrawBoundary();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool OleCellHeaderHandler::_ClaimElement (ElementHandleCR el)
    {
    DgnOleInfo  info;

    return IsOleElement (el, &info) && (info.m_version >= DGNOLE_VERSION);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::_Draw (ElementHandleCR el, ViewContextR context)
    {
    DgnOleInfo  info;

    if (!IsOleElement (el, &info))
        {
        BeAssert (false);
        return;
        }

    // make sure the OLE linkage says to draw
    if (info.m_flags.doNotDisplayOleObj)
        return;

    DgnOleBitmap  oleBitmap (&el, context, info);

    oleBitmap.Draw ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrew.Edge                     02/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::PlotRasterized
(
ElementHandleCR el,
ViewContextR    context,
double          plotRasterScale
)
    {
    DgnOleInfo  info;

    if (!IsOleElement (el, &info))
        {
        BeAssert (false);
        return;
        }

    // make sure the OLE linkage says to draw
    if (info.m_flags.doNotDisplayOleObj)
        return;

    DgnOleBitmap oleBitmap (&el, context, info);
    oleBitmap.SetPlotRasterScale (plotRasterScale);
    oleBitmap.Draw ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrew.Edge                     01/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::PlotNonRasterized
(
ElementHandleCR elHandle,
ViewContextR    context,
void*           hdcIn,
const BSIRect&  olePlotRange
)
    {
#if defined (BENTLEY_WIN32)    // WIP_NONPORT

    HDC         hdc = (HDC) hdcIn;
    DgnOleInfo  info;

    if (!IsOleElement (elHandle, &info))
        {
        BeAssert (false);
        return;
        }

    // Read the raw object from the dgnstore within this cell into a buffer.
    void*  buf;
    UInt32 bufSize;

    if (SUCCESS != DgnStoreHdrHandler::ExtractFromCell (&buf, &bufSize, DGNSTOREID_OLE, DGNSTOREAPPID_OLE_STORAGE, elHandle))
        return;

    // Create an ILockBytes that points to (and assumes ownership of) the buffer.
    DgnStoreLockBytes   lockBytes (buf, bufSize);
    IStoragePtr         tmpStorage;

    // Open an IStorage on the LockBytes.
    if (!SUCCEEDED (StgOpenStorageOnILockBytes (&lockBytes, ___, STD_ACCESS, ___, ___, &tmpStorage)))
        return;

    // Just create a data cache rather than calling OleLoad. OleLoad actually tries to load a handler for the object, etc.
    // If the object is linked, it also attempts to connect to the link (which we don't care about just to draw it)
    // and if the link is broken over a network, there can be a long timeout. All we need is the data cache to draw
    // the cached presentation anyway.
    IPersistStoragePtr iPersist;
    if (!SUCCEEDED (CreateDataCache (___, CLSID_NULL, IID_IPersistStorage, (void**) &iPersist)))
        return;

    // Load the data cache from the storage.
    if (!iPersist || !SUCCEEDED (iPersist->Load (tmpStorage)))
        return;

    // olePlotRange is expected to be in Windows coordinates (origin is top left corner).
    RECT olePlotRect;
    olePlotRect.left   = olePlotRange.origin.x;
    olePlotRect.top    = olePlotRange.origin.y;
    olePlotRect.bottom = olePlotRange.corner.y;
    olePlotRect.right  = olePlotRange.corner.x;

    InflateRect (&olePlotRect, -1, -1);

    SaveDC (hdc);

    bool isTransparentBg = isTransparentBackground (info, context.GetViewport());

    HBRUSH bgBrush = (isTransparentBg) ? NULL : (HBRUSH) GetStockObject (WHITE_BRUSH);
    if (bgBrush)
        FillRect (hdc, &olePlotRect, bgBrush);

    SetPolyFillMode (hdc, ALTERNATE);

    // Get an IViewObject2 pointer, and then use that to plot to the device context.
    IViewObject2Ptr iViewObject = iPersist;
    if (iViewObject)
        {
        iViewObject->Draw (info.m_oleAspect, -1, 0, 0, 0, hdc, (LPRECTL)&olePlotRect, 0, 0, 0);
        iViewObject = (IViewObject2*) 0;
        }

    RestoreDC (hdc, -1);

    if (bgBrush)
        DeleteObject (bgBrush);

#elif defined (BENTLEY_WINRT)
    // WIP_WINRT
#endif
    }

#if defined (BENTLEY_WIN32)    // WIP_NONPORT
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void     getLinkSource (ElementHandleCR el, WStringR descr)
    {
    void*       buf;
    UInt32      bufSize;

    if (SUCCESS != DgnStoreHdrHandler::ExtractFromCell (&buf, &bufSize, DGNSTOREID_OLE, DGNSTOREAPPID_OLE_STORAGE, el))
        return;

    // create an ILockBytes that points to (and assumes ownership of) the buffer
    DgnStoreLockBytes   lockBytes (buf, bufSize);
    IStoragePtr         tmpStorage;

    // Open an IStorage on the LockBytes
    if (!SUCCEEDED (StgOpenStorageOnILockBytes (&lockBytes, ___, STD_ACCESS, ___, ___, &tmpStorage)))
        return;

    // load the data cache from the storage
    IOleObjectPtr iOleObj;
    HRESULT result = OleLoad (tmpStorage, IID_IOleObject, ___, (void**) &iOleObj);
    if (!SUCCEEDED (result))
        return;

    IOleLinkPtr iLink = iOleObj;
    if (!iLink)
        return;

    WChar* tName;
    if (!SUCCEEDED  (iLink->GetSourceDisplayName (&tName)))
        return ;

    descr.append (tName);
    CoTaskMemFree(tName);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void     addCellName (DgnHandlersMessage::Number msgNum, ElementHandleCR el, WStringR descr)
    {
    descr.assign (DgnHandlersMessage::GetStringW(msgNum));

    WChar tmp[256];
    CellUtil::ExtractName (tmp, 128, el);
    descr.append (tmp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_OleObject));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::_GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength)
    {
    DgnOleInfo  info;

    if (!IsOleElement (el, &info))
        return;

    switch (info.m_storageType)
        {
        case DGNOLE_STORAGE_Linked:
            descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_OleTypeLinked));
#if defined (BENTLEY_WIN32)    // WIP_NONPORT
            getLinkSource (el, descr);
#elif defined (BENTLEY_WINRT)
    // WIP_WINRT
#endif
            break;

        case DGNOLE_STORAGE_Embedded:
            addCellName (DgnHandlersMessage::MSGID_OleTypeEmbedded, el, descr);
            break;

        case DGNOLE_STORAGE_Static:
            addCellName (DgnHandlersMessage::MSGID_OleTypePictureOf, el, descr);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::_GetPathDescription
(
ElementHandleCR    el,
WStringR        descr,
DisplayPathCP    path,
WCharCP       levelStr,
WCharCP       modelStr,
WCharCP       groupStr,
WCharCP       delimiter
)
    {
    _GetDescription (el, descr, 100);  // start with element's description

    if (NULL != levelStr && 0 != levelStr[0])
        descr.append(delimiter).append(levelStr);
    if (NULL != modelStr && 0 != modelStr[0])
        descr.append(delimiter).append(modelStr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
DgnOleInfo      OleCellHeaderHandler::GetInfo (ElementHandleCR el)
    {
    DgnOleInfo  info;

    IsOleElement (el, &info);

    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
DgnOleFlags     OleCellHeaderHandler::GetFlags (ElementHandleCR el)
    {
    DgnOleInfo  info;

    IsOleElement (el, &info);

    return info.m_flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void     getOleUnits (DgnModelP modelRef, UnitDefinitionR displayUnit, double& cmToDisplay, double& cmToStorage)
    {
    // get the master file units
    UnitDefinition  masterUnit  = modelRef->GetModelInfo().GetMasterUnit();

    // get standard unit definitions
    UnitDefinition  cmUnit   = UnitDefinition::GetStandardUnit(StandardUnit::MetricCentimeters);
    UnitDefinition  inchUnit = UnitDefinition::GetStandardUnit(StandardUnit::EnglishInches);

    // determine whether to display in Inches or Centimeters
    displayUnit = ( (masterUnit.GetSystem() != UnitSystem::English) && (masterUnit.GetSystem() != UnitSystem::USSurvey) ) ? cmUnit : inchUnit;

    // units on the clipboard are always .001 cm
    displayUnit.GetConversionFactorFrom (cmToDisplay, cmUnit);

    cmToDisplay /= 1000.;
    cmToStorage = .1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       OleCellHeaderHandler::GetSize (ElementHandleCR el, DPoint2d* size, DPoint3d* center, RotMatrixP rMatrix)
    {
    DgnOleInfo  info;

    if (!IsOleElement (el, &info))
        return ERROR;

    ChildElemIter    shapeIter (el, ExposeChildrenReason::Count);
    if (!shapeIter.IsValid())
        return ERROR;

    DgnElementCP shapeEl = shapeIter.GetElementCP();
    if ((shapeEl->GetLegacyType() != SHAPE_ELM) || (5 != LineStringUtil::GetCount(*shapeEl)))
        return ERROR;

    // extract the points from the shape.
    DPoint3d shapePts[5];
    LineStringUtil::Extract (shapePts, NULL, *shapeEl, *shapeIter.GetDgnModelP ());

    DVec3d xVec, yVec;
    xVec.differenceOf (&shapePts[1], &shapePts[0]);
    yVec.differenceOf (&shapePts[2], &shapePts[1]);

    if (center)
        center->interpolate (&shapePts[0], .5, &shapePts[2]);

    if (size)
        {
        size->x = xVec.normalize();
        size->y = yVec.normalize();
        }

    if (rMatrix)
        {
        rMatrix->initFrom2Vectors (&xVec, &yVec);
        rMatrix->squareAndNormalizeColumns (rMatrix, 0, 1);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       OleCellHeaderHandler::GetSourceSize (ElementHandleCR el, DPoint2d& sourceSize, WStringP label)
    {
    DgnOleInfo  info;

    if (!IsOleElement (el, &info))
        return ERROR;

    UnitDefinition units;
    double cmToDisplay, cmToStorage;
    getOleUnits (el.GetDgnModelP(), units, cmToDisplay, cmToStorage);

    sourceSize.x = info.m_defaultSize.x * cmToDisplay;
    sourceSize.y = info.m_defaultSize.y * cmToDisplay;

    if (label)
        label->assign (units.GetLabel());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       OleCellHeaderHandler::GetScale (ElementHandleCR el, DPoint2d& scale)
    {
    DgnOleInfo  info;

    if (!IsOleElement (el, &info))
        return ERROR;

    DgnModelP    modelRef = el.GetDgnModelP();

    DPoint2d size;
    GetSize (el, &size, NULL, NULL);

    UnitDefinition units;
    double cmToDisplay, cmToStorage;
    getOleUnits (modelRef, units, cmToDisplay, cmToStorage);

    scale.x = size.x  / (info.m_defaultSize.x * cmToStorage);
    scale.y = size.y  / (info.m_defaultSize.y * cmToStorage);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       OleCellHeaderHandler::SetScale (EditElementHandleR el, DPoint2d const& newScale)
    {
    DPoint2d    currScale;
    DPoint3d    center;
    RotMatrix   eMatrix;

    if (SUCCESS != GetScale (el, currScale) || SUCCESS != GetSize (el, NULL, &center, &eMatrix))
        return ERROR;

    RotMatrix scaleMtx;

    scaleMtx.scaleColumns (&eMatrix, newScale.x / currScale.x, newScale.y / currScale.y, 1.0);
    eMatrix.inverseOf (&eMatrix);
    scaleMtx.productOf (&scaleMtx, &eMatrix);

    Transform trans;
    trans.initFromMatrixAndFixedPoint (&scaleMtx, &center);

    return ApplyTransform (el, TransformInfo(trans));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnOleInfo* findInfoLinkage (DgnElementP el)
    {
    LinkageHeader* hdr  = (LinkageHeader*) elemUtil_extractLinkage (NULL, NULL, el, LINKAGEID_OLE);
    if (NULL == hdr)
        return  NULL;

    return  (DgnOleInfo*) (hdr+1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::SetTransparent (EditElementHandleR el, bool transparent)
    {
    DgnOleInfo* info = findInfoLinkage (el.GetElementP());
    if (info)
        info->m_flags.transparent = transparent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::SetActivateable (EditElementHandleR el, bool activateable)
    {
    DgnOleInfo* info = findInfoLinkage (el.GetElementP());
    if (info)
        info->m_flags.activateable = activateable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::SetSelectable (EditElementHandleR el, bool selectable)
    {
    DgnOleInfo* info = findInfoLinkage (el.GetElementP());
    if (info)
        info->m_flags.selectable = selectable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::SetLockedAspectRatio (EditElementHandleR el, bool lockedAspectRatio)
    {
    DgnOleInfo* info = findInfoLinkage (el.GetElementP());
    if (info)
        info->m_flags.lockedAspectRatio = lockedAspectRatio;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::SetRotationMode (EditElementHandleR el, DgnOleViewRotationMode rotationMode)
    {
    DgnOleInfo* info = findInfoLinkage (el.GetElementP());
    if (info)
        info->m_flags.viewRotationMode = rotationMode;

    DgnElementP  elem = el.GetElementP();
    if (DGNOLE_ViewRotation_ViewDependent == rotationMode)
        {
        elem->ToCell_2dR().SetSnappable(true);
        elem->ToCell_2dR().SetViewIndependent(false);
        }
    else
        {
        elem->ToCell_2dR().SetSnappable(false);
        elem->ToCell_2dR().SetViewIndependent(true);
        }

    NormalCellHeaderHandler::SetCellRange (el);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrew.Edge                     01/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            OleCellHeaderHandler::ExtractShapePts
(
ElementHandleCR el,
ViewContextR    context,
DPoint3d*       shapePts
)
    {
    DgnOleInfo  info;

    if (!IsOleElement (el, &info))
        {
        BeAssert (false);
        return false;
        }

    // Make sure the OLE linkage says to draw.
    if ((info.m_version < DGNOLE_VERSION) || (info.m_flags.doNotDisplayOleObj))
        return false;

    DgnOleBitmap oleBitmap (&el, context, info);

    if (! oleBitmap.ExtractShapePts())
        return false;

    DPoint3dCP bitmapShapePts = oleBitmap.GetShapePts();
    if (! bitmapShapePts)
        return false;

    if (shapePts)
        memcpy (shapePts, bitmapShapePts, 5 * sizeof (DPoint3d));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool            OleCellHeaderHandler::ExtractShapePts (ElementHandleCR eeh, DPoint3dP points)
    {
    MSElementDescrCP    header = eeh.GetElementDescrCP ();
    if (NULL == header)
        return  false;

    auto shape = header->Components().begin();
    if (shape == header->Components().end() || (SHAPE_ELM != (*shape)->Element().GetLegacyType()) || (5 != (*shape)->Element().ToLine_String_2d().numverts))
        return  false;

    return SUCCESS == LineStringUtil::Extract (points, NULL, (*shape)->Element(), *eeh.GetDgnModelP ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            OleCellHeaderHandler::_IsPlanar (ElementHandleCR eh, DVec3dP normalP, DPoint3dP pointP, DVec3dCP inputDefaultNormalP)
    {
    ChildElemIter   childIter (eh, ExposeChildrenReason::Count);

    // First child should be shape with 5 points that controls size/orientation...
    if (childIter.IsValid () && SHAPE_ELM == childIter.GetLegacyType())
        {
        DisplayHandlerP dHandler = childIter.GetDisplayHandler ();

        if (dHandler)
            return dHandler->IsPlanar (childIter, normalP, pointP, inputDefaultNormalP);
        }

    return T_Super::_IsPlanar (eh, normalP, pointP, inputDefaultNormalP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::_GetOrientation (ElementHandleCR eh, RotMatrixR rMatrix)
    {
    ChildElemIter   childIter (eh, ExposeChildrenReason::Count);

    // First child should be shape with 5 points that controls size/orientation...
    if (childIter.IsValid () && SHAPE_ELM == childIter.GetLegacyType())
        {
        DisplayHandlerP dHandler = childIter.GetDisplayHandler ();

        if (dHandler)
            {
            dHandler->GetOrientation (childIter, rMatrix);

            return;
            }
        }

    T_Super::_GetOrientation (eh, rMatrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       OleCellHeaderHandler::_OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR element, FenceParamsP fp, FenceClipFlags options)
    {
    // Never allow clipping...
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool            OleCellHeaderHandler::IsOleElement (ElementHandleCR eh, DgnOleInfo* dgnOleInfo)
    {
    DgnElementCR el = *eh.GetElementCP ();

    if (CELL_HEADER_ELM != el.GetLegacyType())
        return false;

    byte    buff [sizeof (DgnOleInfo) + sizeof (LinkageHeader) + 200];

    if (!mdlElement_attributePresent (&el, LINKAGEID_OLE, buff))
        return false;

    //  Test for minimum words here to avoid getting V7 OLE linkage elements.
    //  A V7 OLE linkage element only has the first 3 fields of DgnOleInfo.  
    //  However, at least one file has the m_version field set to 4 so without
    //  the size check it is treated as a valid OLE element, OleCellHeaderHandler 
    //  is assigned as the handler, and the OleCellHeaderHandler _Draw fails.
    LinkageHeader const* header = (LinkageHeader const*)buff;
    int         minimumWords = (int)(sizeof (LinkageHeader) + sizeof (DgnOleInfo))/2;
    if (LinkageUtil::GetWords (header) < minimumWords)
        return false;
        
    if (NULL != dgnOleInfo)
        memcpy (dgnOleInfo, (void*) (buff + sizeof (LinkageHeader)), sizeof (DgnOleInfo));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool            OleCellHeaderHandler::IsObjActive (ElementRefP elemRef)
    {
    return (0 != (elemRef->GetElFlags() & 0x01));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            OleCellHeaderHandler::FreeExtractedOleData (void* dataP)
    {
    DgnStoreHdrHandler::FreeExtractedData (dataP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   OleCellHeaderHandler::ExtractOleData (ElementHandleCR eh, void** dataPP, UInt32* dataSizeP)
    {
    return DgnStoreHdrHandler::ExtractFromCell (dataPP, dataSizeP, DGNSTOREID_OLE, DGNSTOREAPPID_OLE_STORAGE, eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   OleCellHeaderHandler::CreateOleElement
(
EditElementHandleR     eeh,
DgnOleInfo const*   infoIn,
WCharCP           name,
WCharCP           description,
DPoint3dCP          shape,
void const*         oleData,
size_t              oleSize,
bool                is3d,
DgnModelR        modelRef
)
    {
    if (!infoIn || !oleData || 0 >= oleSize)
        return ERROR;

    DgnOleInfo  info = *infoIn;

    info.m_version = DGNOLE_VERSION;

    LinkageHeader lh;

    memset (&lh, 0, sizeof (LinkageHeader));

    lh.primaryID = ELM_OLE_SIGNATURE;
    lh.user      = true;
    lh.modified  = true;
    lh.remote    = false;
    lh.info      = true;

    LinkageUtil::SetWords (&lh, (((sizeof (LinkageHeader) + sizeof (DgnOleInfo) + 1) + 7) & ~7) / 2);

    DgnV8ElementBlank   elm;

    // NOTE: Previously center.interpolate (shape+2, .5, shape)...set to center of range now...shouldn't matter?
    NormalCellHeaderHandler::CreateOrphanCellElement (eeh, name, is3d, modelRef);
    eeh.GetElementCP ()->CopyTo (elm);
    linkage_appendToElement (&elm, &lh, (void*) &info, NULL);
    eeh.ReplaceElement (&elm);

    if (DGNOLE_ViewRotation_ViewDependent != info.m_flags.viewRotationMode)
        NormalCellHeaderHandler::SetPointCell (eeh, true);

    if (description)
        NormalCellHeaderHandler::SetDescription (eeh, description);

    EditElementHandle  shapeEeh;

    if (SUCCESS != ShapeHandler::CreateShapeElement (shapeEeh, NULL, shape, 5, is3d, modelRef))
        return ERROR;

    if (SUCCESS != NormalCellHeaderHandler::AddChildElement (eeh, shapeEeh))
        return ERROR;

    if (SUCCESS != DgnStoreHdrHandler::AppendToCell (eeh, (void*) oleData, static_cast<UInt32>(oleSize), DGNSTOREID_OLE, DGNSTOREAPPID_OLE_STORAGE))
        return ERROR;

    return NormalCellHeaderHandler::AddChildComplete (eeh);
    }
