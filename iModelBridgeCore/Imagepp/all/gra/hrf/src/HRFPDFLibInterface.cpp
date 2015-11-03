//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPDFLibInterface.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFPDFLibInterface
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFPDFFile.h>

#if defined(IPP_HAVE_PDF_SUPPORT) 

#include <Imagepp/all/h/HFCBuffer.h>
#include <Imagepp/all/h/HFCMemoryLineStream.h>

#include <Imagepp/all/h/HGF2DProjective.h>

#include <Imagepp/all/h/HMDAnnotationIconsPDF.h>
#include <Imagepp/all/h/HMDLayersPDF.h>
#include <Imagepp/all/h/HMDLayerInfoPDF.h>
#include <Imagepp/all/h/HMDVolatileLayers.h>

#include <Imagepp/all/h/HRFAnnotationsPDF.h>
#include <Imagepp/all/h/HRFAnnotationInfoPDF.h>
#include <Imagepp/all/h/HRFGdalUtilities.h>


#include <Imagepp/all/h/HVE2DComplexShape.h>
#include <Imagepp/all/h/HVE2DDisjointedComplexLinear.h>
#include <Imagepp/all/h/HVE2DEllipse.h>
#include <Imagepp/all/h/HVE2DHoledShape.h>
#include <Imagepp/all/h/HVE2DRectangle.h>
#include <Imagepp/all/h/HVE2DPolygon.h>
#include <Imagepp/all/h/HVE2DPolygonOfSegments.h>
#include <Imagepp/all/h/HVE2DPolySegment.h>
#include <Imagepp/all/h/HVE2DRectangle.h>
#include <Imagepp/all/h/HVE2DSegment.h>
#include <Imagepp/all/h/HVE2DVoidShape.h>
#include <Imagepp/all/h/HCPGeoTiffKeys.h>

#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>

#include "HRFPDFLibInterface.h"


    
#define PAGE_PIXEL_PER_INCH        96

//-----------------------------------------------------------------------------
// public
// Static method that can be used to get the size of the given PDF page.
//-----------------------------------------------------------------------------
void HRFPDFLibInterface::PageSize(const PDDoc pi_pDoc,
                                  uint32_t    pi_Page,
                                  uint32_t&     po_rWidth,
                                  uint32_t&     po_rHeight,
                                  double&    po_rDPI)
    {
    PDPage Page =  PDDocAcquirePage(pi_pDoc, pi_Page);
    ASFixedMatrix ScaleMatrix;
    ScaleMatrix.a = FloatToASFixed(DPI_CONVERT_SCALE_FACTOR);
    ScaleMatrix.b = FloatToASFixed(0);
    ScaleMatrix.c = FloatToASFixed(0);
    ScaleMatrix.d = FloatToASFixed(DPI_CONVERT_SCALE_FACTOR);
    ScaleMatrix.h = FloatToASFixed(0);
    ScaleMatrix.v = FloatToASFixed(0);

    ASFixedRect PageRect;
    ASFixedMatrix Matrix;

    PDPageGetDefaultMatrix(Page, &Matrix);
    ASFixedMatrixConcat(&Matrix, &ScaleMatrix, &Matrix);

    PDPageGetCropBox(Page, &PageRect);

    ASFixedMatrixTransformRect(&PageRect, &Matrix, &PageRect);

    po_rWidth  = (uint32_t)(abs(ASFixedRoundToInt16(PageRect.right) -
                              ASFixedRoundToInt16(PageRect.left)));
    po_rHeight = (uint32_t)(abs(ASFixedRoundToInt16(PageRect.top) -
                              ASFixedRoundToInt16(PageRect.bottom)));

    po_rDPI = PAGE_PIXEL_PER_INCH * PDPageGetUserUnitSize(Page);

    PDPageRelease(Page);
    }

//-----------------------------------------------------------------------------
// public
// Static method that can be used to get the size of the given PDF page.
//-----------------------------------------------------------------------------
void HRFPDFLibInterface::GetMaxResolutionSize(const PDDoc pi_pDoc,
                                              uint32_t   pi_Page,
                                              double      dpiConvertScaleFactor,
                                              uint32_t&    po_maxResSize)
    {
    PDPage Page =  PDDocAcquirePage(pi_pDoc, pi_Page);


    // TFS#8555; 
    // 16bit integer is the data type used by the PDF library API. This means that the maximum size
    // supported by the PDF is 32767. We need to adjust with the DPI scale factor to prevent the 16 bit overflow
    // In TFS #8555, the overflow occurred on the matrix offset y (matrix.v) when multiply by the scale matrix in ReadBlock method,
    // thus I have compensate for that in this method...

    uint32_t PageWidth;
    uint32_t PageHeight;
    double PageDPI;
    PageSize(pi_pDoc,pi_Page, PageWidth, PageHeight, PageDPI);
    uint32_t MaxPageSize = MAX(PageWidth,PageHeight);


    ASFixedMatrix Matrix;
    PDPageGetFlippedMatrix(Page, &Matrix);

   
    double maxOffsetSize = MAX( ASFixedToFloat(Matrix.h), ASFixedToFloat(Matrix.v));
    if (maxOffsetSize==0)
        maxOffsetSize=MaxPageSize;

    po_maxResSize = (uint32_t)( (32767.0 * MaxPageSize / maxOffsetSize)/ dpiConvertScaleFactor);


    PDPageRelease(Page);
    }

//-----------------------------------------------------------------------------
// public
// Static method that creates an editor.
//-----------------------------------------------------------------------------
HRFPDFLibInterface::Editor* HRFPDFLibInterface::CreateEditor(
    const PDDoc pi_pDoc,
    uint32_t    pi_Page,
    double     pi_ResolutionFactor)
    {
    HAutoPtr<Editor> pEditor(new Editor);
    PDPage           pPDPage;

    DURING

    //pEditor->m_PDPage = PDDocAcquirePage(pi_pDoc, pi_Page);

    pEditor->m_pPDDoc          = pi_pDoc;
    pEditor->m_Page            = pi_Page;
    pEditor->m_TileRect.left   = Int32ToFixed(0);
    pEditor->m_TileRect.bottom = Int32ToFixed(0);
    pEditor->m_TileRect.right  = Int32ToFixed(PDF_TILE_WIDTH_HEIGHT);
    pEditor->m_TileRect.top    = Int32ToFixed(PDF_TILE_WIDTH_HEIGHT);
    pEditor->m_ScaleFactor     = pi_ResolutionFactor * DPI_CONVERT_SCALE_FACTOR;

    pPDPage = PDDocAcquirePage(pi_pDoc, pi_Page);

    PDPageGetFlippedMatrix(pPDPage, &pEditor->m_Matrix);

    ASFixedMatrix ScaleMatrix;
    ScaleMatrix.a = FloatToASFixed(pEditor->m_ScaleFactor);
    ScaleMatrix.b = 0;
    ScaleMatrix.c = 0;
    ScaleMatrix.d = FloatToASFixed(pEditor->m_ScaleFactor);
    ScaleMatrix.h = 0;
    ScaleMatrix.v = 0;

    ASFixedMatrixConcat(&pEditor->m_Matrix, &ScaleMatrix, &pEditor->m_Matrix);

    pEditor->m_OffsetX = ASFixedToFloat(pEditor->m_Matrix.h);
    pEditor->m_OffsetY = ASFixedToFloat(pEditor->m_Matrix.v);

    pEditor->m_csAtom = ASAtomFromString("DeviceRGB");

    // calculate the size of the resolution
    ASFixedMatrix Matrix;
    PDPageGetDefaultMatrix(pPDPage, &Matrix);
    ASFixedMatrixConcat(&Matrix, &ScaleMatrix, &Matrix);

    ASFixedRect PageRect;
    PDPageGetCropBox(pPDPage, &PageRect);

    ASFixedMatrixTransformRect(&PageRect, &Matrix, &PageRect);

    pEditor->m_ResWidth = (uint32_t)(abs(ASFixedRoundToInt16(PageRect.right) - ASFixedRoundToInt16(PageRect.left)));
    pEditor->m_ResHeight = (uint32_t)(abs(ASFixedRoundToInt16(PageRect.top) - ASFixedRoundToInt16(PageRect.bottom)));

#ifdef __HMR_DEBUG

    ASFixedMatrix InvertMatrix;
    ASFixedRect   UpdateRect;

    ASFixedMatrixInvert(&InvertMatrix, &Matrix);
    ASFixedMatrixTransformRect(&UpdateRect, &InvertMatrix, &PageRect);

    ASInt32 BufferSize = PDPageDrawContentsToMemory(pPDPage,
                                                    kPDPageDoLazyErase,
                                                    &Matrix, &UpdateRect,
                                                    0,
                                                    pEditor->m_csAtom,
                                                    8,
                                                    &PageRect,
                                                    NULL,
                                                    0,
                                                    NULL, NULL);

    //TR 251344 - The equation used to calculate the width and height of a page isn't known
    //precisely. At least, ensure that the size of the data that can be written by
    //PDPageDrawContentsToMemory is greater than the size of the data based on the calculated
    //width and height.
    HASSERT(BufferSize >= (ASInt32)(pEditor->m_ResWidth * pEditor->m_ResHeight * 3));
#endif

    HANDLER
#if USE_CPLUSPLUS_EXCEPTIONS_FOR_ASEXCEPTIONS
    Exception; // avoid C4101
#endif
    END_HANDLER

    if (pPDPage != 0)
        {
        PDPageRelease(pPDPage);
        }

    return pEditor.release();
    }

// For debugging.
void DisplayErrorAndWarnings(PDDoc pdDoc, ASErrorCode errCode)
{
	if(!pdDoc)	return;

	char errStr[250], pdDocErrStr[80], outputErrStr[350];
	ASErrorCode pdDocErrCode;

	ASInt32 warningCount = 0, errorCount = 0;
	ASBool match = false;
	
	ASInt32 numPDDocErr = PDDocGetNumErrors(pdDoc);
	for(ASInt32 i = 0; i < numPDDocErr; i++)
	{
		PDDocGetNthError(pdDoc, i, &pdDocErrCode, pdDocErrStr, sizeof(pdDocErrStr));
		ASGetErrorString(pdDocErrCode, errStr, sizeof(errStr));

		/* errStr can sometimes be a format string and it will give detailed error
		   information when combined with pdDocErrStr. */
		sprintf_s(outputErrStr, sizeof(outputErrStr), errStr, pdDocErrStr);

		if(pdDocErrCode == errCode)
		{
			fprintf(stderr, "[Error %d] %s, %s\n", errCode, outputErrStr, pdDocErrStr);
			match = true;
			errorCount++;
		}
		else
		{
			fprintf(stdout, "[Warning %d] %s, %s\n", pdDocErrCode, outputErrStr, pdDocErrStr);
			warningCount++;
		}
	}

// 	if(errCode && !match)
// 	{
// 		DisplayError(errCode);
// 		errorCount++;
// 	}
// 
// 	fprintf(stdout, "%d error(s), %d warning(s)\n", errorCount, warningCount);
}

//-----------------------------------------------------------------------------
// public
// Static method that reads an image block.
//-----------------------------------------------------------------------------
bool HRFPDFLibInterface::ReadBlock(Editor*             pi_pEditor,
                                    bool               pi_UseDrawContentToMem,
                                    uint32_t            pi_PosX,
                                    uint32_t            pi_PosY,
                                    uint32_t            pi_Width,
                                    uint32_t            pi_Height,
                                    const HFCPtr<HMDContext>& pi_rpContext,
                                    Byte*              po_pData)
    {
    bool        CommandStatus = true;
    PDPage       pPDPage;

    uint32_t bufferBytes = 0;

    DURING
        {
        pPDPage                = PDDocAcquirePage(pi_pEditor->m_pPDDoc, pi_pEditor->m_Page);
        pi_pEditor->m_Matrix.h = FloatToASFixed(pi_pEditor->m_OffsetX - (double)pi_PosX);
        pi_pEditor->m_Matrix.v = FloatToASFixed(pi_pEditor->m_OffsetY - (double)pi_PosY);

        ASUns32                       DrawFlags   = kPDPageDoLazyErase;
        uint32_t                     smoothingFlags = kPDPageDrawSmoothText | kPDPageDrawSmoothLineArt | kPDPageDrawSmoothImage;
        HFCPtr<HMDAnnotationIconsPDF> pAnnotationIconRastInfo;
        HFCPtr<HMDDrawOptionsPDF>     pDrawOptionsPDF;

        if (pi_rpContext != 0)
            {
            pAnnotationIconRastInfo = static_cast<HMDAnnotationIconsPDF*>(pi_rpContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_ANNOT_ICON_RASTERING_INFO).GetPtr());
            pDrawOptionsPDF = static_cast<HMDDrawOptionsPDF*>(pi_rpContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_PDF_DRAW_OPTIONS).GetPtr());
            }

        if ((pAnnotationIconRastInfo == 0) ||
            ((pAnnotationIconRastInfo != 0) &&
             (pAnnotationIconRastInfo->GetRasterization() == true)))
            {
            DrawFlags |= kPDPageUseAnnotFaces;
            }

        if(pDrawOptionsPDF != 0)
            {
            if(pDrawOptionsPDF->GetSmoothImage())
                smoothingFlags |= kPDPageDrawSmoothImage;
            else
                smoothingFlags &= ~kPDPageDrawSmoothImage;

            if(pDrawOptionsPDF->GetSmoothLineArt())
                smoothingFlags |= kPDPageDrawSmoothLineArt;
            else
                smoothingFlags &= ~kPDPageDrawSmoothLineArt;

            if(pDrawOptionsPDF->GetSmoothText())
                smoothingFlags |= kPDPageDrawSmoothText;
            else
                smoothingFlags &= ~kPDPageDrawSmoothText;
            }

        ASFixedMatrixInvert(&pi_pEditor->m_InvertMatrix, &pi_pEditor->m_Matrix);
        ASFixedMatrixTransformRect(&pi_pEditor->m_UpdateRect, &pi_pEditor->m_InvertMatrix, &pi_pEditor->m_TileRect);
        /* Create bitmap */
        bufferBytes = PDPageDrawContentsToMemory(pPDPage,
                                    DrawFlags | kPDPageSwapComponents,
                                    &pi_pEditor->m_Matrix,
                                    &pi_pEditor->m_UpdateRect,
                                    smoothingFlags,
                                    pi_pEditor->m_csAtom,
                                    8,
                                    &pi_pEditor->m_TileRect,
                                    (char*)po_pData,
                                    (ASInt32)PDF_TILE_SIZE_IN_BYTES,
                                    NULL,
                                    NULL);
        }
    HANDLER
        {
        #if USE_CPLUSPLUS_EXCEPTIONS_FOR_ASEXCEPTIONS
            Exception; // avoid C4101
        #endif

        memset(po_pData, 0, PDF_TILE_SIZE_IN_BYTES);

        CommandStatus = false;
        }
    END_HANDLER

    //DisplayErrorAndWarnings(pi_pEditor->m_pPDDoc, ERRORCODE);

    if (pPDPage != 0)
        {
        PDPageRelease(pPDPage);
        }

    return CommandStatus;
    }

//-----------------------------------------------------------------------------
// public
// Static method that reads an image extent (possibly many blocks).
//-----------------------------------------------------------------------------
bool HRFPDFLibInterface::ReadBlockByExtent(Editor*             pi_pEditor,
                                            bool               pi_UseDrawContentToMem,
                                            uint32_t            pi_PosX,
                                            uint32_t            pi_PosY,
                                            uint32_t            pi_Width,
                                            uint32_t            pi_Height,
                                            const HFCPtr<HMDContext>& pi_rpContext,
                                            Byte*              po_pData)
    {
    bool        CommandStatus = true;
    PDPage       pPDPage;

    DURING

    pPDPage = PDDocAcquirePage(pi_pEditor->m_pPDDoc, pi_pEditor->m_Page);

    pi_pEditor->m_Matrix.h = FloatToASFixed(pi_pEditor->m_OffsetX - (double)pi_PosX);
    pi_pEditor->m_Matrix.v = FloatToASFixed(pi_pEditor->m_OffsetY - (double)pi_PosY);

    ASUns32                       DrawFlags   = kPDPageDoLazyErase;
    uint32_t                     smoothingFlags = kPDPageDrawSmoothText | kPDPageDrawSmoothLineArt | kPDPageDrawSmoothImage;
    HFCPtr<HMDAnnotationIconsPDF> pAnnotationIconRastInfo;
    HFCPtr<HMDDrawOptionsPDF>     pDrawOptionsPDF;

    if (pi_rpContext != 0)
        {
        pAnnotationIconRastInfo = static_cast<HMDAnnotationIconsPDF*>(pi_rpContext->GetMetaDataContainer(
                                      HMDMetaDataContainer::HMD_ANNOT_ICON_RASTERING_INFO).GetPtr());

        pDrawOptionsPDF = static_cast<HMDDrawOptionsPDF*>(pi_rpContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_PDF_DRAW_OPTIONS).GetPtr());
        }

    if ((pAnnotationIconRastInfo == 0) ||
        ((pAnnotationIconRastInfo != 0) &&
         (pAnnotationIconRastInfo->GetRasterization() == true)))
        {
        DrawFlags |= kPDPageUseAnnotFaces;
        }

    if(pDrawOptionsPDF != 0)
        {
        if(pDrawOptionsPDF->GetSmoothImage())
            smoothingFlags |= kPDPageDrawSmoothImage;
        else
            smoothingFlags &= ~kPDPageDrawSmoothImage;

        if(pDrawOptionsPDF->GetSmoothLineArt())
            smoothingFlags |= kPDPageDrawSmoothLineArt;
        else
            smoothingFlags &= ~kPDPageDrawSmoothLineArt;

        if(pDrawOptionsPDF->GetSmoothText())
            smoothingFlags |= kPDPageDrawSmoothText;
        else
            smoothingFlags &= ~kPDPageDrawSmoothText;
        }

    ASFixedRect Rect;
    ASInt32 Width = pi_Width;
    ASInt32 Height = pi_Height;
    Rect.left = Int32ToFixed(0);
    Rect.bottom = Int32ToFixed(0);
    Rect.right = Int32ToFixed(Width);
    Rect.top = Int32ToFixed(Height);

    ASFixedMatrixInvert(&pi_pEditor->m_InvertMatrix, &pi_pEditor->m_Matrix);
    ASFixedMatrixTransformRect(&pi_pEditor->m_UpdateRect, &pi_pEditor->m_InvertMatrix, &Rect);
    if (pi_UseDrawContentToMem == true)
        {
        /* Create bitmap */
        PDPageDrawContentsToMemory(pPDPage,
                                   DrawFlags | kPDPageSwapComponents,
                                   &pi_pEditor->m_Matrix,
                                   &pi_pEditor->m_UpdateRect,
                                   smoothingFlags,
                                   pi_pEditor->m_csAtom,
                                   8,
                                   &Rect,
                                   (char*)po_pData,
                                   (ASInt32)pi_Width * pi_Height * 3,
                                   NULL,
                                   NULL);
        }
    else
        {
#if 0 //DMxx
        //Currently nothing has been done to set the image smoothing corresponding to
        //the application's desire because this code is no longer use.
        HASSERT(PDPrefGetAntialiasLevel() ==
                (kPDPageDrawSmoothImage | kPDPageDrawSmoothLineArt | kPDPageDrawSmoothText));

        HDC MemDC = CreateCompatibleDC(NULL);

        // Allocate enough memory for the BITMAPINFOHEADER and 256 RGBQUAD palette entries
        LPBITMAPINFO pBitmapInfo;
        pBitmapInfo = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD*)];
        memset(pBitmapInfo, 0, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD*));

        pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pBitmapInfo->bmiHeader.biWidth = pi_Width;
        pBitmapInfo->bmiHeader.biHeight = -((int32_t)pi_Height);
        pBitmapInfo->bmiHeader.biPlanes = 1;
        pBitmapInfo->bmiHeader.biBitCount = 24;
        pBitmapInfo->bmiHeader.biCompression = BI_RGB;
        pBitmapInfo->bmiHeader.biSizeImage = 0;
        pBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
        pBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
        pBitmapInfo->bmiHeader.biClrUsed = 0;
        pBitmapInfo->bmiHeader.biClrImportant = 0;

        HBITMAP pHBitmap = CreateDIBSection(MemDC,              // handle to DC
                                            pBitmapInfo,        // bitmap data
                                            DIB_RGB_COLORS,     // data type indicator
                                            (void**)&po_pData,  // bit values
                                            NULL,               // handle to file mapping object
                                            0);

        HASSERT(pHBitmap != 0);

        HBITMAP pOldBitmap = (HBITMAP)SelectObject(MemDC, pHBitmap);

        HASSERT(pOldBitmap != 0);

        //SetICMMode( MemDC, ICM_ON ); // Set colour correction on for this DC
        PDPageDrawContentsToWindowEx(pPDPage,
                                     NULL,
                                     MemDC,
                                     false,
                                     &pi_pEditor->m_Matrix,
                                     DrawFlags,
                                     &pi_pEditor->m_UpdateRect,
                                     NULL,
                                     NULL);


        GetDIBits(MemDC,             // handle to DC
                  pHBitmap,          // handle to bitmap
                  0,                 // first scan line to set
                  pi_Height,          // number of scan lines to copy
                  po_pData,           // array for bitmap bits
                  pBitmapInfo,       // bitmap data buffer
                  DIB_RGB_COLORS);   // RGB or palette index
#endif
        }
    HANDLER
        #if USE_CPLUSPLUS_EXCEPTIONS_FOR_ASEXCEPTIONS
            Exception; // avoid C4101
        #endif
    memset(po_pData, 0, pi_Width * pi_Height * 3);
    CommandStatus = false;
    END_HANDLER

    if (pPDPage != 0)
        {
        PDPageRelease(pPDPage);
        }

    return CommandStatus;
    }

//-----------------------------------------------------------------------------
// public
// Static method that returns the layers found on a given page, if any.
//-----------------------------------------------------------------------------
void HRFPDFLibInterface::GetLayers(const PDDoc        pi_pDoc,
                                   uint32_t           pi_Page,
                                   HFCPtr<HMDLayers>& po_rLayers)
    {
    ASBool           InitVisibleState;
    HMDLayerInfoPDF* pLayerInfoPDF;
    WChar            LayerKey[10];
    PDOCConfig       pDOCCfg = PDDocGetOCConfig(pi_pDoc);
    PDPage           Page = PDDocAcquirePage(pi_pDoc, pi_Page);
    PDOCG*           pDOCG = PDPageGetOCGs(Page);

    if (pDOCG != 0)
        {
        //No high level function returning the number of OCG for a given page has been
        //found and the function PDPageEnumOCGs doesn't seem to work
        //correctly
        int OCGind = 0;

        while (pDOCG[OCGind] != 0)
            {
            OCGind++;
            }

        HASSERT(OCGind < USHRT_MAX);

        po_rLayers = (HMDLayers*)new HMDLayersPDF();

        for (OCGind = 0; pDOCG[OCGind] != 0; OCGind++)
            {
            ASText LayerName = PDOCGGetName(pDOCG[OCGind]);
            PDOCGGetInitialState(pDOCG[OCGind], pDOCCfg, &InitVisibleState);

            errno_t ErrNo = _ultow_s(OCGind, LayerKey, 10, 10);
            HASSERT(ErrNo == 0);

            WString LayerNameStr((WChar*)ASTextGetUnicode(LayerName));

            pLayerInfoPDF = new HMDLayerInfoPDF(WString(LayerKey),
                                                InitVisibleState != 0,
                                                LayerNameStr);

            po_rLayers->AddLayer(pLayerInfoPDF);
            }
        ASfree(pDOCG);
        }
    else
        {
        po_rLayers = 0;
        }

    PDPageRelease(Page);
    }

