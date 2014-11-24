/*--------------------------------------------------------------------------------------+
|
|     $Source: src/unmanaged/DTM/civilDTMext/bcdtmExtEdit.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//int bcdtmExtEdit_resolveIslandsVoidsHolesDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ **resolvedFeaturesDtmPP);
//int bcdtmExtEdit_insertVoidsAndIslandsIntoEditDtmObject(BC_DTM_OBJ *dtmP, BC_DTM_OBJ *dataDtmP);
//int bcdtmExtEdit_deleteTriangleDtmObject(BC_DTM_OBJ *dtmP,long tinPnt1,long tinPnt2,long tinPnt3);
//int bcdtmExtEdit_deleteTrianglesOnDeleteLineDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *deleteLinePtsP,long numDeleteLinePts) ;
struct LinearFeature
    {
    DTMFeatureType featureType;
    int            featureNumber;
    double         featureDistance;
    DPoint3d*      featurePtsP;
    int            numFeaturePts;

    LinearFeature ()
        {
        featurePtsP = NULL;
        }

    ~LinearFeature ()
        {
        if (featurePtsP != NULL)
            {
            free (featurePtsP);
            featurePtsP = NULL;
            }
        }

    LinearFeature (const LinearFeature& dtmString)
        {
        featureType = dtmString.featureType;
        featureNumber = dtmString.featureNumber;
        featureDistance = dtmString.featureDistance;
        featurePtsP = new DPoint3d[dtmString.numFeaturePts];
        numFeaturePts = dtmString.numFeaturePts;
        memcpy (featurePtsP, dtmString.featurePtsP, dtmString.numFeaturePts*sizeof (DPoint3d));
        }

    LinearFeature&  operator=(const LinearFeature& dtmString)
        {
        if (featurePtsP) delete[] featurePtsP;
        featureType = dtmString.featureType;
        featureNumber = dtmString.featureNumber;
        featureDistance = dtmString.featureDistance;
        numFeaturePts = dtmString.numFeaturePts;
        if (dtmString.featurePtsP)
            {
            featurePtsP = new DPoint3d[dtmString.numFeaturePts];
            memcpy (featurePtsP, dtmString.featurePtsP, dtmString.numFeaturePts*sizeof (DPoint3d));
            }
        else
            featurePtsP = NULL;
        return *this;
        }
    };

struct SelectedLinearFeatures : bvector<LinearFeature>
    {
    private:
        static bool DistanceCompare (LinearFeature& m1, LinearFeature& m2)
            {
            return m1.featureDistance < m2.featureDistance;
            }
    public:
        void sortOnAscendingDistance ()
            {
            std::sort (begin (), end (), &DistanceCompare);
            }
    };


void bcdtmExtEdit_setCurrentEditDtmObjects(void *editdtmP,void *saveEditTin);
void bcdtmExtEdit_clearCurrentEditDtmObjects(void);
int bcdtmExtEdit_checkAndUpdateCurrentDtmObject(wchar_t *EditTinFile);
int bcdtmExtEdit_checkTinStructureDtmObject(BC_DTM_OBJ *dtmP);
int bcdtmExtEdit_getDtmEditFeaturePoints(long *editPnt1P,long *editPnt2P,long *editPnt3P);
int bcdtmExtEdit_selectDtmEditFeatureDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,double x,double y,long *featureFoundP,DPoint3d **featurePtsPP,long *numFeaturePtsP );
int bcdtmExtEdit_modifyDtmEditFeatureDtmObject(BC_DTM_OBJ *dtmP, long modifyOption, long *modifyResultP);
int bcdtmExtEdit_cleanVoidDtmObject(BC_DTM_OBJ *dtmP,long VoidFeature);
int bcdtmExtEdit_deleteTriangleDtmObject(BC_DTM_OBJ *dtmP,long tinPnt1,long tinPnt2,long tinPnt3);
int bcdtmExtEdit_removeInsertedVoidsOnTinHullDtmObject(BC_DTM_OBJ *dtmP,long numStartFeatures );
int bcdtmExtEdit_moveVertexZDtmObject(BC_DTM_OBJ *dtmP,long point,double z);
int bcdtmExtEdit_checkPointXYCanBeMovedDtmObject(BC_DTM_OBJ *dtmP,long Point,double x,double y, long &moveFlag);
int bcdtmExtEdit_tempMoveVertexXYZDtmObject(BC_DTM_OBJ *dtmP,long Point,double x,double y,double z);
int bcdtmExtEdit_moveVertexXYZDtmObject(BC_DTM_OBJ *dtmP,long ResetFlag,long Point,double x,double y,double z);
int bcdtmExtEdit_checkPointCanBeDeletedDtmObject(BC_DTM_OBJ *dtmP,long Point,long UpdateFlag,long *Flag);
int bcdtmExtEdit_insertPointDtmObject(BC_DTM_OBJ *dtmP,long pntType,long dtmFeature,long updateOption,long P1,long P2,long P3,double x,double y, double z, long *newPntP);
int bcdtmExtEdit_removePointDtmObject(BC_DTM_OBJ *dtmP,long Point,long Ptype,long Feature,long P1,long P2,long P3);
int bcdtmExtEdit_getIslandsInternalToVoidDtmObject(BC_DTM_OBJ *dtmP,long voidFeature,long **islandsPP, long *numIslandsP);
int bcdtmExtEdit_removeInternalVoidPointsAndLinesDtmObject(BC_DTM_OBJ *dtmP,long VoidFeature,long *Islands, long NumIslands);
int bcdtmExtEdit_deletePointDtmObject(BC_DTM_OBJ *dtmP,long tinPoint,long deleteFlag);
int bcdtmExtEdit_dataPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double *z,long *pntTypeP,long *dtmFeatureP,long *pnt1P,long *pnt2P,long *pnt3P );
int bcdtmExtEdit_deleteLineDtmObject(BC_DTM_OBJ *dtmP,long deleteFlag,long tinPnt1,long tinPnt2,long *tinPnt3,long *tinPnt4);
int bcdtmExtEdit_polygonMoveZDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *polyPtsP,long numPolyPts,long moveOption,double elevation);
int bcdtmExtEdit_insertStringIntoDtmObject(BC_DTM_OBJ *dtmP,long drapeOption,wchar_t *stringFileNameP,long *startPntP);
int bcdtmExtEdit_insertVoidIntoDtmObject(BC_DTM_OBJ *dtmP,DTMUserTag userTag,DPoint3d *voidPtsP,long numVoidPts);
int bcdtmExtEdit_insertIslandIntoDtmObject(BC_DTM_OBJ *dtmP,DTMUserTag userTag,DPoint3d *islandPtsP,long numIslandPts);
int bcdtmExtEdit_tptrMoveZDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long moveOption,double elevation);
int bcdtmExtEdit_tptrPolygonMoveZDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long moveOption,double elevation);
int bcdtmExtEdit_nullTptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt);
int bcdtmExtEdit_getDeletedFeaturesDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long **deletedDtmFeaturesPP,long *numDeletedDtmFeaturesP);
int bcdtmExtEdit_drawPointFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long point,double contourInterval,void *userP );
int bcdtmExtEdit_drawLineFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,double contourInterval,void *userP);
int bcdtmExtEdit_drawTriangleFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,long P3,double contourInterval,void *userP );
int bcdtmExtEdit_drawTriangleDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,long P3,void *userP);
int bcdtmExtEdit_drawTriangleLinesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long DrawMode,long P1,long P2,long P3, void *userP);
int bcdtmExtEdit_drawTriangleBaseLinesForPointDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long DrawMode,long P1, void *userP);
int bcdtmExtEdit_drawPolygonFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,DPoint3d *polyPtsP,long numPolyPts,double contourInterval,void *userP);
int bcdtmExtEdit_drawTptrFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPoint,long updateOption,double contourInterval,void *userP);
int bcdtmExtEdit_drawInternalTptrFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPoint,double contourInterval,void *userP);
int bcdtmExtEdit_drawExternalTptrFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPoint,long updateOption,double contourInterval,void *userP);
int bcdtmExtEdit_removeInternalTptrPointsAndRetriangulateDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long fillOption);
int bcdtmExtEdit_checkForVoidsAlongTptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long *voidsFoundP );
int bcdtmExtEdit_removeVoidsAlongTptrListDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPnt,double contourInterval,void *userP );
int bcdtmExtEdit_deleteTrianglesOnDeleteLineDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *deleteLinePtsP,long numDeleteLinePts);
int bcdtmExtEdit_clipLastDtmFeatureToVoidDtmObject(BC_DTM_OBJ *dtmP );
int bcdtmExtEdit_drawDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long dtmFeature,void *userP);
int bcdtmExtEdit_drawPointPerimeterDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long Point,void *userP );
int bcdtmExtEdit_drawDeletedFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long *deletedFeaturesP,long numDeletedFeatures,double contourInterval,void *userP);


int bcdtmExtEdit_selectPointDtmObject(BC_DTM_OBJ *dtmP, double x,double y,long *pointFound,DPoint3d **featurePtsPP,long *numFeaturePtsP,long *editPointP) ;
int bcdtmExtEdit_findClosestNonVoidPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long *editPointP) ;
int bcdtmExtEdit_selectLineDtmObject(BC_DTM_OBJ *dtmP,double  x,double y,long *lineFoundP,DPoint3d **linePtsPP,long *numLinePtsP,long*pnt1P,long*pnt2P ) ;
int bcdtmExtEdit_selectTriangleDtmObject( BC_DTM_OBJ *dtmP, double x, double y, long *trgFoundP, DPoint3d  **trgPtsPP, long *numTrgPtsP, long *pnt1P, long *pnt2P, long  *pnt3P ) ;
int bcdtmExtEdit_findClosestHullLineDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long *hullPnt1P,long *hullPnt2P) ;
int bcdtmExtEdit_findClosestHullLineForPointAddDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long *hullPnt1P,long *hullPnt2P) ;
int bcdtmExtEdit_checkForIntersectionWithTinHullDtmObject(BC_DTM_OBJ *dtmP,long point,double x,double y,bool& intersects ) ;
int bcdtmExtEdit_findClosestVoidLineDtmObject( BC_DTM_OBJ *dtmP,double x,double y,long *voidPnt1P,long *voidPnt2P,long *dtmFeatureP) ;
int bcdtmExtEdit_findDtmFeatureTypeEnclosingPointDtmObject( BC_DTM_OBJ *dtmP,long point,DTMFeatureType dtmFeatureType,long *dtmFeatureP) ;
int bcdtmExtEdit_testForPointOnDtmFeatureTypeDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureType dtmFeatureType, long point, long *dtmFeatureP, long *numFeaturesP);
int bcdtmExtEdit_getVoidExternalToIslandDtmObject(BC_DTM_OBJ *dtmP,long islandFeature,long *voidFeatureP) ;
int bcdtmExtEdit_getIslandFeaturesInternalToVoidDtmObject(BC_DTM_OBJ *dtmP,long voidFeature,DTM_TIN_POINT_FEATURES **islandsPP,long *numIslandsP) ;
int bcdtmExtEdit_storeDtmFeatureInDtmFeatureList (DTM_TIN_POINT_FEATURES **dtmFeatureListPP, long *numFeatureListP, long *memFeatureListP, long memFeatureListInc, long dtmFeature, DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId userFeatureId, long priorPoint, long nextPoint);
int bcdtmExtEdit_insertVoidsAndIslandsIntoEditDtmObject(BC_DTM_OBJ *dtmP, BC_DTM_OBJ *dataDtmP) ;
int bcdtmExtEdit_getListOfIntersectedIslandVoidHoleFeaturesDtmObject(BC_DTM_OBJ *dtmP,long numStartFeatures,DTM_TIN_POINT_FEATURES **intersectedFeaturesPP,long *numIntersectedFeaturesP) ;
int bcdtmExtEdit_getIslandVoidHoleFeaturesWithACommonHullSegementDtmObject(BC_DTM_OBJ *dtmP,long numStartFeatures,DTM_TIN_POINT_FEATURES **intersectedFeaturesPP,long *numIntersectedFeaturesP) ;
int bcdtmExtEdit_resolveOverlappingIslandsVoidsAndHolesDtmObject(BC_DTM_OBJ *dtmP,long numStartFeatures,DTM_TIN_POINT_FEATURES *intersectedFeaturesP,long numIntersectedFeatures) ;
int bcdtmExtEdit_resolveIslandsVoidsHolesDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ **resolvedFeaturesDtmPP) ;
int bcdtmExtEdit_clipVoidLinesFromDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature );
int bcdtmExtEdit_clipVoidPointsFromGroupSpotsDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature ) ;
int bcdtmExtEdit_deleteAllPointsAndLinesInternalToTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long spnt);
int bcdtmExtEdit_clipDtmFeaturesCoincidentWithTinLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2 ) ;
int bcdtmExtEdit_checkForOneCoincidentTptrVoidPolygonSectionWithTinHullDtmObject(BC_DTM_OBJ *dtmP,long spnt,long *fsP,long *lsP,long *coincidentFlagP) ;
int bcdtmExtEdit_deleteInternalVoidPointsAndLinesAndRetriangulateVoidDtmObject(BC_DTM_OBJ *dtmP,long voidFeature) ;
int bcdtmExtEdit_deleteInternalTptrVoidPolygonPointsAndLinesDtmObject(BC_DTM_OBJ *dtmP,long sPnt) ;
int bcdtmExtEdit_deleteExternalTptrIslandPolygonPointsAndLinesDtmObject(BC_DTM_OBJ *dtmP,long sPnt) ;
int bcdtmExtEdit_insertTptrPolygonAroundPointDtmObject(BC_DTM_OBJ *dtmP,long Point,long *Spnt) ;
int bcdtmExtEdit_addPointToDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double z,long *Point ) ;
int bcdtmExtEdit_storePointFeaturesInDtmFeatureList (DTM_TIN_POINT_FEATURES **bcdtmList, long *numDtmList, long *memDtmList, long memDtmListInc, long dtmFeature, DTMFeatureType dtmFeatureType, DTMUserTag userTag, long priorPoint, long nextPoint);
int bcdtmExtEdit_testForDtmFeatureHullLineDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureType DtmFeatureType, long P1, long P2, long *HullLine, long *Feature, long *Direction);
int bcdtmExtEdit_triangulateVoidDtmObject(BC_DTM_OBJ *dtmP,long voidFeature,long *islandFeaturesP,long numIslandFeatures,long internalPoint) ;
int bcdtmExtEdit_breakDtmFeatureAtPointDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long breakPoint ) ;
int bcdtmExtEdit_insertDtmFeatureIntoDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureType dtmFeatureType, DTMUserTag userTag, DPoint3d *stringPtsP, long numStringPts, long *startPntP);
int bcdtmExtEdit_validateVoidDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *voidPtsP,long *numVoidPtsP);
int bcdtmExtEdit_insertLineIntoDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureType dtmFeatureType, DTMUserTag userTag, DPoint3d *stringPtsP, long numStringPts, long *startPntP);
int bcdtmExtEdit_validateStringDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *stringPtsP,long *numStringPtsP);
int bcdtmExtEdit_validateInsertVoidOrIslandDtmObject(BC_DTM_OBJ *dtmP,long startPoint) ;
int bcdtmExtEdit_retriangualteAlongTptrListDtmObject(BC_DTM_OBJ *dtmP,long leftSide,long rightSide,long firstPoint) ;
int bcdtmExtEdit_drawDtmFeaturesForLineDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,void *userP ) ;
int bcdtmExtEdit_drawContoursForTriangleDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,long P3,double contourInterval,void *userP) ;
int bcdtmExtEdit_drawTptrLineFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPoint,void *userP) ;
int bcdtmExtEdit_clearVoidBitOfInternalVoidPointsDtmObject(BC_DTM_OBJ *dtmP,long voidFeature,long *islandsP, long numIslands) ;
int bcdtmExtEdit_drapeDeleteLineOnEditDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *deleteLinePtsP,long numDeleteLinePts,DPoint3d **drapePoints,long *numDrapePoints) ;
int bcdtmExtEdit_assignTinLinesToDrapePointsDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *drapePtsP,long numDrapePts,DTM_TRG_INDEX_TABLE **drapeTinLinesP,long *numDrapeTinLines) ;
int bcdtmExtEdit_createVoidAndIslandPolygonsForDeletedTrianglesDtmObject(BC_DTM_OBJ *dtmP,DTM_TRG_INDEX_TABLE *delPointsP,long numDelPoints,BC_DTM_OBJ **voidPolygonsPP) ;
int bcdtmExtEdit_extractVoidAndIslandPolygonsFromDeletedTrianglesDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ **voidPolygonsPP) ;
int bcdtmExtEdit_accumulateLinearFeaturesAndCursorDistanceForPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long point,SelectedLinearFeatures& linearFeatures) ;
int bcdtmExtEdit_selectDtmEditLinearFeatureDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double snapTolerance,SelectedLinearFeatures& linearFeatures) ;
int bcdtmExtEdit_deleteLinearFeatureDtmObject (BC_DTM_OBJ *dtmP, long dtmFeature, DTMFeatureType dtmFeatureType);
int bcdtmExtEdit_deletePolygonalFeatureDtmObject (BC_DTM_OBJ *dtmP, long dtmFeature, DTMFeatureType dtmFeatureType);
int bcdtmExtEdit_deleteNonePolygonalFeatureDtmObject (BC_DTM_OBJ *dtmP, long dtmFeature, DTMFeatureType dtmFeatureType);
int bcdtmExtEdit_getVoidFeaturesInternalToIslandDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureType islandFeature, DTM_TIN_POINT_FEATURES **voidsPP, long *numVoidsP);
int bcdtmExtEdit_getVoidsInternalToIslandDtmObject(BC_DTM_OBJ *dtmP,long islandFeature,long **voidsPP,long *numVoidsP) ;

int bcdtmExtEdit_deleteExternalTriangleDtmObject(BC_DTM_OBJ *dtmP,long trgPnt1,long trgPnt2,long trgPnt3) ;
