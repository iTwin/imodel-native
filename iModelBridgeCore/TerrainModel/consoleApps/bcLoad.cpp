/*--------------------------------------------------------------------------------------+
|
|     $Source: consoleApps/bcLoad.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <tchar.h>
#include "bcDTMBaseDef.h"
#include "dtmevars.h"

//XM-- static DTM_DAT_OBJ *glbDataP=NULL ;
static long  numDtmFeatures=0 ;
static long  numTrianglesLoaded=0 ;
int  bcdtmLoad_dllLoadFunction(DTMFeatureType dtmFeatureType,DTMUserTag userTag, DTMFeatureId dtmFeatureId,DPoint3d *featurePtsP,size_t numFeaturePts,void *userP) ;
int  bcdtmBrowse_triangleMesh (DTMFeatureType dtmFeatureType, int numTriangles, int numMeshPts, DPoint3d *meshPtsP, int numMeshFaces, long *meshFacesP, void *userP);
int  bcdtmBrowse_triangleShadeMesh(DTMFeatureType dtmFeatureType,int numTriangles,int numMeshPts,DPoint3d *meshPtsP,DPoint3d *meshVectorsP,int numMeshFaces, long *meshFacesP,void *userP) ;

int wmain(int argc, wchar_t *argv[])
{
 int      dbg=DTM_TRACE_VALUE(1),cdbg=DTM_CHECK_VALUE(1) ;
 long    n,startTime,maxTriangles=0,vectorOption=1,maxSpots=0 ;  
 DTMFeatureType dtmFeatureType;
 long useFence, numBlockPts, numShapePts;
 DTMFenceType fenceType;
 DTMFenceOption fenceOption;
 double  zAxisFactor=1.0 ;
 DPoint3d     *blockPtsP=NULL,*shapePtsP=NULL ;
 BC_DTM_OBJ *dtmP=NULL ;
 void    *userP=NULL ;
/*
** Initialise DTM
*/
 bcdtmInitialise() ;
 if( fpLOG != NULL ) { fclose(fpLOG) ; fpLOG = NULL ; }
 bcdtmInitialise_openLogFile(L"bcLoad.log") ;
/*
** Initialise Variables
*/
 bcdtmWrite_message(0,0,0,"Bentley Civil Partitioned DTM Console Load") ;
 bcdtmWrite_message(0,0,0,"==========================================") ;
 bcdtmWrite_message(0,0,0,"Start") ;
/*
** Get Arguements From Command Prompt
*/
 if( argc < 3 )
   {
     bcdtmWrite_message(0,0,0,"Not Enough Arguements") ;
     bcdtmWrite_message(0,0,0,"C:>bcLoad <Bentley Civil Dtm File> <Dtm Load Feature>") ;
    goto errexit ;
   }
 else
   {
     bcdtmWrite_message(0,0,0,"Dtm File         = %ws",argv[1]) ;
     bcdtmWrite_message(0,0,0,"Dtm Load Feature = %ws",argv[2]) ;
/*
**  Validate Arguements
*/
//    if(  bcdtmData_getDtmFeatureTypeFromDtmFeatureTypeName(argv[2],&dtmFeatureType) )
//      {
//       bcdtmWrite_message(0,0,0,"Unknown DTM Feature Type") ;
//       goto errexit ;
//      }  
   }

