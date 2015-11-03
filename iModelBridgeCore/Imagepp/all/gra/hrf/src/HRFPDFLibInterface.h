//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPDFLibInterface.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HMDAnnotations.h>
#include <Imagepp/all/h/HMDContext.h>
#include <Imagepp/all/h/HMDLayers.h>
#include <Imagepp/all/h/HMDVolatileLayers.h>
#include <Imagepp/all/h/HGF2DCoordSys.h> 
#include <Imagepp/all/h/HGF2DPosition.h>

class HVE2DVector;


#include <PdfLibInitializer/PdfLibInitializerManager.h>

#ifdef _WIN32
    #include <Shtypes.h>    // Needed for some PDF include files
#endif

#include <adobe/Headers/ASCalls.h>
#include <adobe/Headers/ASExpT.h>
#include <adobe/Headers/ASExtraCalls.h>
#include <adobe/Headers/PDCalls.h>
#include <adobe/Headers/PDFLCalls.h>
#include <adobe/Headers/PERCalls.h>
#include <adobe/Headers/PEWCalls.h>
#include <adobe/Headers/CosCalls.h>

#define PDF_TILE_WIDTH_HEIGHT   1024
#define PDF_TILE_SIZE_IN_BYTES  PDF_TILE_WIDTH_HEIGHT*PDF_TILE_WIDTH_HEIGHT*3

#define DPI_CONVERT_SCALE_FACTOR ((double)(96.0 / 72.0))

