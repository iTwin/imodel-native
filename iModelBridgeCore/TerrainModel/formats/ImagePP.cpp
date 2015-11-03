/*--------------------------------------------------------------------------------------+
|
|     $Source: formats/ImagePP.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/WString.h>
#include <list>
#include <TerrainModel/Formats/TerrainImporter.h>
#include <TerrainModel/Core/bcdtminlines.h>
#include <TerrainModel/Formats/ImagePP.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>
#include <ImagePP/h/ImageppAPI.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HPMPool.h>
#include <ImagePP/all/h/HUTDEMRasterXYZPointsExtractor.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>


using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;
USING_NAMESPACE_IMAGEPP
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

#ifdef TODO
void readImage(char *imageFileP,char *proCodeP,double scaleFactor) ;

static char BaseGeoCoordPath[512] ;
static int  BaseGeoCordPathSet=0  ;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmDem_setGeoCoordinateDataPath(char *baseGeoCoordPathP)
{
 int ret=DTM_SUCCESS,dbg=0 ;
 char testPathName[512] ;
 FILE *testFP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Setting Base Geo Coordinate Data Path") ;
    bcdtmWrite_message(0,0,0,"baseGeoCoordPathP = %s",baseGeoCoordPathP) ;
   }
/*
** Check If Folder Exists
*/
 if( baseGeoCoordPathP != NULL && strlen(baseGeoCoordPathP) > 0 )
   {
    strcpy(testPathName,baseGeoCoordPathP) ;
    strcat(testPathName,"\\coordsys.dty") ;
    testFP = fopen(testPathName,"rb") ;
   }
/*
** Test If Path Valid
*/
 if( testFP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Invalid GeoCoordinateData Path") ;
    goto errexit ;
   }
/*
**  Set Path
*/
 strcpy(BaseGeoCoordPath,baseGeoCoordPathP) ;
 BaseGeoCordPathSet = 1 ;
/*
** Clean Up
*/
 cleanup :
 if( testFP != NULL ) fclose(testFP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Setting Base Geo Coordinate Data Path Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Setting Base Geo Coordinate Data Path Error") ;
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
int bcdtmDem_importAndTriangulateImageDtmObject
(
 BC_DTM_OBJ **dtmPP,                           // Pointer To Null Dtm Object
 char       *imageFileNameP,                   // Pointer To Image File Name
 double     imageScaleFactor,                  // Scale Factor For Reducing Number of Image Pixels Stored In DTM
 char       *projectionKeyP,                   // Projection Key For Extracted Image Coordinates
 double     unitConversionFactor,              // Unit Conversation Factor For Elevation Values - Metres To ??
 double     elevationScaleFactor               // Elevation Scale Factor
)
{
 int    ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 double dx,dy,nullValue=-98765.4321 ;
 long   nx,ny,resort=0,numColPts=0,lastColPts=0,colNum=0,dtmFeature,numRows,numCols,numVoids,numIslands,*islandsP=NULL ;
 long   startTime,cleanInternalVoids=0,numImagePoints,imageRegular=TRUE ;
 long   point,point1,point2 ;
 double xImageMin,yImageMin,zImageMin,xImageMax,yImageMax,zImageMax ;
 double xDist,yDist,colSpacing,rowSpacing ;
 DTM_TIN_POINT *p1P ,*p2P,*p3P,*pointP ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Importing Image And Triangulating") ;
    bcdtmWrite_message(0,0,0,"dtmPP                = %p",*dtmPP) ;
    bcdtmWrite_message(0,0,0,"imageFileNameP       = %s",imageFileNameP) ;
    bcdtmWrite_message(0,0,0,"imageScaleFactor     = %8.5lf",imageScaleFactor) ;
    bcdtmWrite_message(0,0,0,"projectionKeyP       = %s",projectionKeyP) ;
    bcdtmWrite_message(0,0,0,"unitConversionFactor = %8.5lf",unitConversionFactor) ;
    bcdtmWrite_message(0,0,0,"elevationScaleFactor = %8.5lf",elevationScaleFactor) ;
   }
/*
**  Read Image
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Calling DEMRasterPointsExtractorTester.cpp Methods") ;
    readImage(imageFileNameP,projectionKeyP,imageScaleFactor) ;
    bcdtmWrite_message(2,0,0,"Geospatial Read Completed") ;
    goto errexit ;        //  Exit With Error So Managed API Throws The Preceding Exception
   }
/*
** Check Scale Factor
*/
 if( imageScaleFactor <= 0.0 ||imageScaleFactor > 1.0 ) imageScaleFactor = 1.0 ;
/*
**  Check For NULL DTM
*/
 if( *dtmPP != NULL )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Null DTM Pointer") ;
    goto errexit ;
   }
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(dtmPP)) goto errexit ;
/*
** Create Image
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Image File = %s",imageFileNameP) ;
 if( bcdtmImagePP_importImageDtmObject(*dtmPP,imageFileNameP,&numRows,&numCols,&nullValue,imageScaleFactor,projectionKeyP)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"**** Image Read Time            = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    bcdtmWrite_message(0,0,0,"**** Number Of Rows             = %8ld",numRows) ;
    bcdtmWrite_message(0,0,0,"**** Number Of Columns          = %8ld",numCols) ;
    bcdtmWrite_message(0,0,0,"**** Number Of Pixels           = %8ld",numCols*numRows) ;
    bcdtmWrite_message(0,0,0,"**** Null Value                 = %12.5lf",nullValue) ;
    bcdtmWrite_message(0,0,0,"**** Number Of DEM Points Read  = %8ld",(*dtmPP)->numPoints) ;
   }
/*
** Check For More Than One Row Or One Column
*/
 if( numRows <= 1 || numCols <= 1 )
   {
    bcdtmWrite_message(1,0,0,"Requires More Than One Row Or One Column") ;
    goto errexit ;
   }
/*
**  Calculate Row And Column Spacing
*/
 dx = (*dtmPP)->xRange / ( double ) ( numCols - 1 ) ;
 dy = (*dtmPP)->yRange / ( double ) ( numRows - 1 ) ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"column spacing  = %15.12lf  row spacing  = %15.12lf",dx,dy) ;
    bcdtmWrite_message(0,0,0,"xMin = %15.12lf yMin = %15.12lf ** xMax = %15.12lf yMax = %15.12lf",(*dtmPP)->xMin,(*dtmPP)->yMin,(*dtmPP)->xMax,(*dtmPP)->yMax) ;
   }
/*
** Count Number Of Missing Values
*/
 if( dbg )
   {
    long point,numValues=0,numMissingValues=0 ;
    for( point = 0 ; point < (*dtmPP)->numPoints ; ++point )
      {
       pointP = pointAddrP(*dtmPP,point) ;
       if( pointP->Z == nullValue ) ++numMissingValues ;
       else                         ++numValues ;
      }
    bcdtmWrite_message(0,0,0,"numMissing = %8ld numValues = %8ld totalValues = %8ld",numMissingValues,numValues,numMissingValues+numValues) ;
   }
/*
** Get Bounding Rectangle For Valid Image Points
*/
 numImagePoints = 0 ;
 xImageMin = (*dtmPP)->xMin ; yImageMin = (*dtmPP)->yMin ; zImageMin = (*dtmPP)->zMin ;
 xImageMax = (*dtmPP)->xMax ; yImageMax = (*dtmPP)->yMax ; zImageMax = (*dtmPP)->zMax ;
 for( point = 0 ; point < (*dtmPP)->numPoints ; ++point )
   {
    pointP = pointAddrP(*dtmPP,point) ;
    if( pointP->Z != nullValue )
      {
       if( numImagePoints == 0 )
         {
          xImageMin = xImageMax = pointP->X ;
          yImageMin = yImageMax = pointP->Y ;
          zImageMin = zImageMax = pointP->Z ;
         }
       else
         {
          if( pointP->X < xImageMin ) xImageMin = pointP->X ;
          if( pointP->X > xImageMax ) xImageMax = pointP->X ;
          if( pointP->Y < yImageMin ) yImageMin = pointP->Y ;
          if( pointP->Y > yImageMax ) yImageMax = pointP->Y ;
          if( pointP->Z < zImageMin ) zImageMin = pointP->Z ;
          if( pointP->Z > zImageMax ) zImageMax = pointP->Z ;
         }
       ++numImagePoints ;
      }
   }
/*
** Write Bounding Rectangle For Valid Image Points
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Valid Image Points = %8ld",numImagePoints) ;
    bcdtmWrite_message(0,0,0,"xImageMin = %12.5lf  xImageMax = %12.5lf xImageRange = %12.5lf",xImageMin,xImageMax,xImageMax-xImageMin) ;
    bcdtmWrite_message(0,0,0,"yImageMin = %12.5lf  yImageMax = %12.5lf yImageRange = %12.5lf",yImageMin,yImageMax,yImageMax-yImageMin) ;
    bcdtmWrite_message(0,0,0,"zImageMin = %12.5lf  zImageMax = %12.5lf zImageRange = %12.5lf",zImageMin,zImageMax,zImageMax-zImageMin) ;
   }
/*
** Remove Missing Values Outside Of Bounding Rectangle For Valid Image Points
*/
 point1 = 0 ;
 p1P = pointAddrP(*dtmPP,point1) ;
 for( point2 = 0 ; point2 < (*dtmPP)->numPoints ; ++point2 )
   {
    p2P = pointAddrP(*dtmPP,point2) ;
    if( p2P->X >= xImageMin && p2P->X <= xImageMax && p2P->Y >= yImageMin && p2P->Y <= yImageMax )
      {
       if( p2P != p1P ) *p1P = *p2P ;
       ++point1 ;
       p1P = pointAddrP(*dtmPP,point1) ;
      }
   }
 (*dtmPP)->numPoints = point1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Image DTM Points = %8ld",(*dtmPP)->numPoints) ;
/*
** Resize Memory
*/
 if( bcdtmObject_resizeMemoryDtmObject(*dtmPP)) goto errexit ;
/*
** Reset Bounding Cube
*/
 if( bcdtmMath_setBoundingCubeDtmObject(*dtmPP)) goto errexit ;
/*
** Determine Row And Column Spacing
*/
 p1P = pointAddrP(*dtmPP,0) ;
 dx = dx / 100.0 ;
 dy = dy / 100.0 ;
 rowSpacing = (*dtmPP)->yRange ;
 colSpacing = (*dtmPP)->xRange ;
 for( point = 1 ; point < (*dtmPP)->numPoints ; ++point )
   {
    p2P = pointAddrP(*dtmPP,point) ;
    xDist = fabs(p1P->X - p2P->X) ;
    yDist = fabs(p1P->Y - p2P->Y) ;
    if( xDist > dx && xDist < colSpacing  ) colSpacing = xDist ;
    if( yDist > dy && yDist < rowSpacing  ) rowSpacing = yDist ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"colSpacing = %12.10lf rowSpacing = %12.10lf",colSpacing,rowSpacing) ;
/*
** Make Row And Column Coordinates Identical
*/
 for( point = 0 ; point < (*dtmPP)->numPoints ; ++point )
   {
    p1P = pointAddrP(*dtmPP,point) ;
    nx    = (long )(( p1P->X - (*dtmPP)->xMin ) / colSpacing + 0.1 ) ;
    p1P->X = (*dtmPP)->xMin + ( double ) nx * colSpacing ;
    ny    = (long )(( p1P->Y - (*dtmPP)->yMin ) / rowSpacing + 0.1 ) ;
    p1P->Y = (*dtmPP)->yMin + ( double ) ny * rowSpacing ;
   }
/*
** Check DTM Is Sorted
*/
 (*dtmPP)->numSortedPoints = (*dtmPP)->numPoints ;
 if( bcdtmDem_checkSortOrderDemDtmObject(*dtmPP,0))
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting DEM DTM") ;
    startTime = bcdtmClock() ;
    (*dtmPP)->numSortedPoints = 1 ;
    (*dtmPP)->dtmState = DTMState::Data ;
    if( bcdtmObject_sortDtmObject(*dtmPP)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"DEM DTM Sort Time = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
**  Recheck Sort Order
*/
    if( cdbg )
      {
       bcdtmWrite_message(0,0,0,"Re Checking DTM Sort Order") ;
       if( bcdtmDem_checkSortOrderDemDtmObject(*dtmPP,0))
         {
          bcdtmWrite_message(1,0,0,"DTM Sort Order Invalid") ;
          goto errexit ;
         }
       bcdtmWrite_message(0,0,0,"DTM Sort Order Valid") ;
     }
  }
/*
** Check Number Of Rows For Each Column Are Consistent
*/
 colNum = 1 ;
 numColPts = 1 ;
 lastColPts = -999 ;
 p1P = pointAddrP(*dtmPP,0) ;
 imageRegular = TRUE ;
 for( point = 1 ; point < (*dtmPP)->numPoints && imageRegular == TRUE ; ++point )
   {
    p2P = pointAddrP(*dtmPP,point) ;
    if( p2P->X != p1P->X )
      {
       if( dbg == 2 )
         {
          p3P = pointAddrP(*dtmPP,point-1) ;
          bcdtmWrite_message(0,0,0,"Column[%8ld] ** numRows = %8ld ** botX = %12.10lf botY = %12.10lf ** topX = %12.10lf toP->Y = %12.10lf",colNum,numColPts,p1P->X,p1P->Y,p3P->X,p3P->Y) ;
         }
       if( lastColPts != -999 && numColPts != lastColPts ) imageRegular = FALSE ;
       lastColPts = numColPts ;
       p1P = p2P ;
       ++colNum ;
       numColPts = 1 ;
      }
    else ++numColPts ;
   }
 if( dbg == 2 && imageRegular == TRUE )
   {
    p3P = pointAddrP(*dtmPP,(*dtmPP)->numPoints-1) ;
    bcdtmWrite_message(0,0,0,"Column[%8ld] ** numrows = %8ld ** botX = %12.10lf botY = %12.10lf ** topX = %12.10lf toP->Y = %12.10lf",colNum,numColPts,p1P->X,p1P->Y,p3P->X,p3P->Y) ;
   }
 if( numColPts != lastColPts ) imageRegular = FALSE ;