#define DEFAULT_TOL_IN_PIXELS 6

//-----------------------------------------------------------------------------
// public
// Static method that returns the annotations found on a given page, if any.
//-----------------------------------------------------------------------------
void HRFPDFLibInterface::GetAnnotations(const PDDoc             pi_pDoc,
                                        uint32_t                pi_Page,
                                        HFCPtr<HMDAnnotations>& po_rAnnotations)
    {
    ASAtom                AnnotSubtypeAtom;
    PDAnnot               Annot;
    PDTextAnnot           TextAnnot;
    PDLinkAnnot           LinkAnnot;

    ASUns32               Flags = 0;
    ASFixedMatrix         Matrix;
    bool                 IsSelectableAnnotation = false;
    bool                 IsFilled               = false;
    bool                 IsSupported            = true;
    double               Tolerance              = DEFAULT_TOL_IN_PIXELS;
    double               PointsToPixelScaleFactor;
    HGF2DPosition         Position;
    HFCPtr<HVE2DVector>   p2DSelectionZone;
    HFCPtr<HGF2DCoordSys> pCoordSys(new HGF2DCoordSys());

    PDPage                Page = PDDocAcquirePage(pi_pDoc, pi_Page);
    ASInt32               NbAnnotations = PDPageGetNumAnnots(Page);

    if (NbAnnotations > 0)
        {
        GetTransfoFromUserToDeviceCS(Page,
                                     1.0,
                                     Matrix);

        PointsToPixelScaleFactor = fabs((double)ASFixedToFloat(Matrix.a));

        po_rAnnotations = new HRFAnnotationsPDF();

        for (unsigned short AnnotInd = 0; AnnotInd < NbAnnotations; AnnotInd++)
            {
            Annot = PDPageGetAnnot(Page, AnnotInd);

            TextAnnot = CastToPDTextAnnot(Annot);
            LinkAnnot = CastToPDLinkAnnot(Annot);

            if (PDAnnotIsValid(Annot))
                {
                AnnotSubtypeAtom = PDAnnotGetSubtype(Annot);

                string AnnotationType(ASAtomGetString(AnnotSubtypeAtom));

                CosObj AnnotObj = PDAnnotGetCosObj(Annot);

                if (BeStringUtilities::Stricmp(AnnotationType.c_str(), "Line") == 0)
                    {
                    Create2DVectForLineAnnot(AnnotObj,
                                             Matrix,
                                             pCoordSys,
                                             p2DSelectionZone);

                    Tolerance = GetToleranceFromBorderWidth(AnnotObj,
                                                            PointsToPixelScaleFactor);

                    IsSelectableAnnotation = true;
                    }
                else if ((BeStringUtilities::Stricmp(AnnotationType.c_str(), "Text") == 0)  ||
                         (BeStringUtilities::Stricmp(AnnotationType.c_str(), "Stamp") == 0) ||
                         (BeStringUtilities::Stricmp(AnnotationType.c_str(), "Caret") == 0) ||
                         (BeStringUtilities::Stricmp(AnnotationType.c_str(), "FreeText") == 0))
                    {
                    Create2DVectForRectAnnot(AnnotObj,
                                             Matrix,
                                             pCoordSys,
                                             p2DSelectionZone);

                    IsFilled = true;
                    IsSelectableAnnotation = true;
                    }
                else if ((BeStringUtilities::Stricmp(AnnotationType.c_str(), "Square") == 0) ||
                         (BeStringUtilities::Stricmp(AnnotationType.c_str(), "Circle") == 0))
                    {
                    Create2DVectForHoledAnnot(AnnotObj,
                                              Matrix,
                                              AnnotSubtypeAtom,
                                              PointsToPixelScaleFactor,
                                              pCoordSys,
                                              p2DSelectionZone,
                                              IsFilled,
                                              Tolerance);

                    IsSelectableAnnotation = true;
                    }
                else if ((BeStringUtilities::Stricmp(AnnotationType.c_str(), "Polygon") == 0) ||
                         (BeStringUtilities::Stricmp(AnnotationType.c_str(), "PolyLine") == 0))
                    {
                    Create2DVectForLinearAnnot(AnnotObj,
                                               Matrix,
                                               AnnotSubtypeAtom,
                                               pCoordSys,
                                               p2DSelectionZone,
                                               IsFilled);

                    Tolerance = GetToleranceFromBorderWidth(AnnotObj,
                                                            PointsToPixelScaleFactor);

                    IsSelectableAnnotation = true;
                    }
                else if ((BeStringUtilities::Stricmp(AnnotationType.c_str(), "Highlight") == 0) ||
                         (BeStringUtilities::Stricmp(AnnotationType.c_str(), "Underline") == 0) ||
                         (BeStringUtilities::Stricmp(AnnotationType.c_str(), "Squiggly") == 0)  ||
                         (BeStringUtilities::Stricmp(AnnotationType.c_str(), "StrikeOut") == 0))
                    {
                    Create2DVectForTxtMarkupAnnot(AnnotObj,
                                                  Matrix,
                                                  pCoordSys,
                                                  p2DSelectionZone);

                    IsFilled               = true;
                    IsSelectableAnnotation = true;
                    }
                else if (BeStringUtilities::Stricmp(AnnotationType.c_str(), "Ink") == 0)
                    {
                    Create2DVectForInkAnnot(AnnotObj,
                                            Matrix,
                                            pCoordSys,
                                            p2DSelectionZone);

                    Tolerance = GetToleranceFromBorderWidth(AnnotObj,
                                                            PointsToPixelScaleFactor);

                    IsSelectableAnnotation = true;
                    }
                else //Unsupported annotations
                    if ((BeStringUtilities::Stricmp(AnnotationType.c_str(), "Link") == 0)           ||
                        (BeStringUtilities::Stricmp(AnnotationType.c_str(), "FileAttachment") == 0) ||
                        (BeStringUtilities::Stricmp(AnnotationType.c_str(), "Sound") == 0)          ||
                        (BeStringUtilities::Stricmp(AnnotationType.c_str(), "Movie") == 0)          ||
                        (BeStringUtilities::Stricmp(AnnotationType.c_str(), "Widget") == 0)         ||
                        (BeStringUtilities::Stricmp(AnnotationType.c_str(), "Screen") == 0)         ||
                        (BeStringUtilities::Stricmp(AnnotationType.c_str(), "PrinterMark") == 0)    ||
                        (BeStringUtilities::Stricmp(AnnotationType.c_str(), "TrapNet") == 0)        ||
                        (BeStringUtilities::Stricmp(AnnotationType.c_str(), "Watermark") == 0)      ||
                        (BeStringUtilities::Stricmp(AnnotationType.c_str(), "3D") == 0))
                        {
                        CosObj  AnnotationRect = CosDictGetKeyString(AnnotObj, "Rect");

                        if (CosObjGetType(AnnotationRect) != CosNull)
                            {
                            Create2DVectForRectAnnot(AnnotObj,
                                                     Matrix,
                                                     pCoordSys,
                                                     p2DSelectionZone);

                            IsSupported            = false;
                            IsFilled               = true;
                            IsSelectableAnnotation = true;
                            }
                        }

                if (IsSelectableAnnotation)
                    {
                    //The selection zone should inherit from one of these classes
                    //See HRFAnnotationInfoPDF::GetSelectZonePoints()
                    HASSERT(p2DSelectionZone->IsCompatibleWith(HVE2DDisjointedComplexLinear::CLASS_ID) ||
                            p2DSelectionZone->IsCompatibleWith(HVE2DLinear::CLASS_ID)                  ||
                            p2DSelectionZone->IsCompatibleWith(HVE2DShape::CLASS_ID));

                    WString AnnotMsg;
                    WString AnnotType;

                    if (IsSupported == true)
                        {
                        //CosObj AnnotationObjPopup = CosDictGetKeyString(AnnotObj, "Popup");

                        //CosType Type = CosObjGetType(AnnotationObjPopup);

                        //PDAnnot PopUp = PDAnnotFromCosObj(AnnotationObjPopup);
                        PDAnnot PopUp = PDAnnotFromCosObj(AnnotObj);

                        char Buffer[10000];

                        PDTextAnnotGetContents(CastToPDTextAnnot(PopUp), Buffer, 10000);

                        BeStringUtilities::CurrentLocaleCharToWChar( AnnotMsg,Buffer);
                        }

                    BeStringUtilities::CurrentLocaleCharToWChar( AnnotType,AnnotationType.c_str());

                    po_rAnnotations->AddAnnotation(new HRFAnnotationInfoPDF(AnnotMsg,
                                                                            (HFCPtr<HVE2DVector>&)p2DSelectionZone,
                                                                            AnnotType,
                                                                            IsSupported,
                                                                            Tolerance,
                                                                            IsFilled));

                    Tolerance              = DEFAULT_TOL_IN_PIXELS;
                    IsSelectableAnnotation = false;
                    IsSupported            = true;
                    IsFilled               = false;
                    }

                //AnnotationInfo.m_Flags = PDAnnotGetFlags(Annot);

#ifdef __HMR_DEBUG
                WString FlagsMsg;

                if (Flags & pdAnnotInvisible)
                    {
                    FlagsMsg = WString(L"AnnotInvisible");
                    }
                else if (Flags & pdAnnotHidden)
                    {
                    FlagsMsg += WString(L":AnnotHidden");
                    }
                else if (Flags & pdAnnotPrint)
                    {
                    FlagsMsg += WString(L":AnnotPrint");
                    }
                else if (Flags & pdAnnotNoZoom)
                    {
                    FlagsMsg += WString(L":AnnotNoZoom");
                    }
                else if (Flags & pdAnnotNoRotate)
                    {
                    FlagsMsg += WString(L":AnnotNoRotate");
                    }
                else if (Flags & pdAnnotNoView)
                    {
                    FlagsMsg += WString(L":AnnotNoView");
                    }
                else if (Flags & pdAnnotReadOnly)
                    {
                    FlagsMsg += WString(L":AnnotReadOnly");
                    }
                else if (Flags & pdAnnotLock)
                    {
                    FlagsMsg += WString(L":AnnotLock");
                    }
                else if (Flags & pdAnnotToggleNoView)
                    {
                    FlagsMsg += WString(L":AnnotToggleNoView");
                    }
                else if (Flags & pdAnnotSequenceAdjust)
                    {
                    FlagsMsg += WString(L":AnnotSequenceAdjust");
                    }
#endif
                //      IsDefaultColorUsed = PDAnnotGetColor(Annot, Color);

                //          m_pAnnotationInfoList->push_back(AnnotationInfo);
                }
            }
        }

    PDPageRelease(Page);
    }

