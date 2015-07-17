/*--------------------------------------------------------------------------------------+
|
|     $Source: consoleApps/bcClip.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "bcDTMBaseDef.h"
#include "dtmevars.h"

//XM-- static DTM_DAT_OBJ *dataP=NULL ;

int wmain(int argc, wchar_t *argv[])
{
 int     dbg=DTM_TRACE_VALUE(1),cdbg=DTM_CHECK_VALUE(1) ;
 long    startTime=0,numClipPts=0,numBlockPts=0,numShapePts=0,numFencePts=0 ;
 auto clipOption = DTMClipOption::External;
 DPoint3d     *clipPtsP=NULL,*blockPtsP=NULL,*shapePtsP=NULL,*fencePtsP=NULL ;
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Initialise DTM
*/
 bcdtmInitialise() ;
 if( fpLOG != NULL ) { fclose(fpLOG) ; fpLOG = NULL ; }
 bcdtmInitialise_openLogFile(L"bcClip.log") ;

 numBlockPts  = 5 ;  // for survey000.dtm
 blockPtsP = ( DPoint3d * ) malloc( numBlockPts * sizeof(DPoint3d)) ;
 (blockPtsP+0)->x =  7096.0 ; (blockPtsP+0)->y = 171222.0 ; (blockPtsP+0)->z = 0.0 ;
 (blockPtsP+1)->x = 10375.0 ; (blockPtsP+1)->y = 171222.0 ; (blockPtsP+1)->z = 0.0 ;
 (blockPtsP+2)->x = 10375.0 ; (blockPtsP+2)->y = 173021.0 ; (blockPtsP+2)->z = 0.0 ;
 (blockPtsP+3)->x =  7096.0 ; (blockPtsP+3)->y = 173021.0 ; (blockPtsP+3)->z = 0.0 ;
 (blockPtsP+4)->x =  7096.0 ; (blockPtsP+4)->y = 171222.0 ; (blockPtsP+4)->z = 0.0 ;

 numShapePts = 8  ;  // for survey000.dtm
 shapePtsP = ( DPoint3d * ) malloc( numShapePts * sizeof(DPoint3d)) ;
 (shapePtsP+0)->x =  7294.8152 ; (shapePtsP+0)->y = 172547.9358 ; (shapePtsP+0)->z = 0.0 ;
 (shapePtsP+1)->x =  8439.9414 ; (shapePtsP+1)->y = 172650.9971 ; (shapePtsP+1)->z = 0.0 ;
 (shapePtsP+2)->x =  9819.8186 ; (shapePtsP+2)->y = 171568.8529 ; (shapePtsP+2)->z = 0.0 ;
 (shapePtsP+3)->x =  9562.1652 ; (shapePtsP+3)->y = 171116.5280 ; (shapePtsP+3)->z = 0.0 ;
 (shapePtsP+4)->x =  8268.1725 ; (shapePtsP+4)->y = 171030.6435 ; (shapePtsP+4)->z = 0.0 ;
 (shapePtsP+5)->x =  8245.2700 ; (shapePtsP+5)->y = 172049.8059 ; (shapePtsP+5)->z = 0.0 ;
 (shapePtsP+6)->x =  7437.9560 ; (shapePtsP+6)->y = 171774.9756 ; (shapePtsP+6)->z = 0.0 ;
 (shapePtsP+7)->x =  7294.8152 ; (shapePtsP+7)->y = 172547.9358 ; (shapePtsP+7)->z = 0.0 ;

/*
** Set Fence Coordinates
*/
 numFencePts = 5    ;  // for survey001.dtm
 fencePtsP = ( DPoint3d * ) malloc( numFencePts * sizeof(DPoint3d)) ;
 (fencePtsP+0)->x =  6812.0590 ; (fencePtsP+0)->y = 168467.7498 ;  (fencePtsP+0)->z = 0.0 ;
 (fencePtsP+1)->x =  6812.0590 ; (fencePtsP+1)->y = 163768.5540 ;  (fencePtsP+1)->z = 0.0 ;
 (fencePtsP+2)->x = 14804.7082 ; (fencePtsP+2)->y = 163768.5540 ;  (fencePtsP+2)->z = 0.0 ;
 (fencePtsP+3)->x = 14804.7082 ; (fencePtsP+3)->y = 168467.7498 ;  (fencePtsP+3)->z = 0.0 ;
 (fencePtsP+4)->x =  6812.0590 ; (fencePtsP+4)->y = 168467.7498 ;  (fencePtsP+4)->z = 0.0 ;

 numFencePts = 5    ;  // for 1m.dtm
 fencePtsP = ( DPoint3d * ) malloc( numFencePts * sizeof(DPoint3d)) ;
 (fencePtsP+0)->x = 2137515.0 ; (fencePtsP+0)->y = 221033.0 ;  (fencePtsP+0)->z = 0.0 ;
 (fencePtsP+1)->x = 2140500.0 ; (fencePtsP+1)->y = 221033.0 ;  (fencePtsP+1)->z = 0.0 ;
 (fencePtsP+2)->x = 2140500.0 ; (fencePtsP+2)->y = 223517.0 ;  (fencePtsP+2)->z = 0.0 ;
 (fencePtsP+3)->x = 2137515.0 ; (fencePtsP+3)->y = 223517.0 ;  (fencePtsP+3)->z = 0.0 ;
 (fencePtsP+4)->x = 2137515.0 ; (fencePtsP+4)->y = 221033.0 ;  (fencePtsP+4)->z = 0.0 ;
 