class HRFPDFLibInterface
    {
public :

    struct Editor
        {
        PDDoc         m_pPDDoc;
        uint32_t      m_Page;
        ASFixedMatrix m_Matrix;
        ASFixedMatrix m_InvertMatrix;
        ASAtom        m_csAtom;
        ASFixedRect   m_TileRect;
        ASFixedRect   m_UpdateRect;
        double       m_OffsetX;
        double       m_OffsetY;
        uint32_t      m_ResWidth;
        uint32_t      m_ResHeight;
        double       m_ScaleFactor;
        };


    static uint32_t CountPages       (const PDDoc pi_pDoc);

    static void     PageSize         (const PDDoc pi_pDoc,
                                      uint32_t    pi_Page,
                                      uint32_t&     po_rWidth,
                                      uint32_t&     po_rHeight,
                                      double&    po_rDPI);

    static void     GetMaxResolutionSize(const PDDoc pi_pDoc, uint32_t pi_Page, double dpiConvertScaleFactor, uint32_t& po_maxResSize);


    static Editor*  CreateEditor     (const PDDoc pi_pDoc,
                                      uint32_t    pi_Page,
                                      double     pi_ResolutionFactor);

    static void     ReleaseEditor    (HRFPDFLibInterface::Editor** pi_ppEditor);

    static bool    ReadBlock        (Editor*                    pi_pEditor,
                                      bool                      pi_UseDrawContentToMem,
                                      uint32_t                 pi_PosX,
                                      uint32_t                 pi_PosY,
                                      uint32_t                 pi_Width,
                                      uint32_t                 pi_Height,
                                      const HFCPtr<HMDContext>& pi_rpContext,
                                      Byte*                     po_pData);

    static bool    ReadBlockByExtent(Editor*                    pi_pEditor,
                                      bool                      pi_UseDrawContentToMem,
                                      uint32_t                 pi_PosX,
                                      uint32_t                 pi_PosY,
                                      uint32_t                 pi_Width,
                                      uint32_t                 pi_Height,
                                      const HFCPtr<HMDContext>& pi_rpContext,
                                      Byte*                     po_pData);

    static void      GetLayers        (const PDDoc        pi_pDoc,
                                       uint32_t           pi_Page,
                                       HFCPtr<HMDLayers>& po_rLayers);

    static void      SetLayerVisibility(PDDoc                      pi_pDoc,
                                        uint32_t                   pi_Page,
                                        HFCPtr<HMDVolatileLayers>& pi_rpVolatileLayers);

    static void      GetAnnotations    (const PDDoc             pi_pDoc,
                                        uint32_t                pi_Page,
                                        HFCPtr<HMDAnnotations>& po_rAnnotations);

    static bool     IsOpRestrictedPDF(const PDDoc pi_pDoc);

    static bool     GetGeocodingAndReferenceInfo(const PDDoc                    pi_pDoc,
                                                  uint32_t                     pi_Page,
                                                  uint32_t                     pi_RasterizePageWidth,
                                                  uint32_t                     pi_RasterizePageHeight,
                                                  GeoCoordinates::BaseGCSPtr&   po_rpGeocoding,
                                                  HFCPtr<HGF2DTransfoModel>&    po_rpGeoreference);

    static void     GetDimensionForDWGUnderlay(const PDDoc                pi_pDoc,
                                               uint32_t                  pi_Page,
                                               double&                    po_xDimension, 
                                               double&                    po_yDimension);     

private :

    static bool    GetGeocodingReferenceFromImage(const PDPage&                     pi_rPage,
                                                   uint32_t                        pi_RasterizePageWidth,
                                                   uint32_t                        pi_RasterizePageHeight,
                                                   GeoCoordinates::BaseGCSPtr&     po_rpGeocoding,
                                                   HFCPtr<HGF2DTransfoModel>&       po_rpGeoreference);

    static bool    GetGeoreference(const CosObj&                  pi_rMeasureCosObj,
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
                                    HFCPtr<HGF2DTransfoModel>&    po_rpGeoreference);

    static void     CreateGeocodingFromWKT(const string& pi_rWKT, GeoCoordinates::BaseGCSPtr& po_rpGeocoding);

    static bool    GetCosObj(const CosObj&  pi_rSearchRootCosObj,
                              vector<char*>& pi_rpRelativeCosObjPath,
                              CosObj&        po_rFoundCosObj);

    static bool    IsMapZoneBBoxEqualToPageCropBox(PDPage& pi_rPage,
                                                    CosObj& pi_rMapZoneCosObj);

    static void    GetTransfoFromUserToDeviceCS  (PDPage                       pi_PDPage,
                                                  double                      pi_ResolutionFactor,
                                                  ASFixedMatrix&               po_rTransfoMatrix);

    static void    Get2DCoordFromCosArray        (CosObj&                      pi_r1DCoorArray,
                                                  uint32_t                     pi_2DCoorIndInArray,
                                                  ASFixedMatrix&               pi_rTransfoMatrix,
                                                  HGF2DPosition&               po_rPosition);

    static double GetToleranceFromBorderWidth    (const CosObj&                pi_rAnnotObj,
                                                  double                       pi_PointsToPixelScaleFactor,
                                                  bool*                        po_pHasBorder = 0);

    static void    Create2DVectForLineAnnot      (const CosObj&                pi_rAnnotObj,
                                                  ASFixedMatrix&               pi_rTransfoMatrix,
                                                  const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                                                  HFCPtr<HVE2DVector>&         po_rp2DSelectionZone);

    static void    Create2DVectForRectAnnot      (const CosObj&                pi_rAnnotObj,
                                                  ASFixedMatrix&               pi_rTransfoMatrix,
                                                  const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                                                  HFCPtr<HVE2DVector>&         po_rp2DSelectionZone);

    static void    Create2DVectForLinearAnnot    (const CosObj&                pi_rAnnotObj,
                                                  ASFixedMatrix&               pi_rTransfoMatrix,
                                                  ASAtom&                      pi_rAnnotSubtypeAtom,
                                                  const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                                                  HFCPtr<HVE2DVector>&         po_rp2DSelectionZone,
                                                  bool&                        po_rIsFilled);

    static void     Create2DVectForHoledAnnot    (const CosObj&                pi_rAnnotObj,
                                                  ASFixedMatrix&               pi_rTransfoMatrix,
                                                  ASAtom&                      pi_rAnnotSubtypeAtom,
                                                  double                       pi_PointsToPixelScaleFactor,
                                                  const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                                                  HFCPtr<HVE2DVector>&         po_rp2DSelectionZone,
                                                  bool&                        po_rIsFilled,
                                                  double&                      po_rTolerance);

    static void     Create2DVectForTxtMarkupAnnot(const CosObj&                pi_rAnnotObj,
                                                  ASFixedMatrix&               pi_rTransfoMatrix,
                                                  const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                                                  HFCPtr<HVE2DVector>&         po_rp2DSelectionZone);

    static void     Create2DVectForInkAnnot      (const CosObj&                pi_rAnnotObj,
                                                  ASFixedMatrix&               pi_rTransfoMatrix,
                                                  const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                                                  HFCPtr<HVE2DVector>&         po_rp2DSelectionZone);

    static bool    HasFillColor                  (const CosObj&                pi_rAnnotObj);

    HRFPDFLibInterface();

    virtual ~HRFPDFLibInterface();
    };
