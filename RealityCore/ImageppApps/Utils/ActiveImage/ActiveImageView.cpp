/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageView.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageView.cpp,v 1.32 2011/07/18 21:10:21 Donald.Morissette Exp $
//
// Class: ActiveImageView
// ----------------------------------------------------------------------------
 
//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "vfw.h"

#include "ActiveImage.h"
#include "ActiveImageDoc.h"
#include "ActiveImageView.h"
#include "ActiveImageFrame.h"
#include <Imagepp/all/h/HRATiledRaster.h>

#include "RecenterDialog.h"
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRPMapFilters8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>

#include <Imagepp/all/h/HGF2DGrid.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRABitmapRLE.h>
#include <Imagepp/all/h/HCDPacketRLE.h>
#include <Imagepp/all/h/HRACopyFromOptions.h>

#include "DisplayExtent.h"
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include "GeoCodingInfoDialog.h"
#include "LayersDialog.h"
#include "ScaleDialog.h"

#include <Imagepp/all/h/HMDAnnotationIconsPDF.h>
#include <Imagepp/all/h/HMDAnnotations.h>
#include <Imagepp/all/h/HMDContext.h>

#include <math.h>
#include <cstringt.h>

// include all the function modes
#include "ActiveImageDefaultMode.h"
#include "ActiveImageEditMode.h"
#include "ActiveImageViewMode.h"

#include <Imagepp/all/h/HIMOnDemandMosaic.h>
#include <Imagepp/all/h/HIMStripAdapter.h>

#include "HMRProgressImage.h"

#include <Imagepp/all/h/HRADrawProgressIndicator.h>
#include <Imagepp/all/h/HRAUpdateSubResProgressIndicator.h>

#include <Imagepp/all/h/HUTImportFromRasterExportToFile.h>
#include <ImageppApps/HUTExport/HUTExportDialog.h>
#include <ImageppApps/HUTExport/HUTProgressDialog.h>
#include <Imagepp/all/h/HUTExportProgressIndicator.h>

#include <Imagepp/all/h/HCDCodecIJG.h>

#include <Imagepp/all/h/HCPGeoTiffKeys.h>
#include <Imagepp/all/h/HCPGCoordModel.h>

//#define ENABLE_PROFILER
#include "Profiler.h"

//#define FETCH_SLO6

enum RedesignSettings
    {
    REDESIGN_DrawOriginalCopyFrom = (1 << (ID_REDESIGN_DRAW_ORIGINAL_COPYFROM-ID_REDESIGN_MENU_BAR)),
    REDESIGN_Draw10Times          = (1 << (ID_REDESIGN_DRAW_10_TIMES-ID_REDESIGN_MENU_BAR)),
    };
//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void CALLBACK EXPORT TimerProcProgressImage(HWND hWnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime);


// IsGray Macro to verify if a color is in the grayscale or not
// Method: Verify that the three values (R, G, and B) are the same
#define IsGray(Color) ((GetRValue(Color) == GetGValue(Color)) && (GetRValue(Color) == GetBValue(Color)))


//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

// Global Reference Coord Sys
extern HFCPtr<HGF2DCoordSys> g_pAIRefCoordSys;

static HMRProgressImage*    s_pProgressImage = 0;

//-----------------------------------------------------------------------------
// Message Maps and other mFC Macros
//-----------------------------------------------------------------------------
IMPLEMENT_DYNCREATE(CActiveImageView, CView)
BEGIN_MESSAGE_MAP(CActiveImageView, CView)
	//{{AFX_MSG_MAP(CActiveImageView)
	ON_COMMAND(ID_VIEW_REPAINT, OnViewRepaint)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REPAINT, OnUpdateViewRepaint)
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_BACKGROUND, OnViewBackground)
	ON_COMMAND(ID_VIEW_RECENTERXY, OnViewRecenterXY)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RECENTERXY, OnUpdateViewRecenterXY)
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSEWHEEL()
    ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
    ON_WM_MBUTTONDOWN()
    ON_WM_MBUTTONUP()
    ON_WM_MBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
    ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_UPDATE_COMMAND_UI(ID_VIEW_VIEWORIGIN_BOTTOMLEFT, OnUpdateViewVieworiginBottomleft)
	ON_COMMAND(ID_VIEW_VIEWORIGIN_BOTTOMLEFT, OnViewVieworiginBottomleft)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VIEWORIGIN_TOPLEFT, OnUpdateViewVieworiginTopleft)
	ON_COMMAND(ID_VIEW_VIEWORIGIN_TOPLEFT, OnViewVieworiginTopleft)
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_HELP_TESTDISPLAY, OnHelpTestdisplay)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_PRECALCULATE_DISPLAY, OnPrecalculateDisplay)
	ON_UPDATE_COMMAND_UI(ID_PRECALCULATE_DISPLAY, OnUpdatePrecalculateDisplay)
	ON_COMMAND(ID_FILE_EXPORT_REGION, OnFileExportRegion)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_EDIT_COMPUTE_HISTOGRAM, OnComputeHistogram)
	ON_COMMAND(ID_INFO_DISPLAY_EXTENT, OnInfoDisplayExtent)
    ON_COMMAND(ID_GEOCODING_INFO, OnGeocodingInfo)
    ON_COMMAND(ID_VIEW_LAYERS, OnViewLayers)


	//}}AFX_MSG_MAP

	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CActiveImageView::OnHMRPrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CActiveImageView::OnHMRPrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

    // Redesign menu
    ON_COMMAND_RANGE(ID_REDESIGN_MENU_BAR, ID_REDESIGN_DRAW_10_TIMES, OnRedesignMenuBar)
    ON_UPDATE_COMMAND_UI_RANGE(ID_REDESIGN_MENU_BAR, ID_REDESIGN_DRAW_10_TIMES, OnUpdateRedesignMenuBar)

    // Function Objects commands and Updates
    ON_COMMAND_RANGE(AIFM_MIN_COMMAND_ID, AIFM_MAX_COMMAND_ID, OnFunctionModeCommand)
    ON_UPDATE_COMMAND_UI_RANGE(AIFM_MIN_COMMAND_ID, AIFM_MAX_COMMAND_ID, OnFunctionModeUpdate)

    ON_COMMAND_RANGE(AIFM_MIN_RC_COMMAND_ID, AIFM_MAX_RC_COMMAND_ID, OnFunctionModeCommand)
    ON_UPDATE_COMMAND_UI_RANGE(AIFM_MIN_RC_COMMAND_ID, AIFM_MAX_RC_COMMAND_ID, OnFunctionModeUpdate)

    //Reprojection
    ON_COMMAND_RANGE(ID_REPROJECTIONMODEL_EXACTREPROJECTION, ID_REPROJECTIONMODEL_BESTMODEL, OnReprojectionModelUpdate)
    ON_UPDATE_COMMAND_UI_RANGE(ID_REPROJECTIONMODEL_EXACTREPROJECTION, ID_REPROJECTIONMODEL_BESTMODEL, OnUpdateReprojectionModel)

    // Tile Sizing
    ON_COMMAND_RANGE(ID_VIEW_TILESIZE_32, ID_VIEW_TILESIZE_CUSTOM, OnTileSize)
    ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_TILESIZE_32, ID_VIEW_TILESIZE_CUSTOM, OnTileSizeUpdate)

    // Resampling
    ON_COMMAND_RANGE(ID_VIEW_RESAMPLING_NEAREST, ID_VIEW_RESAMPLING_CONVOLUTION, OnResampling)
    ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_RESAMPLING_NEAREST, ID_VIEW_RESAMPLING_CONVOLUTION, OnResamplingUpdate)

    // Resampling on file
    ON_COMMAND_RANGE(ID_VIEW_RESAMPLING_NEAREST, ID_VIEW_RESAMPLING_CONVOLUTION, OnResampling)
    ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_RESAMPLING_NEAREST, ID_VIEW_RESAMPLING_CONVOLUTION, OnResamplingUpdate)

    // Color Space conversion
    ON_COMMAND_RANGE(ID_CHANGECOLORSPACE_ORIGINAL, ID_CHANGECOLORSPACE_AUTO, OnColorSpaceConversion)
    ON_UPDATE_COMMAND_UI_RANGE(ID_CHANGECOLORSPACE_ORIGINAL, ID_CHANGECOLORSPACE_AUTO, OnColorSpaceConversionUpdate)

    // Respond to WM_HSCROLL and WM_VSCROLL without ClassWizard because
    // the standard MFC implementation of these handlers uses UINT when
    // the scroll message sends signed integers.
    ON_COMMAND(ID_DISPLAY_ANNOTATION_ICONS, &CActiveImageView::OnDisplayAnnotationIcon)
    ON_UPDATE_COMMAND_UI(ID_DISPLAY_ANNOTATION_ICONS, &CActiveImageView::OnUpdateDisplayAnnotationIcons)
    ON_COMMAND(ID_MENU_EDIT_SUBSAMPLING_NEAREST, &CActiveImageView::OnMenuEditSubsamplingNearest)
    ON_UPDATE_COMMAND_UI(ID_MENU_EDIT_SUBSAMPLING_NEAREST, &CActiveImageView::OnUpdateMenuEditSubsamplingNearest)
    ON_COMMAND(ID_MENU_EDIT_SUBSAMPLING_AVERAGE, &CActiveImageView::OnMenuEditSubsamplingAverage)
    ON_UPDATE_COMMAND_UI(ID_MENU_EDIT_SUBSAMPLING_AVERAGE, &CActiveImageView::OnUpdateMenuEditSubsamplingAverage)
    ON_COMMAND(ID_MENU_EDIT_SUBSAMPLING_BILINEAR, &CActiveImageView::OnMenuEditSubsamplingBilinear)
    ON_UPDATE_COMMAND_UI(ID_MENU_EDIT_SUBSAMPLING_BILINEAR, &CActiveImageView::OnUpdateMenuEditSubsamplingBilinear)
    ON_COMMAND(ID_MENU_EDIT_SUBSAMPLING_BICUBIC, &CActiveImageView::OnMenuEditSubsamplingBicubic)
    ON_UPDATE_COMMAND_UI(ID_MENU_EDIT_SUBSAMPLING_BICUBIC, &CActiveImageView::OnUpdateMenuEditSubsamplingBicubic)
    ON_COMMAND(ID_VIEW_SCALE, &CActiveImageView::OnViewScale)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SCALE, &CActiveImageView::OnUpdateViewScale)
    ON_NOTIFY(TVN_SELCHANGED, IDC_OBJECT_TREE, &CActiveImageView::OnItemChangedObjectTree)
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// Protected
// Default Constructor
//-----------------------------------------------------------------------------
CActiveImageView::CActiveImageView()
        : HFCProgressListener(),
           m_Resampling(HGSResampling::NEAREST_NEIGHBOUR),
           m_ColorConversionID(ID_CHANGECOLORSPACE_AUTO)
{
    // HMR Progress Image
    s_pProgressImage = 0;

    // Initialize the frame to nothing
    // It will be updated in OnInitialUpdate()
    m_pFrame        = 0;

    m_ExportFence = false;

    // Set the Default tile size
    m_TileSizeX = 256;
    m_TileSizeY = 256;

    m_DisplayAnnotationIcon = true;

    // View origin Bottom_Left
    m_ViewOrigin = ID_VIEW_VIEWORIGIN_TOPLEFT;

    // create the log for the buffered image
    m_pObjectLog = new HPMPool(theApp.GetBufferImg_Mem(), NULL);
    HASSERT(m_pObjectLog != 0);

    // Initialize the function object
    m_pFunction     = 0;

    // set the default color
    m_DefaultColor          = RGB(255, 255, 255);
    m_pBkgBrush = new CBrush(m_DefaultColor);

    m_PrecalculateDisplay = false;
    //m_redesignSettings = REDESIGN_DisplayDifference | REDESIGN_DrawOriginalCopyFrom | REDESIGN_DrawLegacyCopyFrom;
    m_redesignSettings = REDESIGN_DrawOriginalCopyFrom;

    HUTExportProgressIndicator::GetInstance()->AddListener(this);
	HRADrawProgressIndicator::GetInstance()->AddListener(this);
    m_EyePos  = 10;  
}