/*
**  Count Number Of Rows And Columns
*/
 if( imageRegular == TRUE )
   {
/*
    point   = 1 ;
    numRows = 1 ;
    p1P = pointAddrP(*dtmPP,0) ;
    p2P = pointAddrP(*dtmPP,point) ;
    while( p2P->X == p1P->X && point < (*dtmPP)->numPoints )
      {
       ++numRows ;
       p1P = p2P ;
       ++point  ;
       p2P = pointAddrP(*dtmPP,point) ;
      }
    numCols = (*dtmPP)->numPoints/numRows ;
*/
    numRows = lastColPts ;
    numCols = colNum ;
/*
** Triangulate Using DEM Triangulation Engine
*/
   startTime = bcdtmClock() ;
   if( dbg ) bcdtmWrite_toFileDtmObject(*dtmPP,L"demImageDataState.bcdtm") ;
   if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating %8ld Point DEM DTM ** numRows = %6ld numCols = %6ld",(*dtmPP)->numPoints,numRows,numCols) ;
   if( bcdtmObject_triangulateDemDtmObject(*dtmPP,numRows,numCols,nullValue)) goto errexit ;
   if( dbg ) bcdtmWrite_message(0,0,0,"DEM DTM Triangulation Time = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
   if( dbg ) bcdtmWrite_toFileDtmObject(*dtmPP,L"demImageTinState.bcdtm") ;
  }
/*
** Triangulate Using Random Triangulation Engine
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating %8ld Point DTM",(*dtmPP)->numPoints) ;
    (*dtmPP)->edgeOption = 1 ;
    (*dtmPP)->dtmState = DTMState::Data ;
     bcdtmMath_calculateMachinePrecisionForDtmObject(*dtmPP) ;
    (*dtmPP)->ppTol = (*dtmPP)->plTol = (*dtmPP)->mppTol * 10000.0 ;
    if( bcdtmObject_triangulateDtmObject(*dtmPP)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"DTM Triangulation Time = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
**  Void Missing Values
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Voiding Missing Values") ;
    if( bcdtmObject_placeVoidsAroundNullValuesDtmObject(*dtmPP,nullValue)) goto errexit ;
   }
 goto apply  ;
/*
** Count Number Of Voids
*/
 numVoids = 0 ;
 numIslands = 0 ;
 for( dtmFeature = 0 ; dtmFeature < (*dtmPP)->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(*dtmPP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ) ++numVoids ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"numVoids = %8ld ** numIslands = %8ld",numVoids,numIslands) ;
/*
**  Clean Up Voids Resulting From Null Values
*/
 if( numVoids > 0 )
   {
/*
**  Remove Voids On Tin Hull
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Voids On Tin Hull") ;
    if( bcdtmEdit_removeInsertedVoidsOnTinHullDtmObject(*dtmPP,0)) goto errexit ;
/*
**  Clean Internal Voids
*/
    if( cleanInternalVoids )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Null Values From Internal Voids") ;
       for( dtmFeature = 0 ; dtmFeature < (*dtmPP)->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(*dtmPP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Cleaning Void Feature %8ld",dtmFeature) ;
/*
**           Get Island Features Internal To Void
*/
             if( dbg  == 2 ) bcdtmWrite_message(0,0,0,"Getting IslandS Internal To Void") ;
             if( bcdtmEdit_getIslandsInternalToVoidDtmObject(*dtmPP,dtmFeature,&islandsP,&numIslands)) goto errexit ;
             if( dbg  == 2 ) bcdtmWrite_message(0,0,0,"Number Of Islands = %8ld",numIslands)  ;
/*
**           Remove Internal Void Points And Lines
*/
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Removing Internal Void Points And Lines") ;
             if( bcdtmEdit_removeInternalVoidPointsAndLinesDtmObject(*dtmPP,dtmFeature,islandsP,numIslands)) goto errexit ;
             if( islandsP != NULL ) { free(islandsP) ; islandsP = NULL ; }
            }
         }
      }
/*
**  Clean Dtm Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Dtm Object") ;
    if( bcdtmList_cleanDtmObject(*dtmPP)) goto errexit ;
   }
/*
** Apply Unit Conversion Factor To Elevation Values
*/
 apply :
 if( unitConversionFactor != 1.0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Applying Unit Conversions To Elevations") ;
    (*dtmPP)->zMin = (*dtmPP)->zMin * unitConversionFactor ;
    (*dtmPP)->zMax = (*dtmPP)->zMax * unitConversionFactor ;
    (*dtmPP)->zRange = (*dtmPP)->zRange * unitConversionFactor ;
    for( point = 0 ; point < (*dtmPP)->numPoints ; ++point )
      {
       p1P = pointAddrP((*dtmPP),point) ;
       p1P->Z = p1P->Z * unitConversionFactor ;
      }
   }
/*
** Apply Scale Factor To Elevation Values
*/
 if( elevationScaleFactor != 1.0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Applying Scale Conversions To Elevations") ;
    (*dtmPP)->zRange = (*dtmPP)->zRange * elevationScaleFactor ;
    (*dtmPP)->zMax   = (*dtmPP)->zMin + (*dtmPP)->zRange ;
    for( point = 0 ; point < (*dtmPP)->numPoints ; ++point )
      {
       p1P = pointAddrP((*dtmPP),point) ;
       p1P->Z = (*dtmPP)->zMin + ( p1P->Z - (*dtmPP)->zMin ) * elevationScaleFactor ;
      }
   }
/*
** Write Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of DTM Points = %8ld",(*dtmPP)->numPoints) ;
    bcdtmWrite_message(0,0,0,"X ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",(*dtmPP)->xMin,(*dtmPP)->xMax,(*dtmPP)->xRange) ;
    bcdtmWrite_message(0,0,0,"Y ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",(*dtmPP)->yMin,(*dtmPP)->yMax,(*dtmPP)->yRange) ;
    bcdtmWrite_message(0,0,0,"Z ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",(*dtmPP)->zMin,(*dtmPP)->zMax,(*dtmPP)->zRange) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( islandsP != NULL ) { free(islandsP) ; islandsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing Image And Triangulating Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing Image And Triangulating Error") ;
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
int bcdtmDem_importAndTriangulateImagePointsDtmObject
(
 BC_DTM_OBJ **dtmPP,                           // Pointer To Null Dtm Object
 double     *imagePointsP,                     // Pointer To The Image Points
 long       numRows,                           // Number of rows
 long       numCols,                           // Number of columns
 double     unitConversionFactor,              // Unit Conversation Factor For Elevation Values - Metres To ??
 double     elevationScaleFactor               // Elevation Scale Factor
)
{
 int ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 double dx,dy,nullValue=-98765.4321 ;
 long   nx,ny,point,resort=0,dtmFeature,numVoids,numIslands,*islandsP=NULL ;
 long   startTime,cleanInternalVoids=0 ;
 DTM_TIN_POINT *p1P ,*p2P ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Importing Image And Triangulating") ;
    bcdtmWrite_message(0,0,0,"dtmPP                = %p",*dtmPP) ;
    bcdtmWrite_message(0,0,0,"unitConversionFactor = %8.5lf",unitConversionFactor) ;
    bcdtmWrite_message(0,0,0,"elevationScaleFactor = %8.5lf",elevationScaleFactor) ;
   }

/*
**  Check For NULL DTM
*/
 if( *dtmPP != NULL )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Null DTM Pointer") ;
    goto errexit ;
   }
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(dtmPP)) goto errexit ;
/*
** Create Image
*/
 startTime = bcdtmClock() ;
 if( bcdtmImagePP_importImagePointsDtmObject(*dtmPP,imagePointsP,numRows*numCols,numRows,numCols,&nullValue)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"**** Number Of Rows             = %8ld",numRows) ;
    bcdtmWrite_message(0,0,0,"**** Number Of Columns          = %8ld",numCols) ;
    bcdtmWrite_message(0,0,0,"**** Number Of Pixels           = %8ld",numCols*numRows) ;
    bcdtmWrite_message(0,0,0,"**** Null Value                 = %12.5lf",nullValue) ;
    bcdtmWrite_message(0,0,0,"**** Number Of DEM Points Read  = %8ld",(*dtmPP)->numPoints) ;
   }
/*
** Check For More Than One Row Or One Column
*/
 if( numRows <= 1 || numCols <= 1 )
   {
    bcdtmWrite_message(1,0,0,"Requires More Than One Row Or One Column") ;
    goto errexit ;
   }
/*
** Count Number Of Null Values
*/
 if( dbg )
   {
    long point,numValues=0,numMissingValues=0 ;
    DTM_TIN_POINT *pointP ;
    for( point = 0 ; point < (*dtmPP)->numPoints ; ++point )
      {
       pointP = pointAddrP(*dtmPP,point) ;
       if( pointP->Z == nullValue ) ++numMissingValues ;
       else                         ++numValues ;
      }
    bcdtmWrite_message(0,0,0,"numMissing = %8ld numValues = %8ld totalValues = %8ld",numMissingValues,numValues,numMissingValues+numValues) ;
   }
/*
**  Make Row And Column Coordinates Identical
*/
 dx = (*dtmPP)->xRange / ( double ) ( numCols - 1 ) ;
 dy = (*dtmPP)->yRange / ( double ) ( numRows - 1 ) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"dx = %15.12lf dy = %15.12lf ** xMin = %15.12lf yMin = %15.12lf",dx,dy,(*dtmPP)->xMin,(*dtmPP)->yMin) ;
 for( point = 0 ; point < (*dtmPP)->numPoints ; ++point )
   {
    p1P = pointAddrP(*dtmPP,point) ;
    nx    = (long )(( p1P->X - (*dtmPP)->xMin ) / dx + 0.1 ) ;
    p1P->X = (*dtmPP)->xMin + ( double ) nx * dx ;
    ny    = (long )(( p1P->Y - (*dtmPP)->yMin ) / dy + 0.1 ) ;
    p1P->Y = (*dtmPP)->yMin + ( double ) ny * dy ;
   }
/*
** Check DTM Is Sorted
*/
 (*dtmPP)->numSortedPoints = (*dtmPP)->numPoints ;
 if( bcdtmDem_checkSortOrderDemDtmObject(*dtmPP,0))
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting DEM DTM") ;
    startTime = bcdtmClock() ;
    (*dtmPP)->numSortedPoints = 1 ;
    (*dtmPP)->dtmState = DTMState::Data ;
    if( bcdtmObject_sortDtmObject(*dtmPP)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"DEM DTM Sort Time = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
**  Count Number Of Rows And Columns
*/
    point   = 1 ;
    numRows = 1 ;
    p1P = pointAddrP(*dtmPP,0) ;
    p2P = pointAddrP(*dtmPP,point) ;
    while( p2P->X == p1P->X && point < (*dtmPP)->numPoints )
      {
       ++numRows ;
       p1P = p2P ;
       ++point  ;
       p2P = pointAddrP(*dtmPP,point) ;
      }
    numCols = (*dtmPP)->numPoints/numRows ;
   }
/*
** Triangulate
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating %8ld Point DEM DTM ** numRows = %6ld numCols = %6ld",(*dtmPP)->numPoints,numRows,numCols) ;
 if( bcdtmObject_triangulateDemDtmObject(*dtmPP,numRows,numCols,nullValue)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"DEM DTM Triangulation Time = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 goto apply  ;
/*
** Count Number Of Voids
*/
 numVoids = 0 ;
 numIslands = 0 ;
 for( dtmFeature = 0 ; dtmFeature < (*dtmPP)->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(*dtmPP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ) ++numVoids ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"numVoids = %8ld ** numIslands = %8ld",numVoids,numIslands) ;
/*
**  Clean Up Voids Resulting From Null Values
*/
 if( numVoids > 0 )
   {
/*
**  Remove Voids On Tin Hull
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Voids On Tin Hull") ;
    if( bcdtmEdit_removeInsertedVoidsOnTinHullDtmObject(*dtmPP,0)) goto errexit ;
/*
**  Clean Internal Voids
*/
    if( cleanInternalVoids )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Null Values From Internal Voids") ;
       for( dtmFeature = 0 ; dtmFeature < (*dtmPP)->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(*dtmPP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Cleaning Void Feature %8ld",dtmFeature) ;
/*
**           Get Island Features Internal To Void
*/
             if( dbg  == 2 ) bcdtmWrite_message(0,0,0,"Getting IslandS Internal To Void") ;
             if( bcdtmEdit_getIslandsInternalToVoidDtmObject(*dtmPP,dtmFeature,&islandsP,&numIslands)) goto errexit ;
             if( dbg  == 2 ) bcdtmWrite_message(0,0,0,"Number Of Islands = %8ld",numIslands)  ;
/*
**           Remove Internal Void Points And Lines
*/
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Removing Internal Void Points And Lines") ;
             if( bcdtmEdit_removeInternalVoidPointsAndLinesDtmObject(*dtmPP,dtmFeature,islandsP,numIslands)) goto errexit ;
             if( islandsP != NULL ) { free(islandsP) ; islandsP = NULL ; }
            }
         }
      }
/*
**  Clean Dtm Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Dtm Object") ;
    if( bcdtmList_cleanDtmObject(*dtmPP)) goto errexit ;
   }
/*
** Apply Unit Conversion Factor To Elevation Values
*/
 apply :
 if( unitConversionFactor != 1.0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Applying Unit Conversions To Elevations") ;
    (*dtmPP)->zMin = (*dtmPP)->zMin * unitConversionFactor ;
    (*dtmPP)->zMax = (*dtmPP)->zMax * unitConversionFactor ;
    (*dtmPP)->zRange = (*dtmPP)->zRange * unitConversionFactor ;
    for( point = 0 ; point < (*dtmPP)->numPoints ; ++point )
      {
       p1P = pointAddrP((*dtmPP),point) ;
       p1P->Z = p1P->Z * unitConversionFactor ;
      }
   }
/*
** Apply Scale Factor To Elevation Values
*/
 if( elevationScaleFactor != 1.0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Applying Scale Conversions To Elevations") ;
    (*dtmPP)->zRange = (*dtmPP)->zRange * elevationScaleFactor ;
    (*dtmPP)->zMax   = (*dtmPP)->zMin + (*dtmPP)->zRange ;
    for( point = 0 ; point < (*dtmPP)->numPoints ; ++point )
      {
       p1P = pointAddrP((*dtmPP),point) ;
       p1P->Z = (*dtmPP)->zMin + ( p1P->Z - (*dtmPP)->zMin ) * elevationScaleFactor ;
      }
   }
/*
** Write Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of DTM Points = %8ld",(*dtmPP)->numPoints) ;
    bcdtmWrite_message(0,0,0,"X ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",(*dtmPP)->xMin,(*dtmPP)->xMax,(*dtmPP)->xRange) ;
    bcdtmWrite_message(0,0,0,"Y ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",(*dtmPP)->yMin,(*dtmPP)->yMax,(*dtmPP)->yRange) ;
    bcdtmWrite_message(0,0,0,"Z ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",(*dtmPP)->zMin,(*dtmPP)->zMax,(*dtmPP)->zRange) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( islandsP != NULL ) { free(islandsP) ; islandsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing Image And Triangulating Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing Image And Triangulating Error") ;
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
int bcdtmDem_createLatticeFromImageLatticeObject(DTM_LAT_OBJ **latticePP,char *imageFileNameP)
{
 int ret=DTM_SUCCESS,dbg=0 ;
 long numRows=0,numCols=0 ;
 long startTime ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Lattice From Image") ;
    bcdtmWrite_message(0,0,0,"latticePP      = %p",*latticePP) ;
    bcdtmWrite_message(0,0,0,"imageFileNameP = %s",imageFileNameP) ;
   }
/*
**  Check For NULL DTM
*/
 if( *latticePP != NULL )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Null Lattice Pointer") ;
    goto errexit ;
   }
/*
** Create Lattice Object
*/
 if( bcdtmObject_createLatticeObject(latticePP)) goto errexit ;
/*
** Create Image
*/
 startTime = bcdtmClock() ;
 bcdtmWrite_message(0,0,0,"Reading Image File = %s",imageFileNameP) ;
 if( bcdtmImagePP_importImageLatticeObject(*latticePP,imageFileNameP)) goto errexit ;
