/*--------------------------------------------------------------------------------------+
|
|     $Source: consoleApps/bcDrainage.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#include "stdafx.h"
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtmInlines.h"
#include "bcDtmClass.h"

#include <TerrainModel/Drainage/Drainage.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL


class DTMDrainageTables ;

int bcdtmDrainage_testDrainageMethods(BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr ibcDtm) ;
int bcdtmDrainage_callBackFunction(DTMFeatureType dtmFeatureType,DTMUserTag userTag,DTMFeatureId featureId,DPoint3d *featurePtsP,size_t numFeaturePts,void *userP) ;

static int numDtmFeatures = 0 ;

int wmain(int argc, wchar_t *argv[])
    {
    int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(1),cdbg=DTM_CHECK_VALUE(0) ;
    long    startTime=bcdtmClock() ;
    BC_DTM_OBJ *dtmP=nullptr ;
    BcDTMPtr ibcDtm=nullptr ;

    // Initialise DTM

    bcdtmInitialise() ;
    if(fpLOG != NULL) { fclose(fpLOG) ; fpLOG = NULL ; }
    bcdtmInitialise_openLogFile(L"bcDrainage.log") ;
    bcdtmWrite_message(0,0,0,"Number Of Processors = %2ld",DTM_NUM_PROCESSORS) ;

    // Log Command Line Arguments

    if(argc < 2)
        {
        bcdtmWrite_message(0,0,0,"Not Enough Arguements") ;
        bcdtmWrite_message(0,0,0,"C:>bcDrainage <BcDTM File>") ;
        goto errexit ;
        }
    else
        {
        bcdtmWrite_message(0,0,0,"BcDTM File = %ws",argv[1]) ;
        }  

    // Read DTM From File

    bcdtmWrite_message(0,0,0,"Reading File = %ws",argv[1]) ;
    if(bcdtmRead_fromFileDtmObject(&dtmP,argv[1])) goto errexit ; 
    if(dbg == 2) bcdtmObject_reportStatisticsDtmObject(dtmP) ;

    // Check Tin

    if(cdbg)
        { 
        bcdtmWrite_message(0,0,0,"Checking DTM Tin") ;
        if(bcdtmCheck_tinComponentDtmObject(dtmP)) 
            {
            bcdtmWrite_message(0,0,0,"Tin Invalid") ;
            goto errexit ;
            }
        else  bcdtmWrite_message(0,0,0,"Tin Valid") ; 
        } 

    // Create IBcDTM

    ibcDtm = BcDTM::CreateFromDtmHandle(dtmP);
    if(ibcDtm.IsNull())
        {
        bcdtmWrite_message(1,0,0,"Error Creating IbcDTM") ;
        goto errexit ;
        }

    // Call Drainage Tests

    if(bcdtmDrainage_testDrainageMethods(ibcDtm)) goto errexit ;

    // CleanUp
    
cleanup :
 
    // Return
    
    return(ret) ;
    
    // Error Exit
    
errexit :
    if(ret == DTM_SUCCESS) ret = DTM_ERROR ;
    goto cleanup ;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int  bcdtmDrainage_callBackFunction
    (
    DTMFeatureType   dtmFeatureType,
    DTMUserTag       userTag,
    DTMFeatureId     featureId,
    DPoint3d         *featurePtsP,
    size_t           numFeaturePts,
    void             *userP
   )
    /*
    ** Sample DTM Interrupt Load Function
    **
    ** This Function Receives The Load Features From The DTM
    ** As The DTM Reuses The Feature Points Memory Do Not Free It
    ** If You Require The Feature Points Then Make A Copy
    **
    */
    {
    int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    char  dtmFeatureTypeName[100] ;
    BC_DTM_OBJ *dtmP=NULL ;
    DPoint3d  *p3dP ;
    /*
    ** Check For DTMFeatureType::CheckStop
    */
    // if(dtmFeatureType == DTMFeatureType::CheckStop) bcdtmWrite_message(0,0,0,"DTMFeatureType::CheckStop") ;
    /*
    ** Initialise
    */
    // if(numDtmFeatures % 1000 == 0) bcdtmWrite_message(0,0,0,"numDtmFeatures = %8ld",numDtmFeatures) ;
    ++numDtmFeatures ;
    /*
    ** Write Record
    */
    if(dbg == 1) 
        {
        bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
        bcdtmWrite_message(0,0,0,"Feature[%8ld] ** %s userTag = %10I64d featureId = %10I64d featurePtsP = %p numFeaturePts = %6ld userP = %p",numDtmFeatures,dtmFeatureTypeName,userTag,featureId,featurePtsP,numFeaturePts,userP) ;
        } 
    /*
    ** Write Points
    */
    if(dbg == 1)
        {
        bcdtmWrite_message(0,0,0,"Number Of Feature Points = %6ld",numFeaturePts) ;
        for(p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP)
            {
            bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.4lf %12.4lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
            }
        } 
    /*
    ** Store DTM Features In DTM
    */
//    if( userP != NULL && ( dtmFeatureType == DTMFeatureType::LowPointPond || dtmFeatureType == DTMFeatureType::DescentTrace ))
    if( userP != NULL && dtmFeatureType != DTMFeatureType::CheckStop )
        {
        dtmP = (BC_DTM_OBJ *) userP ;
        if(bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;

        if( dtmFeatureType ==  DTMFeatureType::SumpLine || dtmFeatureType == DTMFeatureType::RidgeLine )
          {
           DPoint3d *lineP =  featurePtsP ;
           for( lineP = featurePtsP ; lineP < featurePtsP + numFeaturePts * 2 ; lineP = lineP + 2 )
              {
               if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,lineP,2)) goto errexit ;
              }
          }
        else
          {
           if(bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,featurePtsP,(long)numFeaturePts)) goto errexit ;
          }