//-----------------------------------------------------------------------------
// Public 
// Destructor
//-----------------------------------------------------------------------------
CActiveImageView::~CActiveImageView()
{
    HUTExportProgressIndicator::GetInstance()->RemoveListener(this);
	HRADrawProgressIndicator::GetInstance()->RemoveListener(this);
    
    // Destroy handle progressImage
    delete s_pProgressImage;

    if (m_pObjectLog != 0)
        delete m_pObjectLog;
}


//-----------------------------------------------------------------------------
// Public
// OnDraw - Repint the view
//-----------------------------------------------------------------------------
void CActiveImageView::DrawBitmapToGDI(CDC* pDC, HRABitmapBase& bitmapBase)
    {
    uint64_t size64X, size64Y;
    bitmapBase.GetSize(&size64X, &size64Y);

    uint32_t sizeX = (uint32_t)size64X;
    uint32_t sizeY = (uint32_t)size64Y;

    CImage image;
    image.Create(sizeX, sizeY, 24/*BGR*/, 0);

    HFCPtr<HRPPixelType> pGDIPixelType = new HRPPixelTypeV24B8G8R8();

    BYTE* pbDestRow = static_cast<BYTE*>(image.GetBits());

    if (bitmapBase.IsCompatibleWith(HRABitmap::CLASS_ID))
        {
        HRABitmap& bitmap = static_cast<HRABitmap&>(bitmapBase);

        if (bitmap.GetPacket()->GetCodec()->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID))
            {
            HFCPtr<HRPPixelType> pRLEPixelType = new HRPPixelTypeI1R8G8B8RLE();
            HFCPtr<HRPPixelConverter> pConvBitmapToGDI = pRLEPixelType->GetConverterTo(pGDIPixelType);
            HCDCodecHMRRLE1& rleCodec = static_cast<HCDCodecHMRRLE1&>(*bitmap.GetPacket()->GetCodec());

            uint16_t const* pSrcBuffer = (uint16_t const*)bitmap.GetPacket()->GetBufferAddress();

            for (int y = 0; y < image.GetHeight(); y++)
                {
#ifdef FETCH_SLO6
                pConvBitmapToGDI->Convert(pSrcBuffer + rleCodec.GetLineIndexesTable()[sizeY-y], pbDestRow, sizeX);
#else
                pConvBitmapToGDI->Convert(pSrcBuffer + rleCodec.GetLineIndexesTable()[y], pbDestRow, sizeX);
#endif
                pbDestRow += image.GetPitch();
                }

            }
        else
            {
            HFCPtr<HRPPixelConverter> pConvBitmapToGDI = bitmapBase.GetPixelType()->GetConverterTo(pGDIPixelType);

            int srcBytesPerRow = (bitmap.GetPixelType()->CountPixelRawDataBits() * sizeX + 7) / 8;
#ifdef FETCH_SLO6
            // adjust src to Fill bottom up.
            Byte* pbSrcRow = bitmap.GetPacket()->GetBufferAddress() + (sizeY - 1)*srcBytesPerRow;
            int srcStride = -srcBytesPerRow;
#else
            Byte* pbSrcRow = bitmap.GetPacket()->GetBufferAddress();
            int srcStride = srcBytesPerRow;
#endif
            
            for (int y = 0; y < image.GetHeight(); y++)
                {
                pConvBitmapToGDI->Convert(pbSrcRow, pbDestRow, sizeX);
                pbDestRow += image.GetPitch();
                pbSrcRow += srcStride;
                }
            }
        }
    else if (bitmapBase.IsCompatibleWith(HRABitmapRLE::CLASS_ID))
        {
        HRABitmapRLE& bitmap = static_cast<HRABitmapRLE&>(bitmapBase);
        HFCPtr<HRPPixelType> pRLEPixelType = new HRPPixelTypeI1R8G8B8RLE();
        HFCPtr<HRPPixelConverter> pConvBitmapToGDI = pRLEPixelType->GetConverterTo(pGDIPixelType);
        
        for (int y = 0; y < image.GetHeight(); y++)
            {
#ifdef FETCH_SLO6
            pConvBitmapToGDI->Convert(bitmap.GetPacket()->GetLineBuffer(sizeY-y), pbDestRow, sizeX);
#else
            pConvBitmapToGDI->Convert(bitmap.GetPacket()->GetLineBuffer(y), pbDestRow, sizeX);
#endif
            pbDestRow += image.GetPitch();
            }
        }
    
    image.StretchBlt(pDC->GetSafeHdc(), 0, 0, sizeX, sizeY, SRCCOPY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRABitmap> CActiveImageView::GenerateDiffBitmap(HRABitmap& src1, HRABitmap& src2, unsigned short threshold1, unsigned short threshold2)
    {
    // Compare the 2 result and display. assume perfect 1:1 mapping.
    // black: equal
    // blue:  under or equal to threshold
    // red:   above threshold
          
    HFCPtr<HRPPixelType> pRGBPixelType = new HRPPixelTypeV24R8G8B8();
    HFCPtr<HRPPixelConverter> pConvertSrc1 = src1.GetPixelType()->GetConverterTo(pRGBPixelType);
    HFCPtr<HRPPixelConverter> pConvertSrc2 = src2.GetPixelType()->GetConverterTo(pRGBPixelType);

    uint64_t Width, Height;
    src1.GetSize(&Width, &Height);

    size_t allocWidth = static_cast<size_t>(Width*3);

    HArrayAutoPtr<Byte> pWorkline1(new Byte[allocWidth]); // rgb
    HArrayAutoPtr<Byte> pWorkline2(new Byte[allocWidth]); // rgb

    HFCPtr<HRABitmap> pDiffBitmap =  HRABitmap::Create(static_cast<uint32_t>(Width), static_cast<uint32_t>(Height), NULL, src1.GetCoordSys(), pRGBPixelType);

    pDiffBitmap->Clear();   // will clear black.

    Byte* pBufferSrc1 = src1.GetPacket()->GetBufferAddress();
    Byte* pBufferSrc2 = src2.GetPacket()->GetBufferAddress();
    Byte* pDestBuffer = pDiffBitmap->GetPacket()->GetBufferAddress();

    size_t  bytesPerRowSrc1 = static_cast<size_t>((src1.GetPixelType()->CountPixelRawDataBits() * Width + 7) / 8);
    size_t  bytesPerRowSrc2 = static_cast<size_t>((src2.GetPixelType()->CountPixelRawDataBits() * Width + 7) / 8);

    uint32_t lowDiffCount = 0;
    uint32_t mediumDiffCount = 0;
    uint32_t severeDiffCount = 0;

    for(uint64_t Line=0; Line < Height; ++Line)
        {
        pConvertSrc1->Convert(pBufferSrc1 + bytesPerRowSrc1*Line, pWorkline1, static_cast<size_t>(Width));
        pConvertSrc2->Convert(pBufferSrc2 + bytesPerRowSrc2*Line, pWorkline2, static_cast<size_t>(Width));

        for(uint64_t Column=0; Column < Width; ++Column)
            {
            unsigned short diffRed   = (unsigned short)abs(pWorkline1[Column*3 + 0] - pWorkline2[Column*3 + 0]);
            unsigned short diffGreen = (unsigned short)abs(pWorkline1[Column*3 + 1] - pWorkline2[Column*3 + 1]);
            unsigned short diffBlue  = (unsigned short)abs(pWorkline1[Column*3 + 2] - pWorkline2[Column*3 + 2]);
            if(diffRed > threshold2 || diffGreen > threshold2 || diffBlue > threshold2)
                {
                pDestBuffer[Line*Width*3 + Column*3 + 0] = 0xff; // Set red, assuming it was init to black.
                ++severeDiffCount;
                }
            else if(diffRed > threshold1 || diffGreen > threshold1 || diffBlue > threshold1)
                {
                pDestBuffer[Line*Width*3 + Column*3 + 2] = 0xff; // Set blue, assuming it was init to black.
                ++mediumDiffCount;
                }
            else if(diffRed != 0 || diffGreen != 0  || diffBlue != 0 )
                {
                pDestBuffer[Line*Width*3 + Column*3 + 0] = 0x7F; 
                pDestBuffer[Line*Width*3 + Column*3 + 1] = 0x7F; // Set gray, assuming it was init to black.
                pDestBuffer[Line*Width*3 + Column*3 + 2] = 0x7F; 
                ++lowDiffCount;
                }
            }
        }

    CActiveImageDoc* pDoc = GetDocument();

    bool drawDiffBitmap = false;
    CString logMessage;

    if(severeDiffCount)
        {
        logMessage = L"ERROR: Severe diff were found between Legacy and New display";
        drawDiffBitmap = true;
        }
    else if(mediumDiffCount)
        {
        logMessage = L"WARNING: Medium diff were found between Legacy and New display";
        drawDiffBitmap = true;
        }
    else if(lowDiffCount)
        {
        logMessage = L"INFO: Low diff were found between Legacy and New display";
        drawDiffBitmap = true;
        //drawDiffBitmap = lowDiffCount > 50 || (lowDiffCount/(double)(Height*Width)); // show when more then 50 pixels or 1% pixels are bad.
        }

    if(!logMessage.IsEmpty())
        pDoc->AddStringToLog(logMessage);

    return drawDiffBitmap ? pDiffBitmap : NULL;
    }



//-----------------------------------------------------------------------------
// Public
// OnDraw - Repint the view
//-----------------------------------------------------------------------------
void CActiveImageView::OnDraw(CDC* pDC)
    {
    CActiveImageDoc*   pDoc = GetDocument();    
    HTREEITEM        SelObject = pDoc->GetSelectedObject();

    CRect ScreenRect;
    GetClientRect(&ScreenRect);

    // Draw the raster only if the selection is not null and not the document item
    if (SelObject == NULL || SelObject == pDoc->GetDocumentObject() || pDoc->GetRaster(SelObject) == 0 || m_pViewCoordSys == 0 ||
        !(m_redesignSettings & (REDESIGN_DrawOriginalCopyFrom)))
        {
        pDC->FillRect(ScreenRect, m_pBkgBrush);
        return;
        }

     try
        {
        HRADrawProgressIndicator::GetInstance()->Restart(0);
		
        m_EyePos  = 10;
        m_LeftDir = false;

        clock_t  StartTime = clock();

        // create a page
        HFCPtr<HRARaster> pRaster = pDoc->GetRaster(SelObject);
        HASSERT(pRaster != 0);

        if (pRaster->HasLookAhead())
            {
            // Send the look ahead for the screen extent
            HGF2DExtent ScreenExtent(ScreenRect.left, ScreenRect.top, ScreenRect.right + 1, ScreenRect.bottom + 1, m_pViewCoordSys);
            pRaster->SetLookAhead(ScreenExtent, true);
            }

        HFCPtr<HGF2DTransfoModel> pBitmapModel;
#ifdef FETCH_SLO6
        pBitmapModel = new HGF2DStretch(HGF2DDisplacement(0.0, Height), 1.0, -1.0);
#endif

        uint32_t Width    = ScreenRect.Width();
        uint32_t Height   = ScreenRect.Height();

        HFCPtr<HRABitmapBase> pDisplayBitmap;
       
        switch(m_ColorConversionID)
            {
            case ID_CHANGECOLORSPACE_AUTO:
                if (pRaster->GetPixelType()->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
                    {
                    pDisplayBitmap = HRABitmap::Create(Width, Height, pBitmapModel.GetPtr(), m_pViewCoordSys, new HRPPixelTypeV32R8G8B8A8(), 8);
                    }
                else
                    {
                    pDisplayBitmap = HRABitmap::Create(Width, Height, pBitmapModel.GetPtr(), m_pViewCoordSys, new HRPPixelTypeV24R8G8B8(), 8);
                    }
                break;
            
            case ID_CHANGECOLORSPACE_RLE:
                pDisplayBitmap = HRABitmapRLE::Create(Width, Height, pBitmapModel.GetPtr(), m_pViewCoordSys, new HRPPixelTypeI1R8G8B8(), false/*resizable*/);
                break;

            case ID_CHANGECOLORSPACE_RLELINE:
                pDisplayBitmap = HRABitmapRLE::Create(Width, Height, pBitmapModel.GetPtr(), m_pViewCoordSys, new HRPPixelTypeI1R8G8B8());
                break;

            case ID_CHANGECOLORSPACE_GRAY1:
                pDisplayBitmap = HRABitmap::Create(Width, Height, pBitmapModel.GetPtr(), m_pViewCoordSys, new HRPPixelTypeV1Gray1());
                break;

            case ID_CHANGECOLORSPACE_GRAY8:
                pDisplayBitmap = HRABitmap::Create(Width, Height, pBitmapModel.GetPtr(), m_pViewCoordSys, new HRPPixelTypeV8Gray8());
                break;

            case ID_CHANGECOLORSPACE_RGB:
                pDisplayBitmap = HRABitmap::Create(Width, Height, pBitmapModel.GetPtr(), m_pViewCoordSys, new HRPPixelTypeV24R8G8B8());
                break;

            case ID_CHANGECOLORSPACE_RGBA:
                pDisplayBitmap = HRABitmap::Create(Width, Height, pBitmapModel.GetPtr(), m_pViewCoordSys, new HRPPixelTypeV32R8G8B8A8());
                break;

            case ID_CHANGECOLORSPACE_YCC:
                pDisplayBitmap = HRABitmap::Create(Width, Height, pBitmapModel.GetPtr(), m_pViewCoordSys, new HRPPixelTypeV24PhotoYCC());
                break;

            case ID_CHANGECOLORSPACE_YCCA8:
                pDisplayBitmap = HRABitmap::Create(Width, Height, pBitmapModel.GetPtr(), m_pViewCoordSys, new HRPPixelTypeV32PRPhotoYCCA8());
                break;
            
            case ID_CHANGECOLORSPACE_ORIGINAL:
            default:
                pDisplayBitmap = HRABitmap::Create(Width, Height, pBitmapModel.GetPtr(), m_pViewCoordSys, pRaster->GetPixelType());
                break;
            } 


        Byte defaultRawData[HRPPixelType::MAX_PIXEL_BYTES] = { 0 };

        COLORREF defaultColor = GetDefaultColor();
        HRPPixelTypeV24R8G8B8().GetConverterTo(pDisplayBitmap->GetPixelType())->Convert(&defaultColor, defaultRawData);   
        pDisplayBitmap->GetPixelType()->SetDefaultRawData(defaultRawData);

        HRACopyFromOptions copyFromOpts;
        copyFromOpts.SetAlphaBlend(true);
        copyFromOpts.SetResamplingMode(m_Resampling);

        uint32_t drawLoopCount = 1;
        if(m_redesignSettings & REDESIGN_Draw10Times)
            drawLoopCount = 10;

        CString msg;
        msg.Format(L"--------- Draw loop count: %d ---------", drawLoopCount);
        pDoc->AddStringToLog(msg);

        if(m_redesignSettings & REDESIGN_DrawOriginalCopyFrom)
            {
            pDisplayBitmap->Clear();

            Profiler profiler("CopyFrom..");
            
            for(uint32_t i=0; i < drawLoopCount; ++i)
                {
                profiler.Start();
                pDisplayBitmap->CopyFrom(*pRaster, copyFromOpts);
                profiler.End();
                }          

            DrawBitmapToGDI(pDC, *pDisplayBitmap);
            pDoc->AddStringToLog(profiler.PrintToString().c_str());
            }

        // Place the selection contours
        if (m_pViewCoordSys != NULL && m_pFunction)
            m_pFunction->OnDraw(pDC);

        //Destroy the progress bar, if any
        if (m_WndProgress.GetSafeHwnd() != 0)
            {
            m_WndProgress.DestroyWindow();
            }

        // Reset the status to none
        if(m_pFrame != NULL)        // Can be NULL when a file is pass as a command line arg.
            {
            CString  PaneText;
            PaneText.LoadString(ID_INDICATOR_PROGRESSION);
            m_pFrame->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_PROGRESSION, PaneText, true);				

            // Display display time
            WChar Msg[64];
            _stprintf(Msg,_TEXT("DT:%4.1f Sec."), (double)(clock() - StartTime) / CLOCKS_PER_SEC);
            m_pFrame->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_DRAWTIME, Msg, true);
            }

        //Destroy the progress bar, if any
        if (m_WndProgress.GetSafeHwnd() != 0)
            {
            m_WndProgress.DestroyWindow();
            }
        }
     catch (HFCException& pi_rObj)
         {
         ExceptionHandler(pi_rObj);
         }
     catch(logic_error&)
         {
         AfxMessageBox(_TEXT("Logic Error."));
         }
     catch(runtime_error&)
         {
         AfxMessageBox(_TEXT("Runtime Error."));
         }
     catch(exception&)
         {
         AfxMessageBox(_TEXT("Standard lib. error"));
         }
     catch(...)
         {
         AfxMessageBox(_TEXT("Unknown error"));
         }
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageView::RedrawImage(HVEShape& pi_Source, 
                                   HVEShape& pi_Destination)
{
    CRgn SourceRgn;
    CRgn DestRgn;

    // bring the shapes in the view's coord sys
    pi_Source.ChangeCoordSys(m_pViewCoordSys);
    pi_Destination.ChangeCoordSys(m_pViewCoordSys);

    // get the extent of the shapes
    HVEShape TmpShape(pi_Source.GetExtent());
    TmpShape.ChangeCoordSys(m_pViewCoordSys);
    HGF2DExtent Source(TmpShape.GetExtent());

    TmpShape = pi_Destination.GetExtent();
    TmpShape.ChangeCoordSys(m_pViewCoordSys);
    HGF2DExtent Destination(TmpShape.GetExtent());
    HASSERT(Source.IsDefined());
    HASSERT(Destination.IsDefined());

    // create the source and destination regions
    SourceRgn.CreateRectRgn((int)Source.GetXMin(),
                            (int)Source.GetYMin(),
                            (int)Source.GetXMax(),
                            (int)Source.GetYMax());
    DestRgn.CreateRectRgn((int)Destination.GetXMin(),
                          (int)Destination.GetYMin(),
                          (int)Destination.GetXMax(),
                          (int)Destination.GetYMax());

    // verify if the source and the destination touch
    // if so, create a region unifying both extents
    // Otherwise, redraw each extent
    if (Source.DoTheyOverlap(Destination))
    {
        CRgn InvalidRgn;
        InvalidRgn.CreateRectRgn(0, 0, 1, 1);

        // overlap the regions
        InvalidRgn.CombineRgn(&SourceRgn, &DestRgn, RGN_OR);

        // Redraw the resulting region
        RedrawWindow(NULL, &InvalidRgn);
    }
    else
    {
        // Redraw the source extent
        RedrawWindow(NULL, &SourceRgn);

        // Redraw the destination extent
        RedrawWindow(NULL, &DestRgn);
    }
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnInitialUpdate()
{
    static bool bFirstTime = true;

    if (bFirstTime)
    {
        // get the address of the frame window
        m_pFrame = (CActiveImageFrame*) AfxGetMainWnd();
        HASSERT(m_pFrame != 0);

        // Update for the first time
        OnUpdate(0, UPDATE_FIRST, 0);
        bFirstTime = false;

        // set a time that will generate messages so that OnIdle can be called
        m_OnIdleTimer   = SetTimer(1000, 100, NULL);
        m_ProgressTimer = SetTimer(1001, 500, TimerProcProgressImage);
    }
}


//-----------------------------------------------------------------------------
// 
// Draws the tiles that have been invalidated by the asynchronous/progression
// display.
//-----------------------------------------------------------------------------
void CALLBACK EXPORT TimerProcProgressImage(HWND hWnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime)
{
    if (s_pProgressImage)
        s_pProgressImage->Redraw();
}



#ifdef _DEBUG
//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::AssertValid() const
{
	CView::AssertValid();
}

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
    HTREEITEM         SelObject;
    CActiveImageDoc* pDoc = GetDocument();
    bool            FitImage = false;

	// Create the default function object if lHint has UPDATE_FIRST
    if (!m_pFunction)
    {
        m_pFunction = &m_CommandDefaultMode;
    }

    if ((lHint & UPDATE_FIRST) 
        || (m_pViewCoordSys == NULL && (lHint & UPDATE_REDRAW))) // We always redraw. no need to invalidate and rebuild everything.
    {
        // Delete it before to delete the pBitmap, because
        // ProgressImage has the Raw pointer...
        //delete s_pProgressImage;
        //s_pProgressImage = 0;

        m_pViewCoordSys = 0;

        // Draw the raster only if the selection is not null and not the document item
        SelObject = pDoc->GetSelectedObject();
        if (SelObject != NULL && SelObject != pDoc->GetDocumentObject() &&
            pDoc->GetRaster(SelObject) != 0)
            {
            HFCPtr<HGF2DCoordSys> pCoordSys;
            if(ID_VIEW_VIEWORIGIN_BOTTOMLEFT == m_ViewOrigin)
                {
                pCoordSys = pDoc->GetWorldCluster()->GetCoordSysReference(HGF2DWorld_HMRWORLD);
                }
            else
                {
                HASSERT(ID_VIEW_VIEWORIGIN_TOPLEFT == m_ViewOrigin);
                pCoordSys = pDoc->GetWorldCluster()->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD);
                }             

            HFCPtr<HRARaster> pRaster = pDoc->GetRaster(SelObject);

            HFCPtr<HVEShape> pEffectiveShape = new HVEShape(*pRaster->GetEffectiveShape());
            pEffectiveShape->ChangeCoordSys(pCoordSys);

            HGF2DExtent extent = pEffectiveShape->GetExtent();
            if(extent.IsDefined())      // Empty mosaic are undefined.
                {
                HGF2DExtent PixelSize(pRaster->GetAveragePixelSize());

                HASSERT(PixelSize.IsDefined());

                PixelSize.ChangeCoordSys(pCoordSys);

                double scaleFactor = PixelSize.GetWidth();

                HFCPtr<HGF2DTransfoModel> pTransfoModel = new HGF2DStretch(HGF2DDisplacement(extent.GetOrigin().GetX(), extent.GetOrigin().GetY()), scaleFactor, scaleFactor);
                pDoc->UpdateReprojectionModel();
                HFCPtr<HGF2DTransfoModel> pGCoordModel = pDoc->GetReprojectionModel();
                m_pReprojectedCoordSys = new HGF2DCoordSys(*pGCoordModel, pCoordSys);
                m_pLocalCoordSys = new HGF2DCoordSys(*pTransfoModel, m_pReprojectedCoordSys);

                CRect ScreenRect;
                GetClientRect(&ScreenRect);

                m_pViewCoordSys = new HGF2DCoordSys(HGF2DIdentity(), m_pLocalCoordSys);


                //s_pProgressImage = new HMRProgressImage(pRaster, (CWnd*)this);
                }
            else
                {
                // Need CS.  A lot of code depends on it.
                m_pViewCoordSys = m_pLocalCoordSys = new HGF2DCoordSys(HGF2DIdentity(), pCoordSys);
                }

            }
        FitImage = pDoc->FitImageOnView();
                
        m_pFunction->Setup();
    }

    // Redraw the view.
    RedrawWindow();
}

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnViewRecenterXY() 
    {
    HFCPtr<HRARaster> pRaster = GetSelectedRaster();
        
    if(pRaster == NULL || m_pViewCoordSys == NULL)
        return;

    HFCPtr<HVEShape> pEffectiveShape = new HVEShape(*pRaster->GetEffectiveShape());
    pEffectiveShape->ChangeCoordSys(g_pAIRefCoordSys);

    HGF2DExtent extent = pEffectiveShape->GetExtent();
    if(extent.IsDefined())      // Empty mosaic are undefined.
        {
        // create and display the dialog
        CRecenterDialog dlg(extent.GetXMin(), 
                            extent.GetXMax(), 
                            extent.GetYMin(), 
                            extent.GetYMax(), this);
        if (dlg.DoModal() == IDOK)
            {
            CRect ClientRect;
            GetClientRect(&ClientRect);

            HGF2DLocation fromPt(dlg.m_RecenterX, dlg.m_RecenterY, g_pAIRefCoordSys);
            fromPt.ChangeCoordSys(m_pLocalCoordSys);
           
            HGF2DLocation toPt(ClientRect.Width()*0.5, ClientRect.Height()*0.5, m_pViewCoordSys);
            toPt.ChangeCoordSys(m_pLocalCoordSys);

            if(!fromPt.IsEqualTo(toPt))
                {
                HGF2DDisplacement displacement = fromPt - toPt;

                HFCPtr<HGF2DTransfoModel> pTranslation = new HGF2DTranslation(displacement);

                HFCPtr<HGF2DTransfoModel> pViewToLocal = m_pViewCoordSys->GetTransfoModelTo(m_pLocalCoordSys);

                pViewToLocal = pViewToLocal->ComposeInverseWithDirectOf(*pTranslation);

                m_pViewCoordSys = new HGF2DCoordSys(*pViewToLocal, m_pLocalCoordSys);
                RedrawWindow();
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::UpdateMousePosition()
{   
    CActiveImageDoc* pDoc = GetDocument();
    HTREEITEM        SelObject;

    // Verify if the current selection is the document item
    SelObject = pDoc->GetSelectedObject();
    if (SelObject != NULL &&
        SelObject != pDoc->GetDocumentObject() &&
        pDoc->GetRaster(SelObject) != 0)
    {
        CString          PaneText;
        CString          PositionX;
        CString          PositionY;

        // Get the raster
        HFCPtr<HRARaster> pRaster = pDoc->GetRaster(SelObject);
        HASSERT(pRaster != 0);

        // Get the location of the mouse cursor
        HGF2DLocation CurrentLocation(GetLocation(m_MousePosition));
        CurrentLocation.ChangeCoordSys(g_pAIRefCoordSys);

        // Format the X position
        if (CurrentLocation.GetX() < 1e6)
            PositionX.Format(_TEXT("%.3f"), CurrentLocation.GetX());
        else
            PositionX.Format(_TEXT("%e"), CurrentLocation.GetX());

        // Format the Y position
        if (CurrentLocation.GetY() < 1e6)
            PositionY.Format(_TEXT("%.3f"), CurrentLocation.GetY());
        else
            PositionY.Format(_TEXT("%e"), CurrentLocation.GetY());

        // Generate the pane text
        PaneText = _TEXT("(");
        PaneText += PositionX;
        PaneText += _TEXT(", ");
        PaneText += PositionY;
        PaneText += _TEXT(")");

        // set the pane text
        m_pFrame->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_MOUSEPOSITION, PaneText, true);
    }
    else
    {
        CString PaneText;
        PaneText.LoadString(ID_INDICATOR_MOUSEPOSITION);
        m_pFrame->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_MOUSEPOSITION, PaneText, true);
    } 
}

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
HGF2DLocation 
CActiveImageView::GetLocation(const CPoint& pi_Point) const
{
    HPRECONDITION (m_pViewCoordSys != 0);
    
    // Generate the point location and convert into the raster's CoordSys
	HGF2DLocation Location((double)pi_Point.x,
						   (double)pi_Point.y,
                           m_pViewCoordSys);
    Location.ChangeCoordSys(g_pAIRefCoordSys);

    // return the location
    return (Location);
}



//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
void CActiveImageView::OnHMRPrint()
{
#if 0  // TODO
	// default preparation
	CPrintDialog PrintDialog(false);

	// show the print dialog
	if(PrintDialog.DoModal() == IDOK)
	{
        CActiveImageDoc*        pDoc = GetDocument();
        // get the selection
        HTREEITEM               SelObject;

        SelObject =             pDoc->GetSelectedObject();

        // print the raster only if the selection is not null and not
        // the document item
        if (SelObject!= NULL &&
            SelObject!= pDoc->GetDocumentObject() &&
            pDoc->GetRaster(SelObject) != 0)
        {
            // start a document
            // prepare a doc info structure
            DOCINFO DocInfo = { sizeof(DOCINFO), pDoc->GetTitle(), NULL };	
            StartDoc(PrintDialog.GetPrinterDC(), &DocInfo);

            // GDI start page
            StartPage(PrintDialog.GetPrinterDC());

            // create a page
            HFCPtr<HRARaster> pRaster = pDoc->GetRaster(SelObject);
            HASSERT(pRaster != 0);
    
            // get a printer page
            HFCPtr<HVEShape> pShape(new HVEShape(*pRaster->GetEffectiveShape()));
            pShape->ChangeCoordSys(pDoc->GetWorldCluster()->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
            HGF2DExtent TmpExtent(pShape->GetExtent());
            HFCPtr<HGFMappedSurface> pSurface(new HGFMappedSurface(new HGSGDISurface(PrintDialog.GetPrinterDC()), TmpExtent));

            // Try to print data
//            try 
//            {
                if (m_pBitmap->HasLookAhead())
                    m_pBitmap->SetLookAhead(TmpExtent, false);

	            // print the page inside the extent
                Byte BackgroundColor[3];
                memset(&BackgroundColor, 0xff, 3);
                HFCPtr<HIMStripAdapter> pStripAdapter(new HIMStripAdapter(pRaster, BackgroundColor));
                pStripAdapter->Draw(pSurface, HRADrawOptions());
//            }
//            catch (HRFConnectionException&)
//            {
//            }

            
            EndPage(PrintDialog.GetPrinterDC());

            // end the document
            EndDoc(PrintDialog.GetPrinterDC());
        }
    }
#endif
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
BOOL CActiveImageView::OnPreparePrinting(CPrintInfo* pInfo) 
{
    return DoPreparePrinting(pInfo);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
}

//-----------------------------------------------------------------------------
// OnPrint
//-----------------------------------------------------------------------------
void CActiveImageView::OnPrint(CDC* pDC, CPrintInfo* pInfo )
{
    OnDraw(pDC);
}

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
}



//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
    // Call the function object
    if (m_pFunction)
        m_pFunction->OnLButtonDblClk(nFlags, point);
	
	CView::OnLButtonDblClk(nFlags, point);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    // Call the function object
    if (m_ExportFence)
    {
        CRectTracker tracker;
	    if (tracker.TrackRubberBand(this, point, true))
        {
            CRect rc (tracker.m_rect); 
        
            rc.NormalizeRect();
        
            HGF2DLocation MinCorner = GetLocation(rc.TopLeft());
            HGF2DLocation MaxCorner = GetLocation(rc.BottomRight());
        
            HGF2DExtent Extent (min(MinCorner.GetX(), MaxCorner.GetX()),
                                min(MinCorner.GetY(), MaxCorner.GetY()),
                                max(MinCorner.GetX(), MaxCorner.GetX()),
                                max(MinCorner.GetY(), MaxCorner.GetY()),
                                MinCorner.GetCoordSys());

            Export(new HVEShape(Extent));
        }
    }
    else if (m_pFunction)
    {
        m_pFunction->OnLButtonDown(nFlags, point);
    }

    // Call the base class function
	CView::OnLButtonDown(nFlags, point);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnLButtonUp(UINT nFlags, CPoint point) 
    {
    // Call the function object
    if (m_pFunction)
        m_pFunction->OnLButtonUp(nFlags, point);

    CActiveImageDoc* pDoc = GetDocument();
    HTREEITEM        SelObject;

    SelObject = pDoc->GetSelectedObject();
    if ((SelObject != NULL) &&
        (SelObject != pDoc->GetDocumentObject()) &&
        (pDoc->GetRaster(SelObject) != 0))
        {
        CString PageName = pDoc->GetTreeCtrl()->GetItemText(SelObject);

        PageName.Replace(_TEXT("Page"), _TEXT(""));

        HFCPtr<HRARaster> pSelectedRaster(pDoc->GetRaster(SelObject));

        if(pSelectedRaster->GetStore() != NULL && pSelectedRaster->GetStore()->IsCompatibleWith(HRSObjectStore::CLASS_ID))
            {
            HFCPtr<HRFRasterFile > pRasterFile = static_cast<HRSObjectStore*>(pSelectedRaster->GetStore())->GetRasterFile();
            uint32_t pageIndex = static_cast<HRSObjectStore*>(pSelectedRaster->GetStore())->GetPageIndex();
            if (pRasterFile != NULL && pRasterFile->GetPageDescriptor(pageIndex)->GetListOfMetaDataContainer() != 0)
                {
                HFCPtr<HMDMetaDataContainer> pMDContainer = pRasterFile->GetPageDescriptor(pageIndex)->GetMetaDataContainer(HMDMetaDataContainer::HMD_ANNOTATION_INFO);
    
                //If some annotation info have been found
                if (pMDContainer != 0)
                    {
                    HFCPtr<HMDAnnotations> pAnnotations((HFCPtr<HMDAnnotations>&)pMDContainer);
                    const HMDAnnotationInfo* pAnnotationInfo = 0;

                    // Get the location of the mouse cursor
                    HGF2DLocation CurrentLocation(GetLocation(m_MousePosition));
                    CurrentLocation.ChangeCoordSys(g_pAIRefCoordSys);

                    pAnnotationInfo = pAnnotations->GetAnnotation(CurrentLocation.GetX(),
                        CurrentLocation.GetY());

                    if (pAnnotationInfo != 0)
                        {       
                        WString Msg;
                        Msg.AssignUtf8(pAnnotationInfo->GetAnnotationType().c_str());
                        if (pAnnotationInfo->IsSupported())
                            {
                            AfxMessageBox(Msg.c_str());
                            }
                        else
                            {
                            Msg.append(L" annotations are not supported");
                            AfxMessageBox(Msg.c_str(), MB_ICONSTOP);
                            }
                        }
                    }
                }
            }
        }

    CView::OnLButtonUp(nFlags, point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void CActiveImageView::OnMButtonDown(UINT nFlags, CPoint point)
    {
    m_lastMButtonDown = point;

    CView::OnMButtonDown(nFlags, point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void CActiveImageView::OnMButtonUp(UINT nFlags, CPoint point)
    {
    CView::OnMButtonUp(nFlags, point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void CActiveImageView::OnMButtonDblClk (UINT nFlags, CPoint point)
    {
    m_CommandViewMode.FitAllToView();
    }

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
    // Call the function object
    if (m_pFunction)
        m_pFunction->OnRButtonDblClk(nFlags, point);
	
	CView::OnRButtonDblClk(nFlags, point);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnRButtonDown(UINT nFlags, CPoint point) 
{
    // Call the function object
    if (m_pFunction)
        m_pFunction->OnRButtonDown(nFlags, point);
	
	CView::OnRButtonDown(nFlags, point);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnRButtonUp(UINT nFlags, CPoint point) 
{
    // Call the function object
    if (m_pFunction)
        m_pFunction->OnRButtonUp(nFlags, point);
	
	CView::OnRButtonUp(nFlags, point);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BOOL CActiveImageView::OnMouseWheel (UINT nFlags, short wheelDelta, CPoint point)
    {
    if(m_pViewCoordSys == NULL || m_pLocalCoordSys == NULL)
        return false;

    ScreenToClient(&point);

    CRect ClientRect;
    GetClientRect(&ClientRect);
    if(!ClientRect.PtInRect(point))
        return false;   // Outside our area.

    HGF2DLocation newCenter(point.x, point.y, m_pViewCoordSys);
    newCenter.ChangeCoordSys(m_pLocalCoordSys);

    HGF2DDisplacement CenterToOrigin(newCenter.GetX(), newCenter.GetY());
    CenterToOrigin*=-1;

    HFCPtr<HGF2DTransfoModel> pViewToLocal = m_pViewCoordSys->GetTransfoModelTo(m_pLocalCoordSys);

    HFCPtr<HGF2DTransfoModel> pTranslation = new HGF2DTranslation(CenterToOrigin);
    pViewToLocal = pViewToLocal->ComposeInverseWithDirectOf(*pTranslation);

    double scale = wheelDelta < 0 ? 2.0 : 0.5;

    HFCPtr<HGF2DTransfoModel> pScale = new HGF2DStretch(HGF2DDisplacement(0.0, 0.0), scale, scale);

    // scale
    pViewToLocal = pViewToLocal->ComposeInverseWithDirectOf(*pScale);

    // Move back to the new center.
    HFCPtr<HGF2DTransfoModel> pTranslation2 = new HGF2DTranslation(HGF2DDisplacement(newCenter.GetX(), newCenter.GetY()));
    pViewToLocal = pViewToLocal->ComposeInverseWithDirectOf(*pTranslation2);

    m_pViewCoordSys = new HGF2DCoordSys(*pViewToLocal, m_pLocalCoordSys);

    RedrawWindow();
    
    return  true;
    }

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnMouseMove(UINT nFlags, CPoint point) 
{
    // save the current position for the status bar
    m_MousePosition = point;

    // Call the function object
    if (m_pFunction)
        m_pFunction->OnMouseMove(nFlags, point);

    UpdateMousePosition();

    // call the base class function	
	CView::OnMouseMove(nFlags, point);

    if(m_pLocalCoordSys != NULL && nFlags & MK_MBUTTON)  // Are we panning view?
        {
        HFCPtr<HRARaster> pRaster = GetSelectedRaster();
        
        if(pRaster != NULL)
            {
            HGF2DLocation fromPt((double)m_lastMButtonDown.x, (double)m_lastMButtonDown.y, m_pViewCoordSys);
            fromPt.ChangeCoordSys(m_pLocalCoordSys);
           
            HGF2DLocation toPt((double)point.x, (double)point.y, m_pViewCoordSys);
            toPt.ChangeCoordSys(m_pLocalCoordSys);
            
            if(!fromPt.IsEqualTo(toPt))
                {
                HGF2DDisplacement displacement = fromPt - toPt;

                HFCPtr<HGF2DTransfoModel> pTranslation = new HGF2DTranslation(displacement);

                HFCPtr<HGF2DTransfoModel> pViewToLocal = m_pViewCoordSys->GetTransfoModelTo(m_pLocalCoordSys);

                pViewToLocal = pViewToLocal->ComposeInverseWithDirectOf(*pTranslation);

                m_pViewCoordSys = new HGF2DCoordSys(*pViewToLocal, m_pLocalCoordSys);

                m_lastMButtonDown = point;
                RedrawWindow();
                }
            }
        }
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    // Call the function object
    if (m_pFunction)
        m_pFunction->OnKeyDown(nChar, nRepCnt, nFlags);

    if (nChar == VK_ESCAPE)
	{
        HRADrawProgressIndicator::GetInstance()->StopIteration();
		HUTExportProgressIndicator::GetInstance()->StopIteration();
	}

    CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    // Call the function object
    if (m_pFunction)
        m_pFunction->OnKeyUp(nChar, nRepCnt, nFlags);
	
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

//-----------------------------------------------------------------------------
// 
// OnFunctionModeCommand - This method calls the function mode command handler
// for the current mode to process the command
//-----------------------------------------------------------------------------
void CActiveImageView::OnFunctionModeCommand(UINT pi_CommandID)
{
    CActiveImageDoc* pDoc = GetDocument();
    HPRECONDITION(m_pFunction != 0);

    // Method:
    //
    //  Call the static method "OnStateCommand" for each function object
    // available.  Stop at the first method that returns true.  If no 
    // "OnStateCommand" returns true, call the current object's "OnCommand"
    // method.

    // call the state command functions
    if (m_CommandEditMode.OnCommand(pi_CommandID, this, pDoc))
        goto WRAPUP;

    // call the state command functions
    if (m_CommandViewMode.OnCommand(pi_CommandID, this, pDoc))
        goto WRAPUP;

    // call the state command functions
    m_CommandDefaultMode.OnCommand(pi_CommandID, this, pDoc);

WRAPUP:;
}


//-----------------------------------------------------------------------------
// 
// OnFunctionModeUpdate - This method calls the function mode update handler for
// the current mode to process
//-----------------------------------------------------------------------------
void CActiveImageView::OnFunctionModeUpdate(CCmdUI* pCmdUI)
{
    HPRECONDITION(m_pFunction != 0);
    CActiveImageDoc* pDoc = GetDocument();

    // Method:
    //
    // Same as OnFunctionModeCommand, but for updating menus and toolbars.

    // call the state command functions
    if (m_CommandEditMode.OnCommandUpdate(pCmdUI, this, pDoc))
        goto WRAPUP;

    // call the state command functions
    if (m_CommandViewMode.OnCommandUpdate(pCmdUI, this, pDoc))
        goto WRAPUP;

    // call the state command functions
    m_CommandDefaultMode.OnCommandUpdate(pCmdUI, this, pDoc);

WRAPUP:;
}


//-----------------------------------------------------------------------------
// Public
// SetFunctionMode - Removes the previous mode and sets a new one
//-----------------------------------------------------------------------------
void CActiveImageView::SetFunctionMode(CActiveImageFunctionMode* pi_pFunction)
{
    CClientDC DC(this);

    // delete the previous function mode
    if (m_pFunction)
    {
        // Remove presence
        m_pFunction->OnUndraw(&DC);

        m_pFunction->EndCommand();
    }

    // set the new function
    if (pi_pFunction == 0)
        m_pFunction = &m_CommandDefaultMode;
    else
        m_pFunction = pi_pFunction;

    m_pFunction->Setup();
    m_pFunction->OnDraw(&DC);
}

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnViewBackground() 
{
    COLORREF      CurrentColor;
    COLORREF*     pColor;

    // set the pointer pColor to the default color to change
    pColor = &m_DefaultColor;

    // keep a copy of the current color to verify 
    // if the color actually has changed after
    // the dialog display.
    CurrentColor = *pColor;
        
    // create the dialog with the current color
    CColorDialog dlg(CurrentColor);

    // Change the default color only if the dialog returns IDOK and
    // the color actually changes.
    if ((dlg.DoModal()) && 
        (CurrentColor != dlg.GetColor()))
    {
//         // verify that the in grayscale, the color is actually a gray
//         if ((m_BitsPerPixel != ID_VIEW_GRAYSCALE) || IsGray(dlg.GetColor()) )
//             
//         {
            // change the default color
            *pColor = dlg.GetColor();

             m_pBkgBrush = new CBrush(*pColor);

            // redraw the view
            OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
//         }
//         else
//         {
//             MessageBox(_TEXT("The color chosen is not a gray.\nPrevious default color will remain."),
//                        _TEXT("Grayscale Color Mode"), MB_OK);
//         }
    }
}


//-----------------------------------------------------------------------------
// Public
// BeginOperation - This method is called by a function object when an operation
// is to be performed.  The method will save the information about the current
// state of the view.  IF anything changed after the operation, the EndOperation
// method will adjust the view to reflect the change.
//-----------------------------------------------------------------------------
void CActiveImageView::BeginOperation()
{
#if 0 // todo
    // save the current extent of the view
    m_OperationExtent = m_pBitmap->GetEffectiveShape()->GetExtent();
#endif
}


//-----------------------------------------------------------------------------
// Public
// EndOperation - This method will adjust the view to reflect the modification
// to the view state during a function object operation.
//-----------------------------------------------------------------------------
void CActiveImageView::EndOperation()
{
#if 0 // todo
    // Get the extent before the operation
    HVEShape TmpShape(m_OperationExtent);
    TmpShape.ChangeCoordSys(g_pAIRefCoordSys);
    HGF2DExtent Before(TmpShape.GetExtent());

    // get the new extent of the view
    TmpShape = m_pBitmap->GetEffectiveShape()->GetExtent();
    TmpShape.ChangeCoordSys(g_pAIRefCoordSys);
    HGF2DExtent After(TmpShape.GetExtent());

    // verify if the extent has changed.  If it changed, then adjust
    // the view's scrollbar.
    if (Before != After)
    {
        double MinPosX, MaxPosX;
        double MinPosY, MaxPosY;

        // Get the current Scroll bar information
        HMRGetScrollRange(SB_HORZ, &MinPosX, &MaxPosX);
        HMRGetScrollRange(SB_VERT, &MinPosY, &MaxPosY);

        // verify if XMin has changed.
        // If so, set MinPosX to the minimum between its current value
        // and the new XMin value.
        if (Before.GetXMin() != After.GetXMin())
            MinPosX = min(After.GetXMin(), MinPosX);

        // verify if XMax has changed.
        // If so, set MaxPosX to the maximum between its current value
        // and the new XMax value.
        if (Before.GetXMax() != After.GetXMax())
            MaxPosX = max(After.GetXMax(), MaxPosX);

        // verify if YMin has changed.
        // If so, set MinPosY to the minimum between its current value
        // and the new YMin value.
        if (Before.GetYMin() != After.GetYMin())
            MinPosY = min(After.GetYMin(), MinPosY);

        // verify if YMax has changed.
        // If so, set MaxPosY to the maximum between its current value
        // and the new YMax value.
        if (Before.GetYMax() != After.GetYMax())
            MaxPosY = max(After.GetYMax(), MaxPosY);

        // Reset the scroll ranges
        HMRSetScrollRange(SB_HORZ, MinPosX, MaxPosX);
        HMRSetScrollRange(SB_VERT, MinPosY, MaxPosY);
    } // Extent has changed
#endif
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnTileSize(UINT nID)
{
    bool TileSizeChanged = false;

    switch  (nID)
    {
        case ID_VIEW_TILESIZE_32:
            if ((m_TileSizeX != 32) || 
                (m_TileSizeY != 32))
            {
                m_TileSizeX = 32;
                m_TileSizeY = 32;
                TileSizeChanged = true;
            }
            break;

        case ID_VIEW_TILESIZE_64:
            if ((m_TileSizeX != 64) || 
                (m_TileSizeY != 64))
            {
                m_TileSizeX = 64;
                m_TileSizeY = 64;
                TileSizeChanged = true;
            }
            break;

        case ID_VIEW_TILESIZE_128:
            if ((m_TileSizeX != 128) || 
                (m_TileSizeY != 128))
            {
                m_TileSizeX = 128;
                m_TileSizeY = 128;
                TileSizeChanged = true;
            }
            break;

        case ID_VIEW_TILESIZE_256:
            if ((m_TileSizeX != 256) || 
                (m_TileSizeY != 256))
            {
                m_TileSizeX = 256;
                m_TileSizeY = 256;
                TileSizeChanged = true;
            }
            break;

        case ID_VIEW_TILESIZE_1024:
            if ((m_TileSizeX != 1024) || 
                (m_TileSizeY != 1024))
            {
                m_TileSizeX = 1024;
                m_TileSizeY = 1024;
                TileSizeChanged = true;
            }
            break;

        case ID_VIEW_TILESIZE_2048:
            if ((m_TileSizeX != 2048) || 
                (m_TileSizeY != 2048))
            {
                m_TileSizeX = 2048;
                m_TileSizeY = 2048;
                TileSizeChanged = true;
            }
            break;

        case ID_VIEW_TILESIZE_4096:
            if ((m_TileSizeX != 4096) || 
                (m_TileSizeY != 4096))
            {
                m_TileSizeX = 4096;
                m_TileSizeY = 4096;
                TileSizeChanged = true;
            }
            break;

        case ID_VIEW_TILESIZE_CUSTOM:
            break;
    }

    // recreate the buffered image if needed.
    if (TileSizeChanged)
        OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnTileSizeUpdate(CCmdUI* pCmdUI)
{
    bool IsChecked = false;

    switch  (pCmdUI->m_nID)
    {
        case ID_VIEW_TILESIZE_32:
            if ((m_TileSizeX == 32) && (m_TileSizeY == 32))
                IsChecked = true;
            break;

        case ID_VIEW_TILESIZE_64:
            if ((m_TileSizeX == 64) && (m_TileSizeY == 64))
                IsChecked = true;
            break;

        case ID_VIEW_TILESIZE_128:
            if ((m_TileSizeX == 128) && (m_TileSizeY == 128))
                IsChecked = true;
            break;

        case ID_VIEW_TILESIZE_256:
            if ((m_TileSizeX == 256) && (m_TileSizeY == 256))
                IsChecked = true;
            break;

        case ID_VIEW_TILESIZE_CUSTOM:
            break;
    }

    pCmdUI->SetCheck(IsChecked);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnResampling(UINT nID)
{
    HGSResampling newResampling(m_Resampling.GetResamplingMethod());

    switch  (nID)
    {
        case ID_VIEW_RESAMPLING_NEAREST:
            newResampling = HGSResampling::NEAREST_NEIGHBOUR;
            break;

        case ID_VIEW_RESAMPLING_AVERAGE:
            newResampling = HGSResampling::AVERAGE;
            break;

        case ID_VIEW_RESAMPLING_BILINEAR:
            newResampling = HGSResampling::BILINEAR;
            break;

        case ID_VIEW_RESAMPLING_CONVOLUTION:
            newResampling = HGSResampling::CUBIC_CONVOLUTION;
            break;
        default:
            break;
    }

    // recreate the buffered image if needed.
    if (newResampling != m_Resampling)
        {
        m_Resampling = newResampling;
        OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
        }
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnResamplingUpdate(CCmdUI* pCmdUI)
{
    bool IsChecked = false;

    switch  (pCmdUI->m_nID)
    {
        case ID_VIEW_RESAMPLING_NEAREST:
            if (m_Resampling == HGSResampling::NEAREST_NEIGHBOUR)
                IsChecked = true;
            break;

        case ID_VIEW_RESAMPLING_AVERAGE:
            if (m_Resampling == HGSResampling::AVERAGE)
                IsChecked = true;
            break;

        case ID_VIEW_RESAMPLING_BILINEAR:
            if (m_Resampling == HGSResampling::BILINEAR)
                IsChecked = true;
            break;
            
        case ID_VIEW_RESAMPLING_CONVOLUTION:
            if (m_Resampling == HGSResampling::CUBIC_CONVOLUTION)
                IsChecked = true;
            break;
    }

    pCmdUI->SetCheck(IsChecked);
}

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnColorSpaceConversion(UINT nID)
{
    if (m_ColorConversionID != nID)
        {
        m_ColorConversionID = nID;
        OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
        }
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnColorSpaceConversionUpdate(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(pCmdUI->m_nID == m_ColorConversionID);
}


void CActiveImageView::OnViewVieworiginBottomleft() 
{
    if (m_ViewOrigin != ID_VIEW_VIEWORIGIN_BOTTOMLEFT)
    {
        m_ViewOrigin = ID_VIEW_VIEWORIGIN_BOTTOMLEFT;
        OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
    }
}
void CActiveImageView::OnViewVieworiginTopleft() 
{
    if (m_ViewOrigin != ID_VIEW_VIEWORIGIN_TOPLEFT)
    {
        m_ViewOrigin = ID_VIEW_VIEWORIGIN_TOPLEFT;
        OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
    }
}

BOOL CActiveImageView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
    if (pWnd == this)
    {
        switch (nHitTest)
        {
            case HTHSCROLL:
            case HTVSCROLL:
                break;

            default:
                if (m_pFunction->GetCursor() !=  AfxGetApp()->LoadStandardCursor(IDC_ARROW))
                {
                    SetCursor (m_pFunction->GetCursor());

                    return (true);
                }
                break;
        }
    }
	
	return CView::OnSetCursor(pWnd, nHitTest, message);
}






void CActiveImageView::OnHelpTestdisplay() 
{
    CString Legend;
    Legend += "\nPD_CAN_DRAW_DIB         0x0001\n";
    Legend += "PD_CAN_STRETCHDIB       0x0002\n";
    Legend += "PD_STRETCHDIB_1_1_OK    0x0004\n";
    Legend += "PD_STRETCHDIB_1_2_OK    0x0008\n";
    Legend += "PD_STRETCHDIB_1_N_OK    0x0010\n";


    LRESULT Ret = DrawDibProfileDisplay(0);


    BITMAPINFOHEADER BitmapInfo;

    // 8 bits
    //
    BitmapInfo.biSize          = sizeof(BITMAPINFOHEADER);
    BitmapInfo.biWidth         = 256;
    BitmapInfo.biHeight        = 256;
    BitmapInfo.biPlanes        = 1;
    BitmapInfo.biBitCount      = 8;
    BitmapInfo.biCompression   = BI_RGB;
    BitmapInfo.biSizeImage     = 0;
    BitmapInfo.biXPelsPerMeter = 0;
    BitmapInfo.biYPelsPerMeter = 0;
    BitmapInfo.biClrUsed       = 0;
    BitmapInfo.biClrImportant  = 0;

    LRESULT Ret2 = DrawDibProfileDisplay(&BitmapInfo);


    // 16 bits
    //
    BitmapInfo.biSize          = sizeof(BITMAPINFOHEADER);
    BitmapInfo.biWidth         = 256;
    BitmapInfo.biHeight        = 256;
    BitmapInfo.biPlanes        = 1;
    BitmapInfo.biBitCount      = 16;
    BitmapInfo.biCompression   = BI_RGB;
    BitmapInfo.biSizeImage     = 0;
    BitmapInfo.biXPelsPerMeter = 0;
    BitmapInfo.biYPelsPerMeter = 0;
    BitmapInfo.biClrUsed       = 0;
    BitmapInfo.biClrImportant  = 0;

    LRESULT Ret3 = DrawDibProfileDisplay(&BitmapInfo);

    // 24 bits
    //
    BitmapInfo.biSize          = sizeof(BITMAPINFOHEADER);
    BitmapInfo.biWidth         = 256;
    BitmapInfo.biHeight        = 256;
    BitmapInfo.biPlanes        = 1;
    BitmapInfo.biBitCount      = 24;
    BitmapInfo.biCompression   = BI_RGB;
    BitmapInfo.biSizeImage     = 0;
    BitmapInfo.biXPelsPerMeter = 0;
    BitmapInfo.biYPelsPerMeter = 0;
    BitmapInfo.biClrUsed       = 0;
    BitmapInfo.biClrImportant  = 0;

    LRESULT Ret4 = DrawDibProfileDisplay(&BitmapInfo);


    char aVal[128];

    CString Str;
    sprintf (aVal, "Val0=%lld  Val8=%lld  Val16=%lld  Val24=%lld", (int64_t)Ret, (int64_t)Ret2, (int64_t)Ret3, (int64_t)Ret4);
    Str = (LPCSTR)aVal;
    Str += Legend;
    MessageBox(Str);
}

void CActiveImageView::OnDestroy() 
{
	CView::OnDestroy();
	
    // Kill Progress Timer
    if (m_ProgressTimer != 0)
        KillTimer(m_ProgressTimer);

    if (m_OnIdleTimer != 0)
        KillTimer(m_OnIdleTimer);
}

BOOL CActiveImageView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
//	return CView::OnEraseBkgnd(pDC);
    return true;
}

void CActiveImageView::OnPrecalculateDisplay() 
{
    m_PrecalculateDisplay = !m_PrecalculateDisplay;
}

void CActiveImageView::OnUpdatePrecalculateDisplay(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(m_PrecalculateDisplay ? 1 : 0);
	
}

void CActiveImageView::OnRedesignMenuBar(UINT pi_CommandID) 
{
    uint32_t flag = (1 << (pi_CommandID - ID_REDESIGN_MENU_BAR));

    if(m_redesignSettings & flag)
        m_redesignSettings &= ~flag;
    else
        m_redesignSettings |= flag;
    
    OnViewRepaint();
}

void CActiveImageView::OnUpdateRedesignMenuBar(CCmdUI* pCmdUI) 
{
    uint32_t flag = (1 << (pCmdUI->m_nID - ID_REDESIGN_MENU_BAR));

    pCmdUI->SetCheck((m_redesignSettings & flag) ? 1 : 0);
	
}

void CActiveImageView::Progression(HFCProgressIndicator* pi_pProgressIndicator, // Indicator
                                   uint64_t             pi_Processed,		  // Total processed items count.                              
                                   uint64_t             pi_CountProgression) // number of items.
{    	
	if (pi_CountProgression == 0)
	{
		//Do not display a life signal if a progress bar is already displayed, 
		//since both share the same space in the status bar. 
		if (m_WndProgress.GetSafeHwnd() == 0)
		{		
			// Draw the life signal
			char  LifeString[23] = " .................... ";
		    
			if (m_LeftDir)
				m_EyePos--;
			else
				m_EyePos++;

			if (m_EyePos >= 20)
				m_LeftDir = true;

			if (m_EyePos <= 1)
				m_LeftDir = false;

			LifeString[m_EyePos] = '';

			CString  PaneText;
			PaneText += LifeString;

			m_pFrame->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_PROGRESSION, PaneText, true);
		}
	}
	else
	{		
		if (pi_CountProgression > 0)
		{
			if (m_WndProgress.GetSafeHwnd() == 0)
			{
				CRect BarRect;
				CRect ItemRect;

				m_pFrame->m_wndStatusBar.GetClientRect(&BarRect);
				m_pFrame->m_wndStatusBar.GetItemRect(INDEX_INDICATOR_PROGRESSION, &ItemRect);								
				VERIFY (m_WndProgress.Create(WS_CHILD | WS_VISIBLE, ItemRect,
					    &m_pFrame->m_wndStatusBar, INDEX_INDICATOR_PROGRESSION + 1));

				HASSERT(pi_CountProgression < USHRT_MAX);
			
				m_WndProgress.SetRange(0, (unsigned short)pi_CountProgression);
				m_WndProgress.SetStep(1);	
				
			}	
			HASSERT(m_WndProgress.GetSafeHwnd() != 0);
			m_WndProgress.StepIt();
		}
	}	

	if (pi_pProgressIndicator->HasEvaluator())				
	{
		HFCPtr<HFCProgressEvaluator> pProgressEvaluator = 0;

		if (pi_pProgressIndicator->GetEvaluator(HFCProgressEvaluator::COMPRESSED_DATA_ESTIMATED_SIZE, pProgressEvaluator))
		{	
			TCHAR TempBuffer[100];
			//This estimate doesn't take into account the size of the header, which should be negligeable for big file. 
			_stprintf(TempBuffer, _TEXT("Final Size : %.2f kB"), (pProgressEvaluator->GetValue() / 1000.0)); 

			m_pFrame->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_EXPORTEDFILESIZE, TempBuffer, true);
		}		
	}

    // Check the ESC to CANCEL the iteration
    if (GetAsyncKeyState(VK_ESCAPE))
    {        
		pi_pProgressIndicator->StopIteration();
        
		//Destroy the progress bar, if any
		if (m_WndProgress.GetSafeHwnd() != 0)
		{
			m_WndProgress.DestroyWindow();
		}
		
		// Reset the status bar
        CString  PaneText;
        PaneText.LoadString(ID_INDICATOR_PROGRESSION);
        m_pFrame->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_PROGRESSION, PaneText, true);	

        PaneText.LoadString(ID_INDICATOR_EXPORTSIZE);
        m_pFrame->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_EXPORTEDFILESIZE, PaneText, true);				
    }
}


void CActiveImageView::OnFileExportRegion() 
{	
    m_ExportFence = true;
}


void CActiveImageView::OnFileExport() 
{
    HFCPtr<HRARaster> pRaster;
    HTREEITEM         SelObject;

    SelObject = GetDocument()->GetSelectedObject();
    pRaster = GetDocument()->GetRaster(SelObject);

    Export(pRaster->GetEffectiveShape());
}


void CActiveImageView::OnComputeHistogram() 
{	
	HFCPtr<HRARaster> pRaster;
    HTREEITEM         SelObject;

    SelObject = GetDocument()->GetSelectedObject();

    if (SelObject != NULL &&
        SelObject != GetDocument()->GetDocumentObject() && 
        GetDocument()->GetRaster(SelObject) != 0)
    {
		pRaster = GetDocument()->GetRaster(SelObject);

		if (pRaster->GetPixelType()->GetPalette().GetMaxEntries() > 0) // Assure we have a palette image
		{
			HRAHistogramOptions HistogramOptions(pRaster->GetPixelType());

			// Set quality to 100%
			HRASamplingOptions SamplingOpt;
			SamplingOpt.SetPixelsToScan(100);
			SamplingOpt.SetTilesToScan(100);
			SamplingOpt.SetPyramidImageSize(100);

			HistogramOptions.SetSamplingOptions(SamplingOpt);

			BeginWaitCursor();

			// - Compute histogram -
			pRaster->ComputeHistogram(&HistogramOptions, true /*Force recompute all*/);

			EndWaitCursor();
			
			// Notify modification
			GetDocument()->SetModifiedFlag();            
		}
	}
}


//-----------------------------------------------------------------------------
// public
// Export
//-----------------------------------------------------------------------------
void CActiveImageView::Export(const HFCPtr<HVEShape>& pi_rpShape)
{
    HPRECONDITION(pi_rpShape != 0);
    try
    {
        m_ExportFence = false;
        HFCPtr<HRARaster> pRaster;
        HTREEITEM         SelObject;
 
        SelObject = GetDocument()->GetSelectedObject();
        pRaster   = GetDocument()->GetRaster(SelObject);   

        HAutoPtr<HUTImportFromRasterExportToFile> pImportExport;
        pImportExport = new HUTImportFromRasterExportToFile (pRaster, 
                                                             *pi_rpShape, 
                                                             GetDocument()->GetWorldCluster());

        pImportExport->SetBlendAlpha(true);

        HUTExportDialog SaveAsDialog(pImportExport, this);
    
        if (SaveAsDialog.DoModal() == IDOK)
        {
            bool ListenerAdded = false;
            HUTProgressDialog ProgressDialog(this);
            try 
            {
                HUTExportProgressIndicator::GetInstance()->AddListener(&ProgressDialog);
                HRADrawProgressIndicator::GetInstance()->AddListener(&ProgressDialog);
                ListenerAdded = true;

                HFCPtr<HFCURLFile> DstFilename = new HFCURLFile(HFCURLFile::s_SchemeName() + "://"+ Utf8String(SaveAsDialog.GetExportFileName())); 

                pImportExport->SelectExportFilename((HFCPtr<HFCURL>)DstFilename);

                ExportBlockAccessListener ExportBlockAccessListener; //Auto register listener

                clock_t StartTime = clock();
                pImportExport->StartExport();
                clock_t StopTime = clock();                    

                HSTATUS ExportStatus = ExportBlockAccessListener.GetExportStatus();

                if (ExportStatus == H_SUCCESS)
                {                
#if (0)                    
                    FILE* fp=fopen("k:\\ActiveImageExportTimes.txt", "a");
                    if (fp != 0)
                    {
                        char aDate[12];
                        _ftprintf(fp, _T("%hs SaveAs: IN:LoadedRaster OUT:%s Time(sec):%.3f\n"), _strdate(aDate),
                                    DstFilename->GetFilename().c_str(), (double)(StopTime - StartTime) / CLOCKS_PER_SEC);
                        fclose (fp);
                    }
#endif
                    WChar Msg[1024];
                    WString filenameW(DstFilename->GetFilename().c_str(), BentleyCharEncoding::Utf8);
                    _stprintf(Msg,_TEXT("SaveAs: IN:LoadedRaster OUT:%s\n   Time(sec):%.3f\n"), filenameW.c_str(), (double)(StopTime - StartTime) / CLOCKS_PER_SEC);
                    AfxMessageBox(Msg);
                }
                else
                {
                    switch (ExportStatus)    
                    {
                        case H_WRITE_ERROR :
                            throw HFCWriteFaultException(DstFilename->GetFilename());
                            break;
                        default :
                            HASSERT(0); //Should be replaced by a function doing a generic 
                                        //mapping between HSTATUS codes and ExceptionID codes.
                    }
                }

                HUTExportProgressIndicator::GetInstance()->RemoveListener(&ProgressDialog);
                HRADrawProgressIndicator::GetInstance()->RemoveListener(&ProgressDialog);

				//Destroy the progress bar, if any
				if (m_WndProgress.GetSafeHwnd() != 0)
				{
					m_WndProgress.DestroyWindow();
				}

                CString  PaneText;
                PaneText.LoadString(ID_INDICATOR_PROGRESSION);
                m_pFrame->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_PROGRESSION, PaneText, true);	

                PaneText.LoadString(ID_INDICATOR_EXPORTSIZE);
                m_pFrame->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_EXPORTEDFILESIZE, PaneText, true);				
            }

            catch (...)
            {
                if (ListenerAdded)
                {
                    HUTExportProgressIndicator::GetInstance()->RemoveListener(&ProgressDialog);
                    HRADrawProgressIndicator::GetInstance()->RemoveListener(&ProgressDialog);

					//Destroy the progress bar, if any
					if (m_WndProgress.GetSafeHwnd() != 0)
					{
						m_WndProgress.DestroyWindow();
					}

					CString  PaneText;
                    PaneText.LoadString(ID_INDICATOR_PROGRESSION);
					m_pFrame->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_PROGRESSION, PaneText, true);	

                    PaneText.LoadString(ID_INDICATOR_EXPORTSIZE);
					m_pFrame->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_EXPORTEDFILESIZE, PaneText, true);				
                }
                throw;
            }
        }
    }
    catch (HFCException& pi_rObj)
    {
        ExceptionHandler(pi_rObj);
    }
    catch(logic_error&)
    {
        AfxMessageBox(_TEXT("Logic Error."));
    }
    catch(runtime_error&)
    {
        AfxMessageBox(_TEXT("Runtime Error."));
    }
    catch(exception&)
    {
        AfxMessageBox(_TEXT("Standard lib. error"));
    }
    catch(...)
    {
        AfxMessageBox(_TEXT("Unknown error"));
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CActiveImageView::OnInfoDisplayExtent() 
{
    HFCPtr<HRARaster> pSelectedRaster = GetSelectedRaster();

    if (pSelectedRaster != 0)
    {
        HVEShape Shape(*pSelectedRaster->GetEffectiveShape());
        Shape.ChangeCoordSys(g_pAIRefCoordSys);
        HGF2DExtent Extent(Shape.GetExtent());

        DisplayExtent dlg(this);
        dlg.m_Min_X = Extent.GetXMin();
        dlg.m_Min_Y = Extent.GetYMin();
        dlg.m_Max_X = Extent.GetXMax();
        dlg.m_Max_Y = Extent.GetYMax();

        dlg.DoModal();
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
 
void CActiveImageView::OnGeocodingInfo()
{
    HFCPtr<HRARaster> pSelectedRaster = GetSelectedRaster();
    
    if (pSelectedRaster != 0)
    {
        if (pSelectedRaster->IsStoredRaster())
        {
            HPMObjectStore* pStore = pSelectedRaster->GetStore();
            
            if (pStore && pStore->IsCompatibleWith(HRSObjectStore::CLASS_ID))
            {
                HFCPtr<HRFRasterFile > pCurrentRaster = static_cast<HRSObjectStore*>(pStore)->GetRasterFile();
                if (pCurrentRaster != 0)
                {
                    GeoCodingInfoDialog dlg(pCurrentRaster);
                    dlg.DoModal();
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CActiveImageView::OnViewLayers()
{
    HFCPtr<HRARaster> pSelectedRaster = GetSelectedRaster();
    
    if (pSelectedRaster != 0)
    {    
        CLayersDialog dlg(pSelectedRaster, this);
        dlg.DoModal();
    }    
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CActiveImageView::OnDisplayAnnotationIcon()
{
    m_DisplayAnnotationIcon = !m_DisplayAnnotationIcon;

    HFCPtr<HRARaster> pSelectedRaster = GetSelectedRaster();

    if (pSelectedRaster != 0)
    {    
        HFCPtr<HMDContext> pContext = pSelectedRaster->GetContext();

        if (pContext != 0)
        {
            HFCPtr<HMDAnnotationIconsPDF> pAnnotIconRastInfo;
            pAnnotIconRastInfo = static_cast<HMDAnnotationIconsPDF*>(pContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_ANNOT_ICON_RASTERING_INFO).GetPtr());

            if (pAnnotIconRastInfo != 0)
            {   
                HASSERT(pAnnotIconRastInfo->IsCompatibleWith(HMDAnnotationIconsPDF::CLASS_ID));

                if (pAnnotIconRastInfo->GetRasterization() != m_DisplayAnnotationIcon) 
                {
                    pAnnotIconRastInfo->SetRasterization(m_DisplayAnnotationIcon);

                    pSelectedRaster->InvalidateRaster();
                    OnViewRepaint();                    
                }
            }
        }
    }   
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HFCPtr<HRARaster> CActiveImageView::GetSelectedRaster()
{
    HFCPtr<HRARaster> pSelectedRaster;
    CActiveImageDoc*  pDoc = GetDocument();
    HTREEITEM         SelObject;

    // Verify if the current selection is the document item
    SelObject = pDoc->GetSelectedObject();
    if (SelObject != NULL &&
        SelObject != pDoc->GetDocumentObject() &&
        pDoc->GetRaster(SelObject) != 0)
    {        
        pSelectedRaster = pDoc->GetRaster(SelObject);
    }

    return pSelectedRaster;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CActiveImageView::OnUpdateDisplayAnnotationIcons(CCmdUI *pCmdUI)
{
    HFCPtr<HRARaster> pSelRaster = GetSelectedRaster();
    
    if (pSelRaster != 0 && pSelRaster->IsCompatibleWith(HRAStoredRaster::CLASS_ID))     // Avoid assert in HIMMosaic.
    {
        HFCPtr<HMDContext> pContext = pSelRaster->GetContext();

        if (pContext != 0)
        {
            HFCPtr<HMDAnnotationIconsPDF> pAnnotIconRastInfo;
            pAnnotIconRastInfo = static_cast<HMDAnnotationIconsPDF*>(pContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_ANNOT_ICON_RASTERING_INFO).GetPtr());

            if (pAnnotIconRastInfo != 0)
            {   
                m_DisplayAnnotationIcon = pAnnotIconRastInfo->GetRasterization();
            }
        }
    }

    pCmdUI->Enable(true);
    pCmdUI->SetCheck(m_DisplayAnnotationIcon ? 1 : 0);
}

void CActiveImageView::OnMenuEditSubsamplingNearest()
{    
    m_Resampling = HGSResampling::NEAREST_NEIGHBOUR;  
    OnViewRepaint();
}

void CActiveImageView::OnUpdateMenuEditSubsamplingNearest(CCmdUI *pCmdUI)
{    
    if (m_Resampling == HGSResampling::NEAREST_NEIGHBOUR)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}

void CActiveImageView::OnMenuEditSubsamplingAverage()
{    
    m_Resampling = HGSResampling::AVERAGE;  
    OnViewRepaint();
}

void CActiveImageView::OnUpdateMenuEditSubsamplingAverage(CCmdUI *pCmdUI)
{
    if (m_Resampling == HGSResampling::AVERAGE)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}
void CActiveImageView::OnMenuEditSubsamplingBilinear()
{    
    m_Resampling = HGSResampling::BILINEAR;  
    OnViewRepaint();
}

void CActiveImageView::OnUpdateMenuEditSubsamplingBilinear(CCmdUI *pCmdUI)
{
    if (m_Resampling == HGSResampling::BILINEAR)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}

void CActiveImageView::OnMenuEditSubsamplingBicubic()
{
    m_Resampling = HGSResampling::CUBIC_CONVOLUTION;         
    OnViewRepaint();
}

void CActiveImageView::OnUpdateMenuEditSubsamplingBicubic(CCmdUI *pCmdUI)
{
    if (m_Resampling == HGSResampling::CUBIC_CONVOLUTION)
    {
        pCmdUI->SetCheck(1);
    }
    else
    {
        pCmdUI->SetCheck(0);
    }
}

//-----------------------------------------------------------------------------
// Public 
// 
//-----------------------------------------------------------------------------
CActiveImageDoc* CActiveImageView::GetDocument()
{ 
    HPRECONDITION(m_pDocument->IsKindOf(RUNTIME_CLASS(CActiveImageDoc)));
    return (CActiveImageDoc*)m_pDocument; 
}




//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnViewRepaint() 
{
    OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnUpdateViewRepaint(CCmdUI* pCmdUI) 
{
    CActiveImageDoc* pDoc = GetDocument();
    HTREEITEM        SelObject;

    // Verify if the current selection is the document item
    // If so, do not enable the repaint
    SelObject = pDoc->GetSelectedObject();
    if ((SelObject != NULL) && (SelObject != pDoc->GetDocumentObject()))
        pCmdUI->Enable(true);
    else
        pCmdUI->Enable(false);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::OnUpdateViewRecenterXY(CCmdUI* pCmdUI) 
{
    // Basically, do the same thing as update repaint.
    OnUpdateViewRepaint(pCmdUI);
}



//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
const HFCPtr<HGF2DCoordSys>& CActiveImageView::GetCoordSys() const
{
    return m_pViewCoordSys;
}

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CActiveImageView::SetCoordSys(HFCPtr<HGF2DCoordSys>& newCoordSys)
{
    m_pViewCoordSys = newCoordSys;
}

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
const HFCPtr<HGF2DCoordSys>& CActiveImageView::GetLocalCoordSys() const
{
    return m_pLocalCoordSys;
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
const CActiveImageFunctionMode* 
                            CActiveImageView::GetFunctionMode() const
{
    return (m_pFunction);
}

void CActiveImageView::OnUpdateViewVieworiginBottomleft(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(m_ViewOrigin == ID_VIEW_VIEWORIGIN_BOTTOMLEFT);
}

void CActiveImageView::OnUpdateViewVieworiginTopleft(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(m_ViewOrigin == ID_VIEW_VIEWORIGIN_TOPLEFT);
}

// Utility function to convert RGB values to BGR values (windows DIB format)
uint32_t CActiveImageView::RGB2BGR(uint32_t rgb)
{
    uint32_t bgr;

    memcpy(&bgr, &rgb, 4);
    ((Byte*)&bgr)[0] = ((Byte*)&rgb)[2];
    ((Byte*)&bgr)[2] = ((Byte*)&rgb)[0];

    return bgr;
}


COLORREF CActiveImageView::GetDefaultColor() const
{
    return m_DefaultColor;
}

CActiveImageFrame* CActiveImageView::GetFrame()
{
	return m_pFrame;
}

CProgressCtrl* CActiveImageView::GetProgressBar()
{
	return &m_WndProgress;
    }



void CActiveImageView::OnViewScale()
    {
    ScaleDialog Dlg;
    Dlg.DoModal();
    double pi_scaleFactor = Dlg.m_scale;

    CActiveImageDoc* pDoc = GetDocument();
    HFCPtr<HRARaster> pRaster = GetSelectedRaster();
    HFCPtr<HRARaster> pScale = pRaster;
    HASSERT(pRaster != 0);

    if (pi_scaleFactor != 1)
        {
        BeginOperation();
        HVEShape Before(*(pRaster->GetEffectiveShape()));

        if (pDoc->IsRasterLinked(pRaster))
            {
            // ask the user what he wants to do
            switch(AfxMessageBox(_TEXT("The raster is a link to an external store\nPlace it in a reference?"), MB_YESNOCANCEL))
                {
                case IDYES:
                    pScale = pDoc->PlaceInReference(pRaster);

                    // remove the raster from the selection and add the reference
                    m_CommandDefaultMode.RemoveFromSelection(pRaster);
                    m_CommandDefaultMode.GetSelection().push_back(pScale);
                    break;

                case IDCANCEL:
                    pScale = 0;
                }
            }

        // Get base information
        HFCPtr<HGF2DCoordSys> pImageCS = pScale->GetCoordSys();
        HFCPtr<HGF2DTransfoModel> pBaseToImage = pDoc->GetWorldCluster()->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD)->GetTransfoModelTo(pImageCS);

        HVEShape ImageShape(*pScale->GetEffectiveShape());
        ImageShape.ChangeCoordSys(pImageCS);
        double OriginX = ImageShape.GetExtent().GetXMin();
        double OriginY = ImageShape.GetExtent().GetYMin();
        pBaseToImage->ConvertInverse(&OriginX, &OriginY);

        HFCPtr<HGF2DStretch> pScaleTransfo(new HGF2DStretch(HGF2DDisplacement(0,0), pi_scaleFactor, pi_scaleFactor));

        HFCPtr<HGF2DTransfoModel> pScalingForImage = pBaseToImage->ComposeInverseWithDirectOf(*pScaleTransfo);
        pScalingForImage = pScalingForImage->ComposeInverseWithInverseOf(*pBaseToImage);
        HFCPtr<HGF2DTransfoModel> pSimplifiedScalingForImage(pScalingForImage->CreateSimplifiedModel());

        // Change the image if it is a StoredRaster...
        if (pScale->IsCompatibleWith(HRAStoredRaster::CLASS_ID))
            {
            HFCPtr<HGF2DTransfoModel> pModel(((HFCPtr<HRAStoredRaster>&)pScale)->GetTransfoModel());
            pModel = pModel->ComposeInverseWithDirectOf(pSimplifiedScalingForImage != 0 ?
                *pSimplifiedScalingForImage :
            *pScalingForImage);

            ((HFCPtr<HRAStoredRaster>&)pScale)->SetTransfoModel(*pModel);
            }
        else
            {
            pScale->SetCoordSys(new HGF2DCoordSys(pSimplifiedScalingForImage != 0 ? 
                *pSimplifiedScalingForImage :
            *pScalingForImage, 
                pImageCS));
            }

        // tell the document that it has changed
        pDoc->SetModifiedFlag(true);
        HVEShape After(*(pRaster->GetEffectiveShape()));
        RedrawImage(Before, After);
        EndOperation();

        OnUpdate(0, UPDATE_REDRAW, 0);
        }
    }


void CActiveImageView::OnUpdateViewScale(CCmdUI *pCmdUI)
    {
    CActiveImageDoc* pDoc = GetDocument();
    HTREEITEM        SelObject;

    SelObject = pDoc->GetSelectedObject();
    if ((SelObject != NULL) && (SelObject != pDoc->GetDocumentObject()))
        pCmdUI->Enable(true);
    else
        pCmdUI->Enable(false);
    }



void CActiveImageView::OnReprojectionModelUpdate(UINT nID)
    {
    CActiveImageDoc* pDoc = GetDocument(); 
    pDoc->OnReprojectionModelUpdate(nID);
    
    OnItemChangedObjectTree(0, 0);
    }


void CActiveImageView::OnUpdateReprojectionModel(CCmdUI *pCmdUI)
    {
    CActiveImageDoc* pDoc = GetDocument();
    pDoc->OnReprojectionModelUpdateCheckBox(pCmdUI);
    }


void CActiveImageView::OnItemChangedObjectTree(NMHDR *pNMHDR, LRESULT *pResult)
    {
    CActiveImageDoc* pDoc = GetDocument();
    HTREEITEM  SelObject = pDoc->GetSelectedObject();

    if (SelObject == nullptr || SelObject == pDoc->GetDocumentObject())
        return;

    HFCPtr<HRARaster> selectedRaster = pDoc->GetRaster(SelObject);

    if (m_pReprojectedCoordSys != nullptr && m_pViewCoordSys != nullptr)
        {
        HFCPtr<HGF2DTransfoModel> pViewToReprojected = m_pViewCoordSys->GetTransfoModelTo(m_pReprojectedCoordSys);
        pDoc->UpdateReprojectionModel();
        HFCPtr<HGF2DTransfoModel> pGCoordModel = pDoc->GetReprojectionModel();

        if (selectedRaster->GetClassID() == HIMMosaic::CLASS_ID) //Raster is a mosaic. We invalidate the reprojection.
            {
                m_pReprojectedCoordSys = new HGF2DCoordSys(HGF2DIdentity(), g_pAIRefCoordSys);
                m_pLocalCoordSys = new HGF2DCoordSys(HGF2DIdentity(), m_pReprojectedCoordSys);
                m_pViewCoordSys = new HGF2DCoordSys(HGF2DIdentity(), m_pLocalCoordSys);
            }
        else if (pViewToReprojected != nullptr)
            {
            m_pReprojectedCoordSys = new HGF2DCoordSys(*pGCoordModel, g_pAIRefCoordSys);
            m_pLocalCoordSys = new HGF2DCoordSys(*pViewToReprojected, m_pReprojectedCoordSys);
            m_pViewCoordSys = new HGF2DCoordSys(HGF2DIdentity(), m_pLocalCoordSys);
            }
        }

    OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
    }
