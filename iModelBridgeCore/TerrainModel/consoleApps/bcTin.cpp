/*--------------------------------------------------------------------------------------+
|
|     $Source: consoleApps/bcTin.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TerrainModel/Formats/terrainimporter.h"
#include "bcDTMBaseDef.h"
#include "dtmdefs.h"
#include "dtm2dfns.h"
#include "dtmevars.h"
//#include <TerrainModel\Core\DTMEnumerators.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

static long numDtmFeatures=0 ;
//XM--static DTM_DAT_OBJ *dataP=NULL ;
int  bcdtmLoad_dllLoadFunctionTest(DTMFeatureType dtmFeatureType,DTMUserTag userTag, DTMFeatureId featureId,DPoint3d *featurePtsP,size_t numFeaturePts,void *userP) ;

// BCivilDTM.2.0.lib odbc32.lib odbccp32.lib
int wmain(int argc, wchar_t *argv[])
{
//long    dbg=DTM_TRACE_VALUE(1),cdbg=DTM_CHECK_VALUE(1);
 long    edgeOption=1 ;
 long    fileType,startTime,importStartTime ;
 long    numFeatureErrors,numContourLines,numHardBreaks,numVoids,numIslands,numHoles ;
 long    numBreakVoids,numDrapeVoids,numGroupSpots,numPolygons,numHulls ;
 wchar_t    inputFile[256],dtmFile[256];
 double  maxSide=0.0 ;
 void    *userP=NULL ;
 BC_DTM_OBJ *dtmP=NULL ;
 WString surfaceName;

/*
** Initialise DTM
*/
 bcdtmInitialise() ;
 if( fpLOG != NULL ) { fclose(fpLOG) ; fpLOG = NULL ; }
 bcdtmInitialise_openLogFile(L"bcTin.log") ;
 importStartTime = bcdtmClock() ;
/*
** Initialise Variables
*/
 bcdtmWrite_message(0,0,0,"Bentley Civil Partitioned DTM Console Triangulation") ;
 bcdtmWrite_message(0,0,0,"===================================================") ;
 bcdtmWrite_message(0,0,0,"Start") ;
/*
** Get Arguements From Command Prompt
*/
 if( argc < 4 )
   {
    bcdtmWrite_message(0,0,0,"Not Enough Arguements") ;
    bcdtmWrite_message(0,0,0,"C:>bcTin <Input File Type {1=XYZ,2=DAT,3=DOJ,4=TerrainImporter}> <Input File Name*surfaceName> <Output Bentley Civil Tin File> <Dissolve Option {1,2,3}> < Max Side > ") ;
    bcdtmWrite_message(0,0,0,"if no surfaceName the first surface will be imported, if surfaceName = ? then we will list the available surfaces.") ;
    goto errexit ;
   }
 else
   {
/*
**  Validate Arguements
*/
    swscanf(argv[1],L"%ld",&fileType) ;
    wcscpy(inputFile,argv[2]) ;
    wchar_t* subName = wcsstr (inputFile, L"*");
    if (subName)
        {
        *subName++ = 0;
        surfaceName = subName;
        }

    bcdtmWrite_message(0,0,0,"Input File Type = %1ws",argv[1]) ;
    bcdtmWrite_message(0,0,0,"Input File Name = %ws",inputFile) ;
    if (subName)
        bcdtmWrite_message(0,0,0,"Input File surfaceName = %ws",subName) ;
    bcdtmWrite_message(0,0,0,"Output Tin File = %ws",argv[3]) ;
    if( argc >= 5 ) bcdtmWrite_message(0,0,0,"Dissolve Option = %ws",argv[4]) ; 
    if( argc >= 6 ) bcdtmWrite_message(0,0,0,"Max Side        = %ws",argv[5]) ; 

    wcscpy(dtmFile,argv[3]) ;
    if( fileType < 1 || fileType > 4 ){ bcdtmWrite_message(1,0,0,"Unknown File Type") ; goto errexit ; }
    if( argc >= 5 )
      {
       swscanf(argv[4],L"%ld",&edgeOption) ;
       if( edgeOption < 1 || edgeOption > 3 ){ bcdtmWrite_message(1,0,0,"Illegal Dissolve Option") ; goto errexit ; }
       if( argc == 5 && edgeOption == 3 ) edgeOption = 2 ;
      } 
    if( argc == 6 )
      {
       swscanf(argv[5],L"%lf",&maxSide) ;
       if( maxSide <= 0.0 ) edgeOption = 2 ;
      } 
   }
/*
**  Start Clock
*/
 startTime = importStartTime = bcdtmClock() ;