/*
        if(userTag == 99)
            {
            if(bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::ContourLine,dtmP->nullUserTag,1,&dtmP->nullFeatureId,featurePtsP,(long)numFeaturePts)) goto errexit ;
            } 
        if(userTag == 2)
            {
            if(bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,featurePtsP,(long)numFeaturePts)) goto errexit ;
            } 
*/
        }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if(dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0,0,0,"Call Back Function Completed") ;
    if(dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0,0,0,"Call Back Function Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if(ret == DTM_SUCCESS) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDrainage_testDrainageMethods(BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr ibcDtm)
    {
    int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(1) ;
    long startTime=bcdtmClock() ;
    BC_DTM_OBJ *dtmP=NULL,*depressionDtmP=NULL ;
    BC_DTM_OBJ *traceDtmP=NULL ;
    void  *userP=NULL ;
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMFenceParams  fence ;
    DPoint3d  fencePts[10] ;
    DPoint3d  traceStartPoint,sumpPoint ;
    double    maxPondDepth ;
    BcDTMPtr refinedDtm=nullptr ;
    bool refineOption=false,catchmentDetermined=false ;
    bvector<DPoint3d> catchmentPoints ;


    // Variables To Test Maximum Descent And Ascent Tracing
 
    int pnt1,pnt2,pnt3,clPtr,voidTriangle,numTrianglesTraced=0 ;
    double x,y,falseLowDepth=0.0 ;
   
    //  Drainage Tests
    
    int drainageTest=14 ;
    DTMDrainageTables *drainageTablesP=NULL ;
    DTMFeatureCallback callBackFunction = (DTMFeatureCallback) bcdtmDrainage_callBackFunction ;

    switch (drainageTest)
        {

        //  Test1 - Create And Destroy Drainage Tables

        case 1 :
            BcDTMDrainage::CreateDrainageTables(ibcDtm.get(), drainageTablesP) ;
            break  ;

            //  Test 2 - Create And Check Drainage Tables

        case 2 :

            if(BcDTMDrainage::CreateAndCheckDrainageTables(ibcDtm.get()) == DTM_SUCCESS)
                {
                bcdtmWrite_message(0,0,0,"Drainage Tables Valid") ;
                }
            else
                {
                bcdtmWrite_message(0,0,0,"Drainage Tables Invalid") ;
                } 
            break ;     

            //  Test 3 - Create Ponds

        case 3 :

            if(bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
 //           BcDTMDrainage::CreateDrainageTables(ibcDtm.get(), drainageTablesP) ;
            bcdtmWrite_message(0,0,0,"Determing Ponds") ;
            if(BcDTMDrainage::DeterminePonds(ibcDtm.get(),drainageTablesP,(DTMFeatureCallback) bcdtmDrainage_callBackFunction,dtmP) == DTM_SUCCESS)
                {
                bcdtmWrite_message(0,0,0,"Determing Ponds Completed") ;
                bcdtmWrite_message(0,0,0,"Number Of Ponds = %8ld",dtmP->numFeatures) ;
                bcdtmWrite_message(0,0,0,"Time To Determine Ponds = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
                bcdtmWrite_geopakDatFileFromDtmObject(dtmP,L"ponds.dat") ;

//              Analyse Ponds - Have To Set It Up And Test For The Aborted Way It Is Done In CF

                bool analysePonds=true ;
                if( analysePonds )
                    {
                    long dtmFeature,numFeaturePts ;
                    double area ;
                    DTMDirection direction ;
                    DPoint3d *p3dP,*featurePtsP=NULL ;
                    BC_DTM_FEATURE *dtmFeatureP ;
                    BC_DTM_OBJ *tempDtmP=NULL ;
                    if( bcdtmObject_createDtmObject(&tempDtmP) != DTM_SUCCESS ) goto errexit ;
                    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
                        {
                        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
                        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == DTMFeatureType::Breakline )
                            {
                             if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts) != DTM_SUCCESS ) goto errexit ; 
                             bcdtmMath_getPolygonDirectionP3D(featurePtsP,numFeaturePts,&direction,&area) ;
                             bcdtmWrite_message(0,0,0,"Processing Pond Feature %8ld ** numPondPoints = %8ld ** Direction = %2ld Area = %15.8lf",dtmFeature,numFeaturePts,direction,area);
                             if( dtmFeature == -99 )
                                {
                                 for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
                                    {  
                                     bcdtmWrite_message(0,0,0,"FeaturePoint[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
                                    } 
                                }
                             if( numFeaturePts >= 4 )
                                 {
                                 tempDtmP->ppTol = tempDtmP->plTol = 0.0 ; 
                                 if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,featurePtsP,numFeaturePts) != DTM_SUCCESS ) goto errexit ;
                                 if( bcdtmObject_triangulateDtmObject(tempDtmP) != DTM_SUCCESS ) goto errexit ;
                                 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtmP) != DTM_SUCCESS ) goto errexit ;

                                 //  Get Point For Pond Determination

                                 int ap,np,sp = tempDtmP->hullPoint ;
                                 double x,y,z ;
                                 bool pointFound=false ; 
                                 do
                                     {
                                     np = nodeAddrP(tempDtmP,sp)->hPtr ;
                                     if( ( ap = bcdtmList_nextAntDtmObject(tempDtmP,sp,np)) < 0 ) goto errexit ;
                                     if( nodeAddrP(tempDtmP,ap)->hPtr != sp )
                                         {
                                          pointFound = true ;
                                          x = ( pointAddrP(tempDtmP,sp)->x + pointAddrP(tempDtmP,ap)->x ) / 2.0 ;
                                          y = ( pointAddrP(tempDtmP,sp)->y + pointAddrP(tempDtmP,ap)->y ) / 2.0 ;
                                          z = ( pointAddrP(tempDtmP,sp)->z + pointAddrP(tempDtmP,ap)->z ) / 2.0 ;
                                         } 
                                     sp = np ; 
                                     }  while ( pointFound == false && sp != tempDtmP->hullPoint ) ;

                                 if( ! pointFound && tempDtmP->numPoints == 3 )
                                     {
                                     pointFound = true ;
                                     x = ( pointAddrP(tempDtmP,0)->x + pointAddrP(tempDtmP,1)->x + pointAddrP(tempDtmP,2)->x ) / 3.0 ;
                                     y = ( pointAddrP(tempDtmP,0)->y + pointAddrP(tempDtmP,1)->y + pointAddrP(tempDtmP,2)->y ) / 3.0 ;
                                     z = ( pointAddrP(tempDtmP,0)->z + pointAddrP(tempDtmP,1)->z + pointAddrP(tempDtmP,2)->z ) / 3.0 ;
                                    } 

                                 // Determine Pond

                                 if( pointFound )
                                     {
                                      bool pondDetermined=false ; 
                                      double pondElevation,pondDepth,pondVolume,pondArea ;
                                      BENTLEY_NAMESPACE_NAME::TerrainModel::DTMDynamicFeatureArray pondFeatures ;
                                      if( BcDTMDrainage::CalculatePondForPoint(ibcDtm.get(),x,y,0.0,pondDetermined,pondElevation,pondDepth,pondArea,pondVolume,pondFeatures) == DTM_SUCCESS)
                                      if( dbg && pondDetermined )
                                         {
                                         bcdtmWrite_message(0,0,0,"Pond Determined ** elevation = %8.3lf depth = %8.3lf volume = %12.3lf area = %15.8lf",pondElevation,pondDepth,pondVolume,pondArea) ;  
                                         } 
                                     }
                                 }
                             
                             
                            } 
                         bcdtmObject_initialiseDtmObject(tempDtmP) ;
                        }
                    }   
                }
            else
                {
                bcdtmWrite_message(0,0,0,"Determing Ponds Error") ;
                }  
            break ;


            //  Test 4 - Create Depression DTM

        case 4 :

            bcdtmWrite_message(0,0,0,"Creating Depression DTM") ;
            if(BcDTMDrainage::CreateDepressionDtm(ibcDtm.get(), depressionDtmP, callBackFunction, userP) == DTM_SUCCESS)
                {
                bcdtmWrite_message(0,0,0,"Creating Depression DTM Completed") ;
                bcdtmWrite_message(0,0,0,"Time To Create Depression DTM = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
                //          bcdtmWrite_toFileDtmObject(depressionDtmP,L"depressionDTM.bcdtm") ;
                }
            else
                {
                bcdtmWrite_message(0,0,0,"Creating Depression DTM Error") ;
                }  

            break ;

            //  Test 5 - Test Maximum Descent Tracing From The Centroid Of Each None Void Triangle

        case 5 :

            bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Each Triangle Centroid") ;
            if( bcdtmObject_createDtmObject(&traceDtmP)) goto errexit ;
            numTrianglesTraced = 0 ; 
            dtmP=ibcDtm.get()->GetTinHandle() ; 
            for(  pnt1 = 0 ; pnt1 < dtmP->numPoints ; ++pnt1 )
                {
                if( ( clPtr = nodeAddrP(dtmP,pnt1)->cPtr ) != dtmP->nullPtr )
                    {
                     if( ( pnt2 = bcdtmList_nextAntDtmObject(dtmP,pnt1,clistAddrP(dtmP,clPtr)->pntNum )) < 0 ) goto errexit ;
                     while( clPtr != dtmP->nullPtr )
                         {
                          pnt3  = clistAddrP(dtmP,clPtr)->pntNum  ;
                          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
                          if( pnt2 > pnt1 && pnt3 > pnt1 )
                              { 
                              if( nodeAddrP(dtmP,pnt1)->hPtr != pnt2 )    // Pnt1 On Tin Hull
                                  {
                                  if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,(long *)&voidTriangle)) goto errexit ;
                                  if( ! voidTriangle )
                                      {
                                      if( numTrianglesTraced % 1000 == 0 ) bcdtmWrite_message(0,0,0,"Triangles Traced = %8ld of %8ld",numTrianglesTraced,dtmP->numTriangles) ;
                                      ++numTrianglesTraced ;
                                      x = ( pointAddrP(dtmP,pnt1)->x + pointAddrP(dtmP,pnt2)->x + pointAddrP(dtmP,pnt3)->x ) / 3.0 ;
                                      y = ( pointAddrP(dtmP,pnt1)->y + pointAddrP(dtmP,pnt2)->y + pointAddrP(dtmP,pnt3)->y ) / 3.0 ;
                                      if( BcDTMDrainage::TraceMaximumDescent(ibcDtm.get(),nullptr,falseLowDepth,x,y,callBackFunction,(void *) traceDtmP )!= DTM_SUCCESS )
                                          {
                                          bcdtmWrite_message(0,0,0,"Error Tracing Maximum Descent From ** X = %12.5lf Y = %12.5lf",x,y) ;
                                          }
                                      } 
                                  }
                              }
                          pnt2  = pnt3 ; 
                         }    
                    }
                }
            if( traceDtmP != NULL )
                {
                 bcdtmWrite_geopakDatFileFromDtmObject(traceDtmP,L"descentTrace.dat") ;
                 bcdtmObject_destroyDtmObject(&traceDtmP) ; 
                }    
            bcdtmWrite_message(0,0,0,"Maximum Descent Traced For %8ld Triangles",numTrianglesTraced) ;
            break ;

            //  Test 6 - Test Maximum Ascent Tracing From The Centroid Of Each None Void Triangle

       case 6 :

            bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent From Each Triangle Centroid") ;
            if( bcdtmObject_createDtmObject(&traceDtmP)) goto errexit ;
            numTrianglesTraced = 0 ; 
            dtmP=ibcDtm.get()->GetTinHandle() ; 
            for(  pnt1 = 0 ; pnt1 < dtmP->numPoints ; ++pnt1 )
                {
                if( ( clPtr = nodeAddrP(dtmP,pnt1)->cPtr ) != dtmP->nullPtr )
                    {
                     if( ( pnt2 = bcdtmList_nextAntDtmObject(dtmP,pnt1,clistAddrP(dtmP,clPtr)->pntNum )) < 0 ) goto errexit ;
                     while( clPtr != dtmP->nullPtr )
                         {
                          pnt3  = clistAddrP(dtmP,clPtr)->pntNum  ;
                          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
                          if( pnt2 > pnt1 && pnt3 > pnt1 )
                              { 
                              if( nodeAddrP(dtmP,pnt1)->hPtr != pnt2 )    // Pnt1 On Tin Hull
                                  {
                                  if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,(long *)&voidTriangle)) goto errexit ;
                                  if( ! voidTriangle )
                                      {
                                      if( numTrianglesTraced % 1000 == 0 ) bcdtmWrite_message(0,0,0,"Triangles Traced = %8ld of %8ld",numTrianglesTraced,dtmP->numTriangles) ;
                                      ++numTrianglesTraced ;
                                      x = ( pointAddrP(dtmP,pnt1)->x + pointAddrP(dtmP,pnt2)->x + pointAddrP(dtmP,pnt3)->x ) / 3.0 ;
                                      y = ( pointAddrP(dtmP,pnt1)->y + pointAddrP(dtmP,pnt2)->y + pointAddrP(dtmP,pnt3)->y ) / 3.0 ;
                                      if( BcDTMDrainage::TraceMaximumAscent(ibcDtm.get(),nullptr,falseLowDepth,x,y,callBackFunction,(void *) traceDtmP )!= DTM_SUCCESS )
                                          {
                                          bcdtmWrite_message(0,0,0,"Error Tracing Maximum Ascent From ** X = %12.5lf Y = %12.5lf",x,y) ;
                                          }
                                      } 
                                  }
                              }
                          pnt2  = pnt3 ; 
                         }    
                    }
                }
            if( traceDtmP != NULL )
                {
                 bcdtmWrite_geopakDatFileFromDtmObject(traceDtmP,L"ascentTrace.dat") ;
                 bcdtmObject_destroyDtmObject(&traceDtmP) ; 
                }    
            bcdtmWrite_message(0,0,0,"Maximum Ascent Traced For %8ld Triangles",numTrianglesTraced) ;
            break ;

            //  Test 7 - Test Maximum Descent Tracing From Specific Point

        case 7 :

            x = 2501364.94744 ;
            y = 6012558.00075 ; 
            x = 2137516.10000 ;
            y = 221786.94667 ;
            x = 2138514.3763 ;
            y = 222225.8657 ;
            x = 2138514.0613 ;
            y = 222225.9627 ;

            x = 2138543.1043 ;
            y = 222225.4025  ;

            x = 2138544.6226 ;
            y = 222223.6155 ;
      
            x = 2138541.1709 ;
            y = 222222.3777 ;

            // Create Dtm To Store Boundaries

            if(bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
            bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Point ** X = %12.5lf Y = %12.5lf",x,y) ;
  
          if( BcDTMDrainage::TraceMaximumDescent(ibcDtm.get(),nullptr,falseLowDepth,x,y,callBackFunction,dtmP) != DTM_SUCCESS )
                {
                bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Point ** X = %12.5lf Y = %12.5lf Error",x,y) ;
                }
            else
                {
                bcdtmWrite_message(0,0,0,"Tracing Maximum Descent From Point ** X = %12.5lf Y = %12.5lf Completed",x,y) ;
                if( dtmP != nullptr ) bcdtmWrite_geopakDatFileFromDtmObject(dtmP,L"downStreamTrace.dat") ;
               } 
            break ;


            //  Test 8 - Test Maximum Ascent Tracing From Specific Point

        case 8 :

            x = 397744.32024 ;
            y = 161667.07381 ; 
            x = 2137516.10000 ;
            y = 221786.94667 ;
            bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent From Point ** X = %12.5lf Y = %12.5lf",x,y) ;
            if( BcDTMDrainage::TraceMaximumAscent(ibcDtm.get(),nullptr,falseLowDepth,x,y,callBackFunction,nullptr) != DTM_SUCCESS )
                {
                bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent From Point ** X = %12.5lf Y = %12.5lf Error",x,y) ;
                }
            else
                {
                bcdtmWrite_message(0,0,0,"Tracing Maximum Ascent From Point ** X = %12.5lf Y = %12.5lf Completed",x,y) ;
                } 
            break ;

            //  Test 9 - Return Low Points

        case 9 :

            bcdtmWrite_message(0,0,0,"**** Returning Low Points") ;
            fencePts[0].x = 2604554.0 ; fencePts[0].y = 6160504.0 ; fencePts[0].z = 0.0 ;
            fencePts[1].x = 2610118.0 ; fencePts[1].y = 6149810.0 ; fencePts[1].z = 0.0 ;
            fencePts[2].x = 2604554.0 ; fencePts[2].y = 6137385.0 ; fencePts[2].z = 0.0 ;
            fencePts[3].x = 2639418.0 ; fencePts[3].y = 6137385.0 ; fencePts[3].z = 0.0 ;
            fencePts[4].x = 2632618.0 ; fencePts[4].y = 6149810.0 ; fencePts[4].z = 0.0 ;
            fencePts[5].x = 2639418.0 ; fencePts[5].y = 6160504.0 ; fencePts[5].z = 0.0 ;
            fencePts[6].x = 2604554.0 ; fencePts[6].y = 6160504.0 ; fencePts[6].z = 0.0 ;
            fence.fenceType   = DTMFenceType::Shape ;
            fence.fenceOption = DTMFenceOption::Inside ;  
            fence.numPoints   = 7 ;
            fence.points      = fencePts ; 
             
            if( BcDTMDrainage::ReturnLowPoints(ibcDtm.get(),callBackFunction,fence,nullptr) != DTM_SUCCESS )
                {
                bcdtmWrite_message(0,0,0,"**** Returning Low Points Error") ;
                }
            else
                {
                bcdtmWrite_message(0,0,0,"**** Returning Low Points Completed") ;
                } 

            bcdtmWrite_message(0,0,0,"**** Returning Low Points") ;
            fencePts[0].x = 2604554.0 ; fencePts[0].y = 6160504.0 ; fencePts[0].z = 0.0 ;
            fencePts[1].x = 2610118.0 ; fencePts[1].y = 6149810.0 ; fencePts[1].z = 0.0 ;
            fencePts[2].x = 2604554.0 ; fencePts[2].y = 6137385.0 ; fencePts[2].z = 0.0 ;
            fencePts[3].x = 2639418.0 ; fencePts[3].y = 6137385.0 ; fencePts[3].z = 0.0 ;
            fencePts[4].x = 2632618.0 ; fencePts[4].y = 6149810.0 ; fencePts[4].z = 0.0 ;
            fencePts[5].x = 2639418.0 ; fencePts[5].y = 6160504.0 ; fencePts[5].z = 0.0 ;
            fencePts[6].x = 2604554.0 ; fencePts[6].y = 6160504.0 ; fencePts[6].z = 0.0 ;
            fence.fenceType   = DTMFenceType::Shape ;
            fence.fenceOption = DTMFenceOption::Outside ;  
            fence.numPoints   = 7 ;
            fence.points      = fencePts ; 
             
            if( BcDTMDrainage::ReturnLowPoints(ibcDtm.get(),callBackFunction,fence,nullptr) != DTM_SUCCESS )
                {
                bcdtmWrite_message(0,0,0,"**** Returning Low Points Error") ;
                }
            else
                {
                bcdtmWrite_message(0,0,0,"**** Returning Low Points Completed") ;
                } 
            break ;

            //  Test 10 - Return High Points

        case 10 :

            bcdtmWrite_message(0,0,0,"**** Returning High Points") ;
            fencePts[0].x = 2604554.0 ; fencePts[0].y = 6160504.0 ; fencePts[0].z = 0.0 ;
            fencePts[1].x = 2610118.0 ; fencePts[1].y = 6149810.0 ; fencePts[1].z = 0.0 ;
            fencePts[2].x = 2604554.0 ; fencePts[2].y = 6137385.0 ; fencePts[2].z = 0.0 ;
            fencePts[3].x = 2639418.0 ; fencePts[3].y = 6137385.0 ; fencePts[3].z = 0.0 ;
            fencePts[4].x = 2632618.0 ; fencePts[4].y = 6149810.0 ; fencePts[4].z = 0.0 ;
            fencePts[5].x = 2639418.0 ; fencePts[5].y = 6160504.0 ; fencePts[5].z = 0.0 ;
            fencePts[6].x = 2604554.0 ; fencePts[6].y = 6160504.0 ; fencePts[6].z = 0.0 ;
            fence.fenceType   = DTMFenceType::Shape ;
            fence.fenceOption = DTMFenceOption::Inside ;  
            fence.numPoints   = 7 ;
            fence.points      = fencePts ; 
             
            if( BcDTMDrainage::ReturnHighPoints(ibcDtm.get(),callBackFunction,fence,nullptr) != DTM_SUCCESS )
                {
                bcdtmWrite_message(0,0,0,"**** Returning High Points Error") ;
                }
            else
                {
                bcdtmWrite_message(0,0,0,"**** Returning High Points Completed") ;
                } 

            bcdtmWrite_message(0,0,0,"**** Returning High Points") ;
            fencePts[0].x = 2604554.0 ; fencePts[0].y = 6160504.0 ; fencePts[0].z = 0.0 ;
            fencePts[1].x = 2610118.0 ; fencePts[1].y = 6149810.0 ; fencePts[1].z = 0.0 ;
            fencePts[2].x = 2604554.0 ; fencePts[2].y = 6137385.0 ; fencePts[2].z = 0.0 ;
            fencePts[3].x = 2639418.0 ; fencePts[3].y = 6137385.0 ; fencePts[3].z = 0.0 ;
            fencePts[4].x = 2632618.0 ; fencePts[4].y = 6149810.0 ; fencePts[4].z = 0.0 ;
            fencePts[5].x = 2639418.0 ; fencePts[5].y = 6160504.0 ; fencePts[5].z = 0.0 ;
            fencePts[6].x = 2604554.0 ; fencePts[6].y = 6160504.0 ; fencePts[6].z = 0.0 ;
            fence.fenceType   = DTMFenceType::Shape ;
            fence.fenceOption = DTMFenceOption::Outside ;  
            fence.numPoints   = 7 ;
            fence.points      = fencePts ; 
             
            if( BcDTMDrainage::ReturnHighPoints(ibcDtm.get(),callBackFunction,fence,nullptr) != DTM_SUCCESS )
                {
                bcdtmWrite_message(0,0,0,"**** Returning High Points Error") ;
                }
            else
                {
                bcdtmWrite_message(0,0,0,"**** Returning High Points Completed") ;
                } 
            break ;

            //  Test 11 - Return Sump Lines

        case 11 :

            if(bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
            bcdtmWrite_message(0,0,0,"**** Returning Sump Lines") ;
            fencePts[0].x = 2604554.0 ; fencePts[0].y = 6160504.0 ; fencePts[0].z = 0.0 ;
            fencePts[1].x = 2610118.0 ; fencePts[1].y = 6149810.0 ; fencePts[1].z = 0.0 ;
            fencePts[2].x = 2604554.0 ; fencePts[2].y = 6137385.0 ; fencePts[2].z = 0.0 ;
            fencePts[3].x = 2639418.0 ; fencePts[3].y = 6137385.0 ; fencePts[3].z = 0.0 ;
            fencePts[4].x = 2632618.0 ; fencePts[4].y = 6149810.0 ; fencePts[4].z = 0.0 ;
            fencePts[5].x = 2639418.0 ; fencePts[5].y = 6160504.0 ; fencePts[5].z = 0.0 ;
            fencePts[6].x = 2604554.0 ; fencePts[6].y = 6160504.0 ; fencePts[6].z = 0.0 ;
            fence.fenceType   = DTMFenceType::None ;
            fence.fenceOption = DTMFenceOption::Inside ;  
            fence.numPoints   = 7 ;
            fence.points      = fencePts ; 
             
//            if( BcDTMDrainage::ReturnSumpLines(ibcDtm.get(),callBackFunction,fence,dtmP) != DTM_SUCCESS )
//            if( BcDTMDrainage::ReturnZeroSlopeSumpLines(ibcDtm.get(),callBackFunction,fence,dtmP) != DTM_SUCCESS )
//            if( BcDTMDrainage::ReturnRidgeLines(ibcDtm.get(),callBackFunction,fence,dtmP) != DTM_SUCCESS )
            if( BcDTMDrainage::ReturnZeroSlopePolygons(ibcDtm.get(),callBackFunction,fence,dtmP) != DTM_SUCCESS )
                {
                bcdtmWrite_message(0,0,0,"**** Returning Sump Line Errors") ;
                }
            else
                {
                bcdtmWrite_message(0,0,0,"**** Returning Sump Lines Completed") ;
                if( dtmP != NULL )
                    {
                         if( dtmP->numFeatures > 0 )
                         { 
                         bcdtmWrite_message(0,0,0,"**** Writing %8ld Sump Lines",dtmP->numFeatures) ;
                         bcdtmWrite_geopakDatFileFromDtmObject(dtmP,L"sumpLines.dat") ;
                         bcdtmWrite_message(0,0,0,"**** Writing Sump Lines Completed") ;
                         }
                    }
                } 
            break ;

            //  Test 12 - Create Refined Drainage Dtm

        case 12 :

            bcdtmWrite_message(0,0,0,"Creating Refined Drainage DTM") ;



            // Fence Points For "depressionProblem00.bcdtm"

            fencePts[0].x = 2604554.0 ; fencePts[0].y = 6160504.0 ; fencePts[0].z = 0.0 ;
            fencePts[1].x = 2610118.0 ; fencePts[1].y = 6149810.0 ; fencePts[1].z = 0.0 ;
            fencePts[2].x = 2604554.0 ; fencePts[2].y = 6137385.0 ; fencePts[2].z = 0.0 ;
            fencePts[3].x = 2639418.0 ; fencePts[3].y = 6137385.0 ; fencePts[3].z = 0.0 ;
            fencePts[4].x = 2632618.0 ; fencePts[4].y = 6149810.0 ; fencePts[4].z = 0.0 ;
            fencePts[5].x = 2639418.0 ; fencePts[5].y = 6160504.0 ; fencePts[5].z = 0.0 ;
            fencePts[6].x = 2604554.0 ; fencePts[6].y = 6160504.0 ; fencePts[6].z = 0.0 ;
            fence.fenceType   = DTMFenceType::Shape ;
            fence.fenceOption = DTMFenceOption::Inside ;  
            fence.numPoints   = 7 ;
            fence.points      = fencePts ; 
            bcdtmWrite_message(0,0,0,"Number Of Points = %8ld",ibcDtm.get()->GetTinHandle()->numPoints) ;
            if( BcDTMDrainage::CreateRefinedDrainageDtm(ibcDtm.get(),fence,&refinedDtm) != DTM_SUCCESS )
                {
                bcdtmWrite_message(0,0,0,"Creating Refined Drainage Dtm Error") ;
                }
            else
                {
                BC_DTM_OBJ *coreDtmP=refinedDtm.get()->GetTinHandle() ;
                bcdtmWrite_message(0,0,0,"Creating Refined Drainage Completed") ;
                bcdtmWrite_message(0,0,0,"Number Of Points = %8ld",coreDtmP->numPoints) ;
                bcdtmWrite_toFileDtmObject(coreDtmP,L"coreDtm.tin") ;
                } 
            break ;


        case 13 :

            bcdtmWrite_message(0,0,0,"Returning DTM Catchments") ;

            // Create Drainage Tables
 
            bcdtmWrite_message(0,0,0,"**** Creating Drainage Tables") ;
            startTime = bcdtmClock() ;
            BcDTMDrainage::CreateDrainageTables(ibcDtm.get(), drainageTablesP) ;
            bcdtmWrite_message(0,0,0,"**** Time To Create Draiange Tables = %8.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;

            // Create Dtm To Store Boundaries

            if(bcdtmObject_createDtmObject(&dtmP)) goto errexit ;

            // Fence Points For "depressionProblem00.bcdtm"
            
            fencePts[0].x = 2604554.0 ; fencePts[0].y = 6160504.0 ; fencePts[0].z = 0.0 ;
            fencePts[1].x = 2610118.0 ; fencePts[1].y = 6149810.0 ; fencePts[1].z = 0.0 ;
            fencePts[2].x = 2604554.0 ; fencePts[2].y = 6137385.0 ; fencePts[2].z = 0.0 ;
            fencePts[3].x = 2639418.0 ; fencePts[3].y = 6137385.0 ; fencePts[3].z = 0.0 ;
            fencePts[4].x = 2632618.0 ; fencePts[4].y = 6149810.0 ; fencePts[4].z = 0.0 ;
            fencePts[5].x = 2639418.0 ; fencePts[5].y = 6160504.0 ; fencePts[5].z = 0.0 ;
            fencePts[6].x = 2604554.0 ; fencePts[6].y = 6160504.0 ; fencePts[6].z = 0.0 ;
            fence.fenceType   = DTMFenceType::None ;
            fence.fenceOption = DTMFenceOption::Inside ;  
            fence.numPoints   = 7 ;
            fence.points      = fencePts ; 
            bcdtmWrite_message(0,0,0,"Number Of Points = %8ld",ibcDtm.get()->GetTinHandle()->numPoints) ;
            startTime = bcdtmClock() ;
            falseLowDepth = 0.0 ;
            refineOption = true ;
            if( BcDTMDrainage::ReturnCatchments(ibcDtm.get(),drainageTablesP,falseLowDepth,refineOption,callBackFunction,fence,dtmP) != DTM_SUCCESS )
               {
                bcdtmWrite_message(0,0,0,"Returning DTM Catchments Error") ;
                }
            else
                {
                bcdtmWrite_message(0,0,0,"Returning DTM Catchments Completed") ;
                bcdtmWrite_message(0,0,0,"**** Time To Determine Catchments = %8.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
                if( dtmP != nullptr && dtmP->numFeatures > 0 )
                    {
                     bcdtmWrite_message(0,0,0,"Writing DTM Catchments To Geopak Dat File") ;
                     if( refineOption ) 
                         bcdtmWrite_geopakDatFileFromDtmObject(dtmP,L"refinedCatchments.dat") ;
                     else
                         bcdtmWrite_geopakDatFileFromDtmObject(dtmP,L"unrefinedCatchments.dat") ;                        
                    }
                   
                } 
            break ;

        case 14 :

            bcdtmWrite_message(0,0,0,"Returning Catchment For A Point") ;

            traceStartPoint.x = 2138763.0 ;
            traceStartPoint.y = 222182.0 ;
            traceStartPoint.z = 755.580 ;
            maxPondDepth = 0.0 ;
            if( BcDTMDrainage::TraceCatchmentForPoint(ibcDtm.get(),traceStartPoint,maxPondDepth,catchmentDetermined,sumpPoint,catchmentPoints)) 
                {
                bcdtmWrite_message(0,0,0,"Trace Catchment For Point Error") ;
                }
 
            break ;

        default :
            goto errexit ;
            break ; 
        } ;
    
    // Clean Up
    
cleanup :

    // Return

    if(dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0,0,0,"Drainage Test Completed") ;
    if(dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0,0,0,"Drainage Test Error") ;
    return(ret) ;

    // Error Exit

errexit :
    bcdtmWrite_message(0,0,0,"Error Exiting") ;
    if(ret == DTM_SUCCESS) ret = DTM_ERROR ;
    goto cleanup ;
    }