/*
** Write Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Image Read Time           = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    bcdtmWrite_message(0,0,0,"Number Of Lattice Points  = %8ld",(*latticePP)->NOLATPTS) ;
    bcdtmWrite_message(0,0,0,"Number Of Active Points   = %8ld",(*latticePP)->NOACTPTS) ;
    bcdtmWrite_message(0,0,0,"Number Of Rows            = %8ld",(*latticePP)->NXL) ;
    bcdtmWrite_message(0,0,0,"Number Of Columns         = %8ld",(*latticePP)->NYL) ;
    bcdtmWrite_message(0,0,0,"Row Spacing               = %12.5lf",(*latticePP)->DY) ;
    bcdtmWrite_message(0,0,0,"Column Spacing            = %12.5lf",(*latticePP)->DX) ;
    bcdtmWrite_message(0,0,0,"X ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",(*latticePP)->LXMIN,(*latticePP)->LXMAX,(*latticePP)->LXDIF) ;
    bcdtmWrite_message(0,0,0,"Y ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",(*latticePP)->LYMIN,(*latticePP)->LYMAX,(*latticePP)->LYDIF) ;
    bcdtmWrite_message(0,0,0,"Z ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",(*latticePP)->LZMIN,(*latticePP)->LZMAX,(*latticePP)->LZDIF) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Lattice From Image Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Lattice From Image Error") ;
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
int bcdtmImagePP_getImageSizeAndProjection
(
 char *imageFileNameP,                     // ==>  Image File Name
 long *imageSizeP,                         // <==  Image Size In Pixels
 char **gcsNamePP,                         // <==  GCS Name
 char **gcsDescPP,                         // <==  GCS Description
 char **gcsProjPP,                         // <==  GCS Projection
 char **gcsUnitsPP                         // <==  GCS Units
)
{
 int ret=DTM_SUCCESS,dbg=0 ;
 char *gcsNameP,*gcsDescP,*gcsProjP,*gcsUnitsP ;
 const void *rasterCordSysP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Image Size And Projection Parameters") ;
    bcdtmWrite_message(0,0,0,"imageFileNameP = %s",imageFileNameP) ;
   }
/*
** Set DLL Path
*/
 HRFRasterFileFactory::GetInstance()->SetGeocoordPath(_TEXT("basegeocoord.dll"));
 tstring geoCordPath =   HRFRasterFileFactory::GetInstance()->GetGeocoordPath();
 if( dbg ) bcdtmWrite_message(0,0,0,"geoCordPath = %s",geoCordPath.c_str()) ;
/*
** Initialise Geo Coordinate System
*/
 tstring dataDirectory = _TEXT(".\\GeoCoordinateData");
 if( BaseGeoCordPathSet ) dataDirectory = BaseGeoCoordPath ;
 HCPGCoordUtility::GetInstance()->InitializeGeoCoord(dataDirectory);
 if( dbg ) bcdtmWrite_message(0,0,0,"Geo Coordinate System Initialised") ;
/*
** Allocate Memory Pool To Store Image
*/
 HFCPtr<HPMPool> pPool(new HPMPool(65536, HPMPool::KeepLastBlock));
 if( dbg ) bcdtmWrite_message(0,0,0,"Memory Pool Allocated") ;
/*
**   Instantiate
*/
 try
    {
     if( dbg ) bcdtmWrite_message(0,0,0,"Setting Path") ;
     tstring DEMRasterFilePath = tstring(_TEXT("file://")) + imageFileNameP ;
     HFCPtr<HFCURLFile> pDEMRasterFilePathURL((HFCURLFile*)HFCURL::Instanciate(DEMRasterFilePath));
     if( dbg ) bcdtmWrite_message(0,0,0,"Setting Path Completed") ;

     HUTDEMRasterXYZPointsExtractor RasterPointExtractor(DEMRasterFilePath,pPool);
     HUINT64 WidthInPixels;
     HUINT64 HeightInPixels;
     RasterPointExtractor.GetDimensionInPixels(&WidthInPixels, &HeightInPixels);   // RobC - Inconsisently Crashes here
     if( dbg ) bcdtmWrite_message(0,0,0,"WidthInPixels = %8ld HeightInPixels = %8ld",(long)WidthInPixels,(long)HeightInPixels) ;
/*
**   Set Image Size
*/
     *imageSizeP   = (long) ( WidthInPixels * HeightInPixels ) ;
     if( dbg ) bcdtmWrite_message(0,0,0,"imageSize = %8ld",*imageSizeP) ;

//   Get BaseGCS Pointer To Raster Coordinate System

     BaseGCSCP pRasterCoordSys = RasterPointExtractor.GetDEMRasterCoordSys() ;

//   Set Coordinate System Parameters

     if( pRasterCoordSys != NULL )
       {
        gcsNameP  = ( char *) pRasterCoordSys->GetName() ;
        gcsDescP  = ( char *) pRasterCoordSys->GetDescription() ;
        gcsProjP  = ( char *) pRasterCoordSys->GetProjection() ;
        gcsUnitsP = ( char *) pRasterCoordSys->GetUnits() ;
        if( dbg )
          {
           bcdtmWrite_message(0,0,0,"gcsName  = %s",gcsNameP) ;
           bcdtmWrite_message(0,0,0,"gcsDesc  = %s",gcsDescP) ;
           bcdtmWrite_message(0,0,0,"gcsProj  = %s",gcsProjP) ;
           bcdtmWrite_message(0,0,0,"gcsUnits = %s",gcsUnitsP) ;
          }

//      Convert To Wide Character

        bcdtmUtility_convertMbsToWcs(gcsNameP, ( wchar_t ** ) gcsNamePP) ;
        bcdtmUtility_convertMbsToWcs(gcsDescP, ( wchar_t ** ) gcsDescPP) ;
        bcdtmUtility_convertMbsToWcs(gcsProjP, ( wchar_t ** ) gcsProjPP) ;
        bcdtmUtility_convertMbsToWcs(gcsUnitsP,( wchar_t ** ) gcsUnitsPP) ;
       }
     else
       {
        if( dbg )
          {
           bcdtmWrite_message(0,0,0,"gcsName  = %s","") ;
           bcdtmWrite_message(0,0,0,"gcsDesc  = %s","") ;
           bcdtmWrite_message(0,0,0,"gcsProj  = %s","") ;
           bcdtmWrite_message(0,0,0,"gcsUnits = %s","") ;
          }
        bcdtmUtility_convertMbsToWcs("", ( wchar_t ** ) gcsNamePP) ;
        bcdtmUtility_convertMbsToWcs("", ( wchar_t ** ) gcsDescPP) ;
        bcdtmUtility_convertMbsToWcs("", ( wchar_t ** ) gcsProjPP) ;
        bcdtmUtility_convertMbsToWcs("", ( wchar_t ** ) gcsUnitsPP) ;
       }

    }
  catch( HFCException &rE)
    {
     HUSHORT ExceptionID  = rE.GetID();
     tstring ExceptionMsg = rE.GetExceptionMessage();
     if( ExceptionID == 16 ) bcdtmWrite_message(0,0,0,"rasterCordSysP = %p ** ExceptionMsg = %s",rasterCordSysP,ExceptionMsg.c_str()) ;
     else
       {
        bcdtmWrite_message(1,0,0,"Error %4d ** %s ** Opening/Reading Image File %s",ExceptionID,ExceptionMsg.c_str(),imageFileNameP) ;
        goto errexit ;
       }
    }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Image Size And Projection Parameters Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Image Size And Projection Parameters Error") ;
 if( dbg )  bcdtmWrite_message(0,0,0,"") ;
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
int bcdtmImagePP_getImageSizeAndGCS
(
 char *imageFileNameP,                     // ==>  Image File Name
 long *imageSizeP,                         // <==  Image Size In Pixels
 BaseGCS **geoCoordinateSystem             // <==  GCS
)
{
 int ret=DTM_SUCCESS,dbg=0 ;
 const void *rasterCordSysP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting Image Size And Projection Parameters") ;
    bcdtmWrite_message(0,0,0,"imageFileNameP = %s",imageFileNameP) ;
   }
/*
** Set DLL Path
*/
 HRFRasterFileFactory::GetInstance()->SetGeocoordPath(_TEXT("basegeocoord.dll"));
 tstring geoCordPath =   HRFRasterFileFactory::GetInstance()->GetGeocoordPath();
 if( dbg ) bcdtmWrite_message(0,0,0,"geoCordPath = %s",geoCordPath.c_str()) ;
/*
** Initialise Geo Coordinate System
*/
 tstring dataDirectory = _TEXT(".\\GeoCoordinateData");
 if( BaseGeoCordPathSet ) dataDirectory = BaseGeoCoordPath ;
 HCPGCoordUtility::GetInstance()->InitializeGeoCoord(dataDirectory);
 if( dbg ) bcdtmWrite_message(0,0,0,"Geo Coordinate System Initialised") ;
/*
** Allocate Memory Pool To Store Image
*/
 HFCPtr<HPMPool> pPool(new HPMPool(65536, HPMPool::KeepLastBlock));
 if( dbg ) bcdtmWrite_message(0,0,0,"Memory Pool Allocated") ;
/*
**   Instantiate
*/
 try
    {
     if( dbg ) bcdtmWrite_message(0,0,0,"Setting Path") ;
     tstring DEMRasterFilePath = tstring(_TEXT("file://")) + imageFileNameP ;
     HFCPtr<HFCURLFile> pDEMRasterFilePathURL((HFCURLFile*)HFCURL::Instanciate(DEMRasterFilePath));
     if( dbg ) bcdtmWrite_message(0,0,0,"Setting Path Completed") ;

     HUTDEMRasterXYZPointsExtractor RasterPointExtractor(DEMRasterFilePath,pPool);
     HUINT64 WidthInPixels;
     HUINT64 HeightInPixels;
     RasterPointExtractor.GetDimensionInPixels(&WidthInPixels, &HeightInPixels);   // RobC - Inconsisently Crashes here
     if( dbg ) bcdtmWrite_message(0,0,0,"WidthInPixels = %8ld HeightInPixels = %8ld",(long)WidthInPixels,(long)HeightInPixels) ;
/*
**   Set Image Size
*/
     *imageSizeP   = (long) ( WidthInPixels * HeightInPixels ) ;
     if( dbg ) bcdtmWrite_message(0,0,0,"imageSize = %8ld",*imageSizeP) ;

//   Get BaseGCS Pointer To Raster Coordinate System
     *geoCoordinateSystem = (BaseGCS*)RasterPointExtractor.GetDEMRasterCoordSys() ;
     (*geoCoordinateSystem)->AddRef();
    }
  catch( HFCException &rE)
    {
     HUSHORT ExceptionID  = rE.GetID();
     tstring ExceptionMsg = rE.GetExceptionMessage();
     if( ExceptionID == 16 ) bcdtmWrite_message(0,0,0,"rasterCordSysP = %p ** ExceptionMsg = %s",rasterCordSysP,ExceptionMsg.c_str()) ;
     else
       {
        bcdtmWrite_message(1,0,0,"Error %4d ** %s ** Opening/Reading Image File %s",ExceptionID,ExceptionMsg.c_str(),imageFileNameP) ;
        goto errexit ;
       }
    }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Image Size And Projection Parameters Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Image Size And Projection Parameters Error") ;
 if( dbg )  bcdtmWrite_message(0,0,0,"") ;
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
int bcdtmImagePP_releaseGCS
(
 BaseGCS **geoCoordinateSystem             // <==  GCS
)
{
    (*geoCoordinateSystem)->Release();
    *geoCoordinateSystem = NULL;
    return DTM_SUCCESS;
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmImagePP_importImagePointsDtmObject
(
 BC_DTM_OBJ *dtmP,
 double     *imagePointsP,
 long       numPoints,
 long       numCols,
 long       numRows,
 double     *nullValueP
)
{
 int ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long row,col,offset=0,numPts,gridStore=1 ;
 long noDataValueSet=0,xyLimitsSet=0,zLimitsSet=0,geoCordSysSet=0 ;
 double noDataValue=-987654.321,xMin=0.0,yMin=0.0,zMin=0.0,xMax=0.0,yMax=0.0,zMax=0.0 ;
 DTM_TIN_POINT *pointP ;
 P3D *p3dP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Reading Image To DTM") ;
   }
/*
** Initialise
*/
 *nullValueP = 0.0 ;


/*
**   Instanciate
*/
 try
   {
//  Get No Data Value
/*
    const HDOUBLE *noDataValueP = RasterPointExtractor.GetNoDataValue();
    if( noDataValueP != 0 )
      {
       noDataValueSet = 1 ;
       noDataValue = ( double ) *noDataValueP ;
      }
    if( dbg )
      {
       if( noDataValueSet == 0 ) bcdtmWrite_message(0,0,0,"No Data Value Not Set For Image") ;
       else                      bcdtmWrite_message(0,0,0,"Image No Data Value %12.5lf",noDataValue) ;
      }
*/

/*
**  Set Memory Allocation Parameters For DTM
*/
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,numPoints,10000) ;
/*
**  Initialise Col Row Sequence For Storing Points In DTM For Gridded Triangulation Engine
*/
    col = 1 ;
    row = 0 ;
    numPts = 0 ;
/*
**  Scan Image And Store Coordinates In DTM
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Storing Points In DTM Object") ;
      {
       if( imagePointsP == NULL )
         {
          bcdtmWrite_message(0,0,0,"Null Points Pointer") ;
          goto errexit ;
         }
/*
**     Allocate DTM Memory If First Point
*/
       if( dtmP->numPoints == 0 )
         {
          if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP)) goto errexit ;
          if( gridStore ) dtmP->numPoints = numPoints ;
         }
/*
**     Set Null Values For Points Out Side Limits
*/
/*
       if( ! noDataValueSet && ( xyLimitsSet || zLimitsSet ))
         {
          for( p3dP =  (P3D *)pXYZPoints ; p3dP < (P3D *)pXYZPoints + NbPoints ; ++p3dP )
            {
             if( xyLimitsSet )
               {
                if( p3dP->X < xMin || p3dP->X > xMax ) p3dP->Z = noDataValue ;
                if( p3dP->Y < yMin || p3dP->Y > yMax ) p3dP->Z = noDataValue ;
               }
             if( zLimitsSet )
               {
                if( p3dP->Z < zMin || p3dP->Z > zMax ) p3dP->Z = noDataValue ;
               }
            }
         }
*/
/*
**     Store Image Points In DTM In Sequence Required By The DEM Triangulation Engine
*/
       if( gridStore )
         {
          for( p3dP =  (P3D *)imagePointsP ; p3dP < (P3D *)imagePointsP + numPoints ; ++p3dP )
            {
             ++numPts ;
             offset =  ( numRows - row )  + ( col - 1 ) * numRows - 1  ;
             if( offset < 0 || offset >= numPoints )
               {
                bcdtmWrite_message(1,0,0,"Pixel Offset Range error = %8ld ** row = %8ld col = %8ld numPixels = %8ld",offset,row,col,numPts) ;
                goto errexit ;
               }
             pointP = pointAddrP(dtmP,offset) ;
             pointP->X = p3dP->X ;
             pointP->Y = p3dP->Y ;
             pointP->Z = p3dP->Z ;
             ++col ;
             if( col > numCols )
               {
                ++row ;
                col = 1 ;
               }
            }
         }


