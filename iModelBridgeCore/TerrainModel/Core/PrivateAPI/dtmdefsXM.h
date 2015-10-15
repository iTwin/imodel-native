/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PrivateAPI/dtmdefsXM.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifdef XM

struct DTM_DAT_OBJ;
struct DTM_TIN_OBJ;

#define   DTM_NULL_GUID          {16*0}
/*
** DTMFeatureState::Tin Null Values
*/
#define   TIN_NULL_PNT           888888888
#define   TIN_NULL_PTR           999999999
#define   TIN_NULL_USER_TAG    -9898989898
#define   TIN_NULL_FEATURE_ID  -9898989898
#define   TIN_NULL_GUID        {16*0}

/*-------------------------------------------------------------------+
|                                                                    |
|   Definitions For Data Objects                                     |
|                                                                    |
+-------------------------------------------------------------------*/
/*
** Define Varible For Maximum NumBer Of Data Objects
*/
#define  DTM_MAX_DAT_OBJS 4096
struct DTM_GUID
{ 
 unsigned long  data1    ; // Using Huntsville Definition
 unsigned short data2    ; 
 unsigned short data3    ;
 char           data4[8] ;
};
/*
**  Typedefs For Data Object Variables
*/
// ToDo change too DPoint3d
struct DTM_DATA_POINT : DPoint3d { }  ;
/*
** Typedef For Data Object
*/
struct DTM_DAT_OBJ
  {
   long     dtmFileType,dtmFileVersion ;
   long     numPts,memPts,numFeatPts,incMemPts,iniMemPts,numDecDigits,stateFlag ;
   long     refCount,userStatus ;
   DTMTime32 creationTime,modifiedTime,userTime ;
   double   xMin,yMin,zMin,xMax,yMax,zMax ;
   double   ppTol,plTol ; 
   char     userName[DTM_FILE_SIZE] ;
   char     dataObjectFileName[DTM_FILE_SIZE] ;
   char     userMessage[256] ;
   DTM_FEATURE_CODE   *featureCodeP ;
   DTMUserTag       *userTagP     ; 
   DTM_DATA_POINT     *pointsP      ;
   DTM_GUID           *guidP        ;
  };

/*-------------------------------------------------------------------+
|                                                                    |
|   Definitions For Tin Objects                                      |
|                                                                    |
+-------------------------------------------------------------------*/
/*
** Define Varible For Maximum NumBer Of Tin Objects
*/
#define  DTM_MAX_TIN_OBJS 4096
/*
**   PCWD  Bit Definition Fields
**   Bit = 0     Point In Void
**   Bit = 1     Inserted Point 
**   Bit = 2     Inserted Point That Intersects Existing Tin Line
*/ 
/*
** Note PCWD contains bit fields . Bit 1 == Right Most Bit
**
** Bit 1  = 1  Point In Void
** Bit 2  = 1  Inserted Point
*/
/*
**  Data Structure For DTM Feature Array
*/
struct DTM_FEATURE_TABLE { long  firstPnt; DTMFeatureType dtmFeatureType; long internalToFeature ; DTMUserTag userTag ; DTM_GUID userGuid ; } ;
/*
**  Data Structure For DTM Feature Map
*/
struct DTM_FEATURE_MAP { char  fMap ; } ;
/*
**  Data Structure For Tin Object
*/
struct DTM_TIN_OBJ
{
 long    dtmFileType,dtmFileVersion ;
 long    numPts,memPts,iniMemPts,incMemPts,numSortedPts ;
 long    cListPtr,cListDelPtr,cListLastDelPtr ;
 long    numFeatureTable,memFeatureTable,iniMemFeatureTable,incMemFeatureTable ;
 long    numFeatureList,memFeatureList,iniMemFeatureList,incMemFeatureList,featureListDelPtr ;
 long    numFeatureMap,memFeatureMap,iniMemFeatureMap,incMemFeatureMap ;
 long    numTriangles,numLines,hullPnt,nextHullPnt;
 long    nullPtr,nullPnt,numDecDigits ;
 DTMUserTag nullUserTag ;
 DTM_GUID     nullGuid    ;
 long    refCount,userStatus ;
 double  plTol,ppTol,mppTol ;
 double  xMin,yMin,zMin,xMax,yMax,zMax,xRange,yRange,zRange ;
 DTMTime32     creationTime,modifiedTime,userTime ;
 char    userName[DTM_FILE_SIZE] ;
 char    tinObjectFileName[DTM_FILE_SIZE] ;
 char    userMessage[256] ;
/* Spare variables For Future Use */
 long    SL1,SL2,SL3,SL4,SL5 ;
 DTMUserTag SI641,SI642,SI643,SI644,SI645 ;
 double  SD1,SD2,SD3,SD4,SD5 ; 
 void    *SP1,*SP2,*SP3,*SP4,*SP5 ;
/************************************/
 DTM_TIN_POINT     *pointsP   ;
 DTM_TIN_NODE      *nodesP    ;
 DTM_CIR_LIST      *cListP    ;
 DTM_FEATURE_TABLE *fTableP   ;
 DTM_FEATURE_LIST_VER200  *fListP    ;
 DTM_FEATURE_MAP   *fMapP     ;
};