//-----------------------------------------------------------------------------
// public
// Static method that returns true if some operations are restricted for
// the given PDF
//-----------------------------------------------------------------------------
bool HRFPDFLibInterface::IsOpRestrictedPDF(const PDDoc pi_pDoc)
    {
    static PDPermReqOpr OpRelatedPerms[] =
        {   /** generic */
        PDPermReqOprCreate,
        /** Delete */
        PDPermReqOprDelete,
        /** Modify */
        PDPermReqOprModify,
        /** Copy */
        PDPermReqOprCopy,
        /** For Accessibility use */
        PDPermReqOprAccessible,
        /** For doc or page, selecting (not copying) text or graphics */
        PDPermReqOprSelect,
        /** For document open */
        PDPermReqOprOpen,
        /** for doc, Regular printing */
        PDPermReqOprPrintHigh,
        /** for doc, low quality printing */
        PDPermReqOprPrintLow,
        /** Form fill-in or Sign existing field*/
        PDPermReqOprFillIn,
        /** Rotate */
        PDPermReqOprRotate,
        /** Crop */
        PDPermReqOprCrop,
        /** For summarize notes */
        PDPermReqOprSummarize,
        /** Insert */
        PDPermReqOprInsert,
        /** for page */
        PDPermReqOprReplace,
        /** for page */
        PDPermReqOprReorder,
        /** For doc */
        PDPermReqOprFullSave,
        /** For notes & Image */
        PDPermReqOprImport,
        /** For notes. ExportPS should check print */
        PDPermReqOprExport,
        /* Acrobat 5.1 additions.  */
        /** Submit forms outside of the browser */
        PDPermReqOprSubmitStandalone,
        /** allow form to spawn template page*/
        PDPermReqOprSpawnTemplate,
        /** This should be always the last item  */
        /* Acrobat 7.0 additions */
        /** For annots & form, enabling online functionality */
        PDPermReqOprOnline,
        /** For annots, enabling summary view of annots in Reader */
        PDPermReqOprSummaryView,
        /** For forms, enables form appearance to be rendered as plaintext barcode */
        PDPermReqOprBarcodePlaintext
        };

    bool   IsOpRestricted = false;
    unsigned short NbOfOpPerms    = sizeof(OpRelatedPerms) / sizeof(PDPermReqOpr);

    DURING
    for (unsigned short OpPermInd = 0; OpPermInd < NbOfOpPerms; OpPermInd++)
        {
        PDPermReqStatus PerReqStatus = PDDocPermRequest(pi_pDoc,
                                                        PDPermReqObjDoc,
                                                        OpRelatedPerms[OpPermInd],
                                                        NULL);

        if (PerReqStatus == PDPermReqDenied)
            {
            IsOpRestricted = true;
            break;
            }
        }
    HANDLER
#if USE_CPLUSPLUS_EXCEPTIONS_FOR_ASEXCEPTIONS
    Exception; // avoid C4101
#endif
    //According to the PDF document, numerous exceptions can be thrown
    //during a permission request call.
    HASSERT(0);
    END_HANDLER

    return IsOpRestricted;
    }

//-----------------------------------------------------------------------------
// local
// This structure is used to search for a particular named child object.
//-----------------------------------------------------------------------------
struct CosObjQueryInfo
    {
    CosObjQueryInfo()
        {
        ObjFound = CosNewNull();
        }

    char* pCosObjectName;
    CosObj ObjFound;
    };

//-----------------------------------------------------------------------------
// local
// This callback function is used to count the number of child nodes of
// a parent node.
//-----------------------------------------------------------------------------
ASBool ObjCounterEnumProc(CosObj obj, CosObj value, void* pClientData)
    {
    HPRECONDITION(pClientData != 0);

    (*(uint32_t*)pClientData)++;

    return true;
    }

//-----------------------------------------------------------------------------
// local
// This callback function is called for each child node of a parent node.
//-----------------------------------------------------------------------------
ASBool ObjQueryEnumProc(CosObj obj, CosObj value, void* pClientData)
    {
    HPRECONDITION(pClientData != 0);

    ASBool           ContinueEnum = true;
    CosObjQueryInfo* pQuery       = (CosObjQueryInfo*)pClientData;
    //ASAtom           Name         = CosNameValue(obj);
    const char*      pObjName     = ASAtomGetString(CosNameValue(obj));

    if (strcmp(pQuery->pCosObjectName, pObjName) == 0)
        {
        if (CosObjEqual(value, CosNewNull()))
            {
            pQuery->ObjFound = obj;
            }
        else
            {
            pQuery->ObjFound = value;
            }

        ContinueEnum = false;
        }

    return ContinueEnum;
    }

//-----------------------------------------------------------------------------
// private
// Static method that returns the geocoding info for a given PDF page.
//-----------------------------------------------------------------------------
bool HRFPDFLibInterface::GetGeocodingAndReferenceInfo(const PDDoc                    pi_pDoc,
                                                       uint32_t                       pi_Page,
                                                       uint32_t                       pi_RasterizePageWidth,
                                                       uint32_t                       pi_RasterizePageHeight,
                                                       GeoCoordinates::BaseGCSPtr&    po_rpGeocoding,
                                                       HFCPtr<HGF2DTransfoModel>&     po_rpGeoreference)
    {
    HPRECONDITION(po_rpGeocoding == 0);
    HPRECONDITION(po_rpGeoreference == 0);

    PDPage          Page       = PDDocAcquirePage(pi_pDoc, pi_Page);
    CosObj          PageCosObj = PDPageGetCosObj(Page);
    CosObjQueryInfo Query;
    CosObj          PageRotationCosObj;
    double          PageRotation = 0.0;


    // Get the page rotation
    vector<char*> PageRotationCosObjPath;
    PageRotationCosObjPath.push_back("Rotate");
    if (GetCosObj(PageCosObj, PageRotationCosObjPath, PageRotationCosObj))
        {
        ASFixed pageRotationFixed;
        pageRotationFixed = CosFixedValue(PageRotationCosObj);
        PageRotation = ASFixedToFloat(pageRotationFixed);
        }



    Query.pCosObjectName = "VP";

    //The geocoding information can be found under the following node :
    //trailer/Root/Pages/Kids/0/VP/0/Measure
    CosObjEnum(PageCosObj, ObjQueryEnumProc, &Query);

    if (CosObjEqual(Query.ObjFound, CosNewNull()) == false)
        {
        CosObj        VPCosObj = Query.ObjFound;

        //Query.ObjFound
        ASTArraySize NbOfMaps = CosArrayLength(Query.ObjFound);

        if ((NbOfMaps == 1))
//         &&
//            (IsMapZoneBBoxEqualToPageCropBox(Page, CosArrayGet(Query.ObjFound, 0)) == true))
            {

            CosObj        WKTCosObj;
            vector<char*> WKTCosObjPath;

            WKTCosObjPath.push_back("0");
            WKTCosObjPath.push_back("Measure");
            WKTCosObjPath.push_back("GCS");
            WKTCosObjPath.push_back("WKT");

            if (GetCosObj(Query.ObjFound, WKTCosObjPath, WKTCosObj))
                {
                HASSERT(CosObjGetType(WKTCosObj) == CosString);

                ASTCount Dummy;
                //Put the WKT in a string because the char pointer returned by
                //CosStringValue can be deleted at any time by the PDF library.
                string WKT(CosStringValue(WKTCosObj, &Dummy));

                CreateGeocodingFromWKT(WKT, po_rpGeocoding);
                }

            WKTCosObjPath.pop_back();
            WKTCosObjPath.pop_back();

            CosObj                    MeasureCosObj;
            HFCPtr<HGF2DTransfoModel> pTransfoModel;
            bool                      CosObjFound = GetCosObj(Query.ObjFound,
                                                              WKTCosObjPath,
                                                              MeasureCosObj);
            HASSERT(CosObjFound == true);

            if ((po_rpGeocoding != 0) && (po_rpGeocoding->IsValid() != 0))
                {
                double BBoxXMin = 0.0;
                double BBoxXMax = pi_RasterizePageWidth;
                double BBoxYMin = 0.0;
                double BBoxYMax = pi_RasterizePageHeight;
                double MediaBoxXMin = 0.0;
                double MediaBoxXMax = pi_RasterizePageWidth;
                double MediaBoxYMin = 0.0;
                double MediaBoxYMax = pi_RasterizePageHeight;

                CosObj          BBoxCosObj;
                CosObj          MediaBoxCosObj;
                CosObj          XMinCosObj;
                CosObj          XMaxCosObj;
                CosObj          YMinCosObj;
                CosObj          YMaxCosObj;

                // Obtain the viewport size
                vector<char*> BoxCosObjPath;

                // Obtain the viewport size
                BoxCosObjPath.push_back("0");
                BoxCosObjPath.push_back("BBox");
                if (GetCosObj(VPCosObj, BoxCosObjPath, BBoxCosObj))
                    {
                    HASSERT(CosObjGetType(BBoxCosObj) == CosArray);

                    XMinCosObj = CosArrayGet(BBoxCosObj, 0);
                    BBoxXMin = CosDoubleValue(XMinCosObj);

                    YMinCosObj = CosArrayGet(BBoxCosObj, 1);
                    BBoxYMin = CosDoubleValue(YMinCosObj);

                    XMaxCosObj = CosArrayGet(BBoxCosObj, 2);
                    BBoxXMax = CosDoubleValue(XMaxCosObj);

                    YMaxCosObj = CosArrayGet(BBoxCosObj, 3);
                    BBoxYMax = CosDoubleValue(YMaxCosObj);
                    }

                BoxCosObjPath.clear();

                BoxCosObjPath.push_back("MediaBox");
                if (GetCosObj(PageCosObj, BoxCosObjPath, MediaBoxCosObj))
                    {
                    ASFixed MediaBoxXMinFixed;
                    ASFixed MediaBoxYMinFixed;
                    ASFixed MediaBoxXMaxFixed;
                    ASFixed MediaBoxYMaxFixed;
                    HASSERT(CosObjGetType(MediaBoxCosObj) == CosArray);

                    XMinCosObj = CosArrayGet(MediaBoxCosObj, 0);
                    MediaBoxXMinFixed = CosFixedValue(XMinCosObj);
                    MediaBoxXMin = ASFixedToFloat(MediaBoxXMinFixed);

                    YMinCosObj = CosArrayGet(MediaBoxCosObj, 1);
                    MediaBoxYMinFixed = CosFixedValue(YMinCosObj);
                    MediaBoxYMin = ASFixedToFloat(MediaBoxYMinFixed);

                    XMaxCosObj = CosArrayGet(MediaBoxCosObj, 2);
                    MediaBoxXMaxFixed = CosFixedValue(XMaxCosObj);
                    MediaBoxXMax = ASFixedToFloat(MediaBoxXMaxFixed);

                    YMaxCosObj = CosArrayGet(MediaBoxCosObj, 3);
                    MediaBoxYMaxFixed = CosFixedValue(YMaxCosObj);
                    MediaBoxYMax = ASFixedToFloat(MediaBoxYMaxFixed);
                    }


                GetGeoreference(MeasureCosObj,
                                pi_RasterizePageWidth,
                                pi_RasterizePageHeight,
                                MediaBoxXMin,
                                MediaBoxYMin,
                                MediaBoxXMax,
                                MediaBoxYMax,
                                BBoxXMin,
                                BBoxYMin,
                                BBoxXMax,
                                BBoxYMax,
                                PageRotation,
                                po_rpGeocoding.get(),
                                po_rpGeoreference);
                }
            }
        else
            {
// #error What to do with this: Being here indicates that there is no georeference or that it is not understood
//            po_rpGeocoding = new HRFGeoTiffKeysUtility();
//            po_rpGeocoding->AddKey(GTModelType,
//                                   (UInt32)TIFFGeo_UserDefined);
//            po_rpGeocoding->SetErrorCode(HRFGeoTiffKeysUtility::GEOCODING_PDF_MULTI_MODELS);
            }
        }

    if (po_rpGeocoding == 0)
        {
        GetGeocodingReferenceFromImage(Page,
                                       pi_RasterizePageWidth,
                                       pi_RasterizePageHeight,
                                       po_rpGeocoding,
                                       po_rpGeoreference);
        }

    //The geocoding information is useless without some geo-refence information
    if ((po_rpGeoreference == 0) &&
        (po_rpGeocoding != 0) &&
        (po_rpGeocoding->IsValid()))
        {
        po_rpGeocoding = 0;
        }

    PDPageRelease(Page);

    return po_rpGeocoding != 0;
    }

//-----------------------------------------------------------------------------
// private
// Static method that returns the geocoding info for a given PDF page.
//-----------------------------------------------------------------------------       
void HRFPDFLibInterface::GetDimensionForDWGUnderlay(const PDDoc                pi_pDoc,
                                                    uint32_t                  pi_Page,
                                                    double&                    po_xDimension, 
                                                    double&                    po_yDimension) 
    {               
    PDPage          Page       = PDDocAcquirePage(pi_pDoc, pi_Page);
    CosObj          PageCosObj = PDPageGetCosObj(Page);
    CosObjQueryInfo Query;

    //Currently the translation is not computed.
    CosObj        mediaBoxCosObj;
    vector<char*> CosObjPath;
    
    CosObjPath.push_back("MediaBox");
                                                    
    bool mediaBoxFound = GetCosObj(PageCosObj, CosObjPath, mediaBoxCosObj);

    HASSERT(mediaBoxFound == true);
    HASSERT(CosObjGetType(mediaBoxCosObj) == CosArray);

    CosObj rectangleCoordCosObj;
          
    rectangleCoordCosObj = CosArrayGet(mediaBoxCosObj, 0);        
    double mediaBoxLowX = ASFixedToFloat(CosFixedValue(rectangleCoordCosObj));
    
    rectangleCoordCosObj = CosArrayGet(mediaBoxCosObj, 1);        
    double mediaBoxLowY = ASFixedToFloat(CosFixedValue(rectangleCoordCosObj));
    
    rectangleCoordCosObj = CosArrayGet(mediaBoxCosObj, 2);        
    double mediaBoxHighX = ASFixedToFloat(CosFixedValue(rectangleCoordCosObj));
    
    rectangleCoordCosObj = CosArrayGet(mediaBoxCosObj, 3);        
    double mediaBoxHighY = ASFixedToFloat(CosFixedValue(rectangleCoordCosObj));
    
    po_xDimension = fabs(mediaBoxHighX - mediaBoxLowX);
    po_yDimension = fabs(mediaBoxHighY - mediaBoxLowY);        
        
    Query.pCosObjectName = "VP";

    //The geocoding information can be found under the following node :
    //trailer/Root/Pages/Kids/0/VP/0/Measure
    CosObjEnum(PageCosObj, ObjQueryEnumProc, &Query);

    CosObj       VPArrayCosObj;
    ASTArraySize NbOfVPs = 0;

    //default scale; don't know why 72 but seems to be also in HRFPDFFile::s_dpiConvertScaleFactor
    double scaleX = 1.0/72.0;
    double scaleY = 1.0/72.0;
    if (CosObjEqual(Query.ObjFound, CosNewNull()) == false)
        {
        VPArrayCosObj = Query.ObjFound;

        //Query.ObjFound
        NbOfVPs = CosArrayLength(Query.ObjFound);
        
        if (NbOfVPs == 1)
            {            
            CosObj        measureCosObj;            

            CosObjPath.clear();
            CosObjPath.push_back("0");
            CosObjPath.push_back("Measure");
                        
            if (GetCosObj(VPArrayCosObj, CosObjPath, measureCosObj))
                {
                CosObjPath.clear();
                CosObjPath.push_back("Subtype");  

                CosObj      subTypeCosObj;                
                const char* pSubType = 0;
    
                if (GetCosObj(measureCosObj, CosObjPath, subTypeCosObj))
                    {                           
                    HASSERT(CosObjGetType(subTypeCosObj) == CosName);
                                        
                    pSubType = ASAtomGetString(CosNameValue(subTypeCosObj));

                    HASSERT(pSubType != 0);
                    }

                if ((BeStringUtilities::Stricmp(pSubType, "") == 0) ||
                    (BeStringUtilities::Stricmp(pSubType, "RL") == 0))                    
                    {                                           
                    CosObjPath.clear();
                    CosObjPath.push_back("X");  
                    CosObjPath.push_back("0");  
                    CosObjPath.push_back("C");

                    CosObj scaleCosObj;                

                    if (GetCosObj(measureCosObj, CosObjPath, scaleCosObj))
                        {                        
                        scaleX = ASFixedToFloat(CosFixedValue(scaleCosObj));                        
                        }

                    CosObjPath.clear();
                    CosObjPath.push_back("Y");  
                    CosObjPath.push_back("0");  
                    CosObjPath.push_back("C");

                    if (GetCosObj(measureCosObj, CosObjPath, scaleCosObj))
                        {                        
                        scaleY = ASFixedToFloat(CosFixedValue(scaleCosObj));                        
                        }
                    else
                        {
                        scaleY = scaleX;                    
                        }

                    }                
                }
            }                   
        }

    po_xDimension *= scaleX;
    po_yDimension *= scaleY;        

    // Get the page rotation    
    CosObjPath.clear();
    CosObjPath.push_back("Rotate");

    CosObj PageRotationCosObj;

    if (GetCosObj(PageCosObj, CosObjPath, PageRotationCosObj))
        {               
        double pageRotation = ASFixedToFloat(CosFixedValue(PageRotationCosObj));

        // Normalized rotation will not work with values greater than 360 or smaller than -360 ... it is not a limitation but a feature (we do not want to encourage stupid values)
        // We will support 360 though
        double normalizedRotation = (pageRotation < 0.0 ? pageRotation + 360 : pageRotation);

        // Validate normalized rotation (valid values are discrete values but we nevertheless apply a small 1 degree tolerance to accound for double error)
        if (HDOUBLE_EQUAL (normalizedRotation, 0.0, 0.05) ||
            HDOUBLE_EQUAL (normalizedRotation, 90, 0.05) ||
            HDOUBLE_EQUAL (normalizedRotation, 180, 0.05) ||
            HDOUBLE_EQUAL (normalizedRotation, 270, 0.05) ||
            HDOUBLE_EQUAL (normalizedRotation, 360.0, 0.05))
            {           
            if (HDOUBLE_EQUAL (normalizedRotation, 90, 0.05) || HDOUBLE_EQUAL (normalizedRotation, 270, 0.05))
                {
                double tempXDimension = po_xDimension;
                po_xDimension = po_yDimension;
                po_yDimension = tempXDimension;
                }
            }
        }
  
    PDPageRelease(Page); 
    }
  
//-----------------------------------------------------------------------------
// private
//-----------------------------------------------------------------------------
void HRFPDFLibInterface::CreateGeocodingFromWKT(const string&                  pi_rWKT,
                                                GeoCoordinates::BaseGCSPtr&    po_rpGeocoding)
    {
    HPRECONDITION(pi_rWKT.empty() == false);
    HPRECONDITION(po_rpGeocoding == 0);

    WString TempWKT;
    BeStringUtilities::CurrentLocaleCharToWChar( TempWKT,pi_rWKT.c_str());

    //The flavor of the WKT stored in the PDF is not known yet, so unknown is used.
    GeoCoordinates::BaseGCSPtr pBaseGcs = GeoCoordinates::BaseGCS::CreateGCS();
    if(SUCCESS != pBaseGcs->InitFromWellKnownText (NULL, NULL, GeoCoordinates::BaseGCS::WktFlavorESRI, TempWKT.c_str()))
        {
        pBaseGcs = GeoCoordinates::BaseGCS::CreateGCS();
        HFCPtr<HCPGeoTiffKeys> pGeoKeys = HRFGdalUtilities::ConvertOGCWKTtoGeotiffKeys(pi_rWKT.c_str());
        if(SUCCESS == pBaseGcs->InitFromGeoTiffKeys(NULL, NULL, *pGeoKeys))
            po_rpGeocoding = pBaseGcs;
        }
    }

//-----------------------------------------------------------------------------
// private
//-----------------------------------------------------------------------------
bool HRFPDFLibInterface::GetGeocodingReferenceFromImage(const PDPage&                   pi_rPage,
                                                         uint32_t                      pi_RasterizePageWidth,
                                                         uint32_t                      pi_RasterizePageHeight,
                                                         GeoCoordinates::BaseGCSPtr&   po_rpGeocoding,
                                                         HFCPtr<HGF2DTransfoModel>&     po_rpGeoreference)
    {
    HPRECONDITION(po_rpGeocoding == 0);

    CosObj        ImgXObjectCosObj = CosNewNull();
    CosObj        FoundCosObj;
    vector<char*> CosObjPath;
    string        WKT;
    CosObj        PageCosObj(PDPageGetCosObj(pi_rPage));

    CosObjPath.push_back("Resources");
    CosObjPath.push_back("XObject");

    if (GetCosObj(PageCosObj, CosObjPath, FoundCosObj) == true)
        {
        uint32_t NbXObject  = 0;
        ASBool  EnumResult = CosObjEnum(FoundCosObj, ObjCounterEnumProc, &NbXObject);

        HASSERT(EnumResult);

        if (NbXObject == 1)
            {
            CosObjPath.clear();
            CosObjPath.push_back("Im0");

            GetCosObj(FoundCosObj, CosObjPath, ImgXObjectCosObj);
            }
        }

    //There is one XObject in the document, ensure that it is an image
    if (CosObjEqual(ImgXObjectCosObj, CosNewNull()) == false)
        {
        CosObjPath.clear();
        CosObjPath.push_back("Subtype");

        if (GetCosObj(ImgXObjectCosObj, CosObjPath, FoundCosObj) == true)
            {
            HASSERT(CosObjGetType(FoundCosObj) == CosName);
            const char* pSubType(ASAtomGetString(CosNameValue(FoundCosObj)));

            if (strcmp(pSubType, "Image") != 0)
                {
                ImgXObjectCosObj = CosNewNull();
                }
            }
        }

    //Check if some geocoding information is available of the image
    if (CosObjEqual(ImgXObjectCosObj, CosNewNull()) == false)
        {
        CosObjPath.clear();
        CosObjPath.push_back("Measure");
        CosObjPath.push_back("GCS");
        CosObjPath.push_back("WKT");

        if (GetCosObj(ImgXObjectCosObj, CosObjPath, FoundCosObj) == true)
            {
            HASSERT(CosObjGetType(FoundCosObj) == CosString);

            ASTCount Dummy;
            //Put the WKT in a string because the char pointer returned by
            //CosStringValue can be deleted at any time by the PDF library.
            WKT = CosStringValue(FoundCosObj, &Dummy);
            }
        }

    //If a WKT has been found, ensure that the image covers the whole page.
    if (WKT.empty() == false)
        {
        CosObjPath.clear();
        CosObjPath.push_back("Contents");

        if (GetCosObj(PageCosObj, CosObjPath, FoundCosObj) == true)
            {
            HASSERT(CosObjGetType(FoundCosObj) == CosStream);

            ASTArraySize    StreamLength(CosStreamLength(FoundCosObj));
            HAutoPtr<char> pPageContentStream(new char[StreamLength]);
            ASStm           PageContentStream(CosStreamOpenStm(FoundCosObj, cosOpenFiltered));

            ASStmRead(pPageContentStream.get(), 1, StreamLength, PageContentStream);

            HFCPtr<HFCBuffer>   pBuffer(new HFCBuffer((Byte*)pPageContentStream.release(), StreamLength));

            HFCMemoryLineStream MemLineStream(L"", '\n', pBuffer);

            //The content stream is considered valid (i.e. : describing the placement of one and only
            //one image only if it has the form below.
            /*
            q
            213.4372101 0 0 213.4372101 42.4425354 564.8208313 cm
            /Im0 Do
            Q
            */
            if (MemLineStream.GetNbLines() == 4)
                {
                WString ReadLine[4];

                MemLineStream.ReadLine(0, ReadLine[0]);
                MemLineStream.ReadLine(1, ReadLine[1]);
                MemLineStream.ReadLine(2, ReadLine[2]);
                MemLineStream.ReadLine(3, ReadLine[3]);

                if ((ReadLine[0] == L"q") &&
                    (ReadLine[2] == L"/Im0 Do") &&
                    (ReadLine[3] == L"Q"))
                    {
                    double Tx,  Ty;
                    double Sx,  Sy;
                    double Sxy, Syx;

                    int32_t NbElement = swscanf(ReadLine[1].c_str(),
                                               L"%lf %lf %lf %lf %lf %lf",
                                               &Sx, &Syx, &Sxy, &Sy, &Tx, &Ty);

                    ASFixedRect PageRect;

                    PDPageGetCropBox(pi_rPage, &PageRect);

                    double CropBoxWidth = fabs(ASFixedToFloat(PageRect.left) - ASFixedToFloat(PageRect.right));
                    double CropBoxHeight = fabs(ASFixedToFloat(PageRect.top) - ASFixedToFloat(PageRect.bottom));

                    if ((NbElement == 6) &&
                        (Syx == 0) && (Sxy == 0) &&
                        (Tx == 0)  && (Ty == 0)  &&
                        (HDOUBLE_EQUAL(Sx, CropBoxWidth, 0.2) == true) &&
                        (HDOUBLE_EQUAL(Sy, CropBoxHeight, 0.2) == true))
                        {
                        CreateGeocodingFromWKT(WKT, po_rpGeocoding);
                        }
                    }
                }
            }
        }

    if (po_rpGeocoding != 0)
        {
        CosObjPath.clear();
        CosObjPath.push_back("Measure");

        bool CosObjFound = GetCosObj(ImgXObjectCosObj, CosObjPath, FoundCosObj);

        //If the code execution gets here, the Measure CosObj must exist.
        HASSERT(CosObjFound == true);

        GetGeoreference(FoundCosObj,
                        pi_RasterizePageWidth,
                        pi_RasterizePageHeight,
                        0.0,
                        0.0,
                        pi_RasterizePageWidth,
                        pi_RasterizePageHeight,
                        0.0,
                        0.0,
                        pi_RasterizePageWidth,
                        pi_RasterizePageHeight,
                        0.0,
                        po_rpGeocoding.get(),
                        po_rpGeoreference);
        }

    return (po_rpGeocoding != 0) && (po_rpGeoreference != 0);
    }

//-----------------------------------------------------------------------------
// private
//-----------------------------------------------------------------------------
bool HRFPDFLibInterface::GetGeoreference(const CosObj&                  pi_rMeasureCosObj,
                                          uint32_t                     pi_RasterizePageWidth,
                                          uint32_t                     pi_RasterizePageHeight,
                                          double                        pi_MediaMinX,
                                          double                        pi_MediaMinY,
                                          double                        pi_MediaMaxX,
                                          double                        pi_MediaMaxY,
                                          double                        pi_VPMinX,
                                          double                        pi_VPMinY,
                                          double                        pi_VPMaxX,
                                          double                        pi_VPMaxY,
                                          double                        pi_PageRotation,
                                          GeoCoordinates::BaseGCSCP     pi_rpGeocoding,
                                          HFCPtr<HGF2DTransfoModel>&    po_rpGeoreference)
    {
    HPRECONDITION(pi_rpGeocoding != 0);
    HPRECONDITION(po_rpGeoreference == 0);

    bool         Result = false;
    CosObj        GPTSCosObj;
    CosObj        LPTSCosObj;
    vector<char*> CosObjPath;

    double viewPortHeight;
    double viewPortWidth;
    double rotatedXMin;
    double rotatedYMin;

    // Normalized rotation will not work with values greater than 360 or smaller than -360 ... it is not a limitation but a feature (we do not want to encourage stupid values)
    // We will support 360 though
    double normalizedRotation = (pi_PageRotation < 0.0 ? pi_PageRotation + 360 : pi_PageRotation);

    // Validate normalized rotation (valid values are discrete values but we nevertheless apply a small 1 degree tolerance to accound for double error)
    if (!HDOUBLE_EQUAL (normalizedRotation, 0.0, 0.05) &&
        !HDOUBLE_EQUAL (normalizedRotation, 90, 0.05) &&
        !HDOUBLE_EQUAL (normalizedRotation, 180, 0.05) &&
        !HDOUBLE_EQUAL (normalizedRotation, 270, 0.05) &&
        !HDOUBLE_EQUAL (normalizedRotation, 360.0, 0.05))
        {
        // Invalid rotation value ... we get out right away
        return false;
        }


    double scalingX;
    double scalingY;
    if (HDOUBLE_EQUAL (normalizedRotation, 0.0, 0.05) || HDOUBLE_EQUAL (normalizedRotation, 180, 0.05) || HDOUBLE_EQUAL (normalizedRotation, 360.0, 0.05))
        {
        scalingX = pi_RasterizePageWidth / pi_MediaMaxX;
        scalingY = pi_RasterizePageHeight / pi_MediaMaxY;

        rotatedXMin = pi_VPMinX * scalingX;
        rotatedYMin = (pi_MediaMaxY - pi_VPMaxY) * scalingY; //pi_VPMinY * scalingY;
        viewPortWidth = (pi_VPMaxX - pi_VPMinX) * scalingX;
        viewPortHeight = (pi_VPMaxY - pi_VPMinY) * scalingY;
        }
    else
        {
        scalingX = pi_RasterizePageWidth / pi_MediaMaxY;
        scalingY = pi_RasterizePageHeight / pi_MediaMaxX;

        rotatedXMin = pi_VPMinY * scalingX;
        rotatedYMin = pi_VPMinX * scalingY;
        viewPortWidth = (pi_VPMaxY - pi_VPMinY) * scalingX;
        viewPortHeight = (pi_VPMaxX - pi_VPMinX) * scalingY;
        }

    CosObjPath.push_back("GPTS");

    if (GetCosObj(pi_rMeasureCosObj, CosObjPath, GPTSCosObj))
        {
        HASSERT(CosObjGetType(GPTSCosObj) == CosArray);

        CosObjPath.clear();
        CosObjPath.push_back("LPTS");

        if (GetCosObj(pi_rMeasureCosObj, CosObjPath, LPTSCosObj))
            {
            HASSERT(CosObjGetType(LPTSCosObj) == CosArray);
            HASSERT(CosArrayLength(GPTSCosObj) == CosArrayLength(LPTSCosObj));

            ASTArraySize           NbTiePoints            = CosArrayLength(GPTSCosObj) / 2;
            HArrayAutoPtr<double> pTiePointArray(new double[NbTiePoints * 6]);
            uint32_t              TiePointArrayCoordInd  = 0;
            uint32_t              TiePointCoordInd       = 0;
            double                GeoPt[3];        //0 : Longitude, 1 : Latitude, 2 : Elevation
            double                CartesianPt[3];  //0 : X,         1 : Y,        2 : Z
            StatusInt              ConvertionStatus = SUCCESS;

            //Altitude is set to 0
            GeoPt[2] = 0;

            for (uint32_t TiePointInd = 0; TiePointInd < (uint32_t)NbTiePoints; TiePointInd++)
                {
                GeoPt[0]      = CosDoubleValue(CosArrayGet(GPTSCosObj, TiePointCoordInd + 1));
                GeoPt[1]      = CosDoubleValue(CosArrayGet(GPTSCosObj, TiePointCoordInd));

                double XLogical = CosDoubleValue(CosArrayGet(LPTSCosObj, TiePointCoordInd++));
                double YLogical = CosDoubleValue(CosArrayGet(LPTSCosObj, TiePointCoordInd++));

                double XPageBeforeRotation;
                double YPageBeforeRotation;
                XPageBeforeRotation = XLogical * (pi_VPMaxX - pi_VPMinX) + pi_VPMinX;
                YPageBeforeRotation = YLogical * (pi_VPMaxY - pi_VPMinY) + pi_VPMinY;

                double X = 0.0;
                double Y = 0.0;
                if (HDOUBLE_EQUAL (normalizedRotation, 0.0, 0.05) || HDOUBLE_EQUAL (normalizedRotation, 360.0, 0.05))
                    {
                    X = XPageBeforeRotation;
                    Y = pi_MediaMaxY - YPageBeforeRotation;
                    }
                else if (HDOUBLE_EQUAL (normalizedRotation, 90.0, 0.05))
                    {
                    X = YPageBeforeRotation;
                    Y = XPageBeforeRotation;
                    }
                else if (HDOUBLE_EQUAL (normalizedRotation, 180, 0.05))
                    {
                    X = pi_MediaMaxX - XPageBeforeRotation;
                    Y = YPageBeforeRotation;
                    }
                else if (HDOUBLE_EQUAL (normalizedRotation, 270.0, 0.05))
                    {
                    X = pi_MediaMaxY - YPageBeforeRotation;
                    Y = pi_MediaMaxX - XPageBeforeRotation;
                    }
                else
                    {
                    // Rotation is unknown and unsupported ...
                    return false;
                    }


                pTiePointArray[TiePointArrayCoordInd++] = X * scalingX;
                pTiePointArray[TiePointArrayCoordInd++] = Y * scalingY;


                pTiePointArray[TiePointArrayCoordInd++] = 0;


                // If the GCS is already geographic ... we bypass the calls as often PDF will contain 90 or -90 latitudes
                // and this will result in a conversion problem.

                bool  isGeographic = false;
                if ((pi_rpGeocoding != NULL) && (pi_rpGeocoding->IsValid()) && (pi_rpGeocoding->GetBaseGCS() != NULL))
                    isGeographic = (!pi_rpGeocoding->IsProjected());

                if (isGeographic)
                    {
                    CartesianPt[0] = GeoPt[0];
                    CartesianPt[1] = GeoPt[1];
                    CartesianPt[2] = GeoPt[2];
                    }
                else
                    {
                    //Physical coordinate
                    ConvertionStatus = pi_rpGeocoding->GetCartesianFromLatLong(CartesianPt, GeoPt);
                    }

                // Conversion status can be negative (general error), 1(coordinate outside user domain) or 2(coordinate outside mathematical domain)
                // We can leave 1 go through but the other errors cannot be recoverred from. Either the geocoding is invalid or
                // the reprojection package is badly installed or configured.
                HASSERT(ConvertionStatus == SUCCESS);


                pTiePointArray[TiePointArrayCoordInd++] = CartesianPt[0];
                pTiePointArray[TiePointArrayCoordInd++] = CartesianPt[1];
                pTiePointArray[TiePointArrayCoordInd++] = 0;
                }

            double pMatrix[4][4];
            HRESULT Res = GetTransfoMatrixFromScaleAndTiePts(pMatrix,
                                                             (unsigned short)(NbTiePoints * 6),
                                                             pTiePointArray,
                                                             0,
                                                             0);

            if (Res == H_SUCCESS)
                {
                HFCMatrix<3, 3> TheMatrix;

                TheMatrix[0][0] = pMatrix[0][0];
                TheMatrix[0][1] = pMatrix[0][1];
                TheMatrix[0][2] = pMatrix[0][3];
                TheMatrix[1][0] = pMatrix[1][0];
                TheMatrix[1][1] = pMatrix[1][1];
                TheMatrix[1][2] = pMatrix[1][3];
                TheMatrix[2][0] = pMatrix[3][0];
                TheMatrix[2][1] = pMatrix[3][1];
                TheMatrix[2][2] = pMatrix[3][3];

                po_rpGeoreference = new HGF2DProjective(TheMatrix);


                // Get the simplest model possible.
                HFCPtr<HGF2DTransfoModel> pTempTransfoModel = po_rpGeoreference->CreateSimplifiedModel();

                if (pTempTransfoModel != 0)
                    po_rpGeoreference = pTempTransfoModel;

                RasterFileGeocodingPtr pFileGeocoding(RasterFileGeocoding::Create(pi_rpGeocoding->Clone().get()));
                po_rpGeoreference = pFileGeocoding->TranslateToMeter(po_rpGeoreference, 1.0, false, NULL);

                Result = true;
                }
            }
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// private
//-----------------------------------------------------------------------------
bool HRFPDFLibInterface::GetCosObj(const CosObj&  pi_rSearchRootCosObj,
                                    vector<char*>& pi_rpRelativeCosObjPath,
                                    CosObj&        po_rFoundCosObj)
    {
    CosObjQueryInfo         Query;
    CosObj                  CurrentNode = pi_rSearchRootCosObj;
    vector<char*>::iterator NodeIter    = pi_rpRelativeCosObjPath.begin();
    vector<char*>::iterator NodeIterEnd = pi_rpRelativeCosObjPath.end();

    while (NodeIter != NodeIterEnd)
        {
        if ((CosObjGetType(CurrentNode) == CosDict) ||
            (CosObjGetType(CurrentNode) == CosStream))
            {
            //Search the dictionary of the stream
            if (CosObjGetType(CurrentNode) == CosStream)
                {
                CurrentNode = CosStreamDict(CurrentNode);
                }

            Query.ObjFound       = CosNewNull();
            Query.pCosObjectName = *NodeIter;
            CosObjEnum(CurrentNode, ObjQueryEnumProc, &Query);

            if (CosObjEqual(Query.ObjFound, CosNewNull()))
                {
                break;
                }
            else
                {
                CurrentNode = Query.ObjFound;
                }
            }
        else if (CosObjGetType(CurrentNode) == CosArray)
            {
            int32_t ArrayInd = atoi(*NodeIter);

            if ((0 <= ArrayInd) && (ArrayInd < CosArrayLength(CurrentNode)))
                {
                CurrentNode = CosArrayGet(CurrentNode, ArrayInd);
                }
            else
                {
                break;
                }
            }
#ifdef __HMR_DEBUG
        else
            {
            HASSERT(0);
            }
#endif

        NodeIter++;
        }

    if (NodeIter == NodeIterEnd)
        {
        po_rFoundCosObj = Query.ObjFound;
        }

    return NodeIter == NodeIterEnd;
    }

//-----------------------------------------------------------------------------
// private
//-----------------------------------------------------------------------------
bool HRFPDFLibInterface::IsMapZoneBBoxEqualToPageCropBox(PDPage& pi_rPage,
                                                          CosObj& pi_rMapZoneCosObj)
    {
    bool         IsEqual = false;
    ASFixedRect   PageRect;
    vector<char*> BBoxCosObjPath;
    CosObj        MapZoneBBoxCosObj;

    BBoxCosObjPath.push_back("BBox");

    if (GetCosObj(pi_rMapZoneCosObj, BBoxCosObjPath, MapZoneBBoxCosObj))
        {
        HASSERT(CosObjGetType(MapZoneBBoxCosObj) == CosArray);
        HASSERT(CosArrayLength(MapZoneBBoxCosObj) == 4);

        CosObj MapZoneLeft   = CosArrayGet(MapZoneBBoxCosObj, 0);
        CosObj MapZoneBottom = CosArrayGet(MapZoneBBoxCosObj, 1);
        CosObj MapZoneRight  = CosArrayGet(MapZoneBBoxCosObj, 2);
        CosObj MapZoneTop    = CosArrayGet(MapZoneBBoxCosObj, 3);

        HASSERT(CosObjGetType(MapZoneBottom) == CosFixed);
        HASSERT(CosObjGetType(MapZoneLeft)   == CosFixed);
        HASSERT(CosObjGetType(MapZoneTop)    == CosFixed);
        HASSERT(CosObjGetType(MapZoneRight)  == CosFixed);

        float MapZoneLeftVal   = ASFixedToFloat(CosFixedValue(MapZoneLeft));
        float MapZoneBottomVal = ASFixedToFloat(CosFixedValue(MapZoneBottom));
        float MapZoneRightVal  = ASFixedToFloat(CosFixedValue(MapZoneRight));
        float MapZoneTopVal    = ASFixedToFloat(CosFixedValue(MapZoneTop));

        PDPageGetCropBox(pi_rPage, &PageRect);

        //There seems to be a problem of precision with the bounding box of map, because even when
        //specifiying the whole page as the neatline (map delimiting zone), the bounding box doesn't
        //equals the crop box of the page while the function for getting the bounding box of a
        //page throws. So let's use 1 as the precision value for comparaison purpose.
        if (HDOUBLE_EQUAL(MapZoneLeftVal, ASFixedToFloat(PageRect.left), 2)     &&
            HDOUBLE_EQUAL(MapZoneBottomVal, ASFixedToFloat(PageRect.bottom), 2) &&
            HDOUBLE_EQUAL(MapZoneRightVal, ASFixedToFloat(PageRect.right), 2)   &&
            HDOUBLE_EQUAL(MapZoneTopVal, ASFixedToFloat(PageRect.top), 2))
            {
            IsEqual = true;
            }
        }

    return IsEqual;

    }

//-----------------------------------------------------------------------------
// private
// Static method that returns the transformation matrix between the user
// coordinate system and the device coordinate system.
//-----------------------------------------------------------------------------
void HRFPDFLibInterface::GetTransfoFromUserToDeviceCS(PDPage         pi_PDPage,
                                                      double        pi_ResolutionFactor,
                                                      ASFixedMatrix& po_rTransfoMatrix)
    {
    PDPageGetFlippedMatrix(pi_PDPage, &po_rTransfoMatrix);

    ASFixedMatrix ScaleMatrix;
    ScaleMatrix.a = FloatToASFixed(pi_ResolutionFactor * DPI_CONVERT_SCALE_FACTOR);
    ScaleMatrix.b = 0;
    ScaleMatrix.c = 0;
    ScaleMatrix.d = FloatToASFixed(pi_ResolutionFactor * DPI_CONVERT_SCALE_FACTOR);
    ScaleMatrix.h = 0;
    ScaleMatrix.v = 0;

    ASFixedMatrixConcat(&po_rTransfoMatrix, &ScaleMatrix, &po_rTransfoMatrix);
    }

//-----------------------------------------------------------------------------
// private
// Static method that extracts a location from an array of 2D coordinates.
//-----------------------------------------------------------------------------
void HRFPDFLibInterface::Get2DCoordFromCosArray(CosObj&        pi_r1DCoorArray,
                                                uint32_t      pi_2DCoorIndInArray,
                                                ASFixedMatrix& pi_rTransfoMatrix,
                                                HGF2DPosition& po_rPosition)
    {
    CosObj       CoordObj;
    ASFixedPoint Coord;
    ASFixedPoint TransCoord;

    CoordObj = CosArrayGet(pi_r1DCoorArray, pi_2DCoorIndInArray);
    Coord.h = CosFixedValue(CoordObj);
    pi_2DCoorIndInArray++;
    CoordObj = CosArrayGet(pi_r1DCoorArray, pi_2DCoorIndInArray);
    Coord.v = CosFixedValue(CoordObj);
    pi_2DCoorIndInArray++;

    ASFixedMatrixTransform(&TransCoord,
                           &pi_rTransfoMatrix,
                           &Coord);

    po_rPosition.SetX((double)ASFixedToFloat(TransCoord.h));
    po_rPosition.SetY((double)ASFixedToFloat(TransCoord.v));
    }

//-----------------------------------------------------------------------------
// private
// Get a line tolerance from the border width specified for the given
// annotation
//-----------------------------------------------------------------------------
double HRFPDFLibInterface::GetToleranceFromBorderWidth(const CosObj& pi_rAnnotObj,
                                                        double       pi_PointsToPixelScaleFactor,
                                                        bool*        po_pHasBorder)
    {
    double Tolerance = DEFAULT_TOL_IN_PIXELS;

    po_pHasBorder != 0 ? *po_pHasBorder = false : false;

    CosObj BorderStyleObj = CosDictGetKeyString(pi_rAnnotObj, "BS");

    if (CosObjGetType(BorderStyleObj) != CosNull)
        {
        CosObj WidthObj = CosDictGetKeyString(BorderStyleObj , "W");

        int32_t Width;

        //The width can be zero which means according to the PDF
        //library documentation that no border is drawn
        if ((CosObjGetType(WidthObj) != CosNull) &&
            ((Width = CosIntegerValue(WidthObj)) > 0))
            {
            //The border width is given in points. Multiply by 2 because the
            //rectangle given by the PDF library is greater than the actual
            //drawn rectangle.
            Tolerance = CosIntegerValue(WidthObj) * pi_PointsToPixelScaleFactor;

            po_pHasBorder != 0 ? *po_pHasBorder = true : false;
            }
        }

    return Tolerance;
    }

//---------------------------------------------------------------------------------------------------
// private
// Create a 2D vector object representing the selection zone for a given
// line annotation
//---------------------------------------------------------------------------------------------------
void HRFPDFLibInterface::Create2DVectForLineAnnot(const CosObj&                pi_rAnnotObj,
                                                  ASFixedMatrix&               pi_rTransfoMatrix,
                                                  const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                                                  HFCPtr<HVE2DVector>&         po_rp2DSelectionZone)
    {
    HGF2DPosition Pos1;
    HGF2DPosition Pos2;

    CosObj  AnnotationL = CosDictGetKeyString(pi_rAnnotObj, "L");

    HASSERT(CosArrayLength(AnnotationL) == 4);
    HASSERT(CosObjGetType(AnnotationL) == CosArray);

    //Lower left coordinate or starting point
    Get2DCoordFromCosArray(AnnotationL,
                           0,
                           pi_rTransfoMatrix,
                           Pos1);

    //Upper right coordinate or ending point
    Get2DCoordFromCosArray(AnnotationL,
                           2,
                           pi_rTransfoMatrix,
                           Pos2);

    po_rp2DSelectionZone = new HVE2DSegment(Pos1,
                                            Pos2,
                                            pi_rpCoordSys);
    }

//---------------------------------------------------------------------------------------------------
// private
// Create a 2D vector object representing the selection zone for a given
// line annotation
//---------------------------------------------------------------------------------------------------
void HRFPDFLibInterface::Create2DVectForRectAnnot(const CosObj&                pi_rAnnotObj,
                                                  ASFixedMatrix&               pi_rTransfoMatrix,
                                                  const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                                                  HFCPtr<HVE2DVector>&         po_rp2DSelectionZone)
    {
    HGF2DPosition Position;
    double       X;
    double       Y;

    CosObj  AnnotationRect = CosDictGetKeyString(pi_rAnnotObj, "Rect");

    HASSERT(CosArrayLength(AnnotationRect) == 4);
    HASSERT(CosObjGetType(AnnotationRect) == CosArray);

    //Lower left coordinate or starting point
    Get2DCoordFromCosArray(AnnotationRect,
                           0,
                           pi_rTransfoMatrix,
                           Position);

    X = Position.GetX();
    Y = Position.GetY();

    //Upper right coordinate or ending point
    Get2DCoordFromCosArray(AnnotationRect,
                           2,
                           pi_rTransfoMatrix,
                           Position);

    double XMin = X < Position.GetX() ? X : Position.GetX();
    double YMin = Y < Position.GetY() ? Y : Position.GetY();
    double XMax = X > Position.GetX() ? X : Position.GetX();
    double YMax = Y > Position.GetY() ? Y : Position.GetY();

    po_rp2DSelectionZone = new HVE2DRectangle(XMin,
                                              YMin,
                                              XMax,
                                              YMax,
                                              pi_rpCoordSys);
    }

//-----------------------------------------------------------------------------
// private
// Get a line tolerance from the border width specified for the given
// annotation
//-----------------------------------------------------------------------------
void HRFPDFLibInterface::Create2DVectForLinearAnnot(const CosObj&                pi_rAnnotObj,
                                                    ASFixedMatrix&               pi_rTransfoMatrix,
                                                    ASAtom&                      pi_rAnnotSubtypeAtom,
                                                    const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                                                    HFCPtr<HVE2DVector>&         po_rp2DSelectionZone,
                                                    bool&                       po_rIsFilled)
    {
    int32_t     VertexInd = 0;
    HGF2DPosition Pos1;
    HGF2DPosition Pos2;
    CosObj        Vertices = CosDictGetKeyString(pi_rAnnotObj, "Vertices");

    po_rIsFilled        = false;
    po_rp2DSelectionZone = new HVE2DComplexLinear(pi_rpCoordSys);

    if (CosArrayLength(Vertices) >= 4)
        {
        Get2DCoordFromCosArray(Vertices,
                               0,
                               pi_rTransfoMatrix,
                               Pos1);
        VertexInd += 2;
        }

    while (VertexInd < CosArrayLength(Vertices))
        {
        Get2DCoordFromCosArray(Vertices,
                               VertexInd,
                               pi_rTransfoMatrix,
                               Pos2);

        VertexInd += 2;

        ((HFCPtr<HVE2DComplexLinear>&)po_rp2DSelectionZone)->
        AppendLinearPtrSCS(new HVE2DSegment(Pos1,
                                            Pos2,
                                            pi_rpCoordSys));

        Pos1 = Pos2;
        }

    //Close the polygon
    if (BeStringUtilities::Strnicmp(ASAtomGetString(pi_rAnnotSubtypeAtom), "Polyg", 5) == 0)
        {
        HGF2DLocation StartLocation = ((HFCPtr<HVE2DComplexLinear>&)po_rp2DSelectionZone)->GetStartPoint();

        Pos2.SetX(StartLocation.GetX());
        Pos2.SetY(StartLocation.GetY());

        ((HFCPtr<HVE2DComplexLinear>&)po_rp2DSelectionZone)->
        AppendLinearPtrSCS(new HVE2DSegment(Pos1,
                                            Pos2,
                                            pi_rpCoordSys));

        CosObj ITObj = CosDictGetKeyString(pi_rAnnotObj, "IT");

        if ((CosObjGetType(ITObj) != CosNull) &&
            (BeStringUtilities::Stricmp(ASAtomGetString(CosNameValue(ITObj)), "PolygonCloud") == 0))
            {
            po_rp2DSelectionZone = new HVE2DPolygon(*((HFCPtr<HVE2DComplexLinear>&)po_rp2DSelectionZone));
            po_rIsFilled = true;
            }
        }
    }

//-----------------------------------------------------------------------------
// private
// Get a line tolerance from the border width specified for the given
// annotation
//-----------------------------------------------------------------------------
void HRFPDFLibInterface::Create2DVectForHoledAnnot(const CosObj&                pi_rAnnotObj,
                                                   ASFixedMatrix&               pi_rTransfoMatrix,
                                                   ASAtom&                      pi_rAnnotSubtypeAtom,
                                                   double                      pi_PointsToPixelScaleFactor,
                                                   const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                                                   HFCPtr<HVE2DVector>&         po_rp2DSelectionZone,
                                                   bool&                       po_rIsFilled,
                                                   double&                     po_rTolerance)
    {
    HGF2DPosition Position;
    double       X;
    double       Y;

    CosObj  AnnotationRect = CosDictGetKeyString(pi_rAnnotObj, "Rect");

    HASSERT(CosArrayLength(AnnotationRect) == 4);
    HASSERT(CosObjGetType(AnnotationRect) == CosArray);

    //Lower left coordinate or starting point
    Get2DCoordFromCosArray(AnnotationRect,
                           0,
                           pi_rTransfoMatrix,
                           Position);

    X = Position.GetX();
    Y = Position.GetY();

    //Upper right coordinate or ending point
    Get2DCoordFromCosArray(AnnotationRect,
                           2,
                           pi_rTransfoMatrix,
                           Position);

    //CosObj  BorderStyleObj = CosDictGetKeyString(pi_rAnnotObj, "BS");
    bool   HasBorder      = false;
    double CentralizationValue;

    //If a fill color is specified, the annotation can be selected when
    //the cursor is in it.
    po_rIsFilled = HasFillColor(pi_rAnnotObj);

    po_rTolerance = GetToleranceFromBorderWidth(pi_rAnnotObj,
                                                pi_PointsToPixelScaleFactor,
                                                &HasBorder);

    //According to the PDF library documentation the border of an ellipse or a rectangle is drawn
    //inside the "Rect" dictionary entry. So move the rect toward the inside for half a border
    //width in pixel coordinate (tolerance).
    if (HasBorder == true)
        {
        CentralizationValue = po_rTolerance / 2;
        }
    else
        {
        CentralizationValue = 0;
        }

    if (BeStringUtilities::Strnicmp(ASAtomGetString(pi_rAnnotSubtypeAtom), "Ci", 2) == 0)
        {
        po_rp2DSelectionZone = new HVE2DEllipse(HVE2DRectangle(X + CentralizationValue,
                                                               Position.GetY() + CentralizationValue,
                                                               Position.GetX() - CentralizationValue,
                                                               Y - CentralizationValue,
                                                               pi_rpCoordSys));
        }
    else
        {
        po_rp2DSelectionZone = new HVE2DRectangle(X + CentralizationValue,
                                                  Position.GetY() + CentralizationValue,
                                                  Position.GetX() - CentralizationValue,
                                                  Y - CentralizationValue,
                                                  pi_rpCoordSys);
        }
    }

//-----------------------------------------------------------------------------
// private
// Get a line tolerance from the border width specified for the given
// annotation
//-----------------------------------------------------------------------------
void HRFPDFLibInterface::Create2DVectForTxtMarkupAnnot(const CosObj&                pi_rAnnotObj,
                                                       ASFixedMatrix&               pi_rTransfoMatrix,
                                                       const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                                                       HFCPtr<HVE2DVector>&         po_rp2DSelectionZone)
    {
    CosObj Quads = CosDictGetKeyString(pi_rAnnotObj, "QuadPoints");

    HASSERT(CosArrayLength(Quads) % 8 == 0);

    uint32_t             CoordInd = 0;
    uint32_t             NbQuads = CosArrayLength(Quads);
    HGF2DPosition        PosBuffer1;
    HGF2DPosition        PosBuffer2;
    HGF2DPosition        PosBuffer3;
    HGF2DPosition        PosBuffer4;

    po_rp2DSelectionZone = new HVE2DComplexShape(pi_rpCoordSys);

    while (CoordInd < NbQuads)
        {
        HVE2DComplexLinear Segments(pi_rpCoordSys);

        Get2DCoordFromCosArray(Quads,
                               CoordInd,
                               pi_rTransfoMatrix,
                               PosBuffer1);

        CoordInd += 2;

        Get2DCoordFromCosArray(Quads,
                               CoordInd,
                               pi_rTransfoMatrix,
                               PosBuffer2);

        CoordInd += 2;

        Get2DCoordFromCosArray(Quads,
                               CoordInd,
                               pi_rTransfoMatrix,
                               PosBuffer3);

        CoordInd += 2;

        Get2DCoordFromCosArray(Quads,
                               CoordInd,
                               pi_rTransfoMatrix,
                               PosBuffer4);

        CoordInd += 2;

        if (PosBuffer1 != PosBuffer2)
            {
            Segments.AppendLinearPtrSCS(new HVE2DSegment(PosBuffer1,
                                                         PosBuffer2,
                                                         pi_rpCoordSys));
            }

        if (PosBuffer2 != PosBuffer4)
            {
            Segments.AppendLinearPtrSCS(new HVE2DSegment(PosBuffer2,
                                                         PosBuffer4,
                                                         pi_rpCoordSys));
            }

        if (PosBuffer4 != PosBuffer3)
            {
            Segments.AppendLinearPtrSCS(new HVE2DSegment(PosBuffer4,
                                                         PosBuffer3,
                                                         pi_rpCoordSys));
            }

        if (PosBuffer3 != PosBuffer1)
            {
            Segments.AppendLinearPtrSCS(new HVE2DSegment(PosBuffer3,
                                                         PosBuffer1,
                                                         pi_rpCoordSys));
            }

        //TR 241780 - Assert - Sometime, there are only two different points making
        //up the quad. For now, ignore those one dimension shapes.
        if (Segments.GetNumberOfLinears() > 2)
            {
            HVE2DPolygon QuadShape(Segments);

            po_rp2DSelectionZone = ((HFCPtr<HVE2DShape>&)po_rp2DSelectionZone)->UnifyShapeSCS(QuadShape);
            }
        }
    }

//-----------------------------------------------------------------------------
// private
// Get a line tolerance from the border width specified for the given
// annotation
//-----------------------------------------------------------------------------
void HRFPDFLibInterface::Create2DVectForInkAnnot(const CosObj&                pi_rAnnotObj,
                                                 ASFixedMatrix&               pi_rTransfoMatrix,
                                                 const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                                                 HFCPtr<HVE2DVector>&         po_rp2DSelectionZone)
    {
    CosObj StrokedPaths = CosDictGetKeyString(pi_rAnnotObj, "InkList");

    HASSERT(CosObjGetType(StrokedPaths) == CosArray);

    uint32_t                   NbPaths = CosArrayLength(StrokedPaths);
    CosObj                     StrokedPathObj;
    HGF2DPosition              PosBuffer;
    HAutoPtr<HVE2DPolySegment> pPolyline;

    po_rp2DSelectionZone = new HVE2DDisjointedComplexLinear(pi_rpCoordSys);

    for (uint32_t PathInd = 0; PathInd < NbPaths; PathInd++)
        {
        StrokedPathObj = CosArrayGet(StrokedPaths, PathInd);

        uint32_t NbCoords = CosArrayLength(StrokedPathObj);

        pPolyline = new HVE2DPolySegment(pi_rpCoordSys);

        for (uint32_t CoordInd = 0; CoordInd < NbCoords; CoordInd += 2)
            {
            Get2DCoordFromCosArray(StrokedPathObj,
                                   CoordInd,
                                   pi_rTransfoMatrix,
                                   PosBuffer);

            pPolyline->AppendPoint(HGF2DLocation(PosBuffer.GetX(),
                                                 PosBuffer.GetY(),
                                                 pi_rpCoordSys));
            }

        ((HFCPtr<HVE2DDisjointedComplexLinear>&)po_rp2DSelectionZone)->AppendLinearPtrSCS(pPolyline.release());
        }
    }

//-----------------------------------------------------------------------------
// private
// Return true if the annotation has an appearance stream and that a fill color
// is specified in that stream, otherwise false.
//-----------------------------------------------------------------------------
bool HRFPDFLibInterface::HasFillColor(const CosObj& pi_rAnnotObj)
    {
    bool HasFillColor = false;

    CosObj APObj = CosDictGetKeyString(pi_rAnnotObj, "AP");

    if (CosObjGetType(APObj) != CosNull)
        {
        CosObj NObj = CosDictGetKeyString(APObj, "N");

        if (CosObjGetType(NObj) != CosNull)
            {
            HASSERT(CosObjGetType(NObj) == CosStream);

            int32_t StreamSize = CosStreamLength(NObj);
            HASSERT(StreamSize != 0);

            HAutoPtr<char> pBuffer(new char[StreamSize + 1]);
            pBuffer[StreamSize] = '\0';

            ASStm    Stm = CosStreamOpenStm(NObj, cosOpenFiltered);
            ASTCount Count = ASStmRead(pBuffer.get(), sizeof(char), StreamSize, Stm);
            HASSERT(Count == StreamSize);

            if (strstr(pBuffer.get(), "rg") != NULL)
                {
                HasFillColor = true;
                }
            }
        }

    return HasFillColor;
    }


uint32_t HRFPDFLibInterface::CountPages(const PDDoc pi_pDoc)
    {
    return PDDocGetNumPages(pi_pDoc);
    }

void HRFPDFLibInterface::ReleaseEditor(HRFPDFLibInterface::Editor** pi_ppEditor)
    {
    if (*pi_ppEditor != 0)
        {
        delete (*pi_ppEditor);
        (*pi_ppEditor) = 0;
        }
    }

void HRFPDFLibInterface::SetLayerVisibility(PDDoc                      pi_pDoc,
                                                   uint32_t                   pi_Page,
                                                   HFCPtr<HMDVolatileLayers>& pi_rpVolatileLayers)
    {
    PDPage  Page   = PDDocAcquirePage(pi_pDoc, pi_Page);
    PDOCG*  pDOGCs = PDPageGetOCGs(Page);

    //For performance concern, the set layer visibility function shouldn't
    //be called if the file contains no layer
    HASSERT(pDOGCs != 0);

    PDOCContext pContext = PDDocGetOCContext(pi_pDoc);

    for (unsigned short LayerInd = 0; LayerInd < pi_rpVolatileLayers->GetNbVolatileLayers(); LayerInd++)
        {
        int LayerKey = BeStringUtilities::Wtoi(pi_rpVolatileLayers->GetLayerInfo(LayerInd)->GetKeyName().c_str());
        if(LayerKey >= 0)
            PDOCGSetCurrentState(pDOGCs[LayerKey], pContext, pi_rpVolatileLayers->GetVolatileLayerInfo(LayerInd)->GetVisibleState());
        }

    ASfree(pDOGCs);

    PDPageRelease(Page);
    }


#include <adobe/Source/PDFLInitHFT.c>
#include <adobe/Source/PDFLInitCommon.c>

#endif // IPP_HAVE_PDF_SUPPORT 