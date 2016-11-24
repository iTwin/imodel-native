/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PrivateAPI/bcdtmEdit.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//int bcdtmEdit_resolveIslandsVoidsHolesDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ **resolvedFeaturesDtmPP);
//int bcdtmEdit_insertVoidsAndIslandsIntoEditDtmObject(BC_DTM_OBJ *dtmP, BC_DTM_OBJ *dataDtmP);
//int bcdtmEdit_deleteTriangleDtmObject(BC_DTM_OBJ *dtmP,long tinPnt1,long tinPnt2,long tinPnt3);
//int bcdtmEdit_deleteTrianglesOnDeleteLineDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *deleteLinePtsP,long numDeleteLinePts) ;
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


void bcdtmEdit_setCurrentEditDtmObjects(void *editdtmP,void *saveEditTin);
void bcdtmEdit_clearCurrentEditDtmObjects(void);
int bcdtmEdit_checkAndUpdateCurrentDtmObject(wchar_t *EditTinFile);
int bcdtmEdit_checkTinStructureDtmObject(BC_DTM_OBJ *dtmP);
int bcdtmEdit_getDtmEditFeaturePoints(long *editPnt1P,long *editPnt2P,long *editPnt3P);
int bcdtmEdit_selectDtmEditFeatureDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,double x,double y,bool& featureFound,bvector<DPoint3d>& featurePts);
int bcdtmEdit_modifyDtmEditFeatureDtmObject(BC_DTM_OBJ *dtmP, long modifyOption, long *modifyResultP);
int bcdtmEdit_cleanVoidDtmObject(BC_DTM_OBJ *dtmP,long VoidFeature);
int bcdtmEdit_deleteTriangleDtmObject(BC_DTM_OBJ *dtmP,long tinPnt1,long tinPnt2,long tinPnt3);
int bcdtmEdit_removeInsertedVoidsOnTinHullDtmObject(BC_DTM_OBJ *dtmP,long numStartFeatures );
int bcdtmEdit_moveVertexZDtmObject(BC_DTM_OBJ *dtmP,long point,double z);
BENTLEYDTM_EXPORT int bcdtmEdit_checkPointXYCanBeMovedDtmObject (BC_DTM_OBJ *dtmP, long Point, double x, double y, long &moveFlag);
int bcdtmEdit_tempMoveVertexXYZDtmObject(BC_DTM_OBJ *dtmP,long Point,double x,double y,double z);
int bcdtmEdit_moveVertexXYZDtmObject(BC_DTM_OBJ *dtmP,long ResetFlag,long Point,double x,double y,double z);
int bcdtmEdit_checkPointCanBeDeletedDtmObject(BC_DTM_OBJ *dtmP,long Point,long UpdateFlag,long *Flag);
int bcdtmEdit_insertPointDtmObject(BC_DTM_OBJ *dtmP,long pntType,long dtmFeature,long updateOption,long P1,long P2,long P3,double x,double y, double z, long *newPntP);
int bcdtmEdit_removePointDtmObject(BC_DTM_OBJ *dtmP,long Point,long Ptype,long Feature,long P1,long P2,long P3);
int bcdtmEdit_getIslandsInternalToVoidDtmObject(BC_DTM_OBJ *dtmP,long voidFeature,long **islandsPP, long *numIslandsP);
int bcdtmEdit_removeInternalVoidPointsAndLinesDtmObject(BC_DTM_OBJ *dtmP,long VoidFeature,long *Islands, long NumIslands);
int bcdtmEdit_deletePointDtmObject(BC_DTM_OBJ *dtmP,long tinPoint,long deleteFlag);
int bcdtmEdit_dataPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double *z,long *pntTypeP,long *dtmFeatureP,long *pnt1P,long *pnt2P,long *pnt3P );
int bcdtmEdit_deleteLineDtmObject(BC_DTM_OBJ *dtmP,long deleteFlag,long tinPnt1,long tinPnt2,long *tinPnt3,long *tinPnt4);
int bcdtmEdit_polygonMoveZDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *polyPtsP,long numPolyPts,long moveOption,double elevation);
int bcdtmEdit_insertStringIntoDtmObject(BC_DTM_OBJ *dtmP,long drapeOption,wchar_t *stringFileNameP,long *startPntP);
int bcdtmEdit_insertVoidIntoDtmObject(BC_DTM_OBJ *dtmP,DTMUserTag userTag,DPoint3d *voidPtsP,long numVoidPts);
int bcdtmEdit_insertIslandIntoDtmObject(BC_DTM_OBJ *dtmP,DTMUserTag userTag,DPoint3d *islandPtsP,long numIslandPts);
int bcdtmEdit_tptrMoveZDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long moveOption,double elevation);
int bcdtmEdit_tptrPolygonMoveZDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long moveOption,double elevation);
int bcdtmEdit_nullTptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt);
int bcdtmEdit_getDeletedFeaturesDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long **deletedDtmFeaturesPP,long *numDeletedDtmFeaturesP);
int bcdtmEdit_drawPointFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long point,double contourInterval,void *userP );
int bcdtmEdit_drawLineFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,double contourInterval,void *userP);
int bcdtmEdit_drawTriangleFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,long P3,double contourInterval,void *userP );
int bcdtmEdit_drawTriangleDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,long P3,void *userP);
int bcdtmEdit_drawTriangleLinesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long DrawMode,long P1,long P2,long P3, void *userP);
int bcdtmEdit_drawTriangleBaseLinesForPointDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long DrawMode,long P1, void *userP);
int bcdtmEdit_drawPolygonFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,DPoint3d *polyPtsP,long numPolyPts,double contourInterval,void *userP);
int bcdtmEdit_drawTptrFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPoint,long updateOption,double contourInterval,void *userP);
int bcdtmEdit_drawInternalTptrFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPoint,double contourInterval,void *userP);
int bcdtmEdit_drawExternalTptrFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPoint,long updateOption,double contourInterval,void *userP);
int bcdtmEdit_removeInternalTptrPointsAndRetriangulateDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long fillOption);
int bcdtmEdit_checkForVoidsAlongTptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long *voidsFoundP );
int bcdtmEdit_removeVoidsAlongTptrListDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPnt,double contourInterval,void *userP );
int bcdtmEdit_deleteTrianglesOnDeleteLineDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *deleteLinePtsP,long numDeleteLinePts);
int bcdtmEdit_clipLastDtmFeatureToVoidDtmObject(BC_DTM_OBJ *dtmP );
int bcdtmEdit_drawDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long dtmFeature,void *userP);
int bcdtmEdit_drawPointPerimeterDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long Point,void *userP );
int bcdtmEdit_drawDeletedFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long *deletedFeaturesP,long numDeletedFeatures,double contourInterval,void *userP);


int bcdtmEdit_selectPointDtmObject(BC_DTM_OBJ *dtmP, double x, double y, bool &pointFound, bvector<DPoint3d>& featurePts, long& editPoint);
int bcdtmEdit_findClosestNonVoidPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long *editPointP) ;
int bcdtmEdit_selectLineDtmObject(BC_DTM_OBJ *dtmP,double  x,double y,bool& lineFound,bvector<DPoint3d>& linePts,long& pnt1,long& pnt2 ) ;
int bcdtmEdit_selectTriangleDtmObject( BC_DTM_OBJ *dtmP, double x, double y, bool& trgFound, bvector<DPoint3d>& trgPts, long& pnt1, long& pnt2, long&  pnt3 ) ;
int bcdtmEdit_findClosestHullLineDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long *hullPnt1P,long *hullPnt2P) ;
BENTLEYDTM_Public int bcdtmEdit_findClosestHullLineForPointAddDtmObject (BC_DTM_OBJ *dtmP, double x, double y, long *hullPnt1P, long *hullPnt2P);
int bcdtmEdit_checkForIntersectionWithTinHullDtmObject(BC_DTM_OBJ *dtmP,long point,double x,double y,bool& intersects ) ;
int bcdtmEdit_findClosestVoidLineDtmObject( BC_DTM_OBJ *dtmP,double x,double y,long *voidPnt1P,long *voidPnt2P,long *dtmFeatureP) ;
int bcdtmEdit_findDtmFeatureTypeEnclosingPointDtmObject( BC_DTM_OBJ *dtmP,long point,DTMFeatureType dtmFeatureType,long *dtmFeatureP) ;
int bcdtmEdit_testForPointOnDtmFeatureTypeDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureType dtmFeatureType, long point, long *dtmFeatureP, long *numFeaturesP);
int bcdtmEdit_getVoidExternalToIslandDtmObject(BC_DTM_OBJ *dtmP,long islandFeature,long *voidFeatureP) ;
int bcdtmEdit_getIslandFeaturesInternalToVoidDtmObject(BC_DTM_OBJ *dtmP,long voidFeature,DTMTinPointFeatures **islandsPP,long *numIslandsP) ;
int bcdtmEdit_insertVoidsAndIslandsIntoEditDtmObject(BC_DTM_OBJ *dtmP, BC_DTM_OBJ *dataDtmP) ;
int bcdtmEdit_getListOfIntersectedIslandVoidHoleFeaturesDtmObject(BC_DTM_OBJ *dtmP,long numStartFeatures,DTMTinPointFeatures **intersectedFeaturesPP,long *numIntersectedFeaturesP) ;
int bcdtmEdit_getIslandVoidHoleFeaturesWithACommonHullSegementDtmObject(BC_DTM_OBJ *dtmP,long numStartFeatures,DTMTinPointFeatures **intersectedFeaturesPP,long *numIntersectedFeaturesP) ;
int bcdtmEdit_resolveOverlappingIslandsVoidsAndHolesDtmObject(BC_DTM_OBJ *dtmP,long numStartFeatures,DTMTinPointFeatures *intersectedFeaturesP,long numIntersectedFeatures) ;
int bcdtmEdit_resolveIslandsVoidsHolesDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ **resolvedFeaturesDtmPP) ;
int bcdtmEdit_clipVoidLinesFromDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature );
int bcdtmEdit_clipVoidPointsFromGroupSpotsDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature ) ;
int bcdtmEdit_deleteAllPointsAndLinesInternalToTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long spnt);
int bcdtmEdit_clipDtmFeaturesCoincidentWithTinLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2 ) ;
int bcdtmEdit_checkForOneCoincidentTptrVoidPolygonSectionWithTinHullDtmObject(BC_DTM_OBJ *dtmP,long spnt,long *fsP,long *lsP,long *coincidentFlagP) ;
int bcdtmEdit_deleteInternalVoidPointsAndLinesAndRetriangulateVoidDtmObject(BC_DTM_OBJ *dtmP,long voidFeature) ;
int bcdtmEdit_deleteInternalTptrVoidPolygonPointsAndLinesDtmObject(BC_DTM_OBJ *dtmP,long sPnt) ;
int bcdtmEdit_deleteExternalTptrIslandPolygonPointsAndLinesDtmObject(BC_DTM_OBJ *dtmP,long sPnt) ;
int bcdtmEdit_insertTptrPolygonAroundPointDtmObject(BC_DTM_OBJ *dtmP,long Point,long *Spnt) ;
int bcdtmEdit_addPointToDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double z,long *Point ) ;
int bcdtmEdit_storePointFeaturesInDtmFeatureList (DTMTinPointFeatures **bcdtmList, long *numDtmList, long *memDtmList, long memDtmListInc, long dtmFeature, DTMFeatureType dtmFeatureType, DTMUserTag userTag, long priorPoint, long nextPoint);
int bcdtmEdit_testForDtmFeatureHullLineDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureType DtmFeatureType, long P1, long P2, long *HullLine, long *Feature, long *Direction);
int bcdtmEdit_triangulateVoidDtmObject(BC_DTM_OBJ *dtmP,long voidFeature,long *islandFeaturesP,long numIslandFeatures,long internalPoint) ;
int bcdtmEdit_breakDtmFeatureAtPointDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long breakPoint ) ;
int bcdtmEdit_insertDtmFeatureIntoDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureType dtmFeatureType, DTMUserTag userTag, DPoint3d *stringPtsP, long numStringPts, long *startPntP);
int bcdtmEdit_validateVoidDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *voidPtsP,long *numVoidPtsP);
int bcdtmEdit_insertLineIntoDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureType dtmFeatureType, DTMUserTag userTag, DPoint3d *stringPtsP, long numStringPts, long *startPntP);
int bcdtmEdit_validateStringDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *stringPtsP,long *numStringPtsP);
int bcdtmEdit_validateInsertVoidOrIslandDtmObject(BC_DTM_OBJ *dtmP,long startPoint) ;
int bcdtmEdit_retriangualteAlongTptrListDtmObject(BC_DTM_OBJ *dtmP,long leftSide,long rightSide,long firstPoint) ;
int bcdtmEdit_drawDtmFeaturesForLineDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,void *userP ) ;
int bcdtmEdit_drawContoursForTriangleDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,long P3,double contourInterval,void *userP) ;
int bcdtmEdit_drawTptrLineFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPoint,void *userP) ;
int bcdtmEdit_clearVoidBitOfInternalVoidPointsDtmObject(BC_DTM_OBJ *dtmP,long voidFeature,long *islandsP, long numIslands) ;
int bcdtmEdit_drapeDeleteLineOnEditDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *deleteLinePtsP,long numDeleteLinePts,DPoint3d **drapePoints,long *numDrapePoints) ;
int bcdtmEdit_assignTinLinesToDrapePointsDtmObject(BC_DTM_OBJ *dtmP,DPoint3d *drapePtsP,long numDrapePts,DTM_TRG_INDEX_TABLE **drapeTinLinesP,long *numDrapeTinLines) ;
int bcdtmEdit_createVoidAndIslandPolygonsForDeletedTrianglesDtmObject(BC_DTM_OBJ *dtmP,DTM_TRG_INDEX_TABLE *delPointsP,long numDelPoints,BC_DTM_OBJ **voidPolygonsPP) ;
int bcdtmEdit_extractVoidAndIslandPolygonsFromDeletedTrianglesDtmObject(BC_DTM_OBJ *dtmP,BC_DTM_OBJ **voidPolygonsPP) ;
int bcdtmEdit_accumulateLinearFeaturesAndCursorDistanceForPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long point,SelectedLinearFeatures& linearFeatures) ;
int bcdtmEdit_selectDtmEditLinearFeatureDtmObject(BC_DTM_OBJ *dtmP,double x,double y,double snapTolerance,SelectedLinearFeatures& linearFeatures) ;
int bcdtmEdit_deleteLinearFeatureDtmObject (BC_DTM_OBJ *dtmP, long dtmFeature, DTMFeatureType dtmFeatureType);
int bcdtmEdit_deletePolygonalFeatureDtmObject (BC_DTM_OBJ *dtmP, long dtmFeature, DTMFeatureType dtmFeatureType);
int bcdtmEdit_deleteNonePolygonalFeatureDtmObject (BC_DTM_OBJ *dtmP, long dtmFeature, DTMFeatureType dtmFeatureType);
int bcdtmEdit_getVoidFeaturesInternalToIslandDtmObject (BC_DTM_OBJ *dtmP, DTMFeatureType islandFeature, DTMTinPointFeatures **voidsPP, long *numVoidsP);
int bcdtmEdit_getVoidsInternalToIslandDtmObject(BC_DTM_OBJ *dtmP,long islandFeature,long **voidsPP,long *numVoidsP) ;

BENTLEYDTM_Public int bcdtmEdit_deleteExternalTriangleDtmObject (BC_DTM_OBJ *dtmP, long trgPnt1, long trgPnt2, long trgPnt3);