/*
** Initialise Variables
*/
 bcdtmWrite_message(0,0,0,"Bentley Civil Partitioned DTM Console Clip") ;
 bcdtmWrite_message(0,0,0,"==========================================") ;
 bcdtmWrite_message(0,0,0,"Start") ;
 clipOption = DTMClipOption::External ;
 startTime = bcdtmClock() ;

 if( bcdtmRead_fromFileDtmObject(&dtmP,L"1m.bcdtm"))  goto errexit ;
 if( bcdtmClip_toPolygonDtmObject(dtmP,fencePtsP,numFencePts,clipOption)) goto errexit ;
 if( bcdtmWrite_toFileDtmObject(dtmP,L"clipped1M.bcdtm")) goto errexit ;
 goto cleanup ;
 

 if( bcdtmClip_toPolygonDtmFile(L"1m.bcdtm",L"clipped1m.bcdtm",fencePtsP,numFencePts,clipOption)) goto errexit ;
 goto cleanup ;

 /*
 if( bcdtmRead_binaryFileP3D(L"points_30.bin",&clipPtsP,&numClipPts)) goto errexit ; 

 numClipPts = 29 ;
// (clipPtsP+numClipPts-1)->x = clipPtsP->x ;
// (clipPtsP+numClipPts-1)->y = clipPtsP->y ;
 
 if( bcdtmClip_toPolygonDtmFile(L"parent1.tin",L"clipParent1.dtm",clipPtsP,numClipPts,clipOption)) goto errexit ;
 bcdtmWrite_message(0,0,0,"Time To Clip Dtm File = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
*/

 
 goto cleanup ;
/*
** Clip survey000.Dtm File
*/
 bcdtmWrite_message(0,0,0,"Clipping Dtm File Internally") ;
 clipOption = DTMClipOption::External ;
 startTime = bcdtmClock() ;
 if( bcdtmClip_toPolygonDtmFile(L"survey000.dtm",L"intClipSurvey000.dtm",blockPtsP,numBlockPts,clipOption)) goto errexit ;
 bcdtmWrite_message(0,0,0,"Time To Clip Dtm File = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
** Clip Dtm File
*/
 bcdtmWrite_message(0,0,0,"Clipping Dtm File Externally") ;
 clipOption = DTMClipOption::External ;
 startTime = bcdtmClock() ;
 if( bcdtmClip_toPolygonDtmFile(L"survey000.dtm",L"extClipSurvey000.dtm",blockPtsP,numBlockPts,clipOption)) goto errexit ;
 bcdtmWrite_message(0,0,0,"Time To Clip Dtm File = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
 goto cleanup ;
/*
** Clip survey001.Dtm File
*/
 bcdtmWrite_message(0,0,0,"Clipping Dtm File Internally") ;
 clipOption = DTMClipOption::External;
 startTime = bcdtmClock() ;
 if( bcdtmClip_toPolygonDtmFile(L"survey001.dtm",L"intClipSurvey001.dtm",fencePtsP,numFencePts,clipOption)) goto errexit ;
 bcdtmWrite_message(0,0,0,"Time To Clip Dtm File = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
** Clip Dtm File
*/
 bcdtmWrite_message(0,0,0,"Clipping Dtm File Externally") ;
 clipOption = DTMClipOption::External;
 startTime = bcdtmClock() ;
 if( bcdtmClip_toPolygonDtmFile(L"survey001.dtm",L"extClipSurvey001.dtm",fencePtsP,numFencePts,clipOption)) goto errexit ;
 bcdtmWrite_message(0,0,0,"Time To Clip Dtm File = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
 goto cleanup ;
/*
** Write Stats
*/ 
/*
 bcdtmWrite_message(0,0,0,"Dtm Tin Statistics") ;
 bcdtmWrite_message(0,0,0,"==================") ;
 bcdtmWrite_message(0,0,0,"Number Of Tin Points    = %9ld",dtmP->numPoints) ; 
 bcdtmWrite_message(0,0,0,"Number Of Tin Lines     = %9ld",dtmP->numLines) ; 
 bcdtmWrite_message(0,0,0,"Number Of Tin Triangles = %9ld",dtmP->numTriangles) ; 
 bcdtmWrite_message(0,0,0,"Dtm Tin Coordinate Ranges") ;
 bcdtmWrite_message(0,0,0,"=========================") ;
 bcdtmWrite_message(0,0,0,"x ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->xMin,dtmP->xMax,dtmP->xMax-dtmP->xMin) ;
 bcdtmWrite_message(0,0,0,"y ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->yMin,dtmP->yMax,dtmP->yMax-dtmP->yMin) ;
 bcdtmWrite_message(0,0,0,"z ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->zMin,dtmP->zMax,dtmP->zMax-dtmP->zMin) ;
*/
/*
** Create Data Object
*/
//XM--  if( bcdtmObject_createDataObject(&dataP)) goto errexit ;
/*
** Scan Load The Dtm Feature 
*/
/*
** Clean Up
*/
 cleanup :
 bcdtmWrite_message(0,0,0,"Cleaning Up") ;
 if( dtmP  != NULL ) bcdtmObject_destroyAllDtmObjects() ;
//XM--  if( dataP != NULL ) bcdtmObject_deleteDataObject(&dataP) ;
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