/*
**     Store Points Sequentially In DTM
*/
       else
         {
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,(P3D *)imagePointsP,numPoints)) goto errexit ;
         }
/*
**     Get Next Set Of Points
*/
      }
    }
  catch(...)
    {
     bcdtmWrite_message(1,0,0,"Unknown Error** Opening/Reading Image Files") ;
     goto errexit ;
    }

/*
** Set Bounding Cube
*/
 if( gridStore ) if( bcdtmMath_setBoundingCubeDtmObject(dtmP)) goto errexit ;
/*
** Set Return Values
*/
 *nullValueP = noDataValue ;

/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Image To DTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Image To DTM Error") ;
 if( dbg )  bcdtmWrite_message(0,0,0,"") ;
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
int bcdtmImagePP_importImageDtmObject
(
 BC_DTM_OBJ *dtmP,
 char       *imageFileNameP,
 long       *numRowsP,
 long       *numColsP,
 double     *nullValueP ,
 double     scaleFactor,
 char       *projectionKeyP
)
{
 int ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long row,col,offset=0,heightInPixels,widthInPixels,numPixels,numPts,gridStore=1 ;
 long noDataValueSet=0,xyLimitsSet=0,zLimitsSet=0,geoCordSysSet=0 ;
 double noDataValue=-987654.321,xMin=0.0,yMin=0.0,zMin=0.0,xMax=0.0,yMax=0.0,zMax=0.0 ;
 DTM_TIN_POINT *pointP ;
 P3D *p3dP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Reading Image To DTM") ;
    bcdtmWrite_message(0,0,0,"imageFileNameP = %s",imageFileNameP) ;
    bcdtmWrite_message(0,0,0,"scaleFactor    = %8.5lf",scaleFactor) ;
    bcdtmWrite_message(0,0,0,"projectionKeyP = %8s",projectionKeyP) ;
   }
/*
** Initialise
*/
 *numRowsP = 0 ;
 *numColsP = 0 ;
 *nullValueP = 0.0 ;
/*
** Allocate Memory Pool To Store Image
*/
 HFCPtr<HPMPool> pPool(new HPMPool(65536, HPMPool::KeepLastBlock));
/*
** Set DLL Path
*/
 HRFRasterFileFactory::GetInstance()->SetGeocoordPath(_TEXT("basegeocoord.dll"));
 tstring geoCordPath =   HRFRasterFileFactory::GetInstance()->GetGeocoordPath();
 if( dbg ) bcdtmWrite_message(0,0,0,"geoCordPath = %s",geoCordPath.c_str()) ;
/*
** Initialise Geo Coordinate System
*/
 tstring dataDirectory = _TEXT(".\\GeoCoordinateData");
 if( BaseGeoCordPathSet ) dataDirectory = BaseGeoCoordPath ;
 HCPGCoordUtility::GetInstance()->InitializeGeoCoord(dataDirectory);
 if( dbg ) bcdtmWrite_message(0,0,0,"Geo Coordinate System Initialised ** dataDirectory = %s",dataDirectory.c_str()) ;
/*
** Set Image File Name
*/
 tstring DEMRasterFilePath = tstring(_TEXT("file://")) + imageFileNameP ;
/*
**   Instanciate
*/
 try
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Instantiating") ;
    HFCPtr<HFCURLFile> pDEMRasterFilePathURL((HFCURLFile*)HFCURL::Instanciate(DEMRasterFilePath));
    HUTDEMRasterXYZPointsExtractor RasterPointExtractor(DEMRasterFilePath,pPool);

    if( dbg ) bcdtmWrite_message(0,0,0,"Instantiated") ;

//  Get  Pointer To Raster Coordinate System Instance

    BaseGCS* pRasterCoordSys = (BaseGCS*)RasterPointExtractor.GetDEMRasterCoordSys() ;
    if( pRasterCoordSys != NULL ) geoCordSysSet = 1 ;
    if( dbg )
      {
       if( geoCordSysSet == 0 ) bcdtmWrite_message(0,0,0,"Image Does Not Have An Associated Projection") ;
       else                     bcdtmWrite_message(0,0,0,"Image Projection = %s",pRasterCoordSys->GetProjection()) ;
      }

//  Set Re Projection Coordinate System

    tstring destCoordSysKeyName = tstring(_TEXT(""))  ;
    if( geoCordSysSet ) destCoordSysKeyName = destCoordSysKeyName + projectionKeyP  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"destCoordSysKeyName = %s",destCoordSysKeyName.c_str()) ;

//  Get No Data Value

    const HDOUBLE *noDataValueP = RasterPointExtractor.GetNoDataValue();
    if( noDataValueP != 0 )
      {
       noDataValueSet = 1 ;
       noDataValue = ( double ) *noDataValueP ;
      }
    if( dbg )
      {
       if( noDataValueSet == 0 ) bcdtmWrite_message(0,0,0,"No Data Value Not Set For Image") ;
       else                      bcdtmWrite_message(0,0,0,"Image No Data Value %12.5lf",noDataValue) ;
      }

//  Get XY Limit Values

    HDOUBLE XMin, XMax, YMin, YMax, ZMin, ZMax;
    RasterPointExtractor.Get2DCoordMinMaxValues(&XMin,&XMax,&YMin, &YMax);
    xMin = ( double) XMin ;
    yMin = ( double) YMin ;
    xMax = ( double) XMax ;
    yMax = ( double) YMax ;
    xyLimitsSet = 1 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"XY Limits ** xMin = %12.4lf xMax = %12.4lf ** yMin = %12.4lf yMax = %12.4lf",xMin,xMax,yMin,yMax) ;

//  Get Z Limit Values

    HBOOL isZLimitSet = RasterPointExtractor.GetZCoordMinMaxValues(&ZMin, &ZMax);
    if( isZLimitSet == TRUE )
      {
       zLimitsSet = 1 ;
       zMin = ( double) ZMin ;
       zMax = ( double) ZMax ;
      }
    if( dbg )
      {
       if( zLimitsSet == 0 ) bcdtmWrite_message(0,0,0,"Z  Limits Not Set") ;
       else                  bcdtmWrite_message(0,0,0,"Z  Limits ** zMin = %12.4lf zMax = %12.4lf",zMin,zMax) ;
      }
/*
**  Instantiate Points Iterator
*/
    HULONG         NbPoints;
    const HDOUBLE* pXYZPoints;
    HAutoPtr<HUTDEMRasterXYZPointsIterator> PointsIterator(RasterPointExtractor.CreateXYZPointsIterator(destCoordSysKeyName ,scaleFactor)); // RobC - Inconsisently Crashes here and never reprojects
    if( dbg )
      {
       if( PointsIterator->IsDestCoordSysCreationFailed() == true )  bcdtmWrite_message(0,0,0,"Failed To Create Destination Projection %s",destCoordSysKeyName.c_str()) ;
       else                                                          bcdtmWrite_message(0,0,0,"Destination Projection %s Created",destCoordSysKeyName.c_str()) ;
      }
/*
**  Get Number Of Filtered Rows And Columns
*/
    HUINT64 WidthInPixels;
    HUINT64 HeightInPixels;
    PointsIterator->GetFilteredDimensionInPixels(&WidthInPixels, &HeightInPixels);
    heightInPixels = (long) HeightInPixels ;
    widthInPixels  = (long) WidthInPixels  ;
    numPixels      = heightInPixels * widthInPixels ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Filtered ** Image Width = %8ld Image Height = %8ld numPixels = %8ld",widthInPixels,heightInPixels,numPixels) ;
/*
**  Set Memory Allocation Parameters For DTM
*/
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,widthInPixels*heightInPixels,10000) ;
/*
**  Initialise Col Row Sequence For Storing Points In DTM For Gridded Triangulation Engine
*/
    col = 1 ;
    row = 0 ;
    numPts = 0 ;
/*
**  Scan Image And Store Coordinates In DTM
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Storing Points In DTM Object") ;
    while ( (pXYZPoints = PointsIterator->GetXYZPoints(&NbPoints)) != 0 )
      {
       if( pXYZPoints == NULL )
         {
          bcdtmWrite_message(0,0,0,"Null Points Pointer") ;
          goto errexit ;
         }
/*
**     Allocate DTM Memory If First Point
*/
       if( dtmP->numPoints == 0 )
         {
          if( bcdtmObject_allocatePointsMemoryDtmObject(dtmP)) goto errexit ;
          if( gridStore ) dtmP->numPoints = widthInPixels*heightInPixels ;
         }
/*
**     Set Null Values For Points Out Side Limits
*/
/*
       if( ! noDataValueSet && ( xyLimitsSet || zLimitsSet ))
         {
          for( p3dP =  (P3D *)pXYZPoints ; p3dP < (P3D *)pXYZPoints + NbPoints ; ++p3dP )
            {
             if( xyLimitsSet )
               {
                if( p3dP->X < xMin || p3dP->X > xMax ) p3dP->Z = noDataValue ;
                if( p3dP->Y < yMin || p3dP->Y > yMax ) p3dP->Z = noDataValue ;
               }
             if( zLimitsSet )
               {
                if( p3dP->Z < zMin || p3dP->Z > zMax ) p3dP->Z = noDataValue ;
               }
            }
         }
*/
/*
**     Store Image Points In DTM In Sequence Required By The DEM Triangulation Engine
*/
       if( gridStore )
         {
          for( p3dP =  (P3D *)pXYZPoints ; p3dP < (P3D *)pXYZPoints + NbPoints ; ++p3dP )
            {
             ++numPts ;
             offset =  ( heightInPixels - row )  + ( col - 1 ) * heightInPixels - 1  ;
             if( offset < 0 || offset >= numPixels )
               {
                bcdtmWrite_message(1,0,0,"Pixel Offset Range error = %8ld ** row = %8ld col = %8ld numPixels = %8ld",offset,row,col,numPts) ;
                goto errexit ;
               }
             pointP = pointAddrP(dtmP,offset) ;
             pointP->X = p3dP->X ;
             pointP->Y = p3dP->Y ;
             pointP->Z = p3dP->Z ;
             ++col ;
             if( col > widthInPixels )
               {
                ++row ;
                col = 1 ;
               }
            }
         }
/*
**     Store Points Sequentially In DTM
*/
       else
         {
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,(P3D *)pXYZPoints,NbPoints)) goto errexit ;
         }
/*
**     Get Next Set Of Points
*/
       PointsIterator->Next();
      }
   }
/*
** Catch File Open Exception
*/
  catch( HFCException &rE)
    {
     HUSHORT ExceptionID = rE.GetID();
     tstring ExceptionMsg = rE.GetExceptionMessage();
     bcdtmWrite_message(1,0,0,"Error %4d ** %s ** Opening/Reading Image File %s",ExceptionID,ExceptionMsg.c_str(),imageFileNameP) ;
     goto errexit ;
    }
  catch(...)
    {
     bcdtmWrite_message(1,0,0,"Unknown Error** Opening/Reading Image Files") ;
     goto errexit ;
    }

/*
** Set Bounding Cube
*/
 if( gridStore ) if( bcdtmMath_setBoundingCubeDtmObject(dtmP)) goto errexit ;
/*
** Set Return Values
*/
 *nullValueP = noDataValue ;
 *numRowsP   = heightInPixels ;
 *numColsP   = widthInPixels ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Image To DTM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Reading Image To DTM Error") ;
 if( dbg )  bcdtmWrite_message(0,0,0,"") ;
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
int bcdtmImagePP_importImageLatticeObject(DTM_LAT_OBJ *latticeP,char *imageFileNameP )
{
 int ret=DTM_SUCCESS,dbg=0 ;
 long row,col,offset=0,heightInPixels,widthInPixels,numPixels,process ;
 float nullValue = -98765.4321 ;
 float *latP ;
 P3D *p3dP ;
/*
** Write Entry Message
*/
 bcdtmWrite_message(0,0,0,"Importing Image To Lattice") ;
/*
** Allocate Memory Pool To Store Image
*/
 HFCPtr<HPMPool> pPool(new HPMPool(65536, HPMPool::KeepLastBlock));
 tstring DEMRasterFilePath = tstring(_TEXT("file://")) + imageFileNameP ;
/*
**   Instanciate
*/
 HFCPtr<HFCURLFile> pDEMRasterFilePathURL((HFCURLFile*)HFCURL::Instanciate(DEMRasterFilePath));
 try
   {
    HUTDEMRasterXYZPointsExtractor RasterPointExtractor(DEMRasterFilePath,pPool);
    HUINT64 WidthInPixels;
    HUINT64 HeightInPixels;
    RasterPointExtractor.GetDimensionInPixels(&WidthInPixels, &HeightInPixels);
/*
**  Set DTM Point Memory Allocation Parameters
*/
    heightInPixels = (long) HeightInPixels ;
    widthInPixels  = (long) WidthInPixels ;
    numPixels      = heightInPixels * widthInPixels ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Image Width = %8ld Image Height = %8ld numPixels = %8ld",widthInPixels,heightInPixels,numPixels) ;
/*
**  Scan Image And Store Coordinates In DTM
*/
    HULONG         NbPoints;
    const HDOUBLE* pXYZPoints;
    HAutoPtr<HUTDEMRasterXYZPointsIterator> PointsIterator(RasterPointExtractor.CreateXYZPointsIterator());
    if( dbg ) bcdtmWrite_message(0,0,0,"Storing Points In DTM Object") ;
/*
**  Initialise Col Row Sequence For Storing Points In Lattice
*/
    col = 1 ;
    row = 1 ;
    while ((pXYZPoints = PointsIterator->GetXYZPoints(&NbPoints)) != 0)
      {
/*
**     Allocate Lattice Memory If First Point
*/
       if( latticeP->LAT == NULL )
         {
          latticeP->LAT = ( float * ) malloc(numPixels*sizeof(float)) ;
          if( latticeP->LAT == NULL )
            {
             bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
          latticeP->NULLVAL = nullValue ;
          for( latP = latticeP->LAT ; latP < latticeP->LAT + numPixels ; ++latP ) *latP = latticeP->NULLVAL ;
          p3dP = (P3D *)pXYZPoints ;
          latticeP->LXMIN = latticeP->LXMAX = p3dP->X ;
          latticeP->LYMIN = latticeP->LYMAX = p3dP->Y ;
          latticeP->LZMIN = latticeP->LZMAX = p3dP->Z ;
         }
/*
**     Store Image Points In Sequence Required By The Lattice
*/
       for( p3dP =  (P3D *)pXYZPoints ; p3dP < (P3D *)pXYZPoints + NbPoints ; ++p3dP )
         {
          if( p3dP->X < latticeP->LXMIN ) latticeP->LXMIN = p3dP->X ;
          if( p3dP->X > latticeP->LXMAX ) latticeP->LXMAX = p3dP->X ;
          if( p3dP->Y < latticeP->LYMIN ) latticeP->LYMIN = p3dP->Y ;
          if( p3dP->Y > latticeP->LYMAX ) latticeP->LYMAX = p3dP->Y ;
          if( p3dP->Z < latticeP->LZMIN ) latticeP->LZMIN = p3dP->Z ;
          if( p3dP->Z > latticeP->LZMAX ) latticeP->LZMAX = p3dP->Z ;
          offset =  ( heightInPixels - row ) * widthInPixels  + ( col - 1 ) ;
          if( offset < 0 || offset >= numPixels )
            {
             bcdtmWrite_message(0,0,0,"offset range error = %8ld",offset) ;
             goto errexit ;
            }
          *(latticeP->LAT+offset) = ( float) p3dP->Z ;
          ++col ;
          if( col > widthInPixels )
            {
             ++row ;
             col = 1 ;
            }
         }
       PointsIterator->Next();
      }
   }
/*
** Catch File Open Exception
*/
  catch( HFCException &rE)
    {
     HUSHORT ExceptionID = rE.GetID();
     tstring ExceptionMsg = rE.GetExceptionMessage();
     bcdtmWrite_message(1,0,0,"Error Opening/Reading Image File %s",imageFileNameP) ;
     goto errexit ;
    }
/*
** Set Laticce Bounding Cube
*/
 latticeP->LZMIN = latticeP->LZMAX = *(latticeP->LAT) ;
 latticeP->NOACTPTS = 0 ;
 process = 1 ;
 for( latP = latticeP->LAT ; latP < latticeP->LAT + numPixels && process  ; ++latP )
   {
   if( *latP != latticeP->NULLVAL )
      {
       latticeP->LZMIN = latticeP->LZMAX = *latP  ;
       process = 0 ;
      }
   }
 for( latP = latticeP->LAT ; latP < latticeP->LAT + numPixels ; ++latP )
   {
    if( *latP != latticeP->NULLVAL )
      {
       ++latticeP->NOACTPTS ;
       if( *latP < latticeP->LZMIN ) latticeP->LZMIN = *latP ;
       if( *latP > latticeP->LZMAX ) latticeP->LZMAX = *latP ;
      }
   }
 latticeP->LXDIF = latticeP->LXMAX - latticeP->LXMIN ;
 latticeP->LYDIF = latticeP->LYMAX - latticeP->LYMIN ;
 latticeP->LZDIF = latticeP->LZMAX - latticeP->LZMIN ;
/*
** Set Lattice Parameters
*/
 latticeP->DX    = latticeP->LXDIF / (double)( widthInPixels - 1 ) ;
 latticeP->DY    = latticeP->LYDIF / (double)( heightInPixels - 1 ) ;
 latticeP->NXL   = heightInPixels ;
 latticeP->NYL   = widthInPixels ;
 latticeP->INTMODE = 1 ;
 latticeP->NOLATPTS = numPixels ;
/*
** Clean Up
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing Image To Lattice Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing Image To Lattice Error") ;
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


//SECTION FOR THE SAMPLE - BEGIN
//This code section must NOT be added to bcLIB DTM or the library
//that needs to access points from a DEM raster.
void InitializeGeoCoord();

void WriteXYZCoordinatesToFile(const HDOUBLE*     pi_pXYZPoints,
                               HULONG             pi_NbPoints,
                               HFCLocalBinStream* po_pPointsASCIIFile);

//SECTION FOR THE SAMPLE - END

//-----------------------------------------------------------------------------
// Main,
// It ensures that the source path and the destination path are valid before
// calling the ScanDir function.
//-----------------------------------------------------------------------------
void readImage(char *imageFileP,char *proCodeP,double scaleFactor)
{
    //SECTION TO ADD - BEGIN
    //This code section MUST be added to bcLIB DTM or the library
    //that needs to access points from a DEM raster.
    InitializeGeoCoord();

    tstring DatasetFolder;
    HDOUBLE ScaleFactor;
    tstring DestCoordSysKeyName(_TEXT(""));

    //Ensure that a dialog is popup when an assert failed even if the code is using the
    //release version of the run time library.
    _set_error_mode(_OUT_TO_MSGBOX);

     bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Reading Image %s",imageFileP) ;

     DatasetFolder = tstring(imageFileP) ;
     DestCoordSysKeyName = tstring(proCodeP) ;
     ScaleFactor =  ( HDOUBLE ) scaleFactor ;
     HASSERT(ScaleFactor <= 1.0);

/*
    if (pi_Argc > 1)
    {
        DatasetFolder = tstring(pi_ppArgv[1]);
    }

    if (pi_Argc > 2)
    {
        DestCoordSysKeyName = tstring(pi_ppArgv[2]);
    }

    if (pi_Argc > 3)
    {
        ScaleFactor = _tstof(pi_ppArgv[3]);
        HASSERT(ScaleFactor <= 1.0);
    }
*/
//    ATPFileFinder                            RasterDEMFileFinder;
    tstring                                  SourceFilePaths;
    tstring                                  SourceFile;
    HAutoPtr<HUTDEMRasterXYZPointsExtractor> pDEMRasterPointsExtractor;

    tstring LogFileName = tstring(_TEXT("D:\\BSW\\out\\bcAssemblies\\Desktop\\")) +
                                  _TEXT("image.log.");

    HFCLocalBinStream LogFile(LogFileName, HFC_READ_WRITE_CREATE);

    //Get find all the files within the source folder
    WIN32_FIND_DATA FindFileData;
    HANDLE          hFindFile = 0;
    HTCHAR          LogOutputBuffer[5000];
    HULONG          SizeToWrite;

    SizeToWrite = _stprintf(LogOutputBuffer,
                            _TEXT("\r\n\r\n------- New Test - Folder : %s -------\r\n\r\n"),
                            DatasetFolder.c_str());

    LogFile.Write(LogOutputBuffer, SizeToWrite);

    FindFileData.dwReserved0 = 0;

/*
    RasterDEMFileFinder.FindFiles(DatasetFolder,
                                  SourceFilePaths,
                                  TRUE,
                                  hFindFile,
                                  FindFileData);
*/


    HFCPtr<HFCURL> pURL;

//    while (RasterDEMFileFinder.ParseFilePaths(SourceFilePaths, SourceFile) == TRUE)


    int process = 1 ;
    while ( process )
    {
        process = 0 ;
//        SourceFile = tstring(_TEXT("file://")) + SourceFile;
        SourceFile = tstring(_TEXT("file://")) + imageFileP;

        bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Reading SourceFile = %s",SourceFile.c_str()) ;

        const HRFRasterFileCreator* pRasterFileCreator = 0;

        pURL = HFCURL::Instanciate(SourceFile);

        try
        {
            pRasterFileCreator = HRFRasterFileFactory::GetInstance()->FindCreator(pURL,HFC_READ_ONLY);
        }
        catch (HFCException&)
        {
         bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Failed To Instantiate pRasterFileCreator") ;
        }

         bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** pRasterFileCreator Instantiated") ;

         if( (dynamic_cast<const HRFDtedCreator*>(pRasterFileCreator)        == 0))bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Could Not Instantiate HRFDtedCreator ") ;
         else                                                                      bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Instantiated HRFDtedCreator ") ;
         if( (dynamic_cast<const HRFErdasImgCreator*>(pRasterFileCreator)    == 0))bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Could Not Instantiate HRFErdasImgCreator ") ;
         else                                                                      bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Instantiated HRFErdasImgCreator ") ;
         if( (dynamic_cast<const HRFSpotDimapCreator*>(pRasterFileCreator)   == 0))bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Could Not Instantiate HRFSpotDimapCreator ") ;
         else                                                                      bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Instantiated HRFSpotDimapCreator ")  ;
         if( (dynamic_cast<const HRFUSgsDEMCreator*>(pRasterFileCreator)     == 0))bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Could Not Instantiate HRFUSgsDEMCreator ") ;
         else                                                                      bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Instantiated HRFUSgsDEMCreator ") ;
         if( (dynamic_cast<const HRFUSgsSDTSDEMCreator*>(pRasterFileCreator) == 0))bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Could Not Instantiate HRFUSgsSDTSDEMCreator* ") ;
         else                                                                      bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Instantiated HRFUSgsSDTSDEMCreator ") ;

 /*
        if ((pRasterFileCreator != 0) &&
            ((dynamic_cast<const HRFDtedCreator*>(pRasterFileCreator) != 0)      ||
             (dynamic_cast<const HRFErdasImgCreator*>(pRasterFileCreator) != 0)  ||
             (dynamic_cast<const HRFSpotDimapCreator*>(pRasterFileCreator) != 0) ||
             (dynamic_cast<const HRFUSgsDEMCreator*>(pRasterFileCreator) != 0)   ||
             (dynamic_cast<const HRFUSgsSDTSDEMCreator*>(pRasterFileCreator) != 0)))
*/
        if ( pRasterFileCreator != 0)
        {
            clock_t         StartTime;
            clock_t         Duration = 0;
            HFCPtr<HPMPool> pPool(new HPMPool(65536, HPMPool::KeepLastBlock));

            StartTime = clock();

            HAutoPtr<HUTDEMRasterXYZPointsExtractor> pRasterPointExtractor;

            try
            {
                pRasterPointExtractor = new HUTDEMRasterXYZPointsExtractor(SourceFile,
                                                                           pPool);
            }
            catch (HFCException&)
            {
                //If an error occurs, go to the next raster.
                continue;
            }


            SizeToWrite = _stprintf(LogOutputBuffer,
                                    _TEXT("File to test : %s\r\n"),
                                    SourceFile.c_str());

            LogFile.Write(LogOutputBuffer, SizeToWrite);

            bcdtmWrite_message(0,0,0,"** GEO_SPATIAL **  SourceFile = %s",SourceFile.c_str()) ;

            Duration += clock() - StartTime;

            const HDOUBLE* pNoDataValue;
            HUINT64         WidthInPixels;
            HUINT64         HeightInPixels;
            HUINT64         NumberOfPoints;
            HBOOL           HasGeoCoding;
            HBOOL           IsZLimitValuesAvailable;
            HDOUBLE         XMin, XMax, YMin, YMax, ZMin, ZMax;

            //Width and height
            pRasterPointExtractor->GetDimensionInPixels(&WidthInPixels, &HeightInPixels);

            bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** WidthInPixels = %8ld ** HeightInPixels = %8ld",( long )WidthInPixels,(long) HeightInPixels) ;

            SizeToWrite = _stprintf(LogOutputBuffer,
                                    _TEXT("Dimension in pixels (width : %I64i height : %I64i)\r\n"),
                                    WidthInPixels, HeightInPixels);

            LogFile.Write(LogOutputBuffer, SizeToWrite);



            //Number of points
            pRasterPointExtractor->GetNumberOfPoints(&NumberOfPoints);

             SizeToWrite = _stprintf(LogOutputBuffer,
                                    _TEXT("Number of points : %I64i\r\n"),
                                    NumberOfPoints);

            bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Image Size = %8ld Pixels",(long)NumberOfPoints) ;

            LogFile.Write(LogOutputBuffer, SizeToWrite);

            //No data value
            pNoDataValue = pRasterPointExtractor->GetNoDataValue();

            if (pNoDataValue != 0)
            {
                SizeToWrite = _stprintf(LogOutputBuffer,
                                        _TEXT("No data value : %.8f\r\n"),
                                        (HDOUBLE)*pNoDataValue);

              bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** No Data Value = %.8lf",(HDOUBLE)*pNoDataValue) ;


            }
            else
            {
                SizeToWrite = _stprintf(LogOutputBuffer,
                                        _TEXT("No data value not available\r\n"));

              bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** No Data Value Avaialble") ;
           }

            LogFile.Write(LogOutputBuffer, SizeToWrite);

            //Has geocoding information
            HasGeoCoding = (pRasterPointExtractor->GetDEMRasterCoordSys() != 0);

            SizeToWrite = _stprintf(LogOutputBuffer,
                                    _TEXT("Has geographic coordinate system : %i\r\n"),
                                    (HUBYTE)HasGeoCoding);

              bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Has geographic coordinate system : %i",(HUBYTE)HasGeoCoding) ;

            LogFile.Write(LogOutputBuffer, SizeToWrite);

            //2D limit values
            pRasterPointExtractor->Get2DCoordMinMaxValues(&XMin, &XMax,
                                                        &YMin, &YMax);

            SizeToWrite = _stprintf(LogOutputBuffer,
                                    (tstring(_TEXT("2D Limit Values \r\n")) +
                                     tstring(_TEXT("Minimum X : %.8f, Maximum X : %.8f\r\n")) +
                                     tstring(_TEXT("Minimum Y : %.8f, Maximum Y : %.8f\r\n"))).c_str(),
                                     XMin, XMax, YMin, YMax);

            LogFile.Write(LogOutputBuffer, SizeToWrite);

            bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** XY Limits ** Xmin = %12.5lf Xmax = %12.5lf ** Ymin = %12.5lf Ymax = %12.5lf ",(double)XMin,(double)XMax,(double)YMin,(double)YMax) ;


            //Z limit values
            IsZLimitValuesAvailable = pRasterPointExtractor->GetZCoordMinMaxValues(&ZMin, &ZMax);

            if (IsZLimitValuesAvailable == TRUE)
            {
                SizeToWrite = _stprintf(LogOutputBuffer,
                                        (tstring(_TEXT("Z Limit Values \r\n")) +
                                         tstring(_TEXT("Minimum Z : %.8f, Maximum Z : %.8f\r\n"))).c_str(),
                                        ZMin, ZMax);

                LogFile.Write(LogOutputBuffer, SizeToWrite);
             bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Z Limits ** Zmin = %12.5lf Zmax = %12.5lf",(double)ZMin,(double)ZMax) ;
           }
            else
            {
                SizeToWrite = _stprintf(LogOutputBuffer,
                                        _TEXT("Z Limit Values Not Available\r\n"));

                LogFile.Write(LogOutputBuffer, SizeToWrite);
             bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Z Limit Values Not Available") ;
           }

            HULONG         NbPoints;
            const HDOUBLE* pXYZPoints;
            HULONG         TotalNbPointsReturned = 0;
            HDOUBLE        XMinOfPointsReturned  = HDOUBLE_MAX;
            HDOUBLE        XMaxOfPointsReturned  = HDOUBLE_MIN;
            HDOUBLE        YMinOfPointsReturned  = HDOUBLE_MAX;
            HDOUBLE        YMaxOfPointsReturned  = HDOUBLE_MIN;
            HDOUBLE        ZMinOfPointsReturned  = HDOUBLE_MAX;
            HDOUBLE        ZMaxOfPointsReturned  = HDOUBLE_MIN;
            HUINT64        NumberOfFilteredPoints;
            HUINT64        FilteredWidthInPixels;
            HUINT64        FilteredHeightInPixels;

            StartTime = clock();

            HAutoPtr<HUTDEMRasterXYZPointsIterator> pPointsIterator(pRasterPointExtractor->
                                                                     CreateXYZPointsIterator(DestCoordSysKeyName,
                                                                                              ScaleFactor));

            pPointsIterator->GetNumberOfFilteredPoints(&NumberOfFilteredPoints);

            SizeToWrite = _stprintf(LogOutputBuffer,
                                    _TEXT("Number of filtered points : %I64i\r\n"),
                                    NumberOfFilteredPoints);

            LogFile.Write(LogOutputBuffer, SizeToWrite);

            bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Number of filtered points = %8ld ",(long)NumberOfFilteredPoints) ;


            HBOOL IsDestCoordSysCreationFailed;

            IsDestCoordSysCreationFailed = pPointsIterator->IsDestCoordSysCreationFailed();

            if (IsDestCoordSysCreationFailed == TRUE)
            {
                SizeToWrite = _stprintf(LogOutputBuffer,
//                                        (_TEXT("The destination coordinate system could not be created.\r\n")));
                                          (tstring(_TEXT( "The destination coordinate system %s could not be created.\r\n"))).c_str(),DestCoordSysKeyName.c_str());
             bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** The destination coordinate system %s could not be created ",DestCoordSysKeyName.c_str()) ;
           }
            else
            {
                SizeToWrite = _stprintf(LogOutputBuffer,
//                                       (_TEXT( "The destination coordinate system was successfully created.\r\n")));
                                       (tstring(_TEXT( "The destination coordinate system %s was successfully created.\r\n"))).c_str(),DestCoordSysKeyName.c_str());
            bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** The destination coordinate system %s was successfully created",DestCoordSysKeyName.c_str()) ;
            }

            LogFile.Write(LogOutputBuffer, SizeToWrite);

            pPointsIterator->GetFilteredDimensionInPixels(&FilteredWidthInPixels,
                                                          &FilteredHeightInPixels) ;

            SizeToWrite = _stprintf(LogOutputBuffer,
                                    (tstring(_TEXT("Filtered width : %I64i \r\n")) +
                                     tstring(_TEXT("Filtered height : %I64i \r\n")) +
                                     tstring(_TEXT("Filtered width X filtered height : %I64i \r\n")))
                                     .c_str(),
                                     FilteredWidthInPixels,
                                     FilteredHeightInPixels,
                                     FilteredWidthInPixels * FilteredHeightInPixels);

            LogFile.Write(LogOutputBuffer, SizeToWrite);

           bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Filtered Width =  %8ld Filtered Height = %8ld ** Filtered Pixels = %8ld  ",(long)FilteredWidthInPixels,(long)FilteredHeightInPixels,(long)(FilteredWidthInPixels * FilteredHeightInPixels)) ;

           bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Reading Image Pixels") ;

           while ((pXYZPoints = pPointsIterator->GetXYZPoints(&NbPoints)) != 0)
            {
                /*
                WriteXYZCoordinatesToFile(pXYZPoints,
                                          NbPoints,
                                          &PointsASCIIFile);
                */

                Duration += clock() - StartTime;

                for (HULONG CoordInd = 0; CoordInd < NbPoints * 3; CoordInd += 3)
                {
                    XMinOfPointsReturned = min(pXYZPoints[CoordInd],     XMinOfPointsReturned);
                    YMinOfPointsReturned = min(pXYZPoints[CoordInd + 1], YMinOfPointsReturned);
                    ZMinOfPointsReturned = min(pXYZPoints[CoordInd + 2], ZMinOfPointsReturned);
                    XMaxOfPointsReturned = max(pXYZPoints[CoordInd],     XMaxOfPointsReturned);
                    YMaxOfPointsReturned = max(pXYZPoints[CoordInd + 1], YMaxOfPointsReturned);
                    ZMaxOfPointsReturned = max(pXYZPoints[CoordInd + 2], ZMaxOfPointsReturned);
                }

                TotalNbPointsReturned += NbPoints;

                StartTime = clock();

                pPointsIterator->Next();
            }

           bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** Reading Image Pixels Completed : TotalNbPointsReturned = %10ld",(long)TotalNbPointsReturned) ;

            Duration += clock() - StartTime;

            HDOUBLE DurationInSeconds = (HDOUBLE)Duration / CLOCKS_PER_SEC;

            SizeToWrite = _stprintf(LogOutputBuffer,
                                    (tstring(_TEXT("Number of returned points : %i\r\n")) +
                                     tstring(_TEXT("Duration : %.8f seconds\r\n")) +
                                     tstring(_TEXT("Time per points : %.8f seconds\r\n"))).c_str(),
                                    TotalNbPointsReturned, DurationInSeconds, DurationInSeconds / TotalNbPointsReturned);

            LogFile.Write(LogOutputBuffer, SizeToWrite);

           bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** %s ",LogOutputBuffer) ;


            SizeToWrite = _stprintf(LogOutputBuffer,
                                    (tstring(_TEXT("Limit values of returned points\r\n")) +
                                     tstring(_TEXT("Minimum X : %.8f, Maximum X : %.8f\r\n")) +
                                     tstring(_TEXT("Minimum Y : %.8f, Maximum Y : %.8f\r\n")) +
                                     tstring(_TEXT("Minimum Z : %.8f, Maximum Z : %.8f\r\n\r\n"))).c_str(),
                                    XMinOfPointsReturned, XMaxOfPointsReturned,
                                    YMinOfPointsReturned, YMaxOfPointsReturned,
                                    ZMinOfPointsReturned, ZMaxOfPointsReturned);

            LogFile.Write(LogOutputBuffer, SizeToWrite);

           bcdtmWrite_message(0,0,0,"** GEO_SPATIAL ** %s ",LogOutputBuffer) ;
        }
    }

    return;
}

//SECTION FOR THE SAMPLE - BEGIN
//This code section must NOT be added to bcLIB DTM or the library
//that needs to access points from a DEM raster.
void WriteXYZCoordinatesToFile(const HDOUBLE*     pi_pXYZPoints,
                               HULONG             pi_NbPoints,
                               HFCLocalBinStream* po_pPointsASCIIFile)
{
    static HAutoPtr<HCHAR> pCoordinateBuffer(new HCHAR[300]);
    int    NbChars;
    HULONG CoordinateInd = 0;

    for (HULONG PointInd = 0; PointInd < pi_NbPoints; PointInd++)
    {
        NbChars = sprintf(pCoordinateBuffer.get(),
                          "%.20f,%.20f,%.20f\r\r\n",
                          pi_pXYZPoints[CoordinateInd],
                          pi_pXYZPoints[CoordinateInd + 1],
                          pi_pXYZPoints[CoordinateInd + 2]);

        CoordinateInd += 3;
        po_pPointsASCIIFile->Write((void*)pCoordinateBuffer.get(), NbChars);
    }
}
//SECTION FOR THE SAMPLE - END
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmImagePP_browseImageSizeAndProjection (char *imageFileNameP, ImagePPCallBack callBackFunctionP, void *userP)
    {
    int ret = DTM_SUCCESS, dbg = 0;
    long imageSize = 0;
    char *gcsNameP, *gcsDescP, *gcsProjP;
    wchar_t  *wgcsNameP = NULL, *wgcsDescP = NULL, *wgcsProjP = NULL;
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Browsing Image Size And Projection");
        bcdtmWrite_message (0, 0, 0, "imageFileNameP    = %s", imageFileNameP);
        bcdtmWrite_message (0, 0, 0, "callBackFunctionP = %p", callBackFunctionP);
        bcdtmWrite_message (0, 0, 0, "userP             = %p", userP);
        }
    /*
    ** Allocate Memory Pool To Store Image
    */
    HFCPtr<HPMPool> pPool (new HPMPool (65536, HPMPool::KeepLastBlock));
    tstring DEMRasterFilePath = tstring (_TEXT ("file://")) + imageFileNameP;
    /*
    ** Set DLL Path
    */
    HRFRasterFileFactory::GetInstance ()->SetGeocoordPath (_TEXT ("basegeocoord.dll"));
    tstring geoCordPath = HRFRasterFileFactory::GetInstance ()->GetGeocoordPath ();
    if (dbg) bcdtmWrite_message (0, 0, 0, "geoCordPath = %s", geoCordPath.c_str ());
    /*
    ** Initialise Geo Coordinate System
    */
    tstring dataDirectory = _TEXT (".\\GeoCoordinateData");
    if (BaseGeoCordPathSet) dataDirectory = BaseGeoCoordPath;
    HCPGCoordUtility::GetInstance ()->InitializeGeoCoord (dataDirectory);
    if (dbg) bcdtmWrite_message (0, 0, 0, "Geo Coordinate System Initialised");
    /*
    **   Instantiate
    */
    try
        {
        HFCPtr<HFCURLFile> pDEMRasterFilePathURL ((HFCURLFile*)HFCURL::Instanciate (DEMRasterFilePath));
        if (dbg) bcdtmWrite_message (0, 0, 0, "Instantiated");
        HUTDEMRasterXYZPointsExtractor RasterPointExtractor (DEMRasterFilePath, pPool);
        HUINT64 WidthInPixels;
        HUINT64 HeightInPixels;
        RasterPointExtractor.GetDimensionInPixels (&WidthInPixels, &HeightInPixels);    // RobC - Inconsisently Crashes here

        //   Set Image Size

        imageSize = (long)(WidthInPixels * HeightInPixels);
        if (dbg) bcdtmWrite_message (0, 0, 0, "imageSize = %8ld", imageSize);

        //   Get Opaque Pointer To Raster Coordinate System Instance

        BaseGCS* pRasterCoordSys = (BaseGCS*)RasterPointExtractor.GetDEMRasterCoordSys ();
        if (dbg) bcdtmWrite_message (0, 0, 0, "pRasterCoordSys = %p", pRasterCoordSys);

        //   Set Coordinate System Parameters

        if (pRasterCoordSys != NULL)
            {
            gcsNameP = (char *)pRasterCoordSys->GetName ();
            gcsDescP = (char *)pRasterCoordSys->GetDescription ();
            gcsProjP = (char *)pRasterCoordSys->GetProjection ();
            if (dbg)
                {
                bcdtmWrite_message (0, 0, 0, "gcsName = %s", gcsNameP);
                bcdtmWrite_message (0, 0, 0, "gcsDesc = %s", gcsDescP);
                bcdtmWrite_message (0, 0, 0, "gcsProj = %s", gcsProjP);
                }

            //      Convert To W Chars

            bcdtmUtility_convertMbsToWcs (gcsNameP, (wchar_t **)&wgcsNameP);
            bcdtmUtility_convertMbsToWcs (gcsDescP, (wchar_t **)&wgcsDescP);
            bcdtmUtility_convertMbsToWcs (gcsProjP, (wchar_t **)&wgcsProjP);
            }
        else
            {
            if (dbg)
                {
                bcdtmWrite_message (0, 0, 0, "gcsName = %s", "");
                bcdtmWrite_message (0, 0, 0, "gcsDesc = %s", "");
                bcdtmWrite_message (0, 0, 0, "gcsProj = %s", "");
                }
            bcdtmUtility_convertMbsToWcs ("", (wchar_t **)&wgcsNameP);
            bcdtmUtility_convertMbsToWcs ("", (wchar_t **)&wgcsDescP);
            bcdtmUtility_convertMbsToWcs ("", (wchar_t **)&wgcsProjP);
            }
        /*
        **   Call Managed API Browse Function
        */
        if (dbg) bcdtmWrite_message (0, 0, 0, "Calling Call Back Function = %p", callBackFunctionP);
        if (callBackFunctionP (imageSize, (char *)wgcsNameP, (char *)wgcsDescP, (char *)wgcsProjP, pRasterCoordSys, userP)) goto errexit;
        if (dbg) bcdtmWrite_message (0, 0, 0, "Calling Call Back Function = %p Completed", callBackFunctionP);
        }
    /*
    **   Exception Handling
    */

    catch (HFCException &rE)
        {
        HUSHORT ExceptionID = rE.GetID ();
        tstring ExceptionMsg = rE.GetExceptionMessage ();
        bcdtmWrite_message (1, 0, 0, "Error %4d ** %s ** Opening/Reading Image File %s", ExceptionID, ExceptionMsg.c_str (), imageFileNameP);
        goto errexit;
        }

    /*
    ** Clean Up
    */
cleanup:
    if (wgcsNameP != NULL) free (wgcsNameP);
    if (wgcsDescP != NULL) free (wgcsDescP);
    if (wgcsProjP != NULL) free (wgcsProjP);
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Browsing Image Size And Projection Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Browsing Image Size And Projection Error");
    if (dbg)  bcdtmWrite_message (0, 0, 0, "");
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }


#endif


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static int bcdtmDem_checkSortOrderDemDtmObject(BC_DTM_OBJ *dtmP, long writeError )
    /*
    ** This Function Checks The Sort Order Of A Dtm Object
    */
    {
    int   ret=DTM_SUCCESS;
    long  point ;
    DTM_TIN_POINT *p1P,*p2P ;
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    **   Scan Points And Check Sort Order
    */
    p1P = dtmP->pointsPP[0] ;
    for( point = 1 ; point < dtmP->numSortedPoints && ret == DTM_SUCCESS ; ++point )
        {
        p2P = pointAddrP(dtmP,point) ;
        if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y ))
            {
            ret = DTM_ERROR ;
            if( writeError )
                {
                bcdtmWrite_message(0,0,0,"Points Out Of Sort Order") ;
                bcdtmWrite_message(0,0,0,"Point = %9ld ** %15.5lf %15.8lf",point-1,p1P->x,p1P->y,p1P->z) ;
                bcdtmWrite_message(0,0,0,"Point = %9ld ** %15.5lf %15.8lf",point,p2P->x,p2P->y,p2P->z) ;
                }
            }
        p1P = p2P ;
        }
    /*
    ** Write Error Message
    */
    if( writeError && ret != DTM_SUCCESS ) bcdtmWrite_message(1,0,0,"Dtm Sort Order Invalid") ;
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
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static DTMStatusInt  bcdtmImagePP_importImageDtmObject
(
BC_DTM_OBJ *dtmP,
WCharCP    imageFileNameP,
long       *numRowsP,
long       *numColsP,
double     *nullValueP,
double     scaleFactor,
WCharCP    projectionKeyP
)
    {
    int dbg = DTM_TRACE_VALUE (0);
    DTMStatusInt ret = DTM_SUCCESS;
    long row, col, offset = 0, heightInPixels, widthInPixels, numPixels, numPts, gridStore = 1;
    long noDataValueSet = 0, xyLimitsSet = 0, zLimitsSet = 0, geoCordSysSet = 0;
    double noDataValue = -987654.321, xMin = 0.0, yMin = 0.0, zMin = 0.0, xMax = 0.0, yMax = 0.0, zMax = 0.0;
    DTM_TIN_POINT *pointP;
    DPoint3dP p3dP;
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Reading Image To DTM");
        bcdtmWrite_message (0, 0, 0, "imageFileNameP = %s", imageFileNameP);
        bcdtmWrite_message (0, 0, 0, "scaleFactor    = %8.5lf", scaleFactor);
        bcdtmWrite_message (0, 0, 0, "projectionKeyP = %8s", projectionKeyP);
        }
    /*
    ** Initialise
    */
    *numRowsP = 0;
    *numColsP = 0;
    *nullValueP = 0.0;
    /*
    ** Allocate Memory Pool To Store Image
    */
    HFCPtr<HPMPool> pPool (new HPMPool (65536, HPMPool::KeepLastBlock));
    /*
    ** Set DLL Path
    */
    //HRFRasterFileFactory::GetInstance ()->SetGeocoordPath (L"basegeocoord.dll");
    //WString geoCordPath = HRFRasterFileFactory::GetInstance ()->GetGeocoordPath ();
    //if (dbg) bcdtmWrite_message (0, 0, 0, "geoCordPath = %s", geoCordPath.GetWCharCP ());
    ///*
    //** Initialise Geo Coordinate System
    //*/
    //WString dataDirectory = L".\\GeoCoordinateData";
    //if (BaseGeoCordPathSet) dataDirectory = BaseGeoCoordPath;
    //HCPGCoordUtility::GetInstance ()->InitializeGeoCoord (dataDirectory);
    //if (dbg) bcdtmWrite_message (0, 0, 0, "Geo Coordinate System Initialised ** dataDirectory = %s", dataDirectory.c_str ());
    /*
    ** Set Image File Name
    */
    WString DEMRasterFilePath = WString (L"file://") + imageFileNameP;
    /*
    **   Instanciate
    */
    try
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "Instantiating");
        HFCPtr<HFCURLFile> pDEMRasterFilePathURL ((HFCURLFile*)HFCURL::Instanciate (DEMRasterFilePath));
        HUTDEMRasterXYZPointsExtractor RasterPointExtractor (DEMRasterFilePath, pPool);

        if (dbg) bcdtmWrite_message (0, 0, 0, "Instantiated");

        //  Get  Pointer To Raster Coordinate System Instance

        BaseGCSCP pRasterCoordSys = RasterPointExtractor.GetDEMRasterCoordSysCP ();
        if (pRasterCoordSys != NULL) geoCordSysSet = 1;
        if (dbg)
            {
            if (geoCordSysSet == 0) bcdtmWrite_message (0, 0, 0, "Image Does Not Have An Associated Projection");
            else                     bcdtmWrite_message (0, 0, 0, "Image Projection = %s", pRasterCoordSys->GetProjection ());
            }

        //  Set Re Projection Coordinate System

        WString destCoordSysKeyName = L"";
        if (geoCordSysSet) destCoordSysKeyName = destCoordSysKeyName + projectionKeyP;
        if (dbg) bcdtmWrite_message (0, 0, 0, "destCoordSysKeyName = %s", destCoordSysKeyName.c_str ());

        //  Get No Data Value

        const double *noDataValueP = RasterPointExtractor.GetNoDataValue ();
        if (noDataValueP != 0)
            {
            noDataValueSet = 1;
            noDataValue = (double)*noDataValueP;
            }
        if (dbg)
            {
            if (noDataValueSet == 0) bcdtmWrite_message (0, 0, 0, "No Data Value Not Set For Image");
            else                      bcdtmWrite_message (0, 0, 0, "Image No Data Value %12.5lf", noDataValue);
            }

        //  Get XY Limit Values

        double XMin, XMax, YMin, YMax, ZMin, ZMax;
        RasterPointExtractor.Get2DCoordMinMaxValues (&XMin, &XMax, &YMin, &YMax);
        xMin = (double)XMin;
        yMin = (double)YMin;
        xMax = (double)XMax;
        yMax = (double)YMax;
        xyLimitsSet = 1;
        if (dbg) bcdtmWrite_message (0, 0, 0, "XY Limits ** xMin = %12.4lf xMax = %12.4lf ** yMin = %12.4lf yMax = %12.4lf", xMin, xMax, yMin, yMax);

        //  Get Z Limit Values

        bool isZLimitSet = RasterPointExtractor.GetZCoordMinMaxValues (&ZMin, &ZMax);
        if (isZLimitSet == true)
            {
            zLimitsSet = 1;
            zMin = (double)ZMin;
            zMax = (double)ZMax;
            }
        if (dbg)
            {
            if (zLimitsSet == 0) bcdtmWrite_message (0, 0, 0, "Z  Limits Not Set");
            else                  bcdtmWrite_message (0, 0, 0, "Z  Limits ** zMin = %12.4lf zMax = %12.4lf", zMin, zMax);
            }
        /*
        **  Instantiate Points Iterator
        */
        uint32_t NbPoints;
        const double* pXYZPoints;
        HAutoPtr<HUTDEMRasterXYZPointsIterator> PointsIterator (RasterPointExtractor.CreateXYZPointsIterator (destCoordSysKeyName, scaleFactor)); // RobC - Inconsisently Crashes here and never reprojects
        if (dbg)
            {
            if (PointsIterator->IsDestCoordSysCreationFailed () == true)  bcdtmWrite_message (0, 0, 0, "Failed To Create Destination Projection %s", destCoordSysKeyName.c_str ());
            else                                                          bcdtmWrite_message (0, 0, 0, "Destination Projection %s Created", destCoordSysKeyName.c_str ());
            }
        /*
        **  Get Number Of Filtered Rows And Columns
        */
        uint64_t WidthInPixels;
        uint64_t HeightInPixels;
        PointsIterator->GetFilteredDimensionInPixels (&WidthInPixels, &HeightInPixels);
        heightInPixels = (long)HeightInPixels;
        widthInPixels = (long)WidthInPixels;
        numPixels = heightInPixels * widthInPixels;
        if (dbg) bcdtmWrite_message (0, 0, 0, "Filtered ** Image Width = %8ld Image Height = %8ld numPixels = %8ld", widthInPixels, heightInPixels, numPixels);
        /*
        **  Set Memory Allocation Parameters For DTM
        */
        bcdtmObject_setPointMemoryAllocationParametersDtmObject (dtmP, widthInPixels*heightInPixels, 10000);
        /*
        **  Initialise Col Row Sequence For Storing Points In DTM For Gridded Triangulation Engine
        */
        col = 1;
        row = 0;
        numPts = 0;
        /*
        **  Scan Image And Store Coordinates In DTM
        */
        if (dbg) bcdtmWrite_message (0, 0, 0, "Storing Points In DTM Object");
        while ((pXYZPoints = PointsIterator->GetXYZPoints (&NbPoints)) != 0)
            {
            if (pXYZPoints == NULL)
                {
                bcdtmWrite_message (0, 0, 0, "Null Points Pointer");
                goto errexit;
                }
            /*
            **     Allocate DTM Memory If First Point
            */
            if (dtmP->numPoints == 0)
                {
                if (bcdtmObject_allocatePointsMemoryDtmObject (dtmP)) goto errexit;
                if (gridStore) dtmP->numPoints = widthInPixels*heightInPixels;
                }
            /*
            **     Store Image Points In DTM In Sequence Required By The DEM Triangulation Engine
            */
            if (gridStore)
                {
                for (p3dP = (DPoint3d *)pXYZPoints; p3dP < (DPoint3d *)pXYZPoints + NbPoints; ++p3dP)
                    {
                    ++numPts;
                    offset = (heightInPixels - row) + (col - 1) * heightInPixels - 1;
                    if (offset < 0 || offset >= numPixels)
                        {
                        bcdtmWrite_message (1, 0, 0, "Pixel Offset Range error = %8ld ** row = %8ld col = %8ld numPixels = %8ld", offset, row, col, numPts);
                        goto errexit;
                        }
                    pointP = pointAddrP (dtmP, offset);
                    pointP->x = p3dP->x;
                    pointP->y = p3dP->y;
                    pointP->z = p3dP->z;
                    ++col;
                    if (col > widthInPixels)
                        {
                        ++row;
                        col = 1;
                        }
                    }
                }
            /*
            **     Store Points Sequentially In DTM
            */
            else
                {
                if (bcdtmObject_storeDtmFeatureInDtmObject (dtmP, DTMFeatureType::RandomSpots, dtmP->nullUserTag, 1, &dtmP->nullFeatureId, (DPoint3dP)pXYZPoints, NbPoints)) goto errexit;
                }
            /*
            **     Get Next Set Of Points
            */
            PointsIterator->Next ();
            }
        }
    /*
    ** Catch File Open Exception
    */
    catch (HFCException &rE)
        {
        /*WString msg = */rE.GetExceptionMessage ();
        //    bcdtmWrite_message (1, 0, 0, "Error %4d ** %s ** Opening/Reading Image File %s", ExceptionID, ExceptionMsg.c_str (), imageFileNameP);
        //    goto errexit;
        //    }
        goto errexit;
        }
    catch (...)
        {
        bcdtmWrite_message (1, 0, 0, "Unknown Error** Opening/Reading Image Files");
        goto errexit;
        }

    /*
    ** Set Bounding Cube
    */
    if (gridStore) if (bcdtmMath_setBoundingCubeDtmObject (dtmP)) goto errexit;
    /*
    ** Set Return Values
    */
    *nullValueP = noDataValue;
    *numRowsP = heightInPixels;
    *numColsP = widthInPixels;
    /*
    ** Clean Up
    */
