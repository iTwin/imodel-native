/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmcleanup.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TerrainModel\Core\bcDTMBaseDef.h>
#include <TerrainModel\Core\dtmevars.h>
#include <TerrainModel\Core\bcdtminlines.h>
#include <TerrainModel\Core\partitionarray.h>

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmList_getDtmFeatureNumsLineDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,bvector<long>& featureNumList)
/*
** This Function Tests If The Line pnt1-pnt2 is A DtmFeature Type Line
*/
    {
    long clc ;
    clc = nodeAddrP(dtmP,pnt1)->fPtr ;
    while ( clc != dtmP->nullPtr )
        {
        if( flistAddrP(dtmP,clc)->nextPnt == pnt2 )
            {
            long num = flistAddrP(dtmP,clc)->dtmFeature;
            bool found = false;

            for (unsigned int i = 0; i < featureNumList.size(); i++)
                {
                if (featureNumList[i] == num)
                    {
                    found = true;
                    break;
                    }
                }
            if (!found)
                featureNumList.push_back (num);
            }
        clc = flistAddrP(dtmP,clc)->nextPtr ;  
        } 
    clc = nodeAddrP(dtmP,pnt2)->fPtr ;
    while ( clc != dtmP->nullPtr )
        {
        if( flistAddrP(dtmP,clc)->nextPnt == pnt1 )
            {
            long num = flistAddrP(dtmP,clc)->dtmFeature;
            bool found = false;

            for (unsigned int i = 0; i < featureNumList.size(); i++)
                {
                if (featureNumList[i] == num)
                    {
                    found = true;
                    break;
                    }
                }
            if (!found)
                featureNumList.push_back (num);
            }
        clc = flistAddrP(dtmP,clc)->nextPtr ;  
        } 
    /*
    ** Job Completed
    */
    return(0) ;
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmList_writeVectorList (bvector<long>& featureNumList)
    {
    for (unsigned int i = 0; i < featureNumList.size(); i++)
        {
        bcdtmWrite_message(0,0,0,"FeatureNum %d", featureNumList[i]) ;
        }
    return (0);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmCleanUp_resolvePolygonalHolesFeatureTypeDtmObject (BC_DTM_OBJ* dtmP, BC_DTM_OBJ* cleanedDtmP, bvector<long>& usedFeatureIndexes, DTMFeatureType featureType, bvector <DTM_TIN_POINT>& points, DTMFeatureId dtmFeatureId)
    {
    int    ret=DTM_SUCCESS;
    long   dtmFeature,numFeaturePts=0 ;  
    DPoint3d    *featurePtsP=NULL ; 
    BC_DTM_OBJ *tempDtmP=NULL ;
    BC_DTM_FEATURE *dtmFeatureP ;

    /*
    **  Create Temporary Object To Store Feature Occurrences
    */
    if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;

    for (unsigned int i = 0; i < usedFeatureIndexes.size(); i++)
        {
        dtmFeatureP = ftableAddrP(dtmP, usedFeatureIndexes [i]) ;

        // This feature was part of an invalid combination so just add this as is to the DTM.
        if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject (dtmP, usedFeatureIndexes [i], &featurePtsP, &numFeaturePts)) goto errexit ;
        if( bcdtmObject_storeDtmFeatureInDtmObject (tempDtmP,DTMFeatureType::Void, dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId, featurePtsP, numFeaturePts)) goto errexit;
        if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
        }

    if (bcdtmData_getHullsForIntersectingPolyonalFeaturesDtmObject (tempDtmP, DTMFeatureType::Void) ) goto errexit;
//    if (bcdtmData_resolveIntersectingPolygonalDtmFeatureTypeDtmObject (tempDtmP, featureType) ) goto errexit;

    if( bcdtmObject_storeDtmFeatureInDtmObject (cleanedDtmP,featureType,cleanedDtmP->nullUserTag,2,&dtmFeatureId,(DPoint3d*)&points[0],(long)points.size())) goto errexit ; 

    for( dtmFeature = 0 ; dtmFeature < tempDtmP->numFeatures ; ++dtmFeature )
        {
        DTMFeatureType newFeatureType;
        dtmFeatureP = ftableAddrP(tempDtmP,dtmFeature) ;
        if (dtmFeatureP->dtmFeatureType == DTMFeatureType::Void)
            newFeatureType = featureType;
        else if (featureType == DTMFeatureType::Island)
            newFeatureType = DTMFeatureType::Void;
        else
            newFeatureType = DTMFeatureType::Island;

        if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureType != DTMFeatureType::Void)
            {
            if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject (tempDtmP, dtmFeature, &featurePtsP, &numFeaturePts)) goto errexit ;
            DTMFeatureId dtmFeatureId = (DTMFeatureId)(-1 - usedFeatureIndexes [0]);
            if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP, DTMFeatureType::Breakline, (DTMUserTag)newFeatureType,2,&dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
            if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
            }
        }

    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
    if( tempDtmP    != NULL )   bcdtmObject_destroyDtmObject(&tempDtmP) ; 
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCleanUp_resolveMultipleIntersectingPolygonalDtmObject
(
 BC_DTM_OBJ *dtmP
 ) 
 /*
 ** This Function Resolves Multiple Intersecting Polygonal Features
 */
    {
    int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   dtmFeature,numFeatureTypes=0,numFeaturePts=0 ;  
    DPoint3d    *featurePtsP=NULL ; 
    BC_DTM_OBJ *polyDtmP=NULL ;
    BC_DTM_OBJ *cleanedDtmP=NULL ;
    BC_DTM_FEATURE *dtmFeatureP ;
    char     dtmFeatureTypeName[50];
    int numVoids;
    long sp, np, hp, ss;
    long numStartFeatures;
    bool isLookingForVoids = true;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Resolving Intersecting Polygonal DTM Feature Type") ;
        bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
        }  
    /*
    ** Check For Valid DTM Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check For Untriangulated DTM
    */
    if( dtmP->dtmState != DTMState::Data )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Check For Valid Feature Types
    */
    //      case DTMFeatureType::Void :
    //      case DTMFeatureType::BreakVoid :
    //      case DTMFeatureType::DrapeVoid :
    //      case DTMFeatureType::Island :
    //      case DTMFeatureType::Hole :
    /*
    ** Count Number Of Feature Types In DTM
    */
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray) && (dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island))
            ++numFeatureTypes ;
        }
    /*
    ** Only Process If There Are More Than One Occurrence Of The Feature Type
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"numFeatureTypes = %8ld",numFeatureTypes) ;
    if( numFeatureTypes > 1 )
        {
        /*
        **  Create Temporary Object To Store Feature Occurrences
        */
        if( bcdtmObject_createDtmObject(&polyDtmP)) goto errexit ;

        /*
        **  Create Temporary Object to Store the Cleaned FeaturesResult
        */
        if( bcdtmObject_createDtmObject(&cleanedDtmP)) goto errexit ;
        bcdtmObject_setPointMemoryAllocationParametersDtmObject(polyDtmP,10000,10000) ;
        polyDtmP->ppTol = dtmP->ppTol ; 
        polyDtmP->plTol = dtmP->plTol ;
        /*
        **  Move Features To Temporary DTM
        */
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray) && (dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island))
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                DTMFeatureId dtmFeatureId = (DTMFeatureId)dtmFeature;
                if( bcdtmObject_storeDtmFeatureInDtmObject(polyDtmP,DTMFeatureType::Breakline, (DTMUserTag)dtmFeatureP->dtmFeatureType,2,&dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                }
            }

        /*
        **   Loop till they are no features.
        */
        bool changed = false;
        do
            {
            changed = false;
            polyDtmP->ppTol = 0.0 ;
            polyDtmP->plTol = 0.0 ;  
            if( bcdtmObject_createTinDtmObject(polyDtmP,1,0.0, false)) goto errexit ;

//            bcdtmList_nullTptrValuesDtmObject (polyDtmP);

            /*
            ** Remove None Feature Hull Lines
            */
            if( dbg ) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
            if( bcdtmList_removeNoneFeatureHullLinesDtmObject(polyDtmP)) goto errexit ;
            if( dbg ) bcdtmWrite_toFileDtmObject(polyDtmP,L"voidHulls.bcdtm") ;
            /*
            ** Report DTM Stats
            */
            if( dbg == 2 ) bcdtmObject_reportStatisticsDtmObject(polyDtmP) ;

            /*
            ** Set Pointer To Last Feature In Tin
            */
            numStartFeatures = dtmP->numFeatures ;
            /*
            ** Scan Tin Hull To Get Voids
            */
            if( dbg )bcdtmWrite_message(0,0,0,"Scanning For External %s",dtmFeatureTypeName) ;
            numVoids = 0 ;
            sp = polyDtmP->hullPoint ;
            do
                {
                np = nodeAddrP(polyDtmP,sp)->hPtr ;
                if( nodeAddrP(polyDtmP,sp)->tPtr == polyDtmP->nullPnt && bcdtmList_testForBreakLineDtmObject(polyDtmP,sp,np) )
                    {
                    bvector<DTM_TIN_POINT> points;
                    bvector<long> usedFeatureIndexes;

                    /*
                    **        Scan Around External Edge Of Break Lines and get the Feature Numbers which are on this feature.
                    */
//                    bcdtmList_getDtmFeatureNumsLineDtmObject (polyDtmP, sp,np, usedFeatureIndexes);
                    hp = sp ;
                    nodeAddrP(polyDtmP,sp)->tPtr = np ;
                    do
                        { 
                        if( ( hp = bcdtmList_nextAntDtmObject(polyDtmP,np,hp)) < 0 ) goto errexit ;
                        while ( ! bcdtmList_testForBreakLineDtmObject(polyDtmP,np,hp))
                            {
                            if( ( hp = bcdtmList_nextAntDtmObject(polyDtmP,np,hp)) < 0 ) goto errexit ;
                            }

                        nodeAddrP(polyDtmP,np)->tPtr = hp ;
                        ss = hp ;
                        hp = np ;
                        np = ss ;
                        } while ( hp != sp ) ;
                        /*
                        **        Check Connectivity
                        */
                        if( bcdtmList_checkConnectivityTptrListDtmObject(polyDtmP,sp,1)) goto errexit ;
                        if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(polyDtmP,sp) ;
                        if( dbg == 2 ) bcdtmList_writeVectorList (usedFeatureIndexes);

                            {
                            int tp = sp;
                            int np;
                            points.push_back( *pointAddrP(polyDtmP, sp));
                            while ( nodeAddrP(polyDtmP,tp)->tPtr != polyDtmP->nullPnt && nodeAddrP(polyDtmP,tp)->tPtr >= 0 )
                                {
                                np = nodeAddrP(polyDtmP,tp)->tPtr ; 
                                nodeAddrP(polyDtmP,tp)->tPtr = -(np+1) ;
                                points.push_back( *pointAddrP(polyDtmP, np));
                                bcdtmList_getDtmFeatureNumsLineDtmObject (polyDtmP, tp,np, usedFeatureIndexes);
                                tp = np ;
                                }
                                bcdtmList_getDtmFeatureNumsLineDtmObject (polyDtmP, tp,np, usedFeatureIndexes);
                            /*
                            **  Reset Tptr Values Positive
                            */
                            tp = sp ;
                            while( nodeAddrP(polyDtmP,tp)->tPtr < 0  ) 
                                {
                                np = -(nodeAddrP(polyDtmP,tp)->tPtr + 1 ) ;
                                nodeAddrP(polyDtmP,tp)->tPtr = np ;
                                tp = np ;
                                }
                            }
                                                

                        /*
                        **        Store Void Feature In Tin
                        */
                        if (usedFeatureIndexes.size () == 1)
                            {
                            /*
                            ** As they are only one feature which made up this element, it must be closed so add this element to the cleaned DTM as it should be the same as the original.
                            */
                            dtmFeatureP = ftableAddrP(polyDtmP, usedFeatureIndexes [0]);
                            DTMFeatureType useFeatureType = (DTMFeatureType)dtmFeatureP->dtmUserTag;
                            // If this is a type we are looking for then process it otherwise it will be processed on the next go round.
                            if ((useFeatureType == DTMFeatureType::Island && !isLookingForVoids) || (useFeatureType != DTMFeatureType::Island && isLookingForVoids))
                                {
                                dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted;
                                // If the dtmFeatureId is negative this is a result of a hole in more than one feature.
                                if (dtmFeatureP->dtmFeatureId < 0)
                                    {
                                    dtmFeatureP = ftableAddrP(dtmP, -1 - (long)dtmFeatureP->dtmFeatureId);
                                    if( bcdtmObject_storeDtmFeatureInDtmObject (cleanedDtmP,useFeatureType, -999 ,2,&dtmFeatureP->dtmFeatureId,(DPoint3d*)&points[0],(long)points.size())) goto errexit ;
                                    }
                                else
                                    {
                                    dtmFeatureP = ftableAddrP(dtmP, (long)dtmFeatureP->dtmFeatureId);
                                    if( bcdtmObject_storeDtmFeatureInDtmObject (cleanedDtmP,useFeatureType, dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId,(DPoint3d*)&points[0],(long)points.size())) goto errexit ;
                                    }
                                }
                            changed = true;
                            }
                        else
                            {
                            /*
                            ** OK This polygon feature is made from multiple features.
                            ** See if they are made from the same feature Type.
                            ** Ignore Islands if they are voids, so we can have an island on the edge of a void. (disabled for now as the engine doesn't like this.)
                            */
                            DTMFeatureType voidFeatureType = DTMFeatureType::None;
                            bool valid = false;
                            int numVoids = 0;
                            int numIslands = 0;
//                            int numHoles = 0;

                            dtmFeatureP = ftableAddrP(polyDtmP, usedFeatureIndexes [0]);
                            dtmFeatureP = ftableAddrP(dtmP, dtmFeatureP->dtmFeatureId < 0 ? -1 - (long)dtmFeatureP->dtmFeatureId : (long)dtmFeatureP->dtmFeatureId);
                            DTMFeatureId useFeatureId = dtmFeatureP->dtmFeatureId;

                            for (unsigned int i = 0; i < usedFeatureIndexes.size(); i++)
                                {
                                dtmFeatureP = ftableAddrP(polyDtmP, usedFeatureIndexes [i]) ;
                                DTMFeatureType dtmFeatureType = (DTMFeatureType)dtmFeatureP->dtmUserTag;
                                if (dtmFeatureType == DTMFeatureType::Island)
                                    numIslands++;
//                                else if (dtmFeatureType == DTMFeatureType::Hole)
//                                    numHoles++;
                                else
                                    {
                                    numVoids++;
                                    if (voidFeatureType == DTMFeatureType::None)
                                        {
                                        valid = true;
                                        voidFeatureType = dtmFeatureType;
                                        }
                                    else
                                        {
                                        if (voidFeatureType != dtmFeatureType)
                                            valid = false;
                                        }
                                    }
                                }

                            if (!isLookingForVoids)
                                {
                                if (numIslands == 0)
                                    {
                                    valid = true;   // Don't care that there are a mixure as they are in voids anyway.
                                    }
                                else
                                    {
                                    if (numVoids == 0)
                                        {
                                        bcdtmCleanUp_resolvePolygonalHolesFeatureTypeDtmObject (polyDtmP, cleanedDtmP, usedFeatureIndexes, DTMFeatureType::Island, points, useFeatureId);
                                        }
                                    else
                                        {
                                        // Mixure of voids and islands. Need to get the island and clip it with the island.
                                        // Fail for now.
                                        valid = false;
                                        }
                                    }
                                }
                            else
                                {
                                if (numVoids == 0)
                                    {
                                    // Islands not in a void.
                                    valid = true;
                                    }
                                else
                                    {
                                    if (numIslands == 0)
                                        {
                                        bcdtmCleanUp_resolvePolygonalHolesFeatureTypeDtmObject (polyDtmP, cleanedDtmP, usedFeatureIndexes, voidFeatureType, points, useFeatureId);
                                        }
                                    else
                                        {
                                        // Mixure of voids and islands. Need to get the island and clip it with the island.
                                        // Fail for now.
                                        valid = false;
                                        }
                                    }
                                }

                            changed = true;
                            for (unsigned int i = 0; i < usedFeatureIndexes.size(); i++)
                                {
                                dtmFeatureP = ftableAddrP(polyDtmP, usedFeatureIndexes [i]) ;

                                //if (!valid)
                                //    {
                                //    // This feature was part of an invalid combination so just add this as is to the DTM.
                                //    if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(polyDtmP,usedFeatureIndexes [i], &featurePtsP,&numFeaturePts)) goto errexit ;
                                //    if( bcdtmObject_storeDtmFeatureInDtmObject(cleanedDtmP,dtmFeatureP->dtmFeatureType, dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId, featurePtsP, numFeaturePts)) goto errexit;
                                //    if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                                //    }
                                dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted;
                                }

                            }
                        ++numVoids ;
                    }
                sp = np ; 
                } while ( sp != polyDtmP->hullPoint ) ;
                isLookingForVoids = !isLookingForVoids;
                /*
                **  Remove Deleted Features
                */
                if( bcdtmData_compactFeatureTableDtmObject(polyDtmP)) goto errexit ;  

                if (polyDtmP->numFeatures == 0)
                    break;
                if ( bcdtmObject_changeStateDtmObject(polyDtmP,DTMState::Data)) goto errexit ;
            }
            while (changed);

            for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
                {
                dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
                if( (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray) && (dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island))
                    {
                    if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                    }
                }

            /*
            **  Copy Intersected Features To DTM
            */
            for( dtmFeature = 0 ; dtmFeature < cleanedDtmP->numFeatures ; ++dtmFeature )
                {
                dtmFeatureP = ftableAddrP(cleanedDtmP,dtmFeature) ;
                if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray)
                    {
                    if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(cleanedDtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;

                    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                    if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                    } 
                }
        }
    if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"cleaned.bcdtm") ;
    /*
    ** Clean Up
    */