/*
**  Read XYZ To A Dtm Object
*/
 if( fileType == 1  )
   {
    bcdtmWrite_message(0,0,0,"Reading XYZ File %ws",inputFile) ;
    if( bcdtmRead_xyzFileToDtmObject(&dtmP,inputFile) ) goto errexit ; 
   }
/*
** Read Geopak Dat File
*/
 if( fileType == 2 ) 
   {
    bcdtmWrite_message(0,0,0,"Reading Geopak Dat File %ws",inputFile) ;
    if( bcdtmRead_geopakDatFileToDtmObject(&dtmP,inputFile)) goto errexit ; 
   }
/*
** Read Geopak Data Object
*/
 if( fileType == 3 ) 
   {
    bcdtmWrite_message(0,0,0,"Reading Geopak Data Object File %ws",inputFile) ;
    if( bcdtmRead_fromFileDtmObject(&dtmP,inputFile)) goto errexit ; 
   }
 if( fileType == 4 ) 
   {
    bcdtmWrite_message(0,0,0,"Reading TerrainImporter File %ws",inputFile) ;

    startTime = bcdtmClock() ;
    TerrainImporterPtr reader = TerrainImporter::CreateImporter (inputFile);

    if (surfaceName.empty() || surfaceName == L"?")
        {
        if (reader.IsValid())
            {
            TerrainInfoList surfaceInfos = reader->GetTerrains ();

            if (surfaceName == L"?")
                bcdtmWrite_message(0,0,0,"Querying Surfaces") ;
            for (TerrainInfoList::const_iterator it = surfaceInfos.begin(); it != surfaceInfos.end(); it++)
                {
                if (surfaceName.empty())
                    {
                    bcdtmWrite_message(0,0,0,"Importing Surface %ws", it->GetName().GetWCharCP()) ;
                    surfaceName = it->GetName();
                    break;
                    }
                else
                    bcdtmWrite_message(0,0,0,"Contains Surface %ws", it->GetName().GetWCharCP()) ;
                }
            if (surfaceName == L"?")
                exit(0);
            }
        }

    startTime = bcdtmClock() ;
    reader = TerrainImporter::CreateImporter (inputFile);

    if (reader.IsValid())
        {
        bvector<WString> surfacesToImport;
        surfacesToImport.push_back (surfaceName);
        ImportedTerrainList dtms = reader->ImportTerrains (surfacesToImport);
        reader = NULL;

        BcDTMPtr dtm = (dtms.begin())->GetTerrain ();
        dtmP = dtm->GetTinHandle ();
        dtmP->refCount++;
        }
      bcdtmWrite_message(0,0,0,"** Import Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   }
 /* 
** Write Stats
*/ 
 bcdtmWrite_message(0,0,0,"Time To Read File %ws = %8.3lf Secs",inputFile,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
 bcdtmWrite_message(0,0,0,"Number Of Points In File = %9ld",dtmP->numPoints) ; 
 bcdtmWrite_message(0,0,0,"x ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->xMin,dtmP->xMax,dtmP->xMax-dtmP->xMin) ;
 bcdtmWrite_message(0,0,0,"y ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->yMin,dtmP->yMax,dtmP->yMax-dtmP->yMin) ;
 bcdtmWrite_message(0,0,0,"z ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",dtmP->zMin,dtmP->zMax,dtmP->zMax-dtmP->zMin) ;
/*
** Create Tin For DTM Object
*/
 startTime = bcdtmClock() ;
 bcdtmWrite_message(0,0,0,"Creating Tin For DTM Object") ;
 bcdtmObject_setTriangulationParametersDtmObject(dtmP, dtmP->ppTol, dtmP->plTol, edgeOption,maxSide);
 if(bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
 bcdtmWrite_message(0,0,0,"** Tin Creation Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check Tin
*/
 startTime = bcdtmClock() ;
 bcdtmWrite_message(0,0,0,"Checking DTM Tin") ;
 if( bcdtmCheck_tinComponentDtmObject(dtmP)) 
   {
    bcdtmWrite_message(0,0,0,"Tin Invalid") ;
    goto errexit ;
   }
 else  bcdtmWrite_message(0,0,0,"Tin Valid") ; 
 bcdtmWrite_message (0, 0, 0, "** Tin Checking Time   = %8.3lf Seconds", bcdtmClock_elapsedTime (bcdtmClock (), startTime));
/*
     {
     BcDTMPtr bcDtm = IBcDTM::CreateFromDtmHandle (dtmP);

     DTMFeatureEnumerator features (bcDtm.get ());
     long count = 0;
     for (DTMFeatureInfo info : features)
         {
         DTMFeatureId id;
         DTMFeatureType type;
         DTMUserTag tag;
         bvector<DPoint3d> points;
         info.GetFeatureInfo (type, id, tag);
         info.GetFeaturePoints (points);
         count++;
         }
     count = dtmP->numFeatures;

     DTMMeshEnumerator en (bcDtm.get ());

     for (PolyfaceQueryP info : en)
         {
         }
     en.Reset ();
     }
*/
 
 
 /*
** Count Number Of Dtm Feature Type Tin Errors
*/
 bcdtmList_countTinFeaturesWithErrorsDtmObject(dtmP,&numFeatureErrors,&numContourLines,&numHardBreaks,&numVoids,&numIslands,&numHoles,&numBreakVoids,&numDrapeVoids,&numGroupSpots,&numPolygons,&numHulls) ;
 bcdtmWrite_message(0,0,0,"Number Tin Feature Errors = %8ld",numFeatureErrors) ; 
 if( numContourLines > 0 ) bcdtmWrite_message(0,0,0,"**** Number Of Contour Lines With Errors = %8ld",numContourLines ) ;
 if( numHardBreaks   > 0 ) bcdtmWrite_message(0,0,0,"**** Number Of Break Lines With Errors   = %8ld",numHardBreaks ) ;
 if( numVoids        > 0 ) bcdtmWrite_message(0,0,0,"**** Number Of Voids With Errors         = %8ld",numVoids ) ;
 if( numIslands      > 0 ) bcdtmWrite_message(0,0,0,"**** Number Of Islands With Errors       = %8ld",numIslands ) ;
 if( numHoles        > 0 ) bcdtmWrite_message(0,0,0,"**** Number Of Holes With Errors         = %8ld",numHoles ) ;
 if( numBreakVoids   > 0 ) bcdtmWrite_message(0,0,0,"**** Number Of Break Voids With Errors   = %8ld",numBreakVoids ) ;
 if( numDrapeVoids   > 0 ) bcdtmWrite_message(0,0,0,"**** Number Of Drape Voids With Errors   = %8ld",numDrapeVoids ) ;
 if( numPolygons     > 0 ) bcdtmWrite_message(0,0,0,"**** Number Of Polygons With Errors      = %8ld",numPolygons ) ;
 if( numGroupSpots   > 0 ) bcdtmWrite_message(0,0,0,"**** Number Of Group Spots With Errors   = %8ld",numGroupSpots ) ;
 if( numHulls        > 0 ) bcdtmWrite_message(0,0,0,"**** Number Of Hulls With Errors          = %8ld",numHulls ) ;

     {
    long    nbTriangles = 0;
    DTMState dummyState;
    long    dummy = 0;
    bool dummyBool;

    bcdtmUtility_getStatisticsDtmObject (dtmP,
                                         dummyState, dummy, dummy,
                                         nbTriangles,
                                         dummy, dummy, dummy, dummy, dummy, dummy, dummy, dummy, dummy, dummyBool, dummy);
    bcdtmWrite_message(0,0,0, "Num Triangles = %8ld", nbTriangles);
     }
/*
** Load Dtm Features With Errors
*/
 if( numFeatureErrors )
   {
    if( bcdtmInterruptLoad_dtmFeaturesWithTinErrorsDtmObject(dtmP,bcdtmLoad_dllLoadFunctionTest,userP)) goto errexit ;
//XM--    if( dataP != NULL ) 
//XM--      {
//XM--       if( bcdtmWrite_dataFileFromDataObject(dataP,L"tinErrorFeatures.dat")) goto errexit ;
//XM--       if( bcdtmObject_deleteDataObject(&dataP) ) goto errexit ;
//XM--      }
   }
/*
** Load Tin Hull
*/
 if( bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject(dtmP,DTMFeatureType::Hull,1000,bcdtmLoad_dllLoadFunctionTest,FALSE,DTMFenceType::Block, DTMFenceOption::Inside,NULL,0,userP)) goto errexit ; 
//XM-- if( dataP != NULL ) 
//XM--   {
//XM--    if( dataP->numPts > 0 ) if( bcdtmWrite_dataFileFromDataObject(dataP,L"hullFeature.dat")) goto errexit ;
//XM--    if( bcdtmObject_deleteDataObject(&dataP) ) goto errexit ;
//XM--   }
 goto writeDtm ;
/*
** Load Void Features
*/
 if (bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (dtmP, DTMFeatureType::Void, 1000, bcdtmLoad_dllLoadFunctionTest, FALSE, DTMFenceType::Block, DTMFenceOption::Inside, NULL, 0, userP)) goto errexit;
 //XM--if( dataP != NULL ) 
//XM--   {
//XM--    if( dataP->numPts > 0 ) if( bcdtmWrite_dataFileFromDataObject(dataP,L"voidFeatures.dat")) goto errexit ;
//XM--    if( bcdtmObject_deleteDataObject(&dataP) ) goto errexit ;
//XM--   }
/*
** Load Break Line Features
*/
 if (bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (dtmP, DTMFeatureType::Breakline, 1000, bcdtmLoad_dllLoadFunctionTest, FALSE, DTMFenceType::Block, DTMFenceOption::Inside, NULL, 0, userP)) goto errexit;
//XM-- if( dataP != NULL ) 
//XM--   {
//XM--    if( dataP->numPts > 0 ) if( bcdtmWrite_dataFileFromDataObject(dataP,L"breakFeatures.dat")) goto errexit ;
//XM--    if( bcdtmObject_deleteDataObject(&dataP) ) goto errexit ;
//XM--   }
/*
** Load Contour Line Features
*/
 if (bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (dtmP, DTMFeatureType::ContourLine, 1000, bcdtmLoad_dllLoadFunctionTest, FALSE, DTMFenceType::Block, DTMFenceOption::Inside, NULL, 0, userP)) goto errexit;
//XM-- if( dataP != NULL ) 
//XM--   {
//XM--    if( dataP->numPts > 0 ) if( bcdtmWrite_dataFileFromDataObject(dataP,L"contourLineFeatures.dat")) goto errexit ;
//XM--    if( bcdtmObject_deleteDataObject(&dataP) ) goto errexit ;
//XM--   } 
/*
** Load Triangle Features
*/
 if (bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (dtmP, DTMFeatureType::Triangle, 1000, bcdtmLoad_dllLoadFunctionTest, FALSE, DTMFenceType::Block, DTMFenceOption::Inside, NULL, 0, userP)) goto errexit;
//XM-- if( dataP != NULL ) 
//XM--   {
//XM--    if( dataP->numPts > 0 ) if( bcdtmWrite_dataFileFromDataObject(dataP,L"triangleFeatures.dat")) goto errexit ;
//XM--    if( bcdtmObject_deleteDataObject(&dataP) ) goto errexit ;
//XM--   }
/*
**  Write Dtm To File
*/
 writeDtm :
 bcdtmWrite_message(0,0,0,"Writing Dtm File") ; 
 startTime = bcdtmClock() ;
 if( bcdtmWrite_toFileDtmObject(dtmP,dtmFile)) goto errexit ;
 bcdtmWrite_message(0,0,0,"Time To Write Dtm File = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
** Clean Up
*/
 cleanup :
 bcdtmWrite_message(0,0,0,"Cleaning Up") ;
 if( dtmP  != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
 //XM-- if( dataP != NULL ) bcdtmObject_deleteDataObject(&dataP) ;
/* 
** Job Completed
*/
 bcdtmWrite_message(0,0,0,"Total Import Time      = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),importStartTime)) ; 
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
int  bcdtmLoad_dllLoadFunctionTest(DTMFeatureType dtmFeatureType,DTMUserTag userTag, DTMFeatureId featureId,DPoint3d *featurePtsP,size_t numFeaturePts,void *userP)
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
** Write Record
*/
 if( dbg ) 
   {
    if( bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Feature[%8ld] ** %s userTag = %10I64d featureId = %10I64d featurePtsP = %p numFeaturePts = %6ld userP = %p",numDtmFeatures,dtmFeatureTypeName,userTag,featureId,featurePtsP,numFeaturePts,userP) ;
   } 
 ++numDtmFeatures ;
/*
** Create Data object
*/
//XM-- if( dataP == NULL ) 
//XM--   {
//XM--    if( bcdtmObject_createDataObject(&dataP)) goto errexit ;
//XM--    dataP->incMemPts = dataP->iniMemPts  ;
//XM--   }
/*
** Write Points
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Feature Points = %6ld",numFeaturePts ) ;
    for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%6ld] = %12.4lf %12.4lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   } 
/*
** Write Feature To Data Object
*/
 if( dtmFeatureType == DTMFeatureType::TinHull ) dtmFeatureType = DTMFeatureType::Hull ;
 if( dtmFeatureType == DTMFeatureType::Triangle ) dtmFeatureType = DTMFeatureType::Breakline ;
// if( bcdtmObject_storeDtmFeatureInDataObject(dataP,dtmFeatureType,nullUserTag,nullGuid,featurePtsP,numFeaturePts)) goto errexit ;
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
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmWrite_message(0,0,0,"Error Exiting From Load Function") ;
 goto cleanup ;
    }

// 4 "d:\SafeData\LandXML\MX Roadway Designer Tutorial No Boundary.xml" l.bcdtm 1 100