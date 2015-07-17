/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/dtmio.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#define  WCHAR wchar_t
#ifdef USEPROJWISE
#include "bcfilewc.fdf"
#endif
#undef LPTR

BENTLEYDTM_EXPORT int bcdtmWrite_dataFileFromDataObject(DTM_DAT_OBJ *Data,WCharCP DataFile)
    {
    return DTM_SUCCESS;
    }

BENTLEYDTM_Public int bcdtmWrite_dataFileFromP3DArray(WCharCP FileName,DPoint3d *DataPts,long numPts,DTMFeatureType DtmFeatureType)
    {
    return DTM_SUCCESS;
    }
BENTLEYDTM_Public int bcdtmData_checkForKnots(DPoint3d *Polygon,long numPts,long *IntFlag)
    {
    return DTM_SUCCESS;
    }
BENTLEYDTM_EXPORT int bcdtmObject_setMemoryAllocationParametersDataObject(DTM_DAT_OBJ *dataP,long inimemPolyPts,long incmemPolyPts)
    {
    return DTM_SUCCESS;
    }
BENTLEYDTM_Public int bcdtmList_writePointsForDtmFeatureTinObject(DTM_TIN_OBJ *tinP,long dtmFeature) 
    {
    return DTM_SUCCESS;
    }
BENTLEYDTM_Public int bcdtmList_writeTptrListTinObject(DTM_TIN_OBJ *Tin,long Point) 
    {
    return DTM_SUCCESS;
    }
BENTLEYDTM_Public int bcdtmList_writeDtmFeaturesForPointTinObject(DTM_TIN_OBJ *tinP,long point) 
    {
    return DTM_SUCCESS;
    }
BENTLEYDTM_Public int bcdtmList_reportAndSetToNullNoneNullTptrValuesTinObject(DTM_TIN_OBJ *Tin,long reportFlag)
    {
    return DTM_SUCCESS;
    }
BENTLEYDTM_EXPORT int  bcdtmObject_storeDtmFeatureInDataObject
(
 DTM_DAT_OBJ   *dataP,     /* ==> Pointer To Dtm Data Object     */
 DTMFeatureType dtmFeatureType,     /* ==> Dtm Feature Type To Be Stored  */
 DTMUserTag userTag,     /* ==> User Tag                       */
 DTM_GUID    userGuid,     /* ==> User Guid                      */
 DPoint3d     *featurePtsP,     /* ==> Feature Points                 */
 long    numFeaturePts     /* ==> Number Of Feature Points       */
)
    {
    return DTM_SUCCESS;
    }
BENTLEYDTM_Public int bcdtmUtl_writeStatisticsLatticeObject(DTM_LAT_OBJ *Lattice)
    {
    return DTM_SUCCESS;
    }
BENTLEYDTM_Public int bcdtmLos_storePointInHorizonTable(DTM_HORIZON_LINE **Htable,long *Htne,long *Htme,double Ang1,double Ang2,double D1,double D2,double X1,double Y1,double Z1,double X2,double Y2,double Z2) 
    {
    return DTM_SUCCESS;
    }
BENTLEYDTM_Public int bcdtmLos_determineVisibilityOfEdge(double Xe,double Ye,double Ze,double X1,double Y1,double Z1,double X2,double Y2,double Z2,double X3,double Y3,double Z3,double X4,double Y4,double Z4,long *Visibility,DPoint3d Point[] )
    {
    return DTM_SUCCESS;
    }
BENTLEYDTM_Public int bcdtmLos_determineIfHorizonLineIsCovered(double Xe,double Ye,DTM_HORIZON_LINE *Hline1,DTM_HORIZON_LINE *Hline2) 
    {
    return DTM_SUCCESS;
    }
BENTLEYDTM_Public int bcdtmLos_findHorizonLineEntryListUsingHorizonLineIndex(DTM_HORIZON_LINE_INDEX *HlineIdx,long NumHlineIdx,long *Hidxlist,double Angle,long *HlineOfs)
    {
    return DTM_SUCCESS;
    }