//  Read Dtm File

 startTime = bcdtmClock() ;
 bcdtmWrite_message(0,0,0,"Reading Dtm File %ws",argv[1]) ;
 if( bcdtmRead_fromFileDtmObject(&dtmP,argv[1]) ) goto errexit ; 
 bcdtmWrite_message(0,0,0,"Time To Read File %ws = %8.3lf Seconds",argv[1],bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 

// Log DTM  Stats

 if( dbg == 2 ) bcdtmObject_reportStatisticsDtmObject(dtmP) ;

// Check Tin

 if( cdbg )
   { 
    startTime = bcdtmClock() ;
    bcdtmWrite_message(0,0,0,"Validating Tin") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) 
      {
       bcdtmWrite_message(0,0,0,"Tin Invalid") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Tin Valid") ;
    bcdtmWrite_message(0,0,0,"Time To Check Tin = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
   }

 goto browse ;
 goto browseSpots ;
 
/*
** Set Fence Parameters
*/
 useFence    = TRUE ;
// useFence    = FALSE ;
 fenceType   = DTMFenceType::Block ;
// fenceType   = DTMFenceType::Shape ;
// fenceOption = DTMFenceOption::Inside ;
 fenceOption = DTMFenceOption::Overlap ;

 numBlockPts  = 5 ;
 blockPtsP = ( DPoint3d * ) malloc( numBlockPts * sizeof(DPoint3d)) ;

/*
** Process Fences
*/
 for( n = 0 ; n < 4 ; ++n )
   {

    switch ( n )
      { 
       case 0 :
         useFence    = FALSE ;
       break ;
    
       case 1 :
         useFence    = TRUE ; 
         (blockPtsP+0)->x = 2104460.0 ; (blockPtsP+0)->y = 174289.0 ; (blockPtsP+0)->z = 0.0 ;
         (blockPtsP+1)->x = 2116081.0 ; (blockPtsP+1)->y = 174289.0 ; (blockPtsP+1)->z = 0.0 ;
         (blockPtsP+2)->x = 2116081.0 ; (blockPtsP+2)->y = 191180.0 ; (blockPtsP+2)->z = 0.0 ;
         (blockPtsP+3)->x = 2104460.0 ; (blockPtsP+3)->y = 191180.0 ; (blockPtsP+3)->z = 0.0 ;
         (blockPtsP+4)->x = 2104460.0 ; (blockPtsP+4)->y = 174289.0 ; (blockPtsP+4)->z = 0.0 ;
       break ;

       case 2 :
         useFence    = TRUE ; 
         (blockPtsP+0)->x = 2131107.0 ; (blockPtsP+0)->y = 189622.0 ; (blockPtsP+0)->z = 0.0 ;
         (blockPtsP+1)->x = 2140460.0 ; (blockPtsP+1)->y = 189622.0 ; (blockPtsP+1)->z = 0.0 ;
         (blockPtsP+2)->x = 2140460.0 ; (blockPtsP+2)->y = 201265.0 ; (blockPtsP+2)->z = 0.0 ;
         (blockPtsP+3)->x = 2131107.0 ; (blockPtsP+3)->y = 201265.0 ; (blockPtsP+3)->z = 0.0 ;
         (blockPtsP+4)->x = 2131107.0 ; (blockPtsP+4)->y = 189622.0 ; (blockPtsP+4)->z = 0.0 ;
       break ;

       case 3 :
         useFence    = TRUE ; 
         (blockPtsP+0)->x = 2133805.0 ; (blockPtsP+0)->y = 193573.0 ; (blockPtsP+0)->z = 0.0 ;
         (blockPtsP+1)->x = 2137474.0 ; (blockPtsP+1)->y = 193573.0 ; (blockPtsP+1)->z = 0.0 ;
         (blockPtsP+2)->x = 2137474.0 ; (blockPtsP+2)->y = 197125.0 ; (blockPtsP+2)->z = 0.0 ;
         (blockPtsP+3)->x = 2133805.0 ; (blockPtsP+3)->y = 197125.0 ; (blockPtsP+3)->z = 0.0 ;
         (blockPtsP+4)->x = 2133805.0 ; (blockPtsP+4)->y = 193573.0 ; (blockPtsP+4)->z = 0.0 ;
       break ;
      } ; 
/*
 numShapePts = 5  ;
 shapePtsP = ( DPoint3d * ) malloc( numShapePts * sizeof(DPoint3d)) ;
 (shapePtsP+0)->x = 2104460.0 ; (shapePtsP+0)->y = 174289.0 ; (shapePtsP+0)->z = 0.0 ;
 (shapePtsP+1)->x = 2116081.0 ; (shapePtsP+1)->y = 174289.0 ; (shapePtsP+1)->z = 0.0 ;
 (shapePtsP+2)->x = 2116081.0 ; (shapePtsP+2)->y = 191180.0 ; (shapePtsP+2)->z = 0.0 ;
 (shapePtsP+3)->x = 2104460.0 ; (shapePtsP+3)->y = 191180.0 ; (shapePtsP+3)->z = 0.0 ;
 (shapePtsP+4)->x = 2104460.0 ; (shapePtsP+4)->y = 174289.0 ; (shapePtsP+4)->z = 0.0 ;
*/
/*
** Create Data Object
*/
    //if( bcdtmObject_createDataObject(&glbDataP)) goto errexit ;
/*
** Load Dtm Feature
*/
    numDtmFeatures = 0 ;
    startTime =  bcdtmClock() ;
    if( fenceType == DTMFenceType::Block ) { if( bcdtmInterruptLoad_trianglesFromDtmObject(dtmP,bcdtmLoad_dllLoadFunction,useFence,fenceType,fenceOption,blockPtsP,numBlockPts,userP)) goto errexit ; }
    if( fenceType == DTMFenceType::Shape ) { if( bcdtmInterruptLoad_trianglesFromDtmObject(dtmP,bcdtmLoad_dllLoadFunction,useFence,fenceType,fenceOption,shapePtsP,numShapePts,userP)) goto errexit ; }
    bcdtmWrite_message(0,0,0,"**** Time To Interrupt Load Dtm Features = %8.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
    bcdtmWrite_message(0,0,0,"Number Of Dtm Features Loaded = %8ld",numDtmFeatures) ;
//XM--     if( glbDataP != NULL ) bcdtmWrite_dataFileFromDataObject(glbDataP,L"loadTest.dat") ;
   }
/*
** Browse Triangle Mesh
*/
 browse :
 maxTriangles = 1000000 ;
 useFence    = FALSE ;
 numShapePts = 0 ;
 fenceType   = DTMFenceType::Block ;
 fenceOption = DTMFenceOption::Overlap ;
 numTrianglesLoaded = 0 ; 
 startTime = bcdtmClock() ;
 if( bcdtmInterruptLoad_triangleMeshFromDtmObject(dtmP,maxTriangles,bcdtmBrowse_triangleMesh,useFence,fenceType,fenceOption,shapePtsP,numShapePts,userP)) goto errexit ; 
 bcdtmWrite_message(0,0,0,"**** Time To Interrupt Load Triangle Mesh With %8ld triangles of %8ld = %8.3lf seconds",numTrianglesLoaded,dtmP->numTriangles,bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
/*
** Browse Triangle Shade Mesh
*/
 vectorOption = 2 ;
 zAxisFactor  = 5.0 ;
 numTrianglesLoaded = 0 ; 

// maxTriangles = 100000 ;

 startTime = bcdtmClock() ;
 if( bcdtmInterruptLoad_triangleShadeMeshFromDtmObject(dtmP,maxTriangles,vectorOption,zAxisFactor,bcdtmBrowse_triangleShadeMesh,useFence,fenceType,fenceOption,shapePtsP,numShapePts,userP)) goto errexit ; 
 bcdtmWrite_message(0,0,0,"**** Time To Interrupt Load Triangle Shade Mesh With %8ld triangles of %8ld = %8.3lf seconds",numTrianglesLoaded,dtmP->numTriangles,bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;

 startTime = bcdtmClock() ;
 BC_DTM_OBJ *regionMeshP=NULL ;
 if( bcdtmObject_createDtmObject(&regionMeshP)) goto errexit ;
 
 if( bcdtmLoad_triangleShadeMeshForRegionDtmObject(dtmP,maxTriangles,vectorOption,zAxisFactor,1,1,0,bcdtmBrowse_triangleShadeMesh,(void *)regionMeshP)) goto errexit ;
 bcdtmWrite_message(0,0,0,"**** Time To Interrupt Load Triangle Shade Mesh For Region With %8ld triangles of %8ld = %8.3lf seconds",numTrianglesLoaded,dtmP->numTriangles,bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
 if( regionMeshP->numFeatures > 0 ) 
    {
     bcdtmObject_triangulateDtmObject(regionMeshP) ;
     bcdtmList_removeNoneFeatureHullLinesDtmObject(regionMeshP) ; 
     bcdtmWrite_toFileDtmObject(regionMeshP,L"regiomMesh.tin") ;
    }

// goto cleanup ;
/*
** Browse Spots
*/
 browseSpots :
/*
** Set Fence Parameters
*/
// useFence    = TRUE ;
 useFence    = FALSE ;
 fenceType   = DTMFenceType::Block ;
// fenceType   = DTMFenceType::Shape ;
// fenceOption = DTMFenceOption::Inside ;
 fenceOption = DTMFenceOption::Overlap ;
 numBlockPts = 0 ;
 maxSpots    = 50000 ;
// dtmFeatureType = DTMFeatureType::TinPoint ;
// if( bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject(dtmP,dtmFeatureType,maxSpots,bcdtmLoad_dllLoadFunction,useFence,fenceType,fenceOption,blockPtsP,numBlockPts,userP)) goto errexit ;
// dtmFeatureType = DTMFeatureType::RandomSpots ;
// if( bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject(dtmP,dtmFeatureType,maxSpots,bcdtmLoad_dllLoadFunction,useFence,fenceType,fenceOption,blockPtsP,numBlockPts,userP)) goto errexit ;
 dtmFeatureType = DTMFeatureType::GroupSpots ;
 if( bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject(dtmP,dtmFeatureType,maxSpots,bcdtmLoad_dllLoadFunction,useFence,fenceType,fenceOption,blockPtsP,numBlockPts,userP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 bcdtmWrite_message(0,0,0,"Cleaning Up") ;
 bcdtmObject_destroyAllDtmObjects() ;
//XM--  if( glbDataP != NULL ) bcdtmObject_deleteDataObject(&glbDataP) ;
/* 
** Job Completed
*/
 bcdtmWrite_message(0,0,0,"End") ;
 bcdtmInitialise_closeLogFile() ;
 return(0) ;
/*
** Error Exit
*/
 errexit :
 bcdtmWrite_message(0,0,0,"Error Exiting") ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int  bcdtmLoad_dllLoadFunction(DTMFeatureType dtmFeatureType,DTMUserTag userTag, DTMFeatureId dtmFeatureId,DPoint3d *featurePtsP,size_t numFeaturePts,void *userP)
/*
** Sample DTM Interrupt Load Function
**
** This Function Receives The Load Features From The DTM
** As The DTM Reuses The Feature Points Memory Do Not Free It
** If You Require The Feature Points Then Make A Copy
** If You wish To Free The Feature Points Memory. Call  bcdtmLoadNgp_freeMemory() ;
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 char  dtmFeatureTypeName[100] ;
 DPoint3d  *p3dP ;
/*
** Check For DTMFeatureType::CheckStop
*/
// if( dtmFeatureType == DTMFeatureType::CheckStop ) bcdtmWrite_message(0,0,0,"DTMFeatureType::CheckStop") ;
/*
** Write Record
*/
 if( dbg == 1 ) 
   {
    bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
    bcdtmWrite_message(0,0,0,"Feature[%8ld] ** %s userTag = %10I64d dtmFeatureId = %8I64d featurePtsP = %p numFeaturePts = %6ld userP = %p",numDtmFeatures,dtmFeatureTypeName,userTag,dtmFeatureId,featurePtsP,numFeaturePts,userP) ;
   } 
 ++numDtmFeatures ;
/*
** Write Points
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Feature Points = %6ld",numFeaturePts ) ;
    for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.4lf %12.4lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   } 
/*
** Write To Data Object
*/
/*
 if( glbDataP != NULL )
   {
    if( dtmFeatureType != DTMFeatureType::TinPoint && dtmFeatureType != DTMFeatureType::RandomSpots && dtmFeatureType != DTMFeatureType::GroupSpots )
      {
       if( bcdtmObject_storeDtmFeatureInDataObject(glbDataP,DTMFeatureType::Breakline,nullUserTag,1,featurePtsP,numFeaturePts)) goto errexit ;
      }
    else
      {
       if( bcdtmObject_storeDtmFeatureInDataObject(glbDataP,DTMFeatureType::RandomSpots,nullUserTag,1,featurePtsP,numFeaturePts)) goto errexit ;
      }  
   } 
*/
 bcdtmWrite_message(0,0,0,"numFeaturePts = %8ld",numFeaturePts) ; 
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Load Function Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Load Function Error") ;
 return(ret) ;
/*
** Error Exit
*/
// errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int  bcdtmBrowse_triangleMesh(DTMFeatureType dtmFeatureType,int numTriangles,int numMeshPts,DPoint3d *meshPtsP,int numMeshFaces, long *meshFacesP,void *userP)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long n ;
/*
** Write Mesh
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeatureType = %4ld ** numTriangles = %8ld numMeshPts = %8ld meshPtsP = %p numMeshFaces = %8ld meshFacesP = %p ** userP = %p",
                        dtmFeatureType,numTriangles,numMeshPts,meshPtsP,numMeshFaces,meshFacesP,userP) ;
/*
** Write Fiest File And Last Five Entries For Each Array
*/
 if( dbg == 2 )
   {
    for( n = 0 ; n < 10 ; ++n )
      {
       bcdtmWrite_message(0,0,0,"meshPnt[%8ld] = %12.5lf %12.5lf %10.4lf",n,(meshPtsP+n)->x,(meshPtsP+n)->y,(meshPtsP+n)->z) ;
      }
    for( n = numMeshPts - 11 ; n < numMeshPts ; ++n )
      {
       bcdtmWrite_message(0,0,0,"meshPnt[%8ld] = %12.5lf %12.5lf %10.4lf",n,(meshPtsP+n)->x,(meshPtsP+n)->y,(meshPtsP+n)->z) ;
      }
    for( n = 0 ; n < 10 ; ++n )
      {
       bcdtmWrite_message(0,0,0,"meshFace[%8ld] = %8ld %8ld %8ld",n,*(meshFacesP+3*n),*(meshFacesP+3*n+1),*(meshFacesP+3*n+2)) ;
      }
    for( n = numMeshFaces - 11 ; n < numMeshFaces ; ++n )
      {
       bcdtmWrite_message(0,0,0,"meshFace[%8ld] = %8ld %8ld %8ld",n,*(meshFacesP+3*n),*(meshFacesP+3*n+1),*(meshFacesP+3*n+2)) ;
      }
   }

/*
** Increment Number Of Triangles loaded
*/
 numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Browse Triangle Mesh Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Browse Triangle Mesh  Error") ;
 return(ret) ;
/*
** Error Exit
*/
// errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int  bcdtmBrowse_triangleShadeMesh
(
 DTMFeatureType dtmFeatureType,
 int            numTriangles,
 int            numMeshPts,
 DPoint3d       *meshPtsP,
 DPoint3d       *meshVectorsP,
 int            numMeshFaces, 
 long           *meshFacesP,
 void           *userP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long n ;
/*
** Write Mesh
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeatureType = %4ld ** numTriangles = %8ld numMeshPts = %8ld meshPtsP = %p meshVectorsP = %p numMeshFaces = %8ld meshFacesP = %p ** userP = %p",
                        dtmFeatureType,numTriangles,numMeshPts,meshPtsP,meshVectorsP,numMeshFaces,meshFacesP,userP) ;
/*
** Write Fiest File And Last Five Entries For Each Array
*/
 if( dbg == 2 )
   {
    for( n = 0 ; n < numMeshPts ; ++n )
      {
       bcdtmWrite_message(0,0,0,"meshPnt[%8ld] = %12.5lf %12.5lf %10.4lf",n,(meshPtsP+n)->x,(meshPtsP+n)->y,(meshPtsP+n)->z) ;
      }
/*
    for( n = 0 ; n < 10 ; ++n )
      {
       bcdtmWrite_message(0,0,0,"meshPnt[%8ld] = %12.5lf %12.5lf %10.4lf",n,(meshPtsP+n)->x,(meshPtsP+n)->y,(meshPtsP+n)->z) ;
      }
    for( n = numMeshPts - 11 ; n < numMeshPts ; ++n )
      {
       bcdtmWrite_message(0,0,0,"meshPnt[%8ld] = %12.5lf %12.5lf %10.4lf",n,(meshPtsP+n)->x,(meshPtsP+n)->y,(meshPtsP+n)->z) ;
      }
*/
    for( n = 0 ; n < 10 ; ++n )
      {
       bcdtmWrite_message(0,0,0,"meshVector[%8ld] = <%12.5lf,%12.5lf,%12.5lf>",n,(meshVectorsP+n)->x,(meshVectorsP+n)->y,(meshVectorsP+n)->z) ;
      }
    for( n = numMeshPts - 11 ; n < numMeshPts ; ++n )
      {
       bcdtmWrite_message(0,0,0,"meshVector[%8ld] = <%12.5lf,%12.5lf,%12.5lf>",n,(meshVectorsP+n)->x,(meshVectorsP+n)->y,(meshVectorsP+n)->z) ;
      }

    for( n = 0 ; n < numMeshFaces / 3 ; ++n )
      {
       bcdtmWrite_message(0,0,0,"meshFace[%8ld] = %8ld %8ld %8ld",n,*(meshFacesP+3*n),*(meshFacesP+3*n+1),*(meshFacesP+3*n+2)) ;
      }
/*
    for( n = 0 ; n < 10 ; ++n )
      {
       bcdtmWrite_message(0,0,0,"meshFace[%8ld] = %8ld %8ld %8ld",n,*(meshFacesP+3*n),*(meshFacesP+3*n+1),*(meshFacesP+3*n+2)) ;
      }
    for( n = numMeshFaces / 3 - 11 ; n < numMeshFaces / 3 ; ++n )
      {
       bcdtmWrite_message(0,0,0,"meshFace[%8ld] = %8ld %8ld %8ld",n,*(meshFacesP+3*n),*(meshFacesP+3*n+1),*(meshFacesP+3*n+2)) ;
      }
*/
   }

// Store Triangles In DTM File

 if( userP != NULL )
   {
    int p1,p2,p3 ;
    DPoint3d  trgPnt[4] ;
    BC_DTM_OBJ *dtmP= ( BC_DTM_OBJ *) userP ;
    for( n = 0 ; n < numTriangles ; ++n )
      {
       p1 = *(meshFacesP+3*n  ) - 1 ;
       p2 = *(meshFacesP+3*n+1) - 1 ;
       p3 = *(meshFacesP+3*n+2) - 1 ;
       trgPnt[0].x = (meshPtsP+p1)->x ;  trgPnt[0].y = (meshPtsP+p1)->y ; trgPnt[0].z = (meshPtsP+p1)->z ;
       trgPnt[1].x = (meshPtsP+p2)->x ;  trgPnt[1].y = (meshPtsP+p2)->y ; trgPnt[1].z = (meshPtsP+p2)->z ;
       trgPnt[2].x = (meshPtsP+p3)->x ;  trgPnt[2].y = (meshPtsP+p3)->y ; trgPnt[2].z = (meshPtsP+p3)->z ;
       trgPnt[3].x = (meshPtsP+p1)->x ;  trgPnt[3].y = (meshPtsP+p1)->y ; trgPnt[3].z = (meshPtsP+p1)->z ;
       if( dbg == 2 )
         {
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,1,&dtmP->nullFeatureId,trgPnt,4)) goto errexit ;
          bcdtmWrite_message(0,0,0,"Triangle[%4ld] Points   %12.5lf %12.5lf %10.4lf",n,trgPnt[0].x,trgPnt[0].y,trgPnt[0].z) ;
          bcdtmWrite_message(0,0,0,"                        %12.5lf %12.5lf %10.4lf",trgPnt[1].x,trgPnt[1].y,trgPnt[1].z) ;
          bcdtmWrite_message(0,0,0,"                        %12.5lf %12.5lf %10.4lf",trgPnt[2].x,trgPnt[2].y,trgPnt[2].z) ;
          bcdtmWrite_message(0,0,0,"                        %12.5lf %12.5lf %10.4lf",trgPnt[3].x,trgPnt[3].y,trgPnt[3].z) ;
         }
      }
   }

// Increment Number Of Triangles loaded

 numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Browse Triangle Shade Mesh Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Browse Triangle Shade Mesh  Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