cleanup:
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Reading Image To DTM Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Reading Image To DTM Error");
    if (dbg)  bcdtmWrite_message (0, 0, 0, "");
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }

ImagePPConverter::ImagePPConverter (WCharCP filename)
    {
    m_filename = filename;
    m_hasProperties = false;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
void ImagePPConverter::GetImageProperties ()
    {
    if (!m_hasProperties)
        {
        m_hasProperties = true;
        ///*
        //** Allocate Memory Pool To Store Image
        //*/
        HFCPtr<HPMPool> pPool = new HPMPool (65536, HPMPool::KeepLastBlock);
        //if (dbg) bcdtmWrite_message (0, 0, 0, "Memory Pool Allocated");
        /*
        **   Instantiate
        */
        try
            {
            WString DEMRasterFilePath = WString (L"file://") + m_filename;
            HFCPtr<HFCURLFile> pDEMRasterFilePathURL = new HFCURLFile (DEMRasterFilePath);

            HUTDEMRasterXYZPointsExtractor RasterPointExtractor (DEMRasterFilePath, pPool);
            RasterPointExtractor.GetDimensionInPixels (&m_widthInPixels, &m_heightInPixels);
            //   Get BaseGCS Pointer To Raster Coordinate System
            if(RasterPointExtractor.GetDEMRasterCoordSysCP () != nullptr)
                m_gcs = GeoCoordinates::BaseGCS::CreateGCS(*RasterPointExtractor.GetDEMRasterCoordSysCP ());
            else
                m_gcs = nullptr;
            }
        catch (HFCException& rE)
            {
            /*WString msg = */rE.GetExceptionMessage();
            //    bcdtmWrite_message (1, 0, 0, "Error %4d ** %s ** Opening/Reading Image File %s", ExceptionID, ExceptionMsg.c_str (), imageFileNameP);
            //    goto errexit;
            //    }
            m_gcs = nullptr;
            m_widthInPixels = 0;
            m_heightInPixels = 0;
            }
        }
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt ImagePPConverter::ImportAndTriangulateImageDtmObject
(
BcDTMPtr&   dtm,
double     imageScaleFactor,                  // Scale Factor For Reducing Number of Image Pixels Stored In DTM
WCharCP    projectionKeyP,                   // Projection Key For Extracted Image Coordinates
double     unitConversionFactor,              // Unit Conversation Factor For Elevation Values - Metres To ??
double     elevationScaleFactor               // Elevation Scale Factor
)
    {
    int dbg = DTM_TRACE_VALUE (0), cdbg = DTM_CHECK_VALUE (0);
    DTMStatusInt ret = DTM_SUCCESS;
    double dx, dy, nullValue = -98765.4321;
    long   nx, ny, numColPts = 0, lastColPts = 0, colNum = 0, /*dtmFeature,*/ numRows, numCols/*, numVoids,numIslands*/;
    long* islandsP = nullptr;
    long   startTime, /*cleanInternalVoids = 0,*/ numImagePoints;
    bool imageRegular = true;
    long   point, point1, point2;
    double xImageMin, yImageMin, zImageMin, xImageMax, yImageMax, zImageMax;
    double xDist, yDist, colSpacing, rowSpacing;
    DTM_TIN_POINT *p1P, *p2P, *p3P, *pointP;
    //BC_DTM_FEATURE *dtmFeatureP;
    BC_DTM_OBJ* dtmP;
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Importing Image And Triangulating");
        bcdtmWrite_message (0, 0, 0, "dtmPP                = %p", dtm.get());
        bcdtmWrite_message (0, 0, 0, "imageFileNameP       = %s", m_filename.GetWCharCP());
        bcdtmWrite_message (0, 0, 0, "imageScaleFactor     = %8.5lf", imageScaleFactor);
        bcdtmWrite_message (0, 0, 0, "projectionKeyP       = %s", projectionKeyP);
        bcdtmWrite_message (0, 0, 0, "unitConversionFactor = %8.5lf", unitConversionFactor);
        bcdtmWrite_message (0, 0, 0, "elevationScaleFactor = %8.5lf", elevationScaleFactor);
        }
    /*
    ** Check Scale Factor
    */
    if (imageScaleFactor <= 0.0 || imageScaleFactor > 1.0) imageScaleFactor = 1.0;
    /*
    ** Create DTM Object
    */
    dtm = BcDTM::Create ();
    dtmP = dtm->GetTinHandle ();
    /*
    ** Create Image
    */
    startTime = bcdtmClock ();
    if (dbg) bcdtmWrite_message (0, 0, 0, "Reading Image File = %s", m_filename.GetWCharCP ());
    if (bcdtmImagePP_importImageDtmObject (dtmP, m_filename.GetWCharCP (), &numRows, &numCols, &nullValue, imageScaleFactor, projectionKeyP)) goto errexit;
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "**** Image Read Time            = %8.3lf Secs", bcdtmClock_elapsedTime (bcdtmClock (), startTime));
        bcdtmWrite_message (0, 0, 0, "**** Number Of Rows             = %8ld", numRows);
        bcdtmWrite_message (0, 0, 0, "**** Number Of Columns          = %8ld", numCols);
        bcdtmWrite_message (0, 0, 0, "**** Number Of Pixels           = %8ld", numCols*numRows);
        bcdtmWrite_message (0, 0, 0, "**** Null Value                 = %12.5lf", nullValue);
        bcdtmWrite_message (0, 0, 0, "**** Number Of DEM Points Read  = %8ld", dtmP->numPoints);
        }
    /*
    ** Check For More Than One Row Or One Column
    */
    if (numRows <= 1 || numCols <= 1)
        {
        bcdtmWrite_message (1, 0, 0, "Requires More Than One Row Or One Column");
        goto errexit;
        }
    /*
    **  Calculate Row And Column Spacing
    */
    dx = dtmP->xRange / (double)(numCols - 1);
    dy = dtmP->yRange / (double)(numRows - 1);
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "column spacing  = %15.12lf  row spacing  = %15.12lf", dx, dy);
        bcdtmWrite_message (0, 0, 0, "xMin = %15.12lf yMin = %15.12lf ** xMax = %15.12lf yMax = %15.12lf", dtmP->xMin, dtmP->yMin, dtmP->xMax, dtmP->yMax);
        }
    /*
    ** Count Number Of Missing Values
    */
    if (dbg)
        {
        long point, numValues = 0, numMissingValues = 0;
        for (point = 0; point < dtmP->numPoints; ++point)
            {
            pointP = pointAddrP (dtmP, point);
            if (pointP->z == nullValue) ++numMissingValues;
            else                         ++numValues;
            }
        bcdtmWrite_message (0, 0, 0, "numMissing = %8ld numValues = %8ld totalValues = %8ld", numMissingValues, numValues, numMissingValues + numValues);
        }
    /*
    ** Get Bounding Rectangle For Valid Image Points
    */
    numImagePoints = 0;
    xImageMin = dtmP->xMin; yImageMin = dtmP->yMin; zImageMin = dtmP->zMin;
    xImageMax = dtmP->xMax; yImageMax = dtmP->yMax; zImageMax = dtmP->zMax;
    for (point = 0; point < dtmP->numPoints; ++point)
        {
        pointP = pointAddrP (dtmP, point);
        if (pointP->z != nullValue)
            {
            if (numImagePoints == 0)
                {
                xImageMin = xImageMax = pointP->x;
                yImageMin = yImageMax = pointP->y;
                zImageMin = zImageMax = pointP->z;
                }
            else
                {
                if (pointP->x < xImageMin) xImageMin = pointP->x;
                if (pointP->x > xImageMax) xImageMax = pointP->x;
                if (pointP->y < yImageMin) yImageMin = pointP->y;
                if (pointP->y > yImageMax) yImageMax = pointP->y;
                if (pointP->z < zImageMin) zImageMin = pointP->z;
                if (pointP->z > zImageMax) zImageMax = pointP->z;
                }
            ++numImagePoints;
            }
        }
    /*
    ** Write Bounding Rectangle For Valid Image Points
    */
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Number Of Valid Image Points = %8ld", numImagePoints);
        bcdtmWrite_message (0, 0, 0, "xImageMin = %12.5lf  xImageMax = %12.5lf xImageRange = %12.5lf", xImageMin, xImageMax, xImageMax - xImageMin);
        bcdtmWrite_message (0, 0, 0, "yImageMin = %12.5lf  yImageMax = %12.5lf yImageRange = %12.5lf", yImageMin, yImageMax, yImageMax - yImageMin);
        bcdtmWrite_message (0, 0, 0, "zImageMin = %12.5lf  zImageMax = %12.5lf zImageRange = %12.5lf", zImageMin, zImageMax, zImageMax - zImageMin);
        }
    /*
    ** Remove Missing Values Outside Of Bounding Rectangle For Valid Image Points
    */
    point1 = 0;
    p1P = pointAddrP (dtmP, point1);
    for (point2 = 0; point2 < dtmP->numPoints; ++point2)
        {
        p2P = pointAddrP (dtmP, point2);
        if (p2P->x >= xImageMin && p2P->x <= xImageMax && p2P->y >= yImageMin && p2P->y <= yImageMax)
            {
            if (p2P != p1P) *p1P = *p2P;
            ++point1;
            p1P = pointAddrP (dtmP, point1);
            }
        }
    dtmP->numPoints = point1;
    if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of Image DTM Points = %8ld", dtmP->numPoints);
    /*
    ** Resize Memory
    */
    if (bcdtmObject_resizeMemoryDtmObject (dtmP)) goto errexit;
    /*
    ** Reset Bounding Cube
    */
    if (bcdtmMath_setBoundingCubeDtmObject (dtmP)) goto errexit;
    /*
    ** Determine Row And Column Spacing
    */
    p1P = pointAddrP (dtmP, 0);
    dx = dx / 100.0;
    dy = dy / 100.0;
    rowSpacing = dtmP->yRange;
    colSpacing = dtmP->xRange;
    for (point = 1; point < dtmP->numPoints; ++point)
        {
        p2P = pointAddrP (dtmP, point);
        xDist = fabs (p1P->x - p2P->x);
        yDist = fabs (p1P->y - p2P->y);
        if (xDist > dx && xDist < colSpacing) colSpacing = xDist;
        if (yDist > dy && yDist < rowSpacing) rowSpacing = yDist;
        }
    if (dbg) bcdtmWrite_message (0, 0, 0, "colSpacing = %12.10lf rowSpacing = %12.10lf", colSpacing, rowSpacing);
    /*
    ** Make Row And Column Coordinates Identical
    */
    for (point = 0; point < dtmP->numPoints; ++point)
        {
        p1P = pointAddrP (dtmP, point);
        nx = (long)((p1P->x - dtmP->xMin) / colSpacing + 0.1);
        p1P->x = dtmP->xMin + (double)nx * colSpacing;
        ny = (long)((p1P->y - dtmP->yMin) / rowSpacing + 0.1);
        p1P->y = dtmP->yMin + (double)ny * rowSpacing;
        }
    /*
    ** Check DTM Is Sorted
    */
    dtmP->numSortedPoints = dtmP->numPoints;
    if (bcdtmDem_checkSortOrderDemDtmObject (dtmP, 0))
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "Sorting DEM DTM");
        startTime = bcdtmClock ();
        dtmP->numSortedPoints = 1;
        dtmP->dtmState = DTMState::Data;
        if (bcdtmObject_sortDtmObject (dtmP)) goto errexit;
        if (dbg) bcdtmWrite_message (0, 0, 0, "DEM DTM Sort Time = %8.3lf Secs", bcdtmClock_elapsedTime (bcdtmClock (), startTime));
        /*
        **  Recheck Sort Order
        */
        if (cdbg)
            {
            bcdtmWrite_message (0, 0, 0, "Re Checking DTM Sort Order");
            if (bcdtmDem_checkSortOrderDemDtmObject (dtmP, 0))
                {
                bcdtmWrite_message (1, 0, 0, "DTM Sort Order Invalid");
                goto errexit;
                }
            bcdtmWrite_message (0, 0, 0, "DTM Sort Order Valid");
            }
        }
    /*
    ** Check Number Of Rows For Each Column Are Consistent
    */
    colNum = 1;
    numColPts = 1;
    lastColPts = -999;
    p1P = pointAddrP (dtmP, 0);
    imageRegular = true;
    for (point = 1; point < dtmP->numPoints && imageRegular == true; ++point)
        {
        p2P = pointAddrP (dtmP, point);
        if (p2P->x != p1P->x)
            {
            if (dbg == 2)
                {
                p3P = pointAddrP (dtmP, point - 1);
                bcdtmWrite_message (0, 0, 0, "Column[%8ld] ** numRows = %8ld ** botX = %12.10lf botY = %12.10lf ** topX = %12.10lf toP->y = %12.10lf", colNum, numColPts, p1P->x, p1P->y, p3P->x, p3P->y);
                }
            if (lastColPts != -999 && numColPts != lastColPts) imageRegular = false;
            lastColPts = numColPts;
            p1P = p2P;
            ++colNum;
            numColPts = 1;
            }
        else ++numColPts;
        }
    if (dbg == 2 && imageRegular == true)
        {
        p3P = pointAddrP (dtmP, dtmP->numPoints - 1);
        bcdtmWrite_message (0, 0, 0, "Column[%8ld] ** numrows = %8ld ** botX = %12.10lf botY = %12.10lf ** topX = %12.10lf toP->y = %12.10lf", colNum, numColPts, p1P->x, p1P->y, p3P->x, p3P->y);
        }
    if (numColPts != lastColPts) imageRegular = false;
    /*
    **  Count Number Of Rows And Columns
    */
    if (imageRegular)
        {
        numRows = lastColPts;
        numCols = colNum;
        /*
        ** Triangulate Using DEM Triangulation Engine
        */
        startTime = bcdtmClock ();
        if (dbg) bcdtmWrite_toFileDtmObject (dtmP, L"demImageDataState.bcdtm");
        if (dbg) bcdtmWrite_message (0, 0, 0, "Triangulating %8ld Point DEM DTM ** numRows = %6ld numCols = %6ld", dtmP->numPoints, numRows, numCols);
        if (bcdtmObject_triangulateDemDtmObject (dtmP, numRows, numCols, nullValue)) goto errexit;
        if (dbg) bcdtmWrite_message (0, 0, 0, "DEM DTM Triangulation Time = %8.3lf Secs", bcdtmClock_elapsedTime (bcdtmClock (), startTime));
        if (dbg) bcdtmWrite_toFileDtmObject (dtmP, L"demImageTinState.bcdtm");
        }
    /*
    ** Triangulate Using Random Triangulation Engine
    */
    else
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "Triangulating %8ld Point DTM", dtmP->numPoints);
        dtmP->edgeOption = 1;
        dtmP->dtmState = DTMState::Data;
        bcdtmMath_calculateMachinePrecisionForDtmObject (dtmP);
        dtmP->ppTol = dtmP->plTol = dtmP->mppTol * 10000.0;
        if (bcdtmObject_triangulateDtmObject (dtmP)) goto errexit;
        if (dbg) bcdtmWrite_message (0, 0, 0, "DTM Triangulation Time = %8.3lf Secs", bcdtmClock_elapsedTime (bcdtmClock (), startTime));
        /*
        **  Void Missing Values
        */
        if (dbg) bcdtmWrite_message (0, 0, 0, "Voiding Missing Values");
        if (bcdtmObject_placeVoidsAroundNullValuesDtmObject (dtmP, nullValue)) goto errexit;
        }
    goto apply;
    ///*
    //** Count Number Of Voids
    //*/
    //numVoids = 0;
    //numIslands = 0;
    //for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
    //    {
    //    dtmFeatureP = ftableAddrP (dtmP, dtmFeature);
    //    if (dtmFeatureP->dtmFeatureType == DTMFeatureType::Void) ++numVoids;
    //    if (dtmFeatureP->dtmFeatureType == DTMFeatureType::Island) ++numIslands;
    //    }
    //if (dbg) bcdtmWrite_message (0, 0, 0, "numVoids = %8ld ** numIslands = %8ld", numVoids, numIslands);
    ///*
    //**  Clean Up Voids Resulting From Null Values
    //*/
    //if (numVoids > 0)
    //    {
    //    /*
    //    **  Remove Voids On Tin Hull
    //    */
    //    if (dbg) bcdtmWrite_message (0, 0, 0, "Removing Voids On Tin Hull");
    //    if (bcdtmEdit_removeInsertedVoidsOnTinHullDtmObject (dtmP, 0)) goto errexit;
    //    /*
    //    **  Clean Internal Voids
    //    */
    //    if (cleanInternalVoids)
    //        {
    //        if (dbg) bcdtmWrite_message (0, 0, 0, "Removing Null Values From Internal Voids");
    //        for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
    //            {
    //            dtmFeatureP = ftableAddrP (dtmP, dtmFeature);
    //            if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Void)
    //                {
    //                if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Cleaning Void Feature %8ld", dtmFeature);
    //                /*
    //                **           Get Island Features Internal To Void
    //                */
    //                if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Getting IslandS Internal To Void");
    //                if (bcdtmEdit_getIslandsInternalToVoidDtmObject (dtmP, dtmFeature, &islandsP, &numIslands)) goto errexit;
    //                if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Number Of Islands = %8ld", numIslands);
    //                /*
    //                **           Remove Internal Void Points And Lines
    //                */
    //                if (dbg == 2) bcdtmWrite_message (0, 0, 0, "Removing Internal Void Points And Lines");
    //                if (bcdtmEdit_removeInternalVoidPointsAndLinesDtmObject (dtmP, dtmFeature, islandsP, numIslands)) goto errexit;
    //                if (islandsP != NULL) { free (islandsP); islandsP = NULL; }
    //                }
    //            }
    //        }
    //    /*
    //    **  Clean Dtm Object
    //    */
    //    if (dbg) bcdtmWrite_message (0, 0, 0, "Cleaning Dtm Object");
    //    if (bcdtmList_cleanDtmObject (dtmP)) goto errexit;
    //    }
    /*
    ** Apply Unit Conversion Factor To Elevation Values
    */