struct DTM_DRAPE_FEATURE_XM { DTMFeatureType dtmFeatureType ; DTMUserTag userTag ; DTM_GUID userGuid ; } ;
struct DTM_DRAPE_DATA_XM { long DrapeType,DrapeLine,numFeatureTable ; double x,y,z ; DTM_DRAPE_FEATURE_XM *Features ; } ;
struct DTM_TIN_POINT_FEATURES_XM { long dtmFeature; DTMFeatureType dtmFeatureType; long priorPoint,nextPoint ; DTMUserTag userTag ; DTM_GUID userGuid ; } ;

struct DTM_FEATURE_XM 
{
 DTMFeatureType dtmFeatureType ;      /* Dtm Feature Type          */
 DTMUserTag   userTag        ;      /* User Tag                  */
 DTM_GUID       userGuid       ;      /* Unique Identifier         */
 DPoint3d            *pointsP       ;      /* Pointer To Feature Points */
 long           numPoints      ;      /* Number Of Feature Points  */
} ;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/ 
struct DTM_DRAPE_POINT_XM
{ 
 long   drapeLine ;                        /* Segment Number Of User Drape String             */
 long   drapeType ;                        /* Type Of Drape Point
                                              ==  0  Drape Point External To Tin 
                                              ==  1  Drape Point Internal To Tin
                                              ==  2  Drape Point On Break Line  
                                              ==  3  Drape Point On Break Triangle
                                              ==  4  Drape Point In Void                      */
 long   numDrapeFeatures ;                 /* Number Of Dtm Features At Drape Point           */
 double drapeX ;                           /* x Coordinate Of Drape Point                     */
 double drapeY ;                           /* y Coordinate Of Drape Point                     */
 double drapeZ ;                           /* z Coordinate Of Drape Point                     */ 
 DTM_TIN_POINT_FEATURES_XM *drapeFeaturesP ;  /* Pointer To DTM Features At Drape Point          */
} ;

/*-------------------------------------------------------------------+
|                                                                    |
|  Typedefs For Side Slopes                                          |
|                                                                    |
+-------------------------------------------------------------------*/ 
struct DTM_SIDE_SLOPE_TABLEXM
{
int     radialStatus;           // Internal DTM Use ** Radial Status.   1 = Calculate Side Slope,  0 = Do Not Calculate Side Slope   
int     radialType;             // Internal DTM Use ** Radial Type,     1 = Concave , 2 = Edge , 3 = Convex  
int     radialGenesis;          // Internal DTM Use ** Radial Genesis,  1 = User , 2 = Elevation Transition , 3 = Transition On Cut Fill Tin, 4 = Transition On Slope Tin, 5 = Convex Corner
double  radialAngle;            // Internal DTM Use ** Radial Angle
double  radialSlope;            // Internal DTM Use ** Radial Slope
int     radialSolution;         // Internal DTM Use ** Radial Solution, 1 = Angle Slope Solution Found For Radial Option  0 = Angle Slope Solution Not Found For Radial Option 
DPoint3d     radialEndPoint;         // Internal DTM Use ** End Point Of Radial
double  surfaceZ ;              // Internal DTM Use ** Surface z Value At the Radial Start Point

DPoint3d     radialStartPoint;       // => Radial origination point
int     radialOption ;          // => Radial option - "radial" "planar" or "radial-cs"
int     sideSlopeOption ;       // =>  1  Side Slope To Tin Object
                                // =>  2  Side Slope To Set Elevation
                                // =>  3  Side Slope Out A  Horizontal Distance
                                // =>  4  Side Slope Up/Down A Vertical Distance
                                // =>  5  Side Slope To Tin Surface Or Set Elevation Whichever Comes First
                                // =>  6  Side Slope To Tin Surface Or Horizontal Distance Whichever Comes First     
                                // =>  7  Side Slope To Tin Surface Or Up/Down A Vertical Distance Whichever Comes First     
                                // =>  8  Side Slope To Set Elevation And Extend Obtuse Radials For Copy parallel applications     
DTM_TIN_OBJ  *slopeToTin;       // => To tin object if slope option = to tin NULL otherwise
double  toElev;                 // => To constant elevation if slope option is to elevation
double  toDeltaElev;            // => To delta elevation if slope option is to delta elevation
double  toHorizOffset;          // => To horizontal offset if slope option is to hor distance
double  cutSlope;               // => Cut Slope - unit per unit
double  fillSlope;              // => Fill Slope - unit per unit

int     cutFillOption ;         // => Option to indicate whether to process in 0-CUTANDFILL,1-CUTONLY, 2-FILLONLY,
DTM_TIN_OBJ  *cutFillTin ;           // => Cut fill tin object if isCutFillOption == CUTONLY or FILLONLY

int     useSlopeTable;          // => Flag to indicate that the slope is to be determined from the Slope Table
int     isForceSlope;           // => Flag to indicate wether the slope is to be forced
double  forcedSlope;            // => Value to use as force slope
int     isRadialDir;            // => Flag to indicate whether slope direction is supplied
double  radialDir;              // => Slope direction if isSlopeDir is true
int     offsetDef ;             // => Offset Definition To Indicate whether the offsets apply ALONG_RADILADIR or ALONG_NORMALDIR 

int     isMinHorizLimit;        // => Flag to indicate whether we have a minimum horizontal limit
double  limitMinHoriz;          // => Value of minimum horizontal limit IF isMinHorizLimit = true
int     isMaxHorizLimit;        // => Flag to inidicate whether there is a maximum horizontal distance to go
double  limitMaxHoriz;          // => Value of max horizontal limit if isMaxHorizLimit = true

int     isCutThreshold;         // => Value to indicate whether a vertical cut threshold ie.
double  minCutThreshold;        // => Value of cut threshold - cut must exceed this before cut is considered
double  maxCutThreshold;        // => Value of cut threshold - cut must exceed this before cut is considered
int     isFillThreshold;        // => Value to indicate whether a vertical fill threshold ie.
double  minFillThreshold;       // => Value of Fill threshold - fill must exceed this before cut is considered
double  maxFillThreshold;       // => Value of Fill threshold - fill must exceed this before cut is considered
} ;

/*-------------------------------------------------------------------+
|                                                                    |
|  Hull Point List                                                   |
|                                                                    |
+-------------------------------------------------------------------*/ 

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
/*
** Data Structure For Sets Of Data Object 
*/
struct DTM_DAT_OBJLIST { DTM_DAT_OBJ *Data ; long ListFlag ; } ;
/*
** Data Structure For Sets Of Data Object 
*/
struct DTM_TIN_OBJLIST { DTM_TIN_OBJ *Tin  ; long TinIndex ; } ;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/ 
struct DTM_SCAN_CONTEXT
{
 long   dtmObjectType ;            /* Dtm Object Type <DTM_DAT_TYPE>,<DTM_TIN_TYPE>,<DTM_LAT_TYPE> */
 DTM_DAT_OBJ  *dataP  ;            /* Pointer To Data Object To Be Scanned           */
 DTM_TIN_OBJ  *tinP   ;            /* Pointer To Tin  Object To Be Scanned           */
 DTM_LAT_OBJ  *latP   ;            /* Pointer To Lattice Object To Be Scanned        */
 DTM_TIN_OBJ  *clipTinP ;          /* Tin Object Pointer For Clipping Dtm Feature    */
 DTMFeatureType  dtmFeatureType ;  /* Dtm Feature Type To Be Loaded                  */    
 long         clipOption ;         /* Clip Option <None(0)>,<Inside(1)>,<Overlap(2)> */
 long         maxSpots ;           /* Max Spots To Be returned                       */
 long         scanOffset1 ;        /* Scan Offset 1                                  */
 long         scanOffset2 ;        /* Scan Offset 2                                  */
 long         scanOffset3 ;        /* Scan Offset 3                                  */
} ;

/*====================================================================*/
/*                            DEPRECATED                              */
/*====================================================================*/
/* Obsolete but added for backward compatibility with old geopak dlls */
/*   Please do not add to this and keep this at the end of this file  */
/*                                                                    */
/*       Do not use anything contained below in any new code          */
/*                                                                    */
/*                    vvvvv  Begins Here  vvvvv                       */
 
#define TINOBJ DTM_TIN_OBJ
#define DATAOBJ DTM_DAT_OBJ
#define LATOBJ DTM_LAT_OBJ

#define   DTM_NTGVAL           DTM_NULL_USER_TAG
#define   DTM_NPTVAL           DTM_NULL_PNT

struct TINOBJLIST { void *Tin  ; long TinIndex ; } ;
struct DATAOBJLIST { DATAOBJ *Data ; long ListFlag ; } ;
typedef   int64_t        DTM_TAG_VAR ;

struct GpkDtmPointFeatureList{ long dtmFeature; DTMFeatureType dtmFeatureType; long priorPoint,nextPoint ; DTM_TAG_VAR userTag ; } ;

struct POLYOBJ
{
 long    NHEAD,MHEAD,HSMPTS,HIMPTS  ;
 long    NPTS,MPTS,PSMPTS,PIMPTS    ;
 void  *PLST ;
 void     *PPTS ;
} ;

struct DTM_CODE_POINT { long Pcode ; double x,y,z ; } ;
struct PADELMTAB : DPoint3d { double CutSlope,FillSlope ; long Type,SideSlopeFlag ;} ;
struct SLOPETAB { double Low,High,Slope  ; } ;

/*                      ^^^^^  Ends Here  ^^^^^                       */
/*====================================================================*/
/*                            DEPRECATED                              */
/*====================================================================*/

// From dtmevars.h
#endif

