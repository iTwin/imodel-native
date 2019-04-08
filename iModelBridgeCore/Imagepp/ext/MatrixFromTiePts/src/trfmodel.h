/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/trfmodel.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*====================================================================
**  Octimage/MicroStation - Transformation Models
**
**      This module contains the define and structures for the
**      transformation models functions. The file who needs this include
**      file have their file name beginning with TRF.
**
**  DATA TYPES
**
**
**         ...
**
**  HMR FUNCTIONS
**         trf_calcCoefTran2D
**         trf_transfoCoordTran2D
**         trf_transfoCoordDoTran2D
**         trf_transfoCoordPrepTran2D
**         trf_calcCoefHelm2D
**         trf_transfoCoordHelm2D
**         trf_transfoCoordDoHelm2D
**         trf_transfoCoordPrepHelm2D
**         trf_calcCoefSimi2D
**         trf_transfoCoordSimi2D
**         trf_transfoCoordDoSimi2D
**         trf_transfoCoordPrepSimi2D
**         trf_calcCoefAffi2D
**         trf_transfoCoordAffi2D
**         trf_transfoCoordDoAffi2D
**         trf_transfoCoordPrepAffi2D
**         trf_calcCoefProj2D
**         trf_transfoCoordProj2D
**         trf_transfoCoordDoProj2D
**         trf_transfoCoordPrepProj2D
**         trf_calcCoefPoly2D1
**         trf_calcCoefPoly2D2
**         trf_calcCoefPoly2D3
**         trf_calcCoefPoly2D4
**         trf_calcCoefPoly2D5
**         trf_transfoCoordPoly2D
**         trf_transfoCoordDoPoly2D
**         trf_transfoCoordPrepPoly2D
**         trf_findNoActivePoints
**         trf_copyActivePoints
**         trf_transfoModelDestroy
**         trf_transfoModelInit
**         trf_getCtrlPtsAddr
**         trf_init
**         trf_end
**         trf_modelNew
**         trf_modelDestroy
**         trf_modelSetCurrent
**         trf_modelSetType
**         trf_modelGetType
**         trf_modelSetCoordinateSystem
**         trf_modelGetCoordinateSystem
**         trf_modelCalcCoef
**         trf_modelTransfoCoord
**         trf_modelTransfoCoordPrep
**         trf_modelTransfoCoordFinal
**         trf_ctrlPtsModif
**         trf_modelGetCoefficients
**  END HMR FUNCTIONS
**
**  FILE
**      trfmodel.h
**
**====================================================================*/

#ifndef __TRFMODEL_H__
#define __TRFMODEL_H__

#include <string.h>
#include <math.h>

#include "oldhtypes.h"
#include "trfmoids.h"
#include "matrix.h"
#include "libgeom.h"
#include "hlist.h"




/*====================================================================
**  tag_CONTROL_POINTS_3D
**
**      Contient un point de controle (en paires), son status et ses
**      residuelles
**
**      Int8      pointActive;    Points actifs ou non.
**      DCOORD_3D  pointBad;       Points du document a geocoder
**      DCOORD_3D  pointOk ;       Points du document de reference
**      DCOORD_3D  residual;       Residuelles
**      double    residualTotal;  Total des residuelles 2D:(XY) 3D:(XYZ)
**
**
**  Auteur          Date - Original version
** Alain Lapierre
**====================================================================*/

typedef struct tag_CONTROL_POINTS_3D {
   int8_t    pointActive;
   DCOORD_3D  pointBad;
   DCOORD_3D  pointOk ;
   DCOORD_3D  residual;                 /* Residuals in the Uncorrected geometry */
   double    residualTotal;
   DCOORD_3D  residualBase;     /* Residuals in the Base geometry */
   double    residualBaseTotal;
}CONTROL_POINTS_3D;

typedef CONTROL_POINTS_3D * pCONTROL_POINTS_3D;

/*====================================================================
**  tag_COEFFICIENTS_3D
**
**      Contient les coefficients direct et inverse d'un modele.
**      Ces derniers sont organises en vecteurs de coefficients differents
**      pour X ,Y et Z.
**
**      Int32 nbCoef[2];                      Nombre de coefficients
**                                            [0] pour X
**                                            [1] pour Y
**                                            [2] pour Z
**      Int32 nbTpsCoef[2];                   Nombre de coefficients
**                                            [0] pour X
**                                            [1] pour Y
**                                            [2] pour Z
**      double coef[2][3][L_COEFFICIENTS];   1er indice [0] Direct [1] Inverse
**                                            2e  indice [0] pour X [1] pour Y [2] pour Z
**
**      double *tpsCoef[2][3];               1er indice [0] Direct [1] Inverse
**                                            2e  indice [0] pour X [1] pour Y [2] pour Z
**
**  Auteur          Date - Original version
** Alain Lapierre
**====================================================================*/

typedef struct tag_COEFFICIENTS_3D {
   int32_t   nbCoef[3];
   int32_t   nbTpsCoef[3];
   int8_t   tpsCoefIsAllocated;
   double   coef[2][3][L_COEFFICIENTS];
   double * tpsCoef[2][3];
   pMAT_     tpsPoints[2];
}COEFFICIENTS_3D;

typedef COEFFICIENTS_3D * pCOEFFICIENTS_3D;

/*====================================================================
**  tag_TRANSFO_MODEL_DATA_3D
**
**      Contient les donnees requises par un modele.
**      Chaque fois qu'on cree un modele, une structure de ce type sera
**      utilisee.
**
**  Int32 modelType;          Modele de transformation utilise
**                                          (voir #define TRANSFO_   )
**      char   modelDesc[L_MODEL_DESC];    Description libre du modele
**      LIST    ctrlPts3D;                  Tableau des paires de point de
**                                          controle
**      COEFFICIENTS_3D coef3D;             Coefficients du modele (direct
**                                          et inverse)
**      double toleranceRadian;            If the model uses Iterations to
**                                          converge, this gives the tolerance in
**                                          radians. Below that delta, the
**                                          iterations stops.
**      double toleranceMeter;             If the model uses Iterations to
**                                          converge, this gives the tolerance in
**                                          meters. Below that delta, the
**                                          iterations stops.
**
**  Auteur          Date - Original version
** Alain Lapierre
**====================================================================*/

typedef struct tag_TRANSFO_MODEL_DATA_3D{
    int32_t modelType;
    char   modelDesc[L_MODEL_DESC];
    LIST    ctrlPts3D;
    COEFFICIENTS_3D coef3D;
    double toleranceRadian;
    double toleranceMeter;
}TRANSFO_MODEL_DATA_3D;

typedef TRANSFO_MODEL_DATA_3D * pTRANSFO_MODEL_DATA_3D;

/*====================================================================
**  tag_TRANSFO_RSP_BUFF_INFO
**
**      This structure contains information about the buffers
**      needed to resample the pixels when actual resampling is needed
**
**      mpOutputBuffer      A pointer to the output buffer
**      mInputBufferOrigin  The coordinates of the first pixel within
**                          the input buffer
**      mInputBufferSize    Width and Height of the input buffer
**      mpInputBuffer       A pointer to the input buffer
**      mImageDimension     Width an Height of the whole input image
**
**  Auteur          Date - Original version
** Jean Papillon   97-12-28
**====================================================================*/
typedef struct tag_TRANSFO_RSP_BUFF_INFO{
    unsigned char       *m_pOutputBuffer;
    LCOORD_2D     m_InputBufferOrigin;
    LCOORD_2D     m_InputBufferSize;
    unsigned char       *m_pInputBuffer;
    DCOORD_2D     m_InputImageOrigin;
    double       m_InputImagePixelSize;
    LCOORD_2D     m_ImageDimension;
}TRANSFO_RSP_BUFF_INFO;


/*====================================================================
**  tag_TRANSFO_RSP_COL_INFO
**
**      This structure contains information about the buffers
**      needed to resample the pixels when actual resampling is needed
**
**      m_ProcessPixel              Tells if the trf_transfoDoMatScanLine2D
**                                  function should try to identify the color
**                                  of pixels (TRUE) or if it should rely to the
**                                  provided function to do so (FALSE). Typically
**                                  this flag is set to TRUE when RSP_NEAREST_NEIGHBOUR
**                                  resampling mode is used
**      m_DefaultPixelValue         The value to assign to an outputpixel when the
**                                  corresponding input pixel is outside the input image
**      m_pPixelRspFunction         The function to use to implement the required
**                                  resampling mode
**      m_pPixelRspFunctionParam    A pointer to a memoty area to pass to the
**                                  resampling fucntion
**      m_UntouchedColorStatus      Tells if there is color that should not be changed
**      m_UntouchedColor            Identify the color that cannot be changed if the
**                                  preceeding flag is set to TRUE. It is usually the color
**                                  used as the background one
**      m_MinColor                  The smallest value that can be put into the output
**                                  buffer
**      m_MaxColor                  The biggest value that can be put into the output
**                                  buffer
**      m_MapInputColors;           Tells if the colors must be mapped
**      m_ColorMapping[256]         The color mapping table to use if the preceding flag
**                                  is set to TRUE
**      m_TransparentInputColors;   Tells if the transparent color must take into account
**      m_pIsTransparentColor[256]  TRUE if the color is transparent FALSE otherwise, only valid
**                                  if the preceding flag is set to TRUE
**
**  Auteur          Date - Original version
** Jean Papillon   97-12-28
** Marc Bedard     98-11-10  Add transparent color processing
**====================================================================*/
typedef struct tag_TRANSFO_RSP_COLOR_INFO{
    int8_t     m_ProcessPixel;
    unsigned char      m_DefaultPixelValue;
    unsigned char     (*m_pPixelRspFunction)(DCOORD_3D*, LCOORD_2D*, void*);
    void      *m_pPixelRspFunctionParam;
    uint32_t     m_UntouchedColor;
    int8_t     m_UntouchedColorStatus;
    int32_t     m_MinColor;
    int32_t     m_MaxColor;
    int8_t     m_MapInputColors;
    unsigned char     *m_pColorMapping;
    int8_t     m_TransparentInputColors;
    int8_t    *m_pIsTransparentColor;
}TRANSFO_RSP_COLOR_INFO;


/*====================================================================
**  tagTRANSFO_MODEL_CODE_2D
**
**      Contient les fonctions relatives a un modele de transformation donne.
**      Chaque fois qu'on ajoute un modele, on doit ecrire les fonctions
**      retrouvees dans cette structure.
**
**  Int32 modelType;          Numero de ce modele (voir TRFMODEL.H)
**  Int32 qtyPtsRequired;     Nombre de points requis pour ce modele
**  Int8 defaultForNbPts;    Determine si ce modele est le defaut pour
**                            un certain nbre de points. Un seul modele
**                            peut etre le DEFAUT pour une meme qte de pts.
**                            Mettre TRF_DEFAULT ou TRF_NON_DEFAULT
**  Int32 *calcCoef();        Calcule les coefficients du modele
**  Int32 *transfoCoord();    Transforme une coordonnee par ce modele
**                                  (phase preparatoire et finale)
**  Int32 *transfoCoordPrep();Transforme une coordonnee par ce modele
**                                  (phase preparatoire seulement,
**                                   ex: on calcule tous les y une seule fois
**                                   pour chaque ligne de pixels )
**  Int32 *transfoCoordDo();  Transforme une coordonnee par ce modele
**                                  (phase finale seulement, ex: on calcule
**                                   seulement les x considerant que les y sont
**                                   deja fait pour cette ligne de pixels)
**  Int32 *transfoCoordDoScanLine();  Transforme une serie de coordonnees d'une meme scan line par ce modele
**                                  (phase finale seulement, ex: on calcule
**                                   seulement les x considerant que les y sont
**                                   deja fait pour cette ligne de pixels)
**  Auteur          Date - Original version
** Alain Lapierre
**====================================================================*/

typedef struct tag_TRANSFO_MODEL_CODE_2D{
    int32_t modelType;
    int32_t qtyPtsRequired;
    int8_t defaultForNbPts;

    int32_t (*calcCoef)(int32_t,pTRANSFO_MODEL_DATA_3D);
    void   (*transfoCoord)(int32_t,pCOEFFICIENTS_3D,pDCOORD_3D,pDCOORD_3D);
    void   (*transfoCoordPrep)(int32_t,pCOEFFICIENTS_3D,pDCOORD_3D,double *);
    void   (*transfoCoordDo)(int32_t,pCOEFFICIENTS_3D,pDCOORD_3D,pDCOORD_3D,double *);
    void   (*transfoCoordDoScanLine)(int32_t,pCOEFFICIENTS_3D,double,double,int32_t,pDCOORD_3D,double *);
    void   (*transfoDoScanLine)(int32_t,pCOEFFICIENTS_3D,double,double,int32_t,double *,TRANSFO_RSP_BUFF_INFO *,TRANSFO_RSP_COLOR_INFO *);

}TRANSFO_MODEL_CODE_2D;

/*====================================================================
**  tag_TRANSFO_MODEL_RW_CODE_3D
**
**      Contient les fonctions relatives a la lecture et l'ecriture
**      des donnees d'un modele de transformation
**      pour un type de fichier donne.
**
**  Int32 *readAll();   Lit toutes les donnees du modele (points,coef..)
**  Int32 *readCoef();  Lit seulement les coefficients du modele
**  Int32 *writeAll();  Ecrit toutes les donnees du modele (points,coef..)
**  Int32 *writeCoef(); Ecrit seulement les coefficients du modele
**
**  Auteur          Date - Original version
** Alain Lapierre
**====================================================================*/

typedef struct tag_TRANSFO_MODEL_RW_CODE_3D{
    int32_t (*readAll)();
    int32_t (*readCoef)();
    int32_t (*writeAll)();
    int32_t (*writeCoef)();
}TRANSFO_MODEL_RW_CODE_3D;


/*====================================================================
** Prototypes
**====================================================================*/

#if defined (__cplusplus)
extern "C" {
#endif

/*
** Matrix transform
*/
int32_t trf_SetCoefficientsMat2D   (pCOEFFICIENTS_3D        pi_pCoefs,
                                             pTRANSFO_MODEL_DATA_3D  po_pModel);

void trf_transfoCoordMat2D      (int32_t                 mapping,
                                             pCOEFFICIENTS_3D        coef3D,
                                             pDCOORD_3D              ptIn,
                                             pDCOORD_3D              ptOut);

void trf_transfoCoordDoMat2D    (int32_t                 mapping,
                                             pCOEFFICIENTS_3D        coef3D,
                                             pDCOORD_3D              ptIn,
                                             pDCOORD_3D              ptOut,
                                             double                *yprod);

void trf_transfoCoordPrepMat2D  (int32_t                 mapping,
                                             pCOEFFICIENTS_3D        coef3D,
                                             pDCOORD_3D              ptIn,
                                             double                *yprod);

void trf_transfoCoordDoMatScanLine2D(int32_t            mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             double                scanLineOriginX,
                                             double                scanLinePixelSizeX,
                                             int32_t               scanLineLength,
                                             pDCOORD_3D             ptOut,
                                             double               *yprod);

void trf_transfoDoMatScanLine2D (int32_t                pi_Mapping,
                                             pCOEFFICIENTS_3D       pi_pCoef3D,
                                             double                pi_ScanLineOriginX,
                                             double                pi_ScanLinePixelSizeX,
                                             int32_t               pi_ScanLineLength,
                                             double               *pi_pYprod,
                                             TRANSFO_RSP_BUFF_INFO *pio_pBufferInfo,
                                             TRANSFO_RSP_COLOR_INFO *pi_pColorInfo);
/*
** Translation transform
*/
int32_t trf_calcCoefTran2D         (int32_t                 code,
                                             pTRANSFO_MODEL_DATA_3D  model);

void trf_transfoCoordTran2D     (int32_t                 mapping,
                                             pCOEFFICIENTS_3D        coef3D,
                                             pDCOORD_3D              ptIn,
                                             pDCOORD_3D              ptOut);

void trf_transfoCoordDoTran2D   (int32_t                 mapping,
                                             pCOEFFICIENTS_3D        coef3D,
                                             pDCOORD_3D              ptIn,
                                             pDCOORD_3D              ptOut,
                                             double                *yprod);

void trf_transfoCoordPrepTran2D (int32_t                 mapping,
                                             pCOEFFICIENTS_3D        coef3D,
                                             pDCOORD_3D              ptIn,
                                             double                *yprod);

/*&& AL Added transfoCoord for a Scan line */
void trf_transfoCoordDoTranScanLine2D(int32_t           mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             double                scanLineOriginX,
                                             double                scanLinePixelSizeX,
                                             int32_t               scanLineLength,
                                             pDCOORD_3D             ptOut,
                                             double               *yprod);
/*&&end Added transfoCoord for a Scan line*/

void trf_transfoDoTranScanLine2D(int32_t                pi_Mapping,
                                             pCOEFFICIENTS_3D       pi_pCoef3D,
                                             double                pi_ScanLineOriginX,
                                             double                pi_ScanLinePixelSizeX,
                                             int32_t               pi_ScanLineLength,
                                             double               *pi_pYprod,
                                             TRANSFO_RSP_BUFF_INFO *pio_pBufferInfo,
                                             TRANSFO_RSP_COLOR_INFO *pi_pColorInfo);
/*
** Similitude transform
*/
int32_t trf_calcCoefHelm2D         (int32_t                code,
                                             pTRANSFO_MODEL_DATA_3D model);

void trf_transfoCoordHelm2D     (int32_t                mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             pDCOORD_3D             ptIn,
                                             pDCOORD_3D             ptOut);

void trf_transfoCoordDoHelm2D   (int32_t                mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             pDCOORD_3D             ptIn,
                                             pDCOORD_3D             ptOut,
                                             double               *yprod);
void trf_transfoCoordPrepHelm2D (int32_t                mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             pDCOORD_3D             ptIn,
                                             double               *yprod);

/*&& AL Added transfoCoord for a Scan line */
void trf_transfoCoordDoHelmScanLine2D(int32_t           mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             double                scanLineOriginX,
                                             double                scanLinePixelSizeX,
                                             int32_t               scanLineLength,
                                             pDCOORD_3D             ptOut,
                                             double               *yprod);

/*&&end Added transfoCoord for a Scan line*/

void trf_transfoDoHelmScanLine2D(int32_t                pi_Mapping,
                                             pCOEFFICIENTS_3D       pi_pCoef3D,
                                             double                pi_ScanLineOriginX,
                                             double                pi_ScanLinePixelSizeX,
                                             int32_t               pi_ScanLineLength,
                                             double               *pi_pYprod,
                                             TRANSFO_RSP_BUFF_INFO *pio_pBufferInfo,
                                             TRANSFO_RSP_COLOR_INFO *pi_pColorInfo);
/*
** Similitude transform
*/
int32_t trf_calcCoefSimi2D         (int32_t                code,
                                             pTRANSFO_MODEL_DATA_3D model);

void trf_transfoCoordSimi2D     (int32_t                mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             pDCOORD_3D             ptIn,
                                             pDCOORD_3D             ptOut);

void trf_transfoCoordDoSimi2D   (int32_t                mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             pDCOORD_3D             ptIn,
                                             pDCOORD_3D             ptOut,
                                             double               *yprod);
void trf_transfoCoordPrepSimi2D (int32_t                mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             pDCOORD_3D             ptIn,
                                             double               *yprod);

/*&& AL Added transfoCoord for a Scan line */
void trf_transfoCoordDoSimiScanLine2D(int32_t           mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             double                scanLineOriginX,
                                             double                scanLinePixelSizeX,
                                             int32_t               scanLineLength,
                                             pDCOORD_3D             ptOut,
                                             double               *yprod);
/*&&end Added transfoCoord for a Scan line*/

void trf_transfoDoSimiScanLine2D(int32_t                pi_Mapping,
                                             pCOEFFICIENTS_3D       pi_pCoef3D,
                                             double                pi_ScanLineOriginX,
                                             double                pi_ScanLinePixelSizeX,
                                             int32_t               pi_ScanLineLength,
                                             double               *pi_pYprod,
                                             TRANSFO_RSP_BUFF_INFO *pio_pBufferInfo,
                                             TRANSFO_RSP_COLOR_INFO *pi_pColorInfo);
/*
** Affine transform
*/
int32_t trf_calcCoefAffi2D         (int32_t                code,
                                             pTRANSFO_MODEL_DATA_3D model);

void trf_transfoCoordAffi2D     (int32_t                mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             pDCOORD_3D             ptIn,
                                             pDCOORD_3D             ptOut);

void trf_transfoCoordDoAffi2D   (int32_t                mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             pDCOORD_3D             ptIn,
                                             pDCOORD_3D             ptOut,
                                             double               *yprod);

void trf_transfoCoordPrepAffi2D (int32_t                mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             pDCOORD_3D             ptIn,
                                             double               *yprod);

/*&& AL Added transfoCoord for a Scan line */
void trf_transfoCoordDoAffiScanLine2D(int32_t           mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             double                scanLineOriginX,
                                             double                scanLinePixelSizeX,
                                             int32_t               scanLineLength,
                                             pDCOORD_3D             ptOut,
                                             double               *yprod);
/*&&end Added transfoCoord for a Scan line*/

void trf_transfoDoAffiScanLine2D(int32_t                pi_Mapping,
                                             pCOEFFICIENTS_3D       pi_pCoef3D,
                                             double                pi_ScanLineOriginX,
                                             double                pi_ScanLinePixelSizeX,
                                             int32_t               pi_ScanLineLength,
                                             double               *pi_pYprod,
                                             TRANSFO_RSP_BUFF_INFO *pio_pBufferInfo,
                                             TRANSFO_RSP_COLOR_INFO *pi_pColorInfo);
/*
** Projective transform
*/
int32_t trf_calcCoefProj2D         (int32_t                code,
                                             pTRANSFO_MODEL_DATA_3D model);

void trf_transfoCoordProj2D     (int32_t                mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             pDCOORD_3D             ptIn,
                                             pDCOORD_3D             ptOut);

void trf_transfoCoordDoProj2D   (int32_t                mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             pDCOORD_3D             ptIn,
                                             pDCOORD_3D             ptOut,
                                             double               *yprod);

void trf_transfoCoordPrepProj2D (int32_t                mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             pDCOORD_3D             ptIn,
                                             double               *yprod);

/*&& AL Added transfoCoord for a Scan line */
void trf_transfoCoordDoProjScanLine2D(int32_t           mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             double                scanLineOriginX,
                                             double                scanLinePixelSizeX,
                                             int32_t               scanLineLength,
                                             pDCOORD_3D             ptOut,
                                             double               *yprod);

/*&&end Added transfoCoord for a Scan line*/
void trf_transfoDoProjScanLine2D(int32_t                pi_Mapping,
                                             pCOEFFICIENTS_3D       pi_pCoef3D,
                                             double                pi_ScanLineOriginX,
                                             double                pi_ScanLinePixelSizeX,
                                             int32_t               pi_ScanLineLength,
                                             double               *pi_pYprod,
                                             TRANSFO_RSP_BUFF_INFO *pio_pBufferInfo,
                                             TRANSFO_RSP_COLOR_INFO *pi_pColorInfo);

/*
** Polynomial transformations
*/
int32_t trf_calcCoefPoly2D1        (int32_t code,
                                             pTRANSFO_MODEL_DATA_3D model);

int32_t trf_calcCoefPoly2D2        (int32_t code,
                                             pTRANSFO_MODEL_DATA_3D model);

int32_t trf_calcCoefPoly2D3        (int32_t code,
                                             pTRANSFO_MODEL_DATA_3D model);

int32_t trf_calcCoefPoly2D4        (int32_t code,
                                             pTRANSFO_MODEL_DATA_3D model);

int32_t trf_calcCoefPoly2D5        (int32_t code,
                                             pTRANSFO_MODEL_DATA_3D model);

void trf_transfoCoordPoly2D     (int32_t mapping,
                                             pCOEFFICIENTS_3D coef3D,
                                             pDCOORD_3D ptIn,
                                             pDCOORD_3D ptOut);

void trf_transfoCoordDoPoly2D   (int32_t mapping,
                                             pCOEFFICIENTS_3D coef3D,
                                             pDCOORD_3D ptIn,
                                             pDCOORD_3D ptOut,
                                             double *yprod);

void trf_transfoCoordPrepPoly2D (int32_t mapping,
                                             pCOEFFICIENTS_3D coef3D,
                                             pDCOORD_3D ptIn,
                                             double *yprod);
/*&& AL Added transfoCoord for a Scan line */
void trf_transfoCoordDoPolyScanLine2D(int32_t           mapping,
                                             pCOEFFICIENTS_3D       coef3D,
                                             double                scanLineOriginX,
                                             double                scanLinePixelSizeX,
                                             int32_t               scanLineLength,
                                             pDCOORD_3D             ptOut,
                                             double               *yprod);

/*&&end Added transfoCoord for a Scan line*/

void trf_transfoDoPolyScanLine2D(int32_t                pi_Mapping,
                                             pCOEFFICIENTS_3D       pi_pCoef3D,
                                             double                pi_ScanLineOriginX,
                                             double                pi_ScanLinePixelSizeX,
                                             int32_t               pi_ScanLineLength,
                                             double               *pi_pYprod,
                                             TRANSFO_RSP_BUFF_INFO *pio_pBufferInfo,
                                             TRANSFO_RSP_COLOR_INFO *pi_pColorInfo);

/*
** Thin Plate Spline Transformation Model
*/
int32_t trf_calcCoefTps2D     (int32_t                pi_direction,
                                        pTRANSFO_MODEL_DATA_3D pio_pModel );

void trf_transfoCoordTps2D (int32_t            mapping,
                                        COEFFICIENTS_3D  * coef3D,
                                        DCOORD_3D        * ptIn,
                                        DCOORD_3D        * ptOut );

void trf_transfoCoordPrepTps2D (int32_t           mapping,
                                            pCOEFFICIENTS_3D   coef3D,
                                            pDCOORD_3D         ptIn,
                                            double           *yprod );

void trf_transfoCoordDoTps2D  ( int32_t            mapping,
                                            pCOEFFICIENTS_3D    coef3D,
                                            pDCOORD_3D          ptIn,
                                            pDCOORD_3D          ptOut,
                                            double            *yprod );

void trf_transfoCoordDoTpsScanLine2D(int32_t            mapping,
                                                                                             pCOEFFICIENTS_3D   coef3D,
                                                                                             double            scanLineOriginX,
                                                                                             double            scanLinePixelSizeX,
                                                                                             int32_t           scanLineLength,
                                                                                             pDCOORD_3D         ptOut,
                                                                                             double           *yprod );

void trf_transfoDoTpsScanLine2D (   int32_t                pi_Mapping,
                                                pCOEFFICIENTS_3D       pi_pCoef3D,
                                                double                pi_ScanLineOriginX,
                                                double                pi_ScanLinePixelSizeX,
                                                int32_t               pi_ScanLineLength,
                                                double               *pi_pYprod,
                                                TRANSFO_RSP_BUFF_INFO *pio_pBufferInfo,
                                                TRANSFO_RSP_COLOR_INFO *pi_pColorInfo );



HSTATUS trf_freeTpsCoefficients  (   COEFFICIENTS_3D         *po_pCoefs );

HSTATUS trf_allocTpsCoefficients (   COEFFICIENTS_3D         *po_pCoefs,
                                                int32_t                  pi_nCoefs );



/*
** Transformation utilities
*/
int32_t trf_findNoActivePoints          (pLIST                  lPoints);

void trf_copyActivePoints            (pMAT_                  pts,
                                                  pLIST                  lPoints,
                                                  uint32_t                badOrOk,
                                                  uint32_t                xOrY,
                                                  double               *minx,
                                                  double               *miny);

void trf_transfoModelDestroy         (pTRANSFO_MODEL_DATA_3D modele);

int32_t trf_transfoModelInit            (pTRANSFO_MODEL_DATA_3D modele);

pCONTROL_POINTS_3D trf_getCtrlPtsAddr (pLIST                  lCtrl);

/*
** Server: Transformation table of function pointers
*/

int32_t      trf_init       (void);

int32_t      trf_end        (void);

TRFHANDLE    trf_modelNew   (void);

int32_t trf_modelDestroy     (TRFHANDLE   trfHdl);

int32_t trf_modelSetCurrent  (TRFHANDLE   trfHdl);

int32_t trf_modelSetType     (TRFHANDLE   trfHdl,
                                        int32_t     modelType);

int32_t trf_modelGetType     (TRFHANDLE   trfHdl,
                                        int32_t    *modelType);

int32_t trf_modelSetCoordinateSystem (TRFHANDLE   trfHdl,
                                                char      *coordinateSystem);

int32_t trf_modelGetCoordinateSystem (TRFHANDLE   trfHdl,
                                                char      *coordinateSystem);

int32_t trf_modelCalcCoef            (TRFHANDLE   trfHdl,
                                                int32_t     mapping);

int32_t trf_modelTransfoCoord        (TRFHANDLE   trfHdl,
                                                pDCOORD_3D  PtsIn,
                                                pDCOORD_3D  PtsOut,
                                                int32_t     mapping);

int32_t trf_modelTransfoCoordPrep    (TRFHANDLE   trfHdl,
                                                pDCOORD_3D  PtsIn,
                                                int32_t     mapping,
                                                double    *yProd);

int32_t trf_modelTransfoCoordFinal   (TRFHANDLE   trfHdl,
                                                pDCOORD_3D  PtsIn,
                                                pDCOORD_3D  PtsOut,
                                                int32_t     mapping,
                                                double    *yProd);

int32_t trf_ctrlPtsModif             (TRFHANDLE          trfHdl,
                                                pCONTROL_POINTS_3D ctrlPts,
                                                int32_t            operation);

int32_t trf_modelGetCoefficients     (TRFHANDLE        trfHdl,
                                                pCOEFFICIENTS_3D coefs);

int32_t trf_modelSetCoefficients     (TRFHANDLE        trfHdl,
                                                pCOEFFICIENTS_3D coefs);


#if defined (__cplusplus)
}
#endif


#endif /* __TRFMODEL_H__ */