cleanup :
    if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
    if( polyDtmP    != NULL )   bcdtmObject_destroyDtmObject(&polyDtmP) ; 
    if( cleanedDtmP != NULL )   bcdtmObject_destroyDtmObject(&cleanedDtmP) ; 
    /*
    ** Return
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Polygonal DTM Feature Type Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Polygonal DTM Feature Type Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmClean_validateDtmFeaturesDtmObject
(
 BC_DTM_OBJ *dtmP,                        // ==> Dtm Object
 long       forceClose,                   // ==> Force Close Polygonal DTM Features
 double     closeTolerance,               // ==> Close Tolerance For DTM Features  
 double     filterTolerance,              // ==> Filter Tolerance For Dtm Feature Points 
 int        onlyValidatePolygonalFeatures,// ==> Only check PolygonalFeatures
 long       *numErrorsP                   // <== Number Of Features With Errors 
)
/*
** This Function Cleans Dtm Features In A Dtm Object
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs1,ofs2,ofs3,ofs4,point,dtmFeature,dtmFeature2,numFeaturePts=0,closeFlag; 
 long   numFeatures=0,validateResult,polygonalFeature=FALSE,numStartFeatures ;
 DPoint3d    *p3dP,*featurePtsP=NULL ; 
 DTM_TIN_POINT  *pointP,*point1P,*point2P ;
 BC_DTM_FEATURE *dtmFeatureP, *dtmFeature2P ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Validating Dtm Features") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"closeTolerance  = %12.8lf",closeTolerance) ;
    bcdtmWrite_message(0,0,0,"filterTolerance = %12.8lf",filterTolerance) ;
   }
/*
** Initialise
*/
 *numErrorsP = 0 ;
/*
** Test For Valid Data Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ; 
/*
** Check DTM Is In Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulted DTM") ;
    goto errexit ;
   }
/*
** Scan Dtm Features For Dtm Feature Type
*/
 numStartFeatures = dtmP->numFeatures ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**  Only Process Data Features
*/
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
      {
       validateResult = 0;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Validating DTM Feature[%8ld] ** Number Of Feature Points = %6ld",dtmFeature,dtmFeatureP->numDtmFeaturePts) ;
       ++numFeatures ;
/*
**     Check For Polygonal Feature
*/
       polygonalFeature = FALSE ;
       if     ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void       ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole       ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island     ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull       ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Polygon    ) polygonalFeature = TRUE ;
       else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Region     ) polygonalFeature = TRUE ;

       if (!polygonalFeature || onlyValidatePolygonalFeatures)
           continue;
/*
**     Allocate Memory For Feature Points
*/
       numFeaturePts = dtmFeatureP->numDtmFeaturePts ;
       featurePtsP = ( DPoint3d * ) malloc ( numFeaturePts * sizeof(DPoint3d)) ;
       if( featurePtsP == NULL ) 
         { 
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
/*
**     Copy Feature Points To Point Array
*/
       ofs1 = dtmFeatureP->dtmFeaturePts.firstPoint ;
       ofs2 = ofs1 + dtmFeatureP->numDtmFeaturePts - 1 ; 
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Feature Start Offset = %6ld Feature End Offset = %6ld",ofs1,ofs2) ;
       for( point = ofs1 , p3dP = featurePtsP ; point <= ofs2 ; ++point , ++p3dP ) 
         { 
          pointP = pointAddrP(dtmP,point) ;
          p3dP->x = pointP->x ; 
          p3dP->y = pointP->y ; 
          p3dP->z = pointP->z ; 
         }
/*
**     Check For Closure
*/
       closeFlag = 0 ;
       if( featurePtsP->x == (featurePtsP+numFeaturePts-1)->x && featurePtsP->y == (featurePtsP+numFeaturePts-1)->y ) closeFlag = 1 ;
       if( dbg == 2 && ! closeFlag ) bcdtmWrite_message(0,0,0,"Open Feature") ;
       if( dbg == 2 &&   closeFlag ) bcdtmWrite_message(0,0,0,"Closed Feature") ;
/*
**     Check For Closure Within Close Tolerance
*/
       if( !closeFlag && bcdtmMath_distance(featurePtsP->x,featurePtsP->y,(featurePtsP+numFeaturePts-1)->x,(featurePtsP+numFeaturePts-1)->y) <= closeTolerance )
         {
          featurePtsP->x = (featurePtsP+numFeaturePts-1)->x ;
          featurePtsP->y = (featurePtsP+numFeaturePts-1)->y ;
          featurePtsP->z = (featurePtsP+numFeaturePts-1)->z ;
          closeFlag = 1 ;
         } 
/*
**     Close Polygonal Features
*/
       if( polygonalFeature == TRUE && ! closeFlag && forceClose )
         {
          featurePtsP->x = (featurePtsP+numFeaturePts-1)->x ;
          featurePtsP->y = (featurePtsP+numFeaturePts-1)->y ;
          featurePtsP->z = (featurePtsP+numFeaturePts-1)->z ;

          long IntFlag = 0;
          if (bcdtmData_checkPolygonForKnots (featurePtsP, numFeaturePts, &IntFlag)) goto errexit;
          if ((IntFlag & 1) != 1)
            closeFlag = 1 ;
         }
/*
**     Check For Close Error
*/
       if( polygonalFeature && ! closeFlag )
         {         
         validateResult = 1;
          if( dbg ) bcdtmWrite_message(0,0,0,"Close Error In Feature") ; 
         } 
/*
**     Validate Feature Points
*/ 
       else
         { 
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Number Of Points Before Validate = %6ld",numFeaturePts) ;
          if( ! closeFlag ) validateResult = bcdtmClean_validateStringP3D(&featurePtsP,&numFeaturePts,filterTolerance) ;
          else
              {
              validateResult = bcdtmClean_validatePointArrayPolygon(&featurePtsP,&numFeaturePts,1,filterTolerance) ;
              if (!validateResult)
            {
                 DTMDirection direction;
                 double area;
                 if (bcdtmMath_getPolygonDirectionP3D (featurePtsP, numFeaturePts, &direction, &area) || area < 1e-8)
                            validateResult = 1;
                    if (area < 1e-8)
                        validateResult = 1;
            }
              }

/*
**        If No Validation Errors Store Validated Points 
*/
            if( !validateResult)
            {
/*
**           Expand for New Points
*/
             if( numFeaturePts > dtmFeatureP->numDtmFeaturePts )
               {
                ofs3 = numFeaturePts - dtmFeatureP->numDtmFeaturePts;
                
                ofs2 = dtmP->numPoints - 1;
                ofs4 = dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts - 1;
                dtmP->numPoints += ofs3;
                if (dtmP->numPoints > dtmP->memPoints)
                    {
                    if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP)) goto errexit  ;
                    }

                // increase the size of points
                ofs1 = dtmP->numPoints - 1;
                while( ofs2 > ofs4 ) 
                  { 
                   point1P = pointAddrP(dtmP,ofs1) ;
                   point2P = pointAddrP(dtmP,ofs2) ;
                   *point1P = *point2P ;
                   --ofs1 ;
                   --ofs2 ; 
                  } 
                dtmFeatureP->numDtmFeaturePts = numFeaturePts ;
/*
**              Adjust First Point Offset For Remaing Features
*/
                for( dtmFeature2 = dtmFeature + 1 ; dtmFeature2 < dtmP->numFeatures ; ++dtmFeature2 )
                  {
                   dtmFeature2P = ftableAddrP(dtmP,dtmFeature2) ;
                   if( dtmFeature2P->dtmFeatureState == DTMFeatureState::Data ) dtmFeature2P->dtmFeaturePts.firstPoint += ofs3 ;
                  }
               ofs1 = dtmFeatureP->dtmFeaturePts.firstPoint ;
               }

             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Number Of Points  After Validate = %6ld",numFeaturePts) ;
             for( point = ofs1 , p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++point , ++p3dP ) 
               { 
                pointP = pointAddrP(dtmP,point) ;
                pointP->x = p3dP->x ; 
                pointP->y = p3dP->y ; 
                pointP->z = p3dP->z ; 
               }
/*
**           Copy Over Deleted Points
*/
             if( numFeaturePts < dtmFeatureP->numDtmFeaturePts )
               {
                ofs3 = dtmFeatureP->numDtmFeaturePts - numFeaturePts ;
                dtmFeatureP->numDtmFeaturePts = numFeaturePts ;
                ofs1 = ofs1 + numFeaturePts ;
                ++ofs2 ;
                while( ofs2 < dtmP->numPoints ) 
                  { 
                   point1P = pointAddrP(dtmP,ofs1) ;
                   point2P = pointAddrP(dtmP,ofs2) ;
                   *point1P = *point2P ;
                   ++ofs1 ;
                   ++ofs2 ; 
                  } 
                dtmP->numPoints = ofs1 ;
/*
**              Adjust First Point Offset For Remaing Features
*/
                for( ofs1 = dtmFeature + 1 ; ofs1 < dtmP->numFeatures ; ++ofs1 )
                  {
                   dtmFeatureP = ftableAddrP(dtmP,ofs1) ;
                   if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data ) dtmFeatureP->dtmFeaturePts.firstPoint -= ofs3 ;
                  }
               }
/*
**           Set Polygonal Feature Anti Clockwise
*/
             if( polygonalFeature == TRUE )
               {
                if( bcdtmClean_setDtmPolygonalFeatureAntiClockwiseDtmObject(dtmP,dtmFeature)) goto errexit ;
               }
            }
                if( validateResult)
                    {
                    ++*numErrorsP ;
                    if (bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeatureP->dtmFeatureId))
                        goto errexit;

                    dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted;
                    dtmFeatureP->numDtmFeaturePts = 0 ;

                    if( dbg ) bcdtmWrite_message(0,0,0,"Validation Errors In Feature") ; 
                    }
                }
         } 
/*
**     Free Feature Points Memory
*/
       free(featurePtsP) ; featurePtsP = NULL ;
      }
/*
** Log Number Of Errors
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number With Errors = %8ld of %8ld",*numErrorsP,numFeatures) ;
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP  != NULL ) { free(featurePtsP)  ; featurePtsP = NULL  ; }
/*
**  Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating DTM Feature Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating DTM Feature Error") ;
 return(ret) ;
/*
** Error Return
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ; 
}



int bcdtmCleanUp_joinVoidsAndHoles (BC_DTM_OBJ *dtmP)
    {
    int ret = DTM_SUCCESS;
    long numBeforeJoin;
    long numAfterJoin;
    DTM_JOIN_USER_TAGS* joinUserTagsP = NULL;
    long numJoinUserTags = 0;
    DTM_ROLLBACK_DATA* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : NULL;

    if (rollBackInfo) rollBackInfo->rollBackMapInitialized = false;
    if (bcdtmJoin_dtmFeatureTypeWithRollbackDtmObject ( dtmP, dtmP->ppTol, DTMFeatureType::VoidLine, DTMFeatureType::Void, &numBeforeJoin, &numAfterJoin, &joinUserTagsP, &numJoinUserTags, 1)) goto errexit;

    if (rollBackInfo) rollBackInfo->rollBackMapInitialized = false;
    if (bcdtmJoin_dtmFeatureTypeWithRollbackDtmObject ( dtmP, dtmP->ppTol, DTMFeatureType::HoleLine, DTMFeatureType::Hole, &numBeforeJoin, &numAfterJoin, &joinUserTagsP, &numJoinUserTags, 1)) goto errexit;
    /*
    ** Clean Up
    */
cleanup :
    if (joinUserTagsP) free (joinUserTagsP);
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }

int bcdtmCleanUp_joinHullLines (BC_DTM_OBJ *dtmP)
    {
    int ret = DTM_SUCCESS;
    int dtmFeature;
    long numBeforeJoin;
    long numAfterJoin;
    DTM_JOIN_USER_TAGS* joinUserTagsP = NULL;
    long numJoinUserTags = 0;
    DTM_ROLLBACK_DATA* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : NULL;

    if (rollBackInfo) rollBackInfo->rollBackMapInitialized = false;
    if (bcdtmJoin_dtmFeatureTypeWithRollbackDtmObject ( dtmP, dtmP->ppTol, DTMFeatureType::HullLine, DTMFeatureType::HullLine, &numBeforeJoin, &numAfterJoin, &joinUserTagsP, &numJoinUserTags, 1)) goto errexit;

    if (numAfterJoin == 1)
        {
        BC_DTM_FEATURE *dtmFeatureP;
        /*
        ** Scan Dtm Features For Dtm Feature Type
        */
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray)
                {
                if (dtmFeatureP->dtmFeatureType == DTMFeatureType::HullLine)
                    {
                    long ofs1 = dtmFeatureP->dtmFeaturePts.firstPoint ;
                    long ofs2 = ofs1 + dtmFeatureP->numDtmFeaturePts - 1 ;
                    DTM_TIN_POINT* startPtP = pointAddrP(dtmP,ofs1) ;
                    DTM_TIN_POINT* endPtP = pointAddrP(dtmP,ofs2) ;
                    if( bcdtmMath_distance(startPtP->x,startPtP->y,endPtP->x,endPtP->y) <= dtmP->ppTol)
                        dtmFeatureP->dtmFeatureType = DTMFeatureType::Hull;
                    break;
                    }
                }
            }
        }

    /*
    ** Clean Up
    */
cleanup :
    if (joinUserTagsP) free (joinUserTagsP);
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmCleanUp_checkHullFeatures (BC_DTM_OBJ* dtmP)
    {
    int    ret=DTM_SUCCESS,dbg=0;
    long dtmFeature;  
    int numHulls = 0;
    int numDrapeHulls= 0;
    int numHullLines = 0;
    bool removeDrapeHulls = false;
    bool removeHullLines = false;
    int numRemoved = 0;
    BC_DTM_FEATURE *dtmFeatureP;
    int n;
    DTMMemPnt featPtsPI = 0;
    DPoint3d     *featPtsP=NULL, *pntP=NULL;

    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"bcdtmCleanUp_checkboundayFeatures") ;
        bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
        }
    /*
    ** Test For Valid Data Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check DTM Is In Data State
    */
    if( dtmP->dtmState != DTMState::Data )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Untriangulted DTM") ;
        goto errexit ;
        }
    /*
    ** Scan Dtm Features For Dtm Feature Type
    */
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray)
            {
            if (dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeHull)
                numDrapeHulls++;
            else if (dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull)
                numHulls++;
            else if (dtmFeatureP->dtmFeatureType == DTMFeatureType::HullLine)
                numHullLines++;
            }
        }

    if (numDrapeHulls != 0 && numHulls != 0)
        removeDrapeHulls = true;

    if (numHullLines != 0 && numHulls != 0 || numDrapeHulls != 0)
        removeHullLines = true;

    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"numHulls = %d", numHulls) ;
        bcdtmWrite_message(0,0,0,"numDrapeHulls = %d", numDrapeHulls) ;
        bcdtmWrite_message(0,0,0,"numHullLines = %d", numHullLines) ;
        bcdtmWrite_message(0,0,0,"removeDrapeHulls = %d", removeDrapeHulls) ;
        bcdtmWrite_message(0,0,0,"removeHullLines = %d", removeHullLines) ;
        }

    if (removeDrapeHulls || removeHullLines)
        {
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray)
                {
                if ((removeDrapeHulls && dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeHull) ||
                    (removeHullLines && dtmFeatureP->dtmFeatureType == DTMFeatureType::HullLine))
                    {
                    numRemoved++;
                    if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Data)
                        {
                        if( dbg ) bcdtmWrite_message(0,0,0,"Feature Insert Error") ;
                        featPtsPI = bcdtmMemory_allocate(dtmP, dtmFeatureP->numDtmFeaturePts * sizeof(DPoint3d));
                        featPtsP  = bcdtmMemory_getPointerP3D(dtmP, featPtsPI);
                        if( featPtsP == NULL )
                            {
                            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                            goto errexit ;
                            }

                        for( n = 0 , pntP = featPtsP ; n < dtmFeatureP->numDtmFeaturePts ; ++n , ++pntP )
                         {
                          int point  = dtmFeatureP->dtmFeaturePts.firstPoint + n ;
                          DPoint3d* pointP = (DPoint3d*)pointAddrP (dtmP, point) ;
                              *pntP  = *pointP ;
                         }
                        dtmFeatureP->dtmFeaturePts.pointsPI = featPtsPI ;
                        featPtsP = NULL ;
                        }
                    dtmFeatureP->dtmFeatureState = DTMFeatureState::TinError ;
                    }
                }
            }
        }

    if (numHullLines != 0 && numHulls == 0 || numDrapeHulls == 0)
        bcdtmCleanUp_joinHullLines (dtmP);

    if (dbg)
        bcdtmWrite_message (0,0,0, "removed %d features", numRemoved);
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }


int bcdtmCleanUp_resolveVoidAndIslandsDtmObject (BC_DTM_OBJ *dtmP)
    {
    int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
    BC_DTM_OBJ* bndyDtmP = NULL;
    BC_DTM_FEATURE *dtmFeatureP;
    int dtmFeature;
    DPoint3d  *featurePtsP=NULL ;
    long numFeaturePts = 0;
    int k = 0;
//    DTM_ROLLBACK_DATA* rollBackInfo = dtmP->extended ? dtmP->extended->rollBackInfoP : NULL;

    DTMFeatureId dtmFeatureId ;
    BC_DTM_FEATURE *dtmFeature2P=NULL ;
    int dtmFeature2;
    bvector<long> voidFeatureIndexes;
    int numVoids, numIslands;

    /*
    ** Write Entry Message
    */
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Resolving Island Void Boundaries") ; 
        bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
        }
    /*
    ** Test For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP))
        goto errexit  ;
    /*
    ** Check If Boundary DTM Is In Data State
    */
    if( dtmP->dtmState != DTMState::Data )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Un-Triangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Log DTM Stats
    */
    if( dbg == 1 )
        {
        bcdtmObject_reportStatisticsDtmObject(dtmP) ;
        } 

    if( bcdtmObject_createDtmObject(&bndyDtmP))
        goto errexit ;

    //  Copy Void Island Features

    if (dbg) bcdtmWrite_message(0,0,0,"Find all voids and Islands etc") ;

    numVoids = 0;
    numIslands = 0;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )
            {
            if( dtmFeatureP->numDtmFeaturePts > 0 )
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject (dtmP,dtmFeature,&featurePtsP,&numFeaturePts))
                    goto errexit ;

                if (featurePtsP[0].x != featurePtsP[numFeaturePts - 1].x || featurePtsP[0].y != featurePtsP[numFeaturePts - 1].y)
                    {
                    if (dbg) bcdtmWrite_message (0,0,0,"None closed feature ignoring");
                    continue;
                    }
                if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::BreakVoid || dtmFeatureP->dtmFeatureType == DTMFeatureType::DrapeVoid) ++numVoids ;
                else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
                Int64 featureId = dtmFeature;
                if( bcdtmObject_storeDtmFeatureInDtmObject(bndyDtmP,dtmFeatureP->dtmFeatureType, featureId, 2, &featureId, featurePtsP, numFeaturePts))
                    goto errexit ;
                voidFeatureIndexes.push_back (dtmFeature);
                }
            } 
        }

    if (voidFeatureIndexes.size() != 0)
        {
        // call clean up
        /*
        **  Resolve Voids
        */
        if( numVoids > 1 || numIslands > 1 )
            {
            if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Voids") ;
            if( bcdtmCleanUp_resolveMultipleIntersectingPolygonalDtmObject (bndyDtmP))
                goto errexit ;
            if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Voids Resolved") ;
            }  

        // Find all features which haven't changed.
        for( dtmFeature = 0 ; dtmFeature < bndyDtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(bndyDtmP,dtmFeature) ;

            if (dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted)
                {
                if (dtmFeatureP->dtmUserTag == dtmFeatureP->dtmFeatureId && dtmFeatureP->dtmFeatureId >= 0)
                    {
                    for (unsigned int i = 0; i < voidFeatureIndexes.size(); i++)
                        {
                        if (voidFeatureIndexes[i] == dtmFeatureP->dtmUserTag)
                            {
                            voidFeatureIndexes[i] = -1;
                            break;
                            }
                        }
                    }
                }
            }

        // Add changed features as Rollback features.
        for (unsigned int i = 0; i < voidFeatureIndexes.size(); i++)
            {
            if (voidFeatureIndexes[i] >= 0)
                {
                dtmFeature2 = (long)voidFeatureIndexes[i];
                dtmFeature2P = ftableAddrP(dtmP, dtmFeature2) ;

                if (bcdtmInsert_rollBackDtmFeatureDtmObject (dtmP, dtmFeature2P->dtmFeatureId))
                    goto errexit;
                if (dtmFeature2P->dtmFeatureState != DTMFeatureState::Data && dtmFeature2P->dtmFeatureState != DTMFeatureState::Tin)
                    {
                    if( dtmFeature2P->dtmFeaturePts.pointsPI != 0)
                        {
                        bcdtmMemory_free(dtmP,dtmFeature2P->dtmFeaturePts.pointsPI) ;
                        dtmFeature2P->dtmFeaturePts.pointsPI = 0;
                        }
                    }
                dtmFeature2P->dtmFeatureState = DTMFeatureState::Deleted;
                dtmFeature2P->numDtmFeaturePts = 0 ;
                }
            }
        }
        // Copy Features back to DTM reusing the Feature Id/UserTags.
        for( dtmFeature = 0 ; dtmFeature < bndyDtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(bndyDtmP,dtmFeature) ;

            if (dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted)
                {
                if (dtmFeatureP->dtmUserTag == dtmFeatureP->dtmFeatureId && dtmFeatureP->dtmFeatureId >= 0)
                    {
                    // No need to do anything here.
                    }
                else
                    {
                    while (k < (int)voidFeatureIndexes.size())
                        {
                        long featureNum = voidFeatureIndexes[k++];
                        if (featureNum != -1)
                            {
                            dtmFeature2 = featureNum;
                            break;
                            }
                        };
                    dtmFeatureId = ftableAddrP(dtmP, dtmFeature2)->dtmFeatureId;
                    if (dbg) bcdtmWrite_message(0,0,0," ----- Adding cleanup void = %d %d %d", (long)dtmFeature2P->dtmFeatureId, (long)dtmFeature2P->dtmUserTag, dtmFeatureP->numDtmFeaturePts) ;
                    if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(bndyDtmP,dtmFeature,&featurePtsP,&numFeaturePts))
                        goto errexit ;
                    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,dtmFeatureP->dtmFeatureType,dtmFeature2P->dtmUserTag,2,&dtmFeatureId,featurePtsP,numFeaturePts))
                        goto errexit ;
                    }
                }
            }

    /*
    ** Clean Up
    */
cleanup :
    if( featurePtsP  != NULL ) free(featurePtsP) ;
    if( bndyDtmP     != NULL ) bcdtmObject_destroyDtmObject(&bndyDtmP) ; 
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }

static int (*bcdtmCleanUp_cleanDtmObjectOverrideP) (BC_DTM_OBJ *dtmP) = NULL;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCleanUp_overrideCleanDtmObject (int (*overrideP) (BC_DTM_OBJ *dtmP))
    {
    bcdtmCleanUp_cleanDtmObjectOverrideP = overrideP;
    return DTM_SUCCESS;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCleanUp_cleanDtmObject (BC_DTM_OBJ *dtmP)
    {
    int dbg=DTM_TRACE_VALUE(0),ret=DTM_SUCCESS;
    long validationErrors;

    if (bcdtmCleanUp_cleanDtmObjectOverrideP)
        return bcdtmCleanUp_cleanDtmObjectOverrideP (dtmP);

    if ( bcdtmObject_testApiCleanUpDtmObject (dtmP, DTMCleanupFlags::VoidsAndIslands))
        {
        bcdtmCleanUp_checkHullFeatures (dtmP);
        //  Join up Void Holes Line Features
        bcdtmCleanUp_joinVoidsAndHoles (dtmP);
        //  Validate Void Island Features
        if( dbg ) bcdtmWrite_message(0,0,0,"Validate Void Island Features") ;
        bcdtmClean_validateDtmFeaturesDtmObject (dtmP,1,dtmP->ppTol,dtmP->ppTol,0,&validationErrors);
        if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Validation Errors = %8ld",validationErrors) ;
        if( validationErrors > 0 ) 
          {
           bcdtmWrite_message(0,0,0,"Errors In Void Island Features") ;
          }
        if (bcdtmCleanUp_resolveVoidAndIslandsDtmObject (dtmP)) goto errexit;
        }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
