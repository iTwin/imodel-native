
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtmInlines.h"
#include "bcDtmClass.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

int bcdtmVolume_callBackFunction(DTMFeatureType dtmFeatureType,DTMUserTag userTag,DTMFeatureId featureId,DPoint3d *featurePtsP,size_t numFeaturePts,void *userP) ;

int wmain(int argc, wchar_t *argv[])
{
 VOLRANGETAB *rangeTableP=NULL ;        /* ==> Pointer To Volume Range Table                */
 long        numRanges=0 ;              /* ==> Number Of Volume Ranges                      */
 DPoint3d    *polygonPtsP=NULL ;        /* ==> Pointer To Volume Polygon Points             */
 long        numPolygonPts=0 ;          /* ==> Number Of Volume Polygon Points              */
 void        *userP=NULL ;
 double      cut,fill,balance,area ;
 BC_DTM_OBJ  *dtmP=NULL,*fromDtmP=NULL,*toDtmP=NULL ;

// Initialise DTM

 bcdtmInitialise() ;
 if( fpLOG != NULL ) { fclose(fpLOG) ; fpLOG = NULL ; }
 bcdtmInitialise_openLogFile(L"bcVolume.log") ;

// Initialise Variables

 bcdtmWrite_message(0,0,0,"Bentley Civil Partitioned DTM Console Volumes") ;
 bcdtmWrite_message(0,0,0,"=============================================") ;
 bcdtmWrite_message(0,0,0,"Start") ;

// Get Arguements From Command Prompt

 if( argc < 3 )
   {
    bcdtmWrite_message(0,0,0,"Not Enough Arguements") ;
    bcdtmWrite_message(0,0,0,"C:>bcVolume < From Tin > < To Tin >") ;
    goto errexit ;
   }
 else
   {
    bcdtmWrite_message(0,0,0,"From Tin = %ws",argv[1]) ;
    bcdtmWrite_message(0,0,0,"To Tin   = %ws",argv[2]) ;
  }

//  Read DTMs

 if( bcdtmRead_fromFileDtmObject(&fromDtmP,argv[1])) goto errexit ;
 if( bcdtmRead_fromFileDtmObject(&toDtmP,argv[2])) goto errexit ;

//  Calculate Volumes

 bcdtmWrite_message(0,0,0,"Calculating Surface To Surface Volume") ;
 if( bcdtmTinVolume_surfaceToSurfaceDtmObjects(fromDtmP,toDtmP,rangeTableP,numRanges,polygonPtsP,numPolygonPts,(DTMFeatureCallback) bcdtmVolume_callBackFunction,userP,&cut,&fill,&balance,&area)) goto errexit ;
 bcdtmWrite_message(0,0,0,"cut = %15.3lf fill = %15.3lf balance = %15.3lf area = %15.3lf",cut,fill,balance,area) ;

// Clean Up

 cleanup :
 bcdtmWrite_message(0,0,0,"Cleaning Up") ;
 if( dtmP  != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
 
// Job Completed

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
int bcdtmVolume_callBackFunction
(    
    DTMFeatureType   dtmFeatureType,
    DTMUserTag       userTag,
    DTMFeatureId     featureId,
    DPoint3d         *featurePtsP,
    size_t           numFeaturePts,
    void             *userP
)

    //  Volume Call Back Function

    {
    int        ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    char       dtmFeatureTypeName[100] ;
    DPoint3d  *p3dP ;

    // Log Call

    if( dbg ) 
       {
       if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
       bcdtmWrite_message(0,0,0,"Feature[%8ld] ** %s userTag = %10I64d dtmFeatureId = %10I64d featurePtsP = %p numFeaturePts = %6ld userP = %p",dtmFeatureTypeName,userTag,featureId,featurePtsP,numFeaturePts,userP) ;
       } 

    // Write Points

    if( dbg == 2 )
       {
       bcdtmWrite_message(0,0,0,"Number Of Feature Points = %6ld",numFeaturePts ) ;
       for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
          {
          bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.4lf %12.4lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
          }
       } 

    // Clean Up

 cleanup :

    // Job Completed

    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Call Back Function Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Call Back Function Error") ;
    return(ret) ;

    // Error Exit

 errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    bcdtmWrite_message(0,0,0,"Error Exiting From call Back Function") ;
    goto cleanup ;
    }