apply:
    if (unitConversionFactor != 1.0)
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "Applying Unit Conversions To Elevations");
        dtmP->zMin = dtmP->zMin * unitConversionFactor;
        dtmP->zMax = dtmP->zMax * unitConversionFactor;
        dtmP->zRange = dtmP->zRange * unitConversionFactor;
        for (point = 0; point < dtmP->numPoints; ++point)
            {
            p1P = pointAddrP (dtmP, point);
            p1P->z = p1P->z * unitConversionFactor;
            }
        }
    /*
    ** Apply Scale Factor To Elevation Values
    */
    if (elevationScaleFactor != 1.0)
        {
        if (dbg) bcdtmWrite_message (0, 0, 0, "Applying Scale Conversions To Elevations");
        dtmP->zRange = dtmP->zRange * elevationScaleFactor;
        dtmP->zMax = dtmP->zMin + dtmP->zRange;
        for (point = 0; point < dtmP->numPoints; ++point)
            {
            p1P = pointAddrP (dtmP, point);
            p1P->z = dtmP->zMin + (p1P->z - dtmP->zMin) * elevationScaleFactor;
            }
        }
    /*
    ** Write Stats
    */
    if (dbg)
        {
        bcdtmWrite_message (0, 0, 0, "Number Of DTM Points = %8ld", dtmP->numPoints);
        bcdtmWrite_message (0, 0, 0, "X ** Min = %13.4lf Max = %13.4lf Range = %13.4lf", dtmP->xMin, dtmP->xMax, dtmP->xRange);
        bcdtmWrite_message (0, 0, 0, "Y ** Min = %13.4lf Max = %13.4lf Range = %13.4lf", dtmP->yMin, dtmP->yMax, dtmP->yRange);
        bcdtmWrite_message (0, 0, 0, "Z ** Min = %13.4lf Max = %13.4lf Range = %13.4lf", dtmP->zMin, dtmP->zMax, dtmP->zRange);
        }
    /*
    ** Clean Up
    */
    /*
    ** Clean Up
    */
cleanup:
    if (islandsP != NULL) { free (islandsP); islandsP = NULL; }
    /*
    ** Job Completed
    */
    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Importing Image And Triangulating Completed");
    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Importing Image And Triangulating Error");
    return(ret);
    /*
    ** Error Exit
    */
errexit:
    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
    goto cleanup;
    }



BcDTMPtr ImagePPConverter::ImportAndTriangulateImage (double imageScaleFactor, WCharCP projectionKeyP, double unitConversionFactor, double elevationScaleFactor)
    {
    BcDTMPtr dtm;
    if (ImportAndTriangulateImageDtmObject (dtm, imageScaleFactor, projectionKeyP, unitConversionFactor, elevationScaleFactor) == DTM_SUCCESS)
        return dtm;
    return nullptr;
    }

uint64_t ImagePPConverter::GetWidth ()
    {
    GetImageProperties ();
    return m_widthInPixels;
    }

uint64_t ImagePPConverter::GetHeight ()
    {
    GetImageProperties ();
    return m_heightInPixels;
    }

uint64_t ImagePPConverter::GetNumberOfPixels ()
    {
    GetImageProperties ();
    return m_heightInPixels * m_widthInPixels;
    }

BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr ImagePPConverter::GetGCS ()
    {
    GetImageProperties ();
    return m_gcs;
    }

ImagePPConverterPtr ImagePPConverter::Create (WCharCP filename)
    {
    return new ImagePPConverter (filename);
    }
