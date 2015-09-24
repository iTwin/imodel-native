/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PublicAPI/DTMDefs.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
/*__PUBLISH_SECTION_START__*/
#ifndef __DTMDEFS_H__
#define __DTMDEFS_H__
#if ! defined (resource)

#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/IDTM.h>
#include <deque>



/*-------------------------------------------------------------------+
|                                                                    |
|   Compiler Include Files                                           |
|                                                                    |
+-------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <float.h>
#include <stdarg.h>
#include <time.h>
typedef __time32_t DTMTime32;
struct DTM_LAT_OBJ;
/*__PUBLISH_SECTION_END__*/
/*-------------------------------------------------------------------+
|                                                                    |
|   DTM Specific                                                     |
|                                                                    |
+-------------------------------------------------------------------*/
/*
** DTMDEFS for BIG_ENDIAN Machines
*/
#if defined(sparc) || defined(hp700) || defined (sgimips)
#define BIG_ENDIAN
#endif
/*
** Typedefs For DTM Variables
*/
typedef   long             DTM_FEATURE_CODE ;
/*__PUBLISH_SECTION_START__*/
enum DTMNullLong : long
    {
    DTM_NULL_PNT = 2138888888,
    DTM_NULL_PTR = 2139999999,
    };
#define   DTM_FILE_SIZE               128
/*__PUBLISH_SECTION_END__*/
/*
**  Precision Tolerances
*/
#define   DTM_NUM_DEC_DIGITS            4
#define   DTM_PPTOL                0.0001
#define   DTM_PLTOL                0.0001
#ifdef _WIN32_WCE
/*
**  Memory Allocation Variables
*/
/* HEREROB */
#define   DTM_INI_MEM_PTS			   5000
#define   DTM_INC_MEM_PTS              1000
#define   DTM_INI_MEM_FEATURES_TABLE    200
#define   DTM_INC_MEM_FEATURES_TABLE    200
#define   DTM_INI_MEM_FEATURES_LIST    1000
#define   DTM_INC_MEM_FEATURES_LIST    1000
#else
/*
**  Memory Allocation Variables
*/
#define   DTM_INI_MEM_PTS			  250000
#define   DTM_INC_MEM_PTS              50000
#define   DTM_INI_MEM_FEATURES_TABLE    1000
#define   DTM_INC_MEM_FEATURES_TABLE    1000
#define   DTM_INI_MEM_FEATURES_LIST    50000
#define   DTM_INC_MEM_FEATURES_LIST    50000
#endif // JGA _WIN32_WCE
/*--------------------------------------------------------------------------------------+
|                                                                                       |
|    TypeDefs For DTM x Attribute Partitions                                            |
|                                                                                       |
+--------------------------------------------------------------------------------------*/
enum class DTMPartition
    {
    None = 0,
    Header = 1,
    Feature = 2,
    Point   = 3,
    Node    = 4,
    CList   = 5,
    FList   = 6,
    };
/*__PUBLISH_SECTION_START__*/
/*
**  Dtm Constatnts
*/
enum class DTMDirection
    {
    Unknown = 0,
    Clockwise = 1,
    AntiClockwise = 2,
    };
enum DTMCornerOptions
    {
    DTM_MITRE_CORNER = 1,
    DTM_ROUND_CORNER = 2
    };
enum DTMAxis
    {
    DTM_X_AXIS = 0,
    DTM_Y_AXIS = 1
    };

enum class DTMClipOption
    {
    Internal = 1,
    External = 2
    };

#define   DTM_CREATE            1
#define   DTM_APPEND            2
#define   DTM_TINY              (1e-20)
#define   DTM_INFINITE          ( ( double )1.0E+38 )
/*
** TypeDefs
*/
struct DRAPELINE : DPoint3d { long LineNo,PointType ; }  ;
struct IP3D : DPoint3d { long Np,Tp ; };
struct VOLRANGETAB { double Low,High,Cut,Fill ;  } ;
struct PNTLINE { long P1,P2 ; } ;
struct DTM_INSERT_POINT : DPoint3d { long PointNumber,PointType,LineType ; }  ;
struct DTM_POINT_ARRAY { DPoint3dP pointsP ; long numPoints ; }  ;
struct DTM_PLANE { double A; double B; double C; }  ;
struct DTM_POINT_ARRAY_TILE
{
 DPoint3dP firstPntP  ;            /* Pointer To First Point In Tile */
 DPoint3dP lastPntP   ;            /* Pointer To Last  Point In Tile */
}  ;
/*__PUBLISH_SECTION_END__*/
/*
** Structure for DTMRPT.C
*/
struct LTAB { long LineNo,LineOfs,SegNo,SegOfs,Order,P1,P2 ; }  ;
struct LSTAB { long P1,P2,Ofs ; }  ;
/*
** Structure For DTMJOIN.C
*/
struct FTAB { long   FeatNo,Ofs,Ofe,Close ; DTMUserTag Usertag ; double Xs,Ys,Xe,Ye ; } ;
struct JOINEDLINES { long   Line,Direction ; }  ;
/*__PUBLISH_SECTION_START__*/
/*
** New Structure For DTMJOIN.C
** Added 17 June 2004 Rob C
*/
struct DTM_JOIN_FEATURE_TABLE { long  featureOfs,startOfs,endOfs,closeFlag ; DTMUserTag userTag ; double Xs,Ys,Zs,Xe,Ye,Ze ; } ;
struct DTM_JOIN_NODE_TABLE { long  node,featureOfs,sortOfs,direction,joinNode ; DTMUserTag userTag ; double x,y,z ; } ;
struct DTM_JOIN_LINE_TABLE { long  lineNum,featureOfs,direction ; } ;
struct DTM_JOIN_USER_TAGS { long  lineNumber,direction ; DTMUserTag userTag ; } ;
/*__PUBLISH_SECTION_END__*/
/*
** Struct For DTMNODE.C
*/
struct NODETABLE { long Line,Node,Fcode,Position,JoinNode ; DTMUserTag UserTag ; double x,y ; } ;
/*__PUBLISH_SECTION_START__*/
/*
** Define Duplicate Data Point Structure - DTMEDATA.C
*/
struct DUPTAB { long Type,Spare ; double z ; } ;
/*__PUBLISH_SECTION_END__*/
/*
** Data Structure For Cleaning Intersecting Features
*/
struct LATAB { long No,Ofs,Type ;    } ;
struct LPTAB { long P1,P2,Order,No ; } ;
struct TRTAB { long Code,No,Pos,P1,P2 ; double D,x,y,z ; } ;
/*
** Data Structure For Extract Interpolation
*/
struct BREAKTABLE { long BrkNo,Ofs,Ofe,Close ; double Xs,Ys,Xe,Ye ; } ;
/*__PUBLISH_SECTION_START__*/
/*
** Define Data Edit Structures
*/
struct INTAB
{
long   Lno1,Type1,P11,P12 ;
double Z1,D1 ;
DPoint3d    Line1[2] ;
long   Lno2,Type2,P21,P22 ;
double Z2,D2 ;
DPoint3d    Line2[2] ;
double Xi,Yi    ;
} ;
/*__PUBLISH_SECTION_END__*/
/*
** Typedefs For Drainage Tools
*/
struct DTM_TRG_INDEX_TABLE { long index,trgPnt1,trgPnt2,trgPnt3,voidTriangle,flatTriangle ; } ;
struct DTM_TRG_HYD_TABLE { long flowPnt,flowDir1,flowDir2,flowDir3 ; double ascentAngle,descentAngle,slope ;} ;
struct DTM_STREAM_TRACE_POINTS : DPoint3d { long P1,P2 ; };
struct DTM_LOW_POINT_POND_TABLE { long lowPoint,exitPoint,priorPoint,nextPoint ; } ;
struct DTM_SUMP_LINE_POND_TABLE { long sP1,sP2,exitPoint,priorPoint,nextPoint  ; } ;
struct DTM_SUMP_LINES  { long sP1,sP2 ; } ;
struct DTM_SUMPS { long line,sPnt ; } ;
struct DTM_CATCHMENT_POLYGON { long Pn,Tp,Tag,Left,Right ; double x,y,z ; } ;
struct DTM_RIDGE_PTS { long Tp,P1,P2,P3 ; } ;
/*
** Typedef For Line Of Sight
*/
struct DTM_HORIZON_LINE { double Ang1,Ang2,D1,X1,Y1,Z1,D2,X2,Y2,Z2;long ActiveFlag ; } ;
struct DTM_HORIZON_DISTANCE { double Dist  ; long HlineOfs ; } ;
struct DTM_HORIZON_LINE_INDEX { double Angle ; long Hofs,Htype,Hlist,Nhlist ; } ;
/*__PUBLISH_SECTION_START__*/
/*
** Typedefs For Points With Features And UserTags
*/
struct FEATTAG { DTMFeatureType DtmFeature ; DTMUserTag UserTag ; } ;
struct P3DTAG : DPoint3d { long NumberOfTags ; FEATTAG *Tags ; } ;
/*__PUBLISH_SECTION_END__*/
/*
**  Data Structure For DTMFeatureState::Tin Data
*/
struct DTM_TIN_POINT : DPoint3d { }  ;
struct DTM_TIN_NODE {  unsigned short  PRGN,PCWD ;long hPtr,cPtr,tPtr,sPtr,fPtr ; } ;
/*
**  Data Structure For Circular List
*/
struct DTM_CIR_LIST { long  pntNum,nextPtr ; } ;
/*
**  Data Structure For DTM Feature List
*/
struct DTM_FEATURE_LIST_VER200 { long  nextPnt,dtmFeature,nextPtr ; }  ;
struct DTM_FEATURE_LIST { long  nextPnt,dtmFeature,nextPtr,pntType ; } ;
/*-------------------------------------------------------------------+
|                                                                    |
|   Definitions For Lattice Objects                                  |
|                                                                    |
+-------------------------------------------------------------------*/
/*
** Define Varible For MAXIMUM NumBer Of Lattice Objects
*/
#define  DTM_MAX_LAT_OBJS 256
/*
** Header Record For LAT Files
*/
struct DTM_LAT_OBJ
{
 long    dtmFileType,dtmFileVersion ;
 long    INTMODE,NXL,NYL,NOLATPTS,NOACTPTS  ;
 long    L1,L2,L3,L4,L5,L6,L7 ;
 float   NULLVAL,FS1 ;
 double  DX,DY,LXMIN,LYMIN,LZMIN,LXMAX,LYMAX,LZMAX,LXDIF,LYDIF,LZDIF ;
 double  D1,D2,D3,D4,D5,D6,D7,D8 ;
 char    userName[DTM_FILE_SIZE] ;
 char    LatticeObjectFile[DTM_FILE_SIZE] ;
 float   *LAT ;
};
/*-------------------------------------------------------------------+
|                                                                    |
|   Definitions For Polygon Objects                                  |
|                                                                    |
+-------------------------------------------------------------------*/
/*__PUBLISH_SECTION_START__*/
/*
** Define Varible For Maximum Number Of Polygon Objects
*/
#define  DTM_MAX_POLYGON_OBJS 256
/*
**  Data Structure For Polygon Header List
*/
struct DTM_POLYGON_LIST { double area,perimeter,d1 ; long  firstPnt,lastPnt,direction,userTag,s1 ; } ;
/*
** Header Record For Polygon Objects
*/
struct DTM_POLYGON_OBJ
{
 long    numPolygons,memPolygons,iniMemPolygons,incMemPolygons  ;
 long    numPolyPts,memPolyPts,iniMemPolyPts,incMemPolyPts    ;
 DTM_POLYGON_LIST  *polyListP ;
 DPoint3dP polyPtsP ;
} ;
/*-------------------------------------------------------------------+
|                                                                    |
|   Definitions For TAG Objects                                      |
|                                                                    |
+-------------------------------------------------------------------*/
/*
** Define Varible For Maximum Number Of Tag Objects
*/
#define  MAXTAOBJ 256
/*
** Typedef Structure For Tag Header List
*/
struct TAGLIST {long FTAG,LTAG,UTAG[4];} ;
/*
**  Typedef Structure For Tag Objects
*/
struct TAGOBJ
{
 long     NTAG,MTAG,SMTAG,IMTAG  ;
 long     NVAL,MVAL,SMVAL,IMVAL  ;
 TAGLIST  *PTAG ;
 long     *PVAL ;
};
/*-------------------------------------------------------------------+
|                                                                    |
|   Definitions For List Objects                                     |
|                                                                    |
+-------------------------------------------------------------------*/
/*
** Define Varible For Maximum Number Of List Objects
*/
#define  MAXLSTOBJ 256
/*
** Header Record For List Objects
*/
struct LSTOBJ
{
 long   Tag,CloseFlag,Direction,numPts,memPts,IntPts,IncPts ;
 double Area,Xmin,Ymin,Zmin,Xmax,Ymax,Zmax ;
 long   *Plist ;
};
/*
** Data Structure For Sets Of Data Object
*/
struct BC_DTM_OBJLIST { BC_DTM_OBJ *Tin  ; long TinIndex ; } ;
/*-------------------------------------------------------------------+
|                                                                    |
|  Typedefs For ImagePP                                              |
|                                                                    |
+-------------------------------------------------------------------*/
typedef        int (__stdcall *ImagePPCallBack)(long numPixels,char *nameP,char *descP,char *projectionP, void *baseCordP, void *userP ) ;
/*-------------------------------------------------------------------+
|                                                                    |
|  Typedefs For 3D Model Applications                                |
|                                                                    |
+-------------------------------------------------------------------*/
struct MODEL_RADIAL
{
int			radialIndex;	// Numeric index of radial - used in managing data
DTMUserTag	fromId;			// The element id of the begin point
DTMUserTag	toId;			// The element id of the end point
void*	fromDgnModelRefP;	// => Model ref of beginning point
void*	toDgnModelRefP;	// => Model ref of ending point
int	       numberOfPoints;		// Number of radial/pattern points - 2 or 3
DPoint3d		point[3];		// radial/pattern point on line 0
                            // [0] is beginning point (__BEGIN_RADIAL_POINT)
                            // [1] is ending point (__END_RADIAL_POINT)
                            // [2] is a extended point out to the hull perhaps (__EXTENDED_RADIAL_POINT)
double		surfaceSlope;	// Intrinsic pavement slope - tangent slope of surface preceding beginning point
double		distanceAlong;
int         sideLtRt;   // Left side of shape = 0;   Right side = 1;
double		station;		// Station along chain
int			region;			// Region along chain
double		absoluteStation;	// Absolute station
DPoint3d			pglPoint;			// PGL point
int			numberOfPavementPoints; // Number of pavement points
DPoint3d			*pavementPointsP;		// Pavement points
} ;
struct TRUNCATED_RADIAL { long radialIndex,truncatingRadialIndex,numberOfPoints ; DPoint3dP radialPoints ;} ;
struct INTERSECT_POINT { long Line,Radial,RadialLine,Active ; double x,y,z,L ; } ;
struct INTERSECT_LINE { long UserRadial,Line,Radial,RadialLine,Active,Direction ; double  X1,Y1,Z1,X2,Y2,Z2 ; INTERSECT_POINT *IntersectPoint ; long NumberOfIntersectPoints ; } ;
struct INTERSECT_QUEUE { long Offset,Line ; double  X1,Y1,Z1,X2,Y2,Z2 ; } ;
struct RADIAL_TABLE { long Status,StartLine,EndLine,NoIntPts ; } ;
struct VOLTININDEX { BC_DTM_OBJ *Tin ; double z ; DTMUserTag Index ; } ;
struct TAG_POLYGON { long PolyId,IntToPoly,NumPolyPts,NumTinIndexes,NumIntPolys ; double IntPointX,IntPointY,PolyArea ; DPoint3dP PolyPts ; VOLTININDEX *TinIndexes ; long  *IntPolys ; } ;
struct MODEL_VOLUME { DTMUserTag BottomIndex,TopIndex ; double Cut,Fill,Area ; } ;
/*-------------------------------------------------------------------+
|                                                                    |
|  Typdefs For Importing Macao Tins                                  |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_MACAO_TRGS { long Tp1,Tp2,Tp3,Activity ; };
struct DTM_MACAO_BRKS { long Bp1,Bp2 ; };
/*-------------------------------------------------------------------+
|                                                                    |
|  Typedefs For V8 Draping Function                                  |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_DRAPE_FEATURE { DTMFeatureType  dtmFeatureType ; DTMUserTag userTag ; DTMFeatureId userFeatureId ; } ;
struct DTM_DRAPE_DATA { long DrapeType,DrapeLine,numFeatureTable ; double x,y,z ; DTM_DRAPE_FEATURE *Features ; } ;
/*-------------------------------------------------------------------+
|                                                                    |
|  Typedefs For Cleaning Functions                                   |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_P3D_LINE_STRING { DPoint3dP stringPts ; long numStringPts ; }  ;
struct DTM_STR_INT_TAB { long String,Segment,Type,Direction ; double X1,Y1,Z1,X2,Y2,Z2 ; }  ;
struct DTM_STR_INT_PTS { long String1,Segment1,String2,Segment2 ; double Distance,x,y,z,Z2 ; } ;
/*-------------------------------------------------------------------+
|                                                                    |
|  Typedefs For Side Slopes                                          |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_SLOPE_TABLE { double Low,High,Slope  ; } ;
struct DTM_OVERLAP_RADIAL_TABLE { long Ofs,Type,Status,TruncatingRadial,TruncatingEdge ; double Px,Py,Pz,Gx,Gy,Gz,Nx,Ny,Nz,EdgeZ,Tx,Ty,Tz ; } ;
struct DTM_SIDE_SLOPE_TABLE
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
BC_DTM_OBJ  *slopeToTin;       // => To tin object if slope option = to tin NULL otherwise
double  toElev;                 // => To constant elevation if slope option is to elevation
double  toDeltaElev;            // => To delta elevation if slope option is to delta elevation
double  toHorizOffset;          // => To horizontal offset if slope option is to hor distance
double  cutSlope;               // => Cut Slope - unit per unit
double  fillSlope;              // => Fill Slope - unit per unit
int     cutFillOption ;         // => Option to indicate whether to process in 0-CUTANDFILL,1-CUTONLY, 2-FILLONLY,
BC_DTM_OBJ *cutFillTin ;           // => Cut fill tin object if isCutFillOption == CUTONLY or FILLONLY
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
| Point Filter Table                                                 |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_POINT_FILTER_TABLE : DPoint3d { long Utag,Next,Prior ; } ;
/*-------------------------------------------------------------------+
|                                                                    |
|  User Tag List                                                     |
|                                                                    |
+-------------------------------------------------------------------*/
struct GpkDtmUserTagList { DTMUserTag userTag ; long list00,list01,list02,list03 ; } ;
/*-------------------------------------------------------------------+
|                                                                    |
|  Hull Point List                                                   |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_TIN_POINT_FEATURES { long dtmFeature; DTMFeatureType dtmFeatureType; long priorPoint,nextPoint ; DTMUserTag userTag ; DTMFeatureId userFeatureId ; } ;
/*__PUBLISH_SECTION_END__*/
/*-------------------------------------------------------------------+
|                                                                    |
|  Contour Index                                                     |
|                                                                    |
+-------------------------------------------------------------------*/
struct DtmContourIndex { long p1,p2 ; } ;       // Athens Version
#include <TerrainModel\Core\partitionarray.h>
typedef PartitionArray<DtmContourIndex, 15, MAllocAllocator> DtmContourIndexArray;
typedef DtmContourIndexArray& DtmContourIndexArrayR;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
struct DtmContourNodeTable { long node,nodePt,dtmFeature,isReversed,connectNode,flag ; double contourValue ; DtmContourNodeTable *joinNode ; } ;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
struct ArcPts { int64_t Fl;long Fc ; double x,y,z,Angle ; } ;
/*__PUBLISH_SECTION_START__*/
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_POINT_ARRAY_TILES
{
 DPoint3dP firstPntP  ;            /* Pointer To First Point In Tile */
 DPoint3dP lastPntP   ;            /* Pointer To Last  Point In Tile */
} ;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
/*
** Line String Interpolation Structures
*/
struct  DTM_INDEXED_LINE_STRING { DPoint3dP pointsP ; long numPts, stringIndex ; }  ;
struct  DTM_INDEXED_POINT { DPoint3d point ; long pointIndex  ; }  ;
struct  DTM_INTERPOLATED_POINT { DPoint3d point ; long stringIndex, pointInterpolated, pointIndex ; double pointDistance ; }  ;
struct  DTM_INTERPOLATED_LINE_STRING{ DTM_INTERPOLATED_POINT *pointsP ; long numPts,closeFlag,interpolated ; } ;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
/*
** Line String Join Structures
*/
struct DTM_JOIN_LINE_STRING { long  featureOfs,startOfs,endOfs,closeFlag,stringIndex ; double xs,ys,zs,xe,ye,ze ; } ;
struct DTM_JOIN_LINE_STRING_NODE { long  node,featureOfs,sortOfs,direction,joinNode,stringIndex ; double x,y,z ; } ;
struct DTM_JOIN_LINE_STRING_LINE { long  lineNum,lineOfs,featureOfs,direction ; } ;
/*-------------------------------------------------------------------+
|                                                                    |
|  Structure For Dtm Features                                        |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_FEATURE
{
 DTMFeatureType dtmFeatureType ;      /* Dtm Feature Type          */
 DTMUserTag   userTag        ;      /* User Tag                  */
 DTMFeatureId userFeatureId  ;      /* Unique Identifier         */
 DPoint3dP    pointsP       ;      /* Pointer To Feature Points */
 long         numPoints      ;      /* Number Of Feature Points  */
} ;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_DRAPE_POINT
{
 long   drapeLine ;                        /* Segment Number Of User Drape String            */
 DTMDrapedLineCode drapeType;              /* Type Of Drape Point
                                              ==  0  Drape Point External To Tin
                                              ==  1  Drape Point Internal To Tin ( Triangle )
                                              ==  2  Drape Point On Break Line
                                              ==  3  Drape Point On Break Triangle
                                              ==  4  Drape Point In Void
                                              ==  5  Drape Point On Tin Point
                                              ==  6  Drape Point In Tin Line                  */
 long   numDrapeFeatures ;                 /* Number Of Dtm Features At Drape Point           */
 double drapeX ;                           /* x Coordinate Of Drape Point                     */
 double drapeY ;                           /* y Coordinate Of Drape Point                     */
 double drapeZ ;                           /* z Coordinate Of Drape Point                     */
 DTM_TIN_POINT_FEATURES *drapeFeaturesP ;  /* Pointer To DTM Features At Drape Point          */
} ;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_ASCENT_LINE
{
 long   ascentType ;                        /* Ridge Line <1> Triangle <2>                        */
 long   pnt1 ;                              /* Point Ascent Starts From                           */
 long   pnt2 ;                              /* Next Point If Ridge Or Base Point If Triangle      */
 long   pnt3 ;                              /* Other Base Point If Triangle                       */
 long   flowSide ;                          /* Flow Direction From Ascent Line <1> Right <2> Left */
 double slope ;                             /* Slope Of Ascent Line                               */
 double ascentAngle ;                       /* Angle Of Ascent Line                               */
 double x ;                                 /* x Coordinate Of Start Trace                        */
 double y ;                                 /* y Coordinate Of Start Trace                        */
 double z ;                                 /* z Coordinate Of Start Trace                        */
} ;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_DESCENT_LINE
{
 long   descentType ;                       /* Sump Line <1> Triangle <2>                          */
 long   pnt1 ;                              /* Point Descent Starts From                           */
 long   pnt2 ;                              /* Next Point If Sump Or Base Point If Triangle        */
 long   pnt3 ;                              /* Other Base Point If Triangle                        */
 long   flowSide ;                          /* Flow Direction From Descent Line <1> Right <2> Left */
 double slope ;                             /* Slope Of Descent Line                               */
 double descentAngle ;                      /* Angle Of Descent Line                               */
 double x ;                                 /* x Coordinate Of Start Trace                         */
 double y ;                                 /* y Coordinate Of Start Trace                         */
 double z ;                                 /* z Coordinate Of Start Trace                         */
} ;
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_DRAINAGE_FLOW_LINE
{
/*
**  Flow Type   == 1  Sump Line           ** Both Sides Flow To Line
**              == 2  Partial Sump Line   ** Flows To On One Side. The Other Side Is A Hull Line ,Zero Slope Triangle or Flows Parallel To Line **
**              == 3  Ridge Line          ** Both Sides Flow Away From Line
**              == 4  Partial Ridge Line  ** Flows Away On One Side. The Other Side Is A Hull Line ,Zero Slope Triangle or Flows Parallel To Line **
**              == 5  Descent Triangle
**              == 6  Ascent Triangle
**
**  Flow Slope  Is Absolute Slope         Sump  Line Slope <= 0.0
**                                        Ridge Line Slope >= 0.0
**                                        Desecnt Triangle Slope < 0.0
**                                        Asecnt  Triangle Slope > 0.0
**
*/
 long   flowType ;                          /* Sump Line <1,2> Ridge Line <3,4>  Triangle <5,6>     */
 long   pnt1 ;                              /* Point Descent Starts From                            */
 long   pnt2 ;                              /* Next Point If Sump, Ridge Or Base Point If Triangle  */
 long   pnt3 ;                              /* Other Base Point If Triangle                         */
 double flowSlope ;                         /* Absolute Slope Of Flow Line                          */
 double flowAngle ;                         /* Angle Of Flow Line                                   */
 double x ;                                 /* x Coordinate Of Start Trace                          */
 double y ;                                 /* y Coordinate Of Start Trace                          */
 double z ;                                 /* z Coordinate Of Start Trace                          */
} ;
/*-------------------------------------------------------------------+
|                                                                    |
|  Structures For Connecting Line Strings                            |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_CONNECTION_LINE_INTERSECT
{
 long  point1 ;                                // Connect Point 1 - Offset To DTM_CONNECT_POINTS Table
 long  point2 ;                                // Connect Point 2 - Offset To DTM_CONNECT_POINTS Table
 long  index ;                                 // Index To Mark Intersected Connections Lines
} ;              // Structure For Intersecting Connection Lines
struct DTM_CONNECTION_LINE
{
 long   fromPoint ;                            // Connects From This Point
 long   toPoint;                               // Connects To This Point - Offset To DTM_CONNECT_POINTS Table
 long   line ;                                 // Connects To This Line  - Offset To DTM_CONNECT_LINE Table
 double distance ;                             // Connection Distance
 long   isMarked ;                             // Connection Has been Marked By A Prior connection And Can Not Be Used
 DTM_CONNECTION_LINE_INTERSECT *intConLineP ;  // Pointer To Array Of Intersecting Connection Lines
 long  numIntConLine ;                         // Number Of Intersected Connection Lines
} ;                        // Structure For Connection Lines Between String End Points
struct DTM_CONNECT_LINE
{
 long   line ;                                 // Line Number
 long   point1 ;                               // StartPoint Number Of Line
 long   point2 ;                               // End Point Number Of Line
 long   isReversed ;                           // Line Reversed <0=No,1=Yes>
 double length ;                               // Line Length
 } ;                         // Structure For Strings To Be Connected
struct DTM_CONNECT_POINT
{
 long line ;                                   // Line  - Offset To DTM_CONNECT_LINE
 long point1 ;                                 // Start Point Number For Line - Offset In DTM_CONNECT_POINT
 long point2 ;                                 // End Point Number For Line - Offset In DTM_CONNECT_POINT
 long isReversed ;                             // Line Reversed <0=No,1=Yes>
 long numConLine ;                             // Number Of Connection Lines To point1
 DTM_CONNECTION_LINE *conLineP ;               // Pointer To Array Of Connection Lines
} ;                          // Structure For Strings End Points To Be Connected
struct DTM_CONNECTED_STRING
{
 long  stringOffset  ;                          // String Offset In Array Of DTM_POINT_ARRAY
 long  isReversed   ;                           // String Reversed <0=No,1=Yes>
} ;                        // Structure To Store Connected String
struct DTM_CONNECT_INPUT_STRING_ERROR
 {
  long errorType ;                              // Error Type <1,2,3,4,5>
                                                // 1 = Closed String
                                                // 2 = Zero length String
                                                // 3 = Knot In String
                                                // 4 = Intersecting Strings
                                                // 5 = More Than Two Strings With A Coincident End Point
  long string1Offset ;                          // String One Offset In Array Of DTM_POINT_ARRAY
  long segment1Offset ;                         // Segment Offset Of String One
  long string2Offset ;                          // String Two Offset In Array Of DTM_POINT_ARRAY
  long segment2Offset ;                         // Segment Offset Of String Two
  double x ;                                    // x Coordinate Of Error
  double y ;                                    // y Coordinate Of Error
  double z ;                                    // z Coordinate Of Error
} ;              // Structure To Report Validation Errors With Strings To Be Connected
/*-------------------------------------------------------------------+
|                                                                    |
|  Structures For DTM String purposes                                |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTM_INTERSECT_POINT
 {
  long string1Offset ;         // String One Offset
  long segment1Offset ;        // Segment Offset Of String One
  long string2Offset ;         // String Two Offset
  long segment2Offset ;        // Segment Offset Of String Two
  double distance ;            // Distance Of Intersect Point From Start Of Segment One Offset
  double x ;                   // x Coordinate Of Intersect Point
  double y ;                   // y Coordinate Of Intersect Point
  double zSegment1 ;           // z Coordinate Of Intersect Point On Segment One
  double zSegment2 ;           // z Coordinate Of Intersect Point On Segment Two
} ;
struct DTM_STRING_INTERSECT_TABLE
{
 long string ;                 //  String Offset
 long segment ;                //  Segment Offset From Start Of String
 long numSegments ;            //  Number Of Segments In String
 long isReversed ;             //  Segment Is Reversed <0,1>
 double X1 ;                   //  x Coordinate Of Segment Start
 double Y1 ;                   //  y Coordinate Of Segment Start
 double Z1 ;                   //  z Coordinate Of Segment Start
 double X2 ;                   //  x Coordinate Of segment End
 double Y2 ;                   //  y Coordinate Of Segment End
 double Z2 ;                   //  z Coordinate Of Segment End
} ;
/*__PUBLISH_SECTION_END__*/
/*********************************************************************************************
**
**  Bentley Civil DTM DEFINITIONS
**
**  Author : Rob Cormack
**  Date   : 11/01/2007
**  Email  : Rob.Cormack@bentley.com
**
*********************************************************************************************/
#define  BC_DTM_MAX_OBJS         4096
/*
**  BC DTM Object Types
*/
enum DTMObjectTypes : long
    {
#ifdef XM
    // XM Types.
    DTM_DAT_TYPE = 0x47504b45,
    DTM_TIN_TYPE = 0x47504b54,
    DTM_LAT_TYPE = 0x47504b4c,
#endif
    // New Types
    BC_DTM_OBJ_TYPE = 0x4344544d,  //  CDTM
    BC_DTM_ELM_TYPE = 0x4544544d,  //  EDTM
    BC_DTM_MRES_TYPE = 0x4d44544d,  //  MDTM
    BC_DTMFeatureType = 0x4644544d,  //  FDTM
    };
/*
** BC DTM Version Control
*/
enum DTMVersionControl : long
    {
#ifdef XM
    // XM Versions
    DTM_DAT_FILE_VERSION = 502,
    DTM_TIN_FILE_VERSION = 501,
    DTM_LAT_FILE_VERSION  = 500,
#endif
    BC_DTM_OBJ_VERSION_100 = -100,             // First Alpha Version
    BC_DTM_OBJ_VERSION_200 = -200,             // Next Alpha  Version
    BC_DTM_OBJ_VERSION = 100,             // First Release Version
    BC_DTM_MRES_VERSION = -100,             // First Alpha Version
    BC_DTM_FEATURE_VERSION = -100,             // First Alpha Version
    };
/*
** BC DTM ARRAY PARTITIONING
*/
enum DTMPartionInformation
    {
    DTM_PARTITION_SHIFT_FEATURE = 11,     // Shift for An Internal Feature Partition
    DTM_PARTITION_SHIFT_POINT = 15,     // Shift for An Internal Point Partition
    DTM_PARTITION_SHIFT_NODE = 15,     // Shift for An Internal Node  Partition
    DTM_PARTITION_SHIFT_CLIST = 15,     // Shift for An Internal Clist Partition
    DTM_PARTITION_SHIFT_FLIST = 15,     // Shift for An Internal Flist Partition

    DTM_PARTITION_SIZE_FEATURE = (1 << DTM_PARTITION_SHIFT_FEATURE),   // Size Of An Internal Feature Partition
    DTM_PARTITION_SIZE_POINT = (1 << DTM_PARTITION_SHIFT_POINT),   // Size Of An Internal Point Partition
    DTM_PARTITION_SIZE_NODE = (1 << DTM_PARTITION_SHIFT_NODE),   // Size Of An Internal Node  Partition
    DTM_PARTITION_SIZE_CLIST = (1 << DTM_PARTITION_SHIFT_CLIST),   // Size Of An Internal Clist Partition
    DTM_PARTITION_SIZE_FLIST = (1 << DTM_PARTITION_SHIFT_FLIST),   // Size Of An Internal Flist Partition
    };
#define BC_DTM_MAX_SPOT_POINTS              10000   // Maximum Number Of Spots Points For An Internal Dtm Feature
/* HEREROB */
#ifdef _WIN32_WCE
/*
** BC MEMORY ALLOCATION PARAMETERS
*/
#define BC_DTM_INI_FEATURES           200           // Initial Feature Allocation For BC_DAT_OBJ
#define BC_DTM_INC_FEATURES           200           // Increment Feature Allocation For BC_DAT_OBJ
#define BC_DTM_INI_POINTS            5000           // Initial Points Allocation For A BC Object
#define BC_DTM_INC_POINTS            1000           // Increment Points Allocation For A BC Object
#define BC_DTM_INI_FLIST              200           // Initial Points Allocation For A BC Object
#define BC_DTM_INC_FLIST              200           // Increment Points Allocation For A BC Object
#else
/*
** BC MEMORY ALLOCATION PARAMETERS
*/
#define BC_DTM_INI_FEATURES          1000           // Initial Feature Allocation For BC_DAT_OBJ
#define BC_DTM_INC_FEATURES          1000           // Increment Feature Allocation For BC_DAT_OBJ
#define BC_DTM_INI_POINTS          100000           // Initial Points Allocation For A BC Object
#define BC_DTM_INC_POINTS           10000           // Increment Points Allocation For A BC Object
#define BC_DTM_INI_FLIST            10000           // Initial Points Allocation For A BC Object
#define BC_DTM_INC_FLIST            10000           // Increment Points Allocation For A BC Object
#endif  // JGA _WIN32_WCE
/*__PUBLISH_SECTION_START__*/
struct BC_DTM_USER_FEATURE
{
 DTMFeatureType    dtmFeatureType    ;    // Type Of Dtm Feature
 DTMUserTag        dtmUserTag        ;    // Dtm Feature User Tag
 DTMFeatureId      dtmFeatureId      ;    // Dtm Feature Identifier
 DPoint3dP         dtmFeaturePtsP   ;    // Pointer To Array Of Dtm Feature Points
 long                numDtmFeaturePts  ;    // Number Of Dtm Feature Points
} ;
/*----------------------------------------------------------------------------------+
|                                                                                   |
|  The Value Of "dtmFeatureState" Implies The Method For The Union "dtmFeaturePts"  |
|                                                                                   |
|  dtmFeatureState =  0   Memory Initialisation Value - For Unused Alloacted Memory |
|                  =  1   First Point Of Feature In Dtm Points Array                |
|                  =  2   Pointer To DPoint3d Array Of Coordinates                       |
|                  =  3   Pointer To Array Of Point Numbers In DTM Points Array     |
|                  =  4   First Point Of Feature In Tin                             |
|                  =  5   Pointer To DPoint3d Array Of Coordinates After                 |
|                         Detecting Error While Inserting Feature Into Tin          |
|                  =  6   Feature Deleted                                           |
|                  =  7   Roll Back Feature                                         |                                           |
|                                                                                   |
+----------------------------------------------------------------------------------*/
enum class DTMFeatureState : long
    {
    Unused = 0,
    Data = 1,
    PointsArray = 2,
    OffsetsArray = 3,
    Tin = 4,
    TinError = 5,
    Deleted = 6,
    Rollback = 7,
    };
#ifdef _M_IX86
typedef long  DTMMemPnt ;
#else
typedef int64_t DTMMemPnt ;
#endif
/*__PUBLISH_SECTION_END__*/
struct BC_DTM_FEATURE_VER100
{
 DTMFeatureState dtmFeatureState       ;    // Dtm Feature State - Use Of The dtmFeaturePts Union
 long            internalToDtmFeature  ;    // This Feature Internal To Feature
 DTMFeatureType  dtmFeatureType        ;    // Type Of Dtm Feature
 DTMUserTag      dtmUserTag            ;    // Dtm Feature User Tag
 DTMFeatureId    dtmFeatureId          ;    // Dtm Feature Identifier
 union
   {
    int  pointsPI  ; // For dtmFeatureState = <2,5>  Pointer To DPoint3d Array Of Points
    int offsetPI  ; // For dtmFeatureState = <3>    Pointer To Array Of Point Offsets
    long firstPoint ; // For dtmFeatureState = <1,4>  Offset To First Point Of Feature
   }                 dtmFeaturePts         ;    // Reference To Feature Points - Dependent On Feature State
 long                numDtmFeaturePts      ;    // Number Of Dtm Feature Points
} ;
struct BC_DTM_FEATURE
{
 DTMFeatureState dtmFeatureState       ;    // Dtm Feature State - Use Of The dtmFeaturePts Union
 long            internalToDtmFeature  ;    // This Feature Internal To Feature
 DTMFeatureType  dtmFeatureType        ;    // Type Of Dtm Feature
 long            numDtmFeaturePts      ;    // Number Of Dtm Feature Points
 DTMUserTag      dtmUserTag            ;    // Dtm Feature User Tag
 DTMFeatureId    dtmFeatureId          ;    // Dtm Feature Identifier
 union
   {
    DTMMemPnt  pointsPI  ; // For dtmFeatureState = <2,5>  Pointer To DPoint3d Array Of Points
    DTMMemPnt  offsetPI  ; // For dtmFeatureState = <3>    Pointer To Array Of Point Offsets
    long firstPoint ; // For dtmFeatureState = <1,4>  Offset To First Point Of Feature
    int64_t _64bitPad ; // Padding for 32 bit. To match 64 bit.
   } dtmFeaturePts;    // Reference To Feature Points - Dependent On Feature State
} ;
enum class DTMAccessMode
    {
    None          = 0,
    NodesOnly    = 1,
    Temporary     = 2,
    Write  = 3,
    Commit        = 4,
    };
struct BC_DTM_OBJ_VER_100
{
 DTMObjectTypes  dtmObjType;
 DTMVersionControl dtmObjVersion ;
 long         numLines,numTriangles ;
 long         numFeatures,memFeatures,iniFeatures,incFeatures  ;
 long         numPoints,memPoints,iniPoints,incPoints,numSortedPoints  ;
 long         numNodes,memNodes  ;
 long         numClist,memClist  ;
 long         numFlist,memFlist,iniFlist,incFlist  ;
 long         numFeaturePartitions,featurePartitionSize ;
 long         numPointPartitions,pointPartitionSize ;
 long         numNodePartitions,nodePartitionSize   ;
 long         numClistPartitions,clistPartitionSize ;
 long         numFlistPartitions,flistPartitionSize ;
 long         hullPoint,nextHullPoint ;
 long         cListPtr,cListDelPtr,fListDelPtr ;
 DTMState     dtmState   ;  // <0=Features Only,1=Points Sorted,2=Duplicates Removed,3=Tin >
 long         nullPtr,nullPnt ;
 DTMUserTag   nullUserTag ;
 DTMFeatureId nullFeatureId    ;
 long         refCount,userStatus ;
 DTMTime32     creationTime,modifiedTime,userTime ;
 double       ppTol,plTol,mppTol ;
 double       xMin,yMin,zMin,xMax,yMax,zMax ;
 double       xRange,yRange,zRange ;
 BC_DTM_FEATURE    **fTablePP  ;
 DTM_TIN_POINT     **pointsPP  ;
 DTM_TIN_NODE      **nodesPP   ;
 DTM_CIR_LIST      **cListPP   ;
 DTM_FEATURE_LIST_VER200  **fListPP   ;
} ;
struct IDTMElementMemoryAllocator;
struct BC_DTM_OBJ_VER_200
{
 DTMObjectTypes  dtmObjType;
 DTMVersionControl dtmObjVersion ;
 long         numLines,numTriangles ;
 long         numFeatures,memFeatures,iniFeatures,incFeatures  ;
 long         numPoints,memPoints,iniPoints,incPoints,numSortedPoints  ;
 long         numNodes,memNodes  ;
 long         numClist,memClist  ;
 long         numFlist,memFlist,iniFlist,incFlist  ;
 long         numFeaturePartitions,featurePartitionSize ;
 long         numPointPartitions,pointPartitionSize ;
 long         numNodePartitions,nodePartitionSize   ;
 long         numClistPartitions,clistPartitionSize ;
 long         numFlistPartitions,flistPartitionSize ;
 long         hullPoint,nextHullPoint ;
 long         cListPtr,cListDelPtr,fListDelPtr ;
 DTMState     dtmState   ;  // <0=Features Only,1=Points Sorted,2=Duplicates Removed,3=Tin >
 long         nullPtr,nullPnt,edgeOption ;
 long         dtmFeatureIndex ;
 DTMUserTag   nullUserTag ;
 DTMFeatureId nullFeatureId    ;
 long         refCount,userStatus ;
 DTMTime32 creationTime,modifiedTime,userTime ;
 double       ppTol,plTol,mppTol,maxSide,transMatrix[4][4] ;
 double       xMin,yMin,zMin,xMax,yMax,zMax ;
 double       xRange,yRange,zRange ;
 BC_DTM_FEATURE    **fTablePP  ;
 DTM_TIN_POINT     **pointsPP  ;
 DTM_TIN_NODE      **nodesPP   ;
 DTM_CIR_LIST      **cListPP   ;
 DTM_FEATURE_LIST_VER200  **fListPP   ;
 // For now the stuff below DTMAllocationClass isn't saved as part of the header.
 IDTMElementMemoryAllocator* DTMAllocationClass;
};
#define DTMIOHeaderSize_VER200  offsetof(BC_DTM_OBJ_VER_200, DTMAllocationClass)
struct DTM_ROLLBACK_DATA;

struct BC_DTM_OBJ_EXTENDED 
{
  int         (*triangulationCheckStopCallBackP)(DTMFeatureType  dtmFeatureType) ;
  struct DTM_ROLLBACK_DATA* rollBackInfoP;
  int         (*progressCallBackP)(WCharCP  string, int pos, int max, void* userP) ;
  void*       progressCallBackUserP;
};

struct BcDTMAppData
    {
    //! Destructor, overridden by subclasses if needed to free resources.
    virtual ~BcDTMAppData () {}

    //! A unique Key to identify each subclass of BcDTMAppData.
    struct Key : DTMAppDataKey {};

    //! Override this method to be notified when host DgnAttachment DgnAttachmentAppData is about to be deleted from memory.
    virtual void _OnCleanup (BC_DTM_OBJ& host)
        {
        delete this;
        }
    };


struct BC_DTM_OBJ
{
 typedef DTMAppDataList<BcDTMAppData, BcDTMAppData::Key, BC_DTM_OBJ&> DTMExtendedAppDataCollection;

 DTMObjectTypes  dtmObjType;
 DTMVersionControl dtmObjVersion ;
 long            numLines,numTriangles ;
 long            numFeatures,memFeatures,iniFeatures,incFeatures  ;
 long            numPoints,memPoints,iniPoints,incPoints,numSortedPoints  ;
 long            numNodes,memNodes  ;
 long            numClist,memClist  ;
 long            numFlist,memFlist,iniFlist,incFlist  ;
 long            numFeaturePartitions,featurePartitionSize ;
 long            numPointPartitions,pointPartitionSize ;
 long            numNodePartitions,nodePartitionSize   ;
 long            numClistPartitions,clistPartitionSize ;
 long            numFlistPartitions,flistPartitionSize ;
 long            hullPoint,nextHullPoint ;
 long            cListPtr,cListDelPtr,fListDelPtr ;
 DTMState        dtmState;  // <0=Features Only,1=Points Sorted,2=Duplicates Removed,3=Tin >
 DTMCleanupFlags dtmCleanUp ; // BitFlags 1 = Clean up features, 2 = Cleanup voids and islands
 short           obsolete_dtmRestoreTriangles ;
 long            nullPtr,nullPnt,edgeOption ;
 long            dtmFeatureIndex ;
 DTMUserTag      nullUserTag ;
 DTMFeatureId    nullFeatureId    ;
 long            userStatus ;
 DTMTime32       creationTime,modifiedTime,userTime ;
 double          ppTol,plTol,mppTol,maxSide ;
 double          xMin,yMin,zMin,xMax,yMax,zMax,xRange,yRange,zRange ;
 int64_t           lastModifiedTime;
 unsigned char   futureUse [12280];
 // The Following Are Not Saved As Part Of The Header
 long            refCount ;
 BC_DTM_OBJ_EXTENDED* extended;
// void              *pointerFutureUse[1023] ;
 BC_DTM_FEATURE    **fTablePP  ;
 DTM_TIN_POINT     **pointsPP  ;
 DTM_TIN_NODE      **nodesPP   ;
 DTM_CIR_LIST      **cListPP   ;
 DTM_FEATURE_LIST  **fListPP   ;
 IDTMElementMemoryAllocator* DTMAllocationClass;
#ifndef __BENTLEYDTM_BUILD__
private:
#endif
    DTMExtendedAppDataCollection appData;

public: BENTLEYDTM_EXPORT BC_DTM_OBJ ();
        BENTLEYDTM_EXPORT ~BC_DTM_OBJ ();
public: BENTLEYDTM_EXPORT BcDTMAppData* FindAppData (const BcDTMAppData::Key& key) const;
public: BENTLEYDTM_EXPORT StatusInt AddAppData (const BcDTMAppData::Key& key, BcDTMAppData* data);
public: BENTLEYDTM_EXPORT void ClearUpAppData ();
public: BENTLEYDTM_EXPORT void CopyHeaderDetails (const BC_DTM_OBJ& copy);
private:
    BENTLEYDTM_EXPORT BC_DTM_OBJ (BC_DTM_OBJ& copy)
        {
        // Not Allowed
        }
    BENTLEYDTM_EXPORT BC_DTM_OBJ& operator=(const BC_DTM_OBJ& copy)
        {
        // Not Allowed
        return *this;
        }
    };
#define DTMIOHeaderSize  offsetof(BC_DTM_OBJ,refCount)
// BcDTMSize this is the data size andy classes with destructors etc should be excluded.
#define BCDTMSize  offsetof(BC_DTM_OBJ,appData) 

/*__PUBLISH_SECTION_START__*/
/*--------------------------------------------------------------------------------------+
|                                                                                       |
|    TypeDefs For Binary Tree Management Of BC_DTM_OBJ Objects                          |
|                                                                                       |
+--------------------------------------------------------------------------------------*/
/*__PUBLISH_SECTION_END__*/
struct BC_DTM_BTREE_NODE
{
 long leftNode      ;                                 // Pointer To Left Node With A Smaller Node Value
 long rightNode     ;                                 // Pointer To Right Node With A Larger Node Value
 long priorNode     ;                                 // Prior Node In BTree
 BC_DTM_OBJ *dtmP   ;                                 // Current Node Value
};
struct BC_DTM_BTREE
{
 long  nullNode    ;                             //  Null Node Value - Used Internally By Btree Functions
 long  headNode    ;                             //  Head Node Of Btree
 long  memNodes    ;                             //  Memory Allocated For This Number Of Nodes
 long  activeNodes ;                             //  Number Of Nodes In Btree
 long  newNode     ;                             //  Next Node To Allocate For New Node
 long  delNodePtr  ;                             //  Pointer To List Of Removed Nodes
 BC_DTM_BTREE_NODE *btreeNodesP ;
};
/*__PUBLISH_SECTION_START__*/
struct BC_DTM_BTREE;
/*-------------------------------------------------------------------------------------+
|                                                                                      |
| TypeDef For DTM Scan Context                                                         |
|                                                                                      |
+-------------------------------------------------------------------------------------*/
struct BC_DTM_SCAN_CONTEXT
{
 DTMObjectTypes dtmObjectType ;      /* Dtm Object Type <DTM_DAT_TYPE>,<BC_DTM_OBJ_TYPE>,<DTM_LAT_TYPE> */
 BC_DTM_OBJ   *tiledDtmP  ;        /* Pointer To Tiled Dtm Object To Be Scanned      */
 BC_DTM_OBJ   *dtmP   ;            /* Pointer To Dtm  Object To Be Scanned           */
 DTM_LAT_OBJ  *latP   ;            /* Pointer To Lattice Object To Be Scanned        */
 BC_DTM_OBJ   *clipTinP ;          /* Tin Object Pointer For Clipping Dtm Feature    */
 DTMFeatureType  dtmFeatureType ;  /* Dtm Feature Type To Be Loaded                  */
 DTMFenceOption   clipOption;         /* Clip Option <None(0)>,<Inside(1)>,<Overlap(2)> */
 DTMFenceType      clipType ;           /* Clip Type   <DTMFenceType::Block,DTMFenceType::Shape>  */
 long         maxSpots ;           /* Max Spots To Be returned                       */
 long         scanOffset1 ;        /* Scan Offset 1                                  */
 long         scanOffset2 ;        /* Scan Offset 2                                  */
 long         scanOffset3 ;        /* Scan Offset 3                                  */
} ;
/*__PUBLISH_SECTION_END__*/
/*-------------------------------------------------------------------------------------+
|                                                                                      |
| TypeDef For DTM Scan Context                                                         |
|                                                                                      |
+-------------------------------------------------------------------------------------*/
struct BC_MRDTM_DISPLAY_INFO
{
 DTMObjectTypes dtmObjectType ;            /* Dtm Object Type <DTM_DAT_TYPE>,<BC_DTM_OBJ_TYPE>,<DTM_LAT_TYPE> */
 long  *tileTolValue;
};
/*__PUBLISH_SECTION_START__*/
/*-------------------------------------------------------------------+
|                                                                    |
|  Typedefs For 3D Model Applications                                |
|                                                                    |
+-------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------+
|                                                                                       |
|    TypeDefs For Reporting Duplicate Point Errors                                      |
|                                                                                       |
+--------------------------------------------------------------------------------------*/
struct DTM_DUPLICATE_POINT_ERROR : DPoint3d
{
 DTMFeatureType dtmFeatureType ;            /* Duplicate Point Feature Type       */
 DTMFeatureId dtmFeatureId ;    /* Duplicate Point Feature Id         */
 DTMUserTag   dtmUserTag  ;     /* Duplicate Point User Tag           */
};
/*--------------------------------------------------------------------------------------+
|                                                                                       |
|    TypeDefs For Reporting Crossing Features                                           |
|                                                                                       |
+--------------------------------------------------------------------------------------*/
struct DTM_CROSSING_FEATURE_ERROR
{
 double  intersectionX ;          /* x Coordinate Of Intersection Point               */
 double  intersectionY ;          /* y Coordiante Of Intersection Point               */
 DTMFeatureType dtmFeatureType1  ;          /* Dtm Feature Type 1                               */
 DTMFeatureId dtmFeatureId1 ;   /* Dtm Feature Id 1                                 */
 long segmentOfset1 ;             /* Segment Offset Of Feature 1                      */
 double elevation1  ;             /* z Coordinate Of Intersection Point For Feature 1 */
 double distance1   ;             /* Distance From Feature 1 Segment Start            */
 DTMFeatureType dtmFeatureType2 ;           /* Dtm Feature Type 2                               */
 DTMFeatureId dtmFeatureId2 ;   /* Segment Offset Of Feature 2                      */
 long segmentOfset2 ;             /* Dtm Feature Type 2                               */
 double elevation2  ;             /* z Coordinate Of Intersection Point For Feature 2 */
 double distance2   ;             /* Distance From Feature 1 Segment Start            */
} ;
/*__PUBLISH_SECTION_END__*/
/*--------------------------------------------------------------------------------------+
|                                                                                       |
|    TypeDefs For Geopak Reporting Functions                                            |
|                                                                                       |
+--------------------------------------------------------------------------------------*/
struct DTM_GEOPAK_REPORT
{
 FILE *fileP ;
 long numErrors ;
 BC_DTM_OBJ *dtmP ;
 DTMFeatureCallback loadFunctionP ;
} ;
struct DTM_MX_TRG_INDEX { long index , trgPnt1, trgPnt2,trgPnt3 ; } ;
/*--------------------------------------------------------------------------------------+
|                                                                                       |
|    TypeDefs For Multi Threading Triangulation                                         |
|                                                                                       |
+--------------------------------------------------------------------------------------*/
struct DTM_MULTI_THREAD
{
 BC_DTM_OBJ *dtmP ;
 long thread,startPoint,numPoints,*sortOfsP,*tempOfsP,leftMostPoint,rightMostPoint,topMostPoint,bottomMostPoint,isColinear ;
 long cListPtr , cListDelPtr ;
} ;
/*__PUBLISH_SECTION_START__*/
struct DTM_POINT_TILE
{
 long tileOffset ;                     // Index Offset To First Tile Point In A Dtm Object
 long numTilePts ;                     // Number Of Points In Tile
} ;
/*__PUBLISH_SECTION_END__*/
struct DTM_MULTI_THREAD_TILE
{
 long           thread ;
 BC_DTM_OBJ     *dtmP ;
 long           *tagP ;
 long           startPoint ;
 long           numPoints ;
 long           minTilePts ;
 DTM_POINT_TILE *pointTilesP ;
 long           numPointTiles ;
} ;
/*--------------------------------------------------------------------------------------+
|                                                                                       |
|    TypeDefs For DTM FEATURES                                                          |
|                                                                                       |
+--------------------------------------------------------------------------------------*/
struct BC_DTM_FEATURE_HEADER
{
 long  dtmFileType ;
 long  dtmVersionNumber ;
 long  numPoints ;
 long  numFeatures ;
 double xMin ;
 double yMin ;
 double zMin ;
 double xMax ;
 double yMax ;
 double zMax ;
 int64_t  featureFileOffset ;
} ;
struct BC_DTM_FEATURE_RECORD
{
 DTMFeatureType dtmFeatureType ;
 DTMUserTag dtmUserTag ;
 DTMFeatureId dtmFeatureId ;
 long    numFeaturePoints ;
} ;
/*__PUBLISH_SECTION_START__*/
struct DTM_QUAD_TREE_TILE
{
 long tileNumber,quadTreeLevel,parentNode,childNodes[4],tileOffset,numTilePts ;
 double xMin,xMax,yMin,yMax,zMin,zMax ;
 BC_DTM_OBJ *dtmP ;
} ;
/*__PUBLISH_SECTION_END__*/
struct DTM_MULTI_THREAD_QUAD_TREE_TILE
{
 long           thread ;
 BC_DTM_OBJ     *dtmP ;
 long           *tagP ;
 long           startPoint ;
 long           numPoints ;
 long           minTilePts ;
 DTM_QUAD_TREE_TILE *pointTilesP ;
 long           numPointTiles ;
 long           fileOffset ;
} ;
struct DTM_FEATURE_INDEX
{
  int64_t  index ;
  long     dtmFeature ;
} ;

struct DTM_ROLLBACK_FEATURE_MAP
    {
    DTMFeatureId featureId;
    long featureIndex;
    };
typedef std::deque<DTM_ROLLBACK_FEATURE_MAP> DtmRollBackFeatureMap;
struct DTM_ROLLBACK_DATA
    {
    BC_DTM_OBJ* rollBackDtmP;
    bool rollBackMapInitialized;
    DtmRollBackFeatureMap rollBackMap;
    DTM_ROLLBACK_DATA()
        {
        rollBackDtmP = NULL;
        rollBackMapInitialized = false;
        }
    };
#define qsortCPP(a,b,c,d) qsort(a,b,c, (int (*)(const void*, const void*))d)
#ifdef XM
#include "dtmdefsXM.h"
#endif

struct DTMPondAppData : BcDTMAppData
    {
    const static Key AppDataID;
    int64_t m_dtmCreationTime;
    BC_DTM_OBJ* pondDtmP;    // Triangulated DTM With the Ponds Stored As Break Lines
    int         hasPonds;
    protected:
        DTMPondAppData () : m_dtmCreationTime (0), pondDtmP (nullptr), hasPonds (false)
            {
            }
        virtual ~DTMPondAppData ()
            {
            }

    public:
        static DTMPondAppData* Create ()
            {
            return new DTMPondAppData();
            }
    };

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE
struct DTMContourParams
    {
     double  interval;                  /* ==> Contour Interval                      */
     double  conReg;                    /* ==> Contour Registration                  */
     bool    loadRange;                 /* ==> Load Contour Range <TRUE,FALSE>       */
     double  conMin;                    /* ==> Contour Range Minimum Value           */
     double  conMax;                    /* ==> Contour Range Maximum Value           */
     long    loadValues;                /* ==> Load Contour Values                   */
     double* conValuesP;                /* ==> Contour Values                        */
     long    numConValues;              /* ==> Number Of Contour Values              */
     DTMContourSmoothing  smoothOption; /* ==> Contour Smoothing Option              */
     double  smoothFactor;              /* ==> Contour Smoothing Factor                            */
     long    smoothDensity;             /* ==> Point Densification For Spline Smoothing            */
     double  smoothLength;              /* ==> Distance Between Spline Vertices                    */
     bool    depressionOption;          /* ==> Mark Depression Contours                            */
     long    maxSlopeOption;            /* ==> Max Slope Option                                    */
     double  maxSlopeValue;             /* ==> Max Slope Value                                     */

     double realInterval;               /* ==> The real interval, interval is just the draw interval*/

     DTMContourParams ()
         {
         realInterval = 0;
         interval = 0;
         conReg = 0;
         loadRange = 0;
         conMin = 0;
         conMax = 0;
         loadValues = 0;
         conValuesP = nullptr;
         numConValues = 0;
         smoothOption = DTMContourSmoothing::None;
         smoothFactor = 0;
         smoothDensity = 0;
         smoothLength = 0;
         depressionOption = false;
         maxSlopeOption = 0;
         maxSlopeValue = 0;
         }
     DTMContourParams (
         double  interval,                  /* ==> Contour Interval                      */
         double  conReg,                    /* ==> Contour Registration                  */
         bool    loadRange,                 /* ==> Load Contour Range <TRUE,FALSE>       */
         double  conMin,                    /* ==> Contour Range Minimum Value           */
         double  conMax,                    /* ==> Contour Range Maximum Value           */
         double* conValuesP,                /* ==> Contour Values                        */
         long    numConValues,              /* ==> Number Of Contour Values              */
         DTMContourSmoothing  smoothOption, /* ==> Contour Smoothing Option              */
         double  smoothFactor,              /* ==> Contour Smoothing Factor                            */
         long    smoothDensity,             /* ==> Point Densification For Spline Smoothing            */
         double  smoothLength,              /* ==> Distance Between Spline Vertices                    */
         bool    depressionOption,          /* ==> Mark Depression Contours                            */
         long    maxSlopeOption,            /* ==> Max Slope Option                                    */
         double  maxSlopeValue              /* ==> Max Slope Value                                     */
         ) :
         interval (interval),
         realInterval (0),
         conReg (conReg),
         loadRange (loadRange),
         conMin (conMin),
         conMax (conMax),
         loadValues (conValuesP != nullptr),
         conValuesP (conValuesP),
         numConValues (numConValues),
         smoothOption (smoothOption),
         smoothFactor (smoothFactor),
         smoothDensity (smoothDensity),
         smoothLength (smoothLength),
         depressionOption (depressionOption),
         maxSlopeOption (maxSlopeOption),
         maxSlopeValue (maxSlopeValue)
         {
         }
    };

END_BENTLEY_TERRAINMODEL_NAMESPACE
//  Defines For Core Debugging And Development

#ifdef DTM_TRACE_DEBUG
#define DTM_TRACE_VALUE(n)   n 
#else
#define DTM_TRACE_VALUE(n)   0 
#endif
#ifdef DTM_CHECK_DEBUG
#define DTM_CHECK_VALUE(n)   n
#else
#define DTM_CHECK_VALUE(n)  0
#endif
#ifdef DTM_TIME_DEBUG
#define DTM_TIME_VALUE(n)   n
#else
#define DTM_TIME_VALUE(n)   0 
#endif

/*__PUBLISH_SECTION_START__*/
#endif
#endif
/*__PUBLISH_SECTION_END__*/
