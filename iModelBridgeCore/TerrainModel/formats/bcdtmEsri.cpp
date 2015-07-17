/*--------------------------------------------------------------------------------------+
|
|     $Source: formats/bcdtmEsri.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TerrainModel/Formats/Formats.h"
#include "TerrainModel/Core/bcDTMBaseDef.h"
#include "TerrainModel/Core/dtmdefs.h"
#include "TerrainModel/Formats/Esri.h"
//#include "dtmevars.h"
#include <TerrainModel/Core/bcdtmInlines.h> 

#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <share.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#pragma pack(2)
typedef struct
{
 long fileCode ;
 long unUsed1 ;
 long unUsed2 ;
 long unUsed3 ; 
 long unUsed4 ;
 long unUsed5 ;
 long fileLength ;
 long version ;
 long shapeType ;
 double xMin ;
 double yMin ;
 double xMax ;
 double yMax ; 
 double zMin ;
 double zMax ;
 double mMin ;
 double mMax ;
} EsriFileHeader ;

typedef struct 
{
 FILE *esriShapeFP ;
 FILE *esriIndexFP ;
 long filePosition ;
 long indexFilePosition ;
 long shapeType ;
 long recordNumber ;
 wchar_t  filePrefix[256]; 
 double xMin,xMax,yMin,yMax,zMin,zMax ; 
} EsriFileIo ;


typedef struct _EsriFileInfo
{
 int shp ;
 int shx ;
 int dbf ;
 long recordNumber ;
 long fileLength ;
 wchar_t esriFilePrefix[256] ;
 double xMin ;
 double yMin ;
 double zMin ;
 double xMax ;
 double yMax ;
 double zMax ;
} EsriFileInfo ;

#ifdef NOTNEEDED
BENTLEYDTM_Private int bcdtmFormatEsri_initialiseEsriFileStructureDtmObject(EsriFileIo *esriFileIoP,wchar_t *esriShapeFilePrefixP) ;
BENTLEYDTM_Private int bcdtmFormatEsri_finaliseEsriFileStructureDtmObject(EsriFileIo *esriFileIoP) ;
#endif

int fc_zero = 0 ;

typedef unsigned char    byte ;
typedef unsigned short   UInt16 ;
typedef unsigned int     UInt32 ;

typedef struct _Dvector3D
{
 DPoint3d org ;
 DPoint3d end ;
} DVector3d ;

#define ESRI_POINT_FILE                  1
#define ESRI_TEXT_FILE                   2
#define ESRI_POLYLINE_FILE               3
#define ESRI_POLYGON_FILE                5
#define ESRI_MULTIPOINT_FILE             8
#define NUM_ESRI_FILE_TYPES              10
#define NUM_DBF_FIELDS                   11
#define NUM_DBF_TEXT_FIELDS              15

#define ESRI_HDR_ADDR_SIGNATURE          0x00
#define ESRI_HDR_ADDR_FILE_LENGTH        0x18
#define ESRI_HDR_ADDR_FILE_VERSION       0x1C
#define ESRI_HDR_ADDR_SHAPE_TYPE         0x20
#define ESRI_HDR_ADDR_BOUNDING_BOX       0x24

#define ESRI_HDR_MASK_SIGNATURE          0x01
#define ESRI_HDR_MASK_FILE_LENGTH        0x02
#define ESRI_HDR_MASK_FILE_VERSION       0x04
#define ESRI_HDR_MASK_SHAPE_TYPE         0x08
#define ESRI_HDR_MASK_BOUNDING_BOX       0x10
#define ESRI_HDR_MASK_UPDATE_ALL         (ESRI_HDR_MASK_SIGNATURE | ESRI_HDR_MASK_FILE_LENGTH | ESRI_HDR_MASK_FILE_VERSION | ESRI_HDR_MASK_SHAPE_TYPE | ESRI_HDR_MASK_BOUNDING_BOX)

#define ESRI_HDR_SIZE_SIGNATURE          0x18
#define ESRI_HDR_SIZE_FILE_LENGTH        0x04
#define ESRI_HDR_SIZE_FILE_VERSION       0x04
#define ESRI_HDR_SIZE_SHAPE_TYPE         0x04
#define ESRI_HDR_SIZE_BOUNDING_BOX       0x40
#define ESRI_HDR_SIZE                    (ESRI_HDR_SIZE_SIGNATURE + ESRI_HDR_SIZE_FILE_LENGTH + ESRI_HDR_SIZE_FILE_VERSION + ESRI_HDR_SIZE_SHAPE_TYPE + ESRI_HDR_SIZE_BOUNDING_BOX)

#define ESRI_RECORD_HDR_ADDR_OFFSET      0x00
#define ESRI_RECORD_HDR_ADDR_LENGTH      0x04

#define ESRI_RECORD_HDR_SIZE_OFFSET      0x04
#define ESRI_RECORD_HDR_SIZE_LENGTH      0x04
#define ESRI_RECORD_HDR_SIZE             (ESRI_RECORD_HDR_SIZE_OFFSET + ESRI_RECORD_HDR_SIZE_LENGTH)

// all the non-null shapes in a shapefile are required to be of the same shape type

typedef enum _SHAPEFILE_TYPE_Types
{
       SHAPEFILE_TYPE_NullShape          = 0,
       SHAPEFILE_TYPE_Point              = 1,
       SHAPEFILE_TYPE_Text               = 2,
       SHAPEFILE_TYPE_PolyLine           = 3,
       SHAPEFILE_TYPE_Polygon            = 5,
       SHAPEFILE_TYPE_MultiPoint         = 8,
       SHAPEFILE_TYPE_PointZ             = 11,
       SHAPEFILE_TYPE_TextZ              = 12,
       SHAPEFILE_TYPE_PolyLineZ          = 13,
       SHAPEFILE_TYPE_PolygonZ           = 15,
       SHAPEFILE_TYPE_MultiPointZ        = 18,
       SHAPEFILE_TYPE_PointM             = 21,
       SHAPEFILE_TYPE_TextM              = 22,
       SHAPEFILE_TYPE_PolyLineM          = 23,
       SHAPEFILE_TYPE_PolygonM           = 25,
       SHAPEFILE_TYPE_MultiPointM        = 28,
       SHAPEFILE_TYPE_MultiPatch         = 31,
} SHAPEFILE_TYPE_Types;

typedef struct _DBASEHeader
{
       byte                                            version;      // version number
       byte                                            year;         // year of last update since 1900
       byte                                            month;        // month of last update
       byte                                            day;          // day of last update
       UInt32                                          nRecords;     // number of records
       UInt16                                          hLength;      // length of header structure
       UInt16                                          rLength;      // length of each record
       UInt16                                          reserved1;    // reserved
       byte                                            incomplete;   // incomplete transaction
       byte                                            encryption;   // encryption flag
       UInt32                                          fRecord;      // free record thread
       UInt32                                          multiuser1;   // reserved for multi-user
       UInt32                                          multiuser2;   // reserved for multi-user
       byte                                            mdx;          // mdx flag
       byte                                            language;     // language
       UInt16                                          reserved2;    // reserved
} DBASEHeader;                                                       // 32 byte fixed length header

typedef struct _DBASERecordHeader
{
       char                                            name[11];     // field name terminated by 00h
       char                                            type;         // field type (ASCII)
       UInt32                                          data;         // field data address (in memory dBase III+)
       byte                                            length;       // field length
       byte                                            decimal;      // decimal count
       byte                                            multiuser1;   // reserved for multi-user dBase
       byte                                            multiuser2;   // reserved for multi-user dBase
       byte                                            workarea;     // work area ID
       byte                                            multiuser3;   // reserved for multi-user dBase
       byte                                            multiuser4;   // reserved for multi-user dBase
       byte                                            flag;         // flag for SET FIELDS
       byte                                            reserved1;    // reserved
       byte                                            reserved2;    // reserved
       byte                                            reserved3;    // reserved
       byte                                            reserved4;    // reserved
       byte                                            reserved5;    // reserved
       byte                                            reserved6;    // reserved
       byte                                            reserved7;    // reserved
       byte                                            indexed;      // index field flag, 00 = no key (ignored), 01 = key exists in MDX 
} DBASERecordHeader;                                                 // 32 byte fixed length header

typedef struct _DBASEHeaderEx
{
       DBASEHeader                                     hdr;          // 32 byte fixed length header
       UInt32                                          nRecords;     // number of records in pRecords
       DBASERecordHeader                               *pRecords;    // array of 32 byte fixed length record headers
} DBASEHeaderEx;                                                     // 32 byte fixed length header + array of nRecord * 32 byte fixed length record headers

typedef struct _ESRIHeader
{
       UInt32                                          code;         // big endian file code (always 9994)
       UInt32                                          unused1;      // unused
       UInt32                                          unused2;      // unused
       UInt32                                          unused3;      // unused
       UInt32                                          unused4;      // unused
       UInt32                                          unused5;      // unused
       UInt32                                          length;       // big endian file length in 16 bit words
       UInt32                                          version;      // big endian version number (always 1000)
       UInt32                                          type;         // shapefile type (point = 1, polyline = 3, polygon = 5)
       double                                          Xmin;         // bounding box Xmin
       double                                          Ymin;         // bounding box Ymin
       double                                          Xmax;         // bounding box Xmax
       double                                          Ymax;         // bounding box Ymax
       double                                          Zmin;         // bounding box Zmin
       double                                          Zmax;         // bounding box Zmax
       double                                          Mmin;         // bounding box Mmin
       double                                          Mmax;         // bounding box Mmax
} ESRIHeader;                                                        // 100 byte fixed length header

typedef struct _ESRIIndexRecord
{
       UInt32                                          offset;       // big endian offset to start of record in 16 bit words
       UInt32                                          length;       // big endian length of record in 16 bit words
} ESRIIndexRecord;                                                   // 8 byte fixed length record

typedef struct _ESRIRecordHeader
{
       UInt32                                          id;           // big endian record number
       UInt32                                          length;       // big endian content length in 16 bit words
} ESRIRecordHeader;                                                  // 8 byte fixed length record header

typedef struct _ESRIPoint
{
       UInt32                                          type;         // shape type (always 1)
       double                                          x;            // coordinates x
       double                                          y;            // coordinates y
} ESRIPoint;

typedef struct _ESRIPointZ
{
       UInt32                                          type;         // shape type (always 11)
       double                                          x;            // coordinates x
       double                                          y;            // coordinates y
       double                                          z;            // coordinates z
       double                                          M;            // coordinates M
} ESRIPointZ;

typedef struct _ESRIPolyLine
{
       UInt32                                          type;         // shape type (always 3)
       double                                          Xmin;         // bounding box Xmin
       double                                          Ymin;         // bounding box Ymin
       double                                          Xmax;         // bounding box Xmax
       double                                          Ymax;         // bounding box Ymax
       UInt32                                          nParts;       // number of component parts (will usually be 1)
       UInt32                                          nPoints;      // total number of points in all component parts
       UInt32                                          *pParts;      // array of part offsets (will usually be 0)
       DPoint2d                                        *pPoints;     // array of points (x,y)
} ESRIPolyLine;

typedef struct _ESRIPolyLineZ
{
       UInt32                                          type;         // shape type (always 13)
       double                                          Xmin;         // bounding box Xmin
       double                                          Ymin;         // bounding box Ymin
       double                                          Xmax;         // bounding box Xmax
       double                                          Ymax;         // bounding box Ymax
       UInt32                                          nParts;       // number of component parts (will usually be 1)
       UInt32                                          nPoints;      // total number of points in all component parts
       UInt32                                          *pParts;      // array of part offsets (will usually be 0)
       DPoint2d                                        *pPoints;     // array of points (x,y)
       double                                          Zmin;         // bounding box Zmin
       double                                          Zmax;         // bounding box Zmax
       double                                          *pPointsZ;    // array of points (z only)
} ESRIPolyLineZ;

typedef struct _ESRIPolygon
{
       UInt32                                          type;         // shape type (always 3)
       double                                          Xmin;         // bounding box Xmin
       double                                          Ymin;         // bounding box Ymin
       double                                          Xmax;         // bounding box Xmax
       double                                          Ymax;         // bounding box Ymax
       UInt32                                          nParts;       // number of component parts (will always be 1)
       UInt32                                          nPoints;      // total number of points in all component parts
       UInt32                                          *pParts;      // array of part offsets
       DPoint2d                                        *pPoints;     // array of points (x,y)
} ESRIPolygon;

typedef struct _ESRIPolygonZ
{
       UInt32                                          type;         // shape type (always 13)
       double                                          Xmin;         // bounding box Xmin
       double                                          Ymin;         // bounding box Ymin
       double                                          Xmax;         // bounding box Xmax
       double                                          Ymax;         // bounding box Ymax
       UInt32                                          nParts;       // number of component parts (will always be 1)
       UInt32                                          nPoints;      // total number of points in all component parts
       UInt32                                          *pParts;      // array of part offsets
       DPoint2d                                        *pPoints;     // array of points (x,y)
       double                                          Zmin;         // bounding box Zmin
       double                                          Zmax;         // bounding box Zmax
       double                                          *pPointsZ;    // array of points (z only)
} ESRIPolygonZ;

typedef struct _ESRIMultiPointZ
{
       UInt32                                          type;         // shape type (always 18)
       double                                          Xmin;         // bounding box Xmin
       double                                          Ymin;         // bounding box Ymin
       double                                          Xmax;         // bounding box Xmax
       double                                          Ymax;         // bounding box Ymax
       UInt32                                          nPoints;      // total number of points in all component parts
       DPoint2d                                        *pPoints;     // array of points (x,y)
       double                                          Zmin;         // bounding box Zmin
       double                                          Zmax;         // bounding box Zmax
       double                                          *pPointsZ;    // array of points (z only)
} ESRIMultiPointZ;

BENTLEYDTM_Private int bcdtmFormatEsri_writeFileHeader( int fh, UInt32 shapeType, DVector3d *pRange, UInt16 mask ) ;
BENTLEYDTM_Private int bcdtmFormatEsri_writeDBaseHeader( int fh ) ;
BENTLEYDTM_Private int bcdtmFormatEsri_appendDBaseRecord(int dbf, char *pRecord ) ;
BENTLEYDTM_Private int bcdtmFormatEsri_appendIndex(int shx, UInt32 offset, UInt32 length ) ;
//NOTNEEDED BENTLEYDTM_Private int bcdtmFormatEsri_appendPoint( int shp, int shx, int dbf, UInt32 record, DPoint3d *pPoint, char *pRecord ) ;
//NOTNEEDED BENTLEYDTM_Private int bcdtmFormatEsri_appendPointZ( int shp, int shx, int dbf, UInt32 record, DPoint3d *pPoint, char *pRecord) ;
//NOTNEEDED BENTLEYDTM_Private int bcdtmFormatEsri_appendPolyLine( int shp, int shx, int dbf, UInt32 record, DVector3d *pRange, UInt32 nParts, UInt32 nPoints, UInt32 *pParts, DPoint3d *pPoints, char *pRecord ) ;
BENTLEYDTM_Private int bcdtmFormatEsri_appendPolyLineZ( int shp, int shx, int dbf, UInt32 record, DVector3d *pRange, UInt32 nParts, UInt32 nPoints, UInt32 *pParts, DPoint3d *pPoints, char *pRecord ) ;
//NOTNEEDED BENTLEYDTM_Private int bcdtmFormatEsri_appendPolygon( int shp, int shx, int dbf, UInt32 record, DVector3d *pRange, UInt32 nParts, UInt32 nPoints, UInt32 *pParts, DPoint3d *pPoints, char *pRecord ) ;
//NOTNEEDED BENTLEYDTM_Private int bcdtmFormatEsri_appendPolygonZ( int shp, int shx, int dbf, UInt32 record, DVector3d *pRange, UInt32  nParts, UInt32  nPoints, UInt32  *pParts, DPoint3d *pPoints, char *pRecord ) ;
BENTLEYDTM_Private int bcdtmFormatEsri_appendMultiPointZ( int shp,int shx,int dbf, UInt32 record, DVector3d *pRange, UInt32 nPoints, DPoint3d *pPoints, char *pRecord ) ;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void dtmCnv_swapByteArray
(
char *bytes, /* <> Byte array to swap */
int   nWrds  /* => number of words (shorts) in byte array */
)
{
/*Description:
  This function will swap the bytes inside of an array of
  bytes.  It was originally coded as part of the HP 700 port.
  It is coded to work just like mdlCnv_swapWordArray;
*/
  int nbytes;
  int     ix;
  char     t;

  for (ix=0,nbytes=nWrds*2; ix<nbytes; ix=ix+2)
  { t           = bytes[ix];
    bytes[ix]   = bytes[ix+1];
    bytes[ix+1] = t;
  }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void dtmCnv_swapWordArray
(
short *words, /* <> Byte array to swap */
int    nLngs  /* => number of longs in word array */
)
{
/*Description:
  This function will swap the words inside of an array of
  words.  It was originally coded as part of the HP 700 port.
  It is coded to work just like mdlCnv_swapWordArray;
*/
  int nwords;
  int     ix;
  short    t;

  for (ix=0,nwords=nLngs*2; ix<nwords; ix=ix+2)
  { t           = words[ix];
    words[ix]   = words[ix+1];
    words[ix+1] = t;
  }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
#ifdef NOTNEEDED
BENTLEYDTM_Private void dtmCnv_swapLongArray
(
long  *longs, /* <> Byte array to swap */
int    nLngs  /* => number of longs in word array */
)
{
/*Description:
  This function will swap the longs inside of an array of
  words.  It was originally coded as part of the HP 700 port.
  It is coded to work just like mdlCnv_swapWordArray;
*/
  int nlongs;
  int     ix;
  long     t;

  for (ix=0,nlongs=nLngs*2; ix<nlongs; ix=ix+2)
  { t           = longs[ix];
    longs[ix]   = longs[ix+1];
    longs[ix+1] = t;
  }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void dtmCnv_toFFLong
(
long   *toLong, /* <= Universal File Format for long */
long *fromLong, /* => Machine format for long        */
int         nL  /* => Size of toLong array           */
)
{
/*Description:
  Convert an array of longs from machine dependent format
  to independent or universal file format.  This is the
  same format as MicroStation vertice coordinates.


  Return: Nothing
*/

  memcpy(toLong,fromLong,nL*4);
#if defined (BIG_ENDIAN)
  dtmCnv_swapByteArray((char*)toLong,nL*4);
  dtmCnv_swapWordArray((short*)toLong,nL*2);
#endif
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void dtmCnv_fromFFLong
(
long   *toLong, /* <= Machine format for long        */
long *fromLong, /* => Universal File Format for long */
int         nL  /* => Size of toLong array           */
)
{
/*Description:
  Convert an array of longs from independent or universal
  file format to machine file format.  The file format
  is the same format as MicroStation vertice coordinates.

  Return: Nothing
*/
  memcpy(toLong,fromLong,nL*4);
#if defined (BIG_ENDIAN)
  dtmCnv_swapByteArray((char*)toLong,nL*4);
  dtmCnv_swapWordArray((short*)toLong,nL*2);

#endif
}
#endif
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int dtmCnv_byteSwap4( int value )
{
 dtmCnv_swapByteArray((char *)&value,2) ;
 dtmCnv_swapWordArray((short*)&value,1) ;
 return(value) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTMFORMATS_EXPORT int bcdtmFormatEsri_importEsriShapeFileDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,wchar_t *esriShapeFileP) 
{
 int ret=DTM_SUCCESS,dbg=1,cdbg=0 ;
 char *cacheP=NULL ;
 long cacheSize=2000000,pointsSize=10000,numPoints,shapeType ;
 DPoint3d  *p3dP=NULL,*pointsP=NULL ;
 double *cord1P,*cord2P ;
 EsriFileHeader esriFileHeader ;
 unsigned long fileLength=0,filePos=0,startFilePos=0,numRecords=0,recordNumber=0,recordLength=0,recordOfs=0 ;
 FILE *esriShapeFP=NULL ;
 DTMFeatureId nullFeatureId=dtmP->nullFeatureId ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Importing Esri Shape File") ;
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"esriShapeFileP = %s",esriShapeFileP) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Open Esri File
*/
 esriShapeFP = bcdtmFile_open(esriShapeFileP,L"rb") ;
 if( esriShapeFP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Failed To Open Esri Shape File %s",esriShapeFileP) ;
    goto errexit ;
   }
/*
** Read File Header
*/
 if( fread(&esriFileHeader,sizeof(EsriFileHeader),1,esriShapeFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Error Reading Esri File %s",esriShapeFileP) ;
    goto errexit ;
   } 
/*
** Do Big Endian Conversions
*/
 dtmCnv_swapByteArray((char *)&esriFileHeader.fileCode,2) ;
 dtmCnv_swapWordArray((short*)&esriFileHeader.fileCode,1) ;
 dtmCnv_swapByteArray((char *)&esriFileHeader.fileLength,2) ;
 dtmCnv_swapWordArray((short*)&esriFileHeader.fileLength,1) ;
/*
** Write Shape Type
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"File Code      = %8ld",esriFileHeader.fileCode) ;
    bcdtmWrite_message(0,0,0,"File Length    = %8ld",esriFileHeader.fileLength*2) ;
    bcdtmWrite_message(0,0,0,"Shape Type     = %8ld",esriFileHeader.shapeType) ;
    bcdtmWrite_message(0,0,0,"xMin = %12.5lf yMin = %12.5lf zMin = %10.4lf",esriFileHeader.xMin,esriFileHeader.yMin,esriFileHeader.zMin ) ;
    bcdtmWrite_message(0,0,0,"xMax = %12.5lf yMax = %12.5lf zMax = %10.4lf",esriFileHeader.xMax,esriFileHeader.yMax,esriFileHeader.zMax ) ;
    bcdtmWrite_message(0,0,0,"mMin = %12.5lf mMax = %12.5lf",esriFileHeader.mMin,esriFileHeader.mMax) ;
   }
/*
** Initialise File Positions
*/
 fileLength = esriFileHeader.fileLength * 2 ;
 filePos    = 100 ;
/*
** Allocate Cache Memory
*/
 cacheP = ( char * ) malloc( cacheSize * sizeof(char)) ;
 if( cacheP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Allocate Points Memory
*/
 pointsP = ( DPoint3d * ) malloc( pointsSize * sizeof(DPoint3d)) ;
 if( pointsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Read The File Records
*/
 while( filePos < fileLength )
   {
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"filePos = %8ld ** fileLength = %8ld",filePos,fileLength) ;
/*
**  Read Record Number
*/
    if( fread(&recordNumber,4,1,esriShapeFP) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Error Reading Esri File %s",esriShapeFileP) ;
       goto errexit ;
      } 
    filePos = filePos + 4 ;
/*
**  Read Record Length
*/
    if( fread(&recordLength,4,1,esriShapeFP) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Error Reading Esri File %s",esriShapeFileP) ;
       goto errexit ;
      } 
    filePos = filePos + 4 ;
/*
**  Do Big Endian Conversions
*/
    dtmCnv_swapByteArray((char *)&recordNumber,2) ;
    dtmCnv_swapWordArray((short*)&recordNumber,1) ;
    dtmCnv_swapByteArray((char *)&recordLength,2) ;
    dtmCnv_swapWordArray((short*)&recordLength,1) ;
    startFilePos = filePos ;
    if( dbg == 2 )  bcdtmWrite_message(0,0,0,"recordNumber   = %8u  recordlength = %8u filePos = %8u",recordNumber,recordLength,filePos) ;
/*
**  Read Record Content
*/
    recordLength = recordLength * 2 ;
    if( fread(cacheP,recordLength,1,esriShapeFP) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Error Reading Esri File %s",esriShapeFileP) ;
       goto errexit ;
      } 
    filePos = filePos + recordLength ;
/*
** Get The Feature Coordinates
*/
   memcpy(&shapeType,cacheP,4);
   if( shapeType != esriFileHeader.shapeType )
     {
      bcdtmWrite_message(1,0,0,"Esri Record Shape Type = %2ld ** Esri File Shape Type = %2ld",shapeType,esriFileHeader.shapeType) ; 
      goto errexit ;
     }
   switch (  esriFileHeader.shapeType )
     {
      case  0  : // Null Shape
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;

      case  1  : // Point
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;

      case  3  :  // PolyLine
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;

      case  5  :  // Polygon
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;  

      case  8  :  // MultiPoint
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;
 
      case 11  :  // PointZ
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;

      case 13  :  // PolyLine z
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;

      case 15  :  // Polygon z
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;

      case 18  :  // MultiPoint z
         memcpy(&numPoints,cacheP+36,4);
         if( dbg == 2 ) bcdtmWrite_message(0,0,0,"numPoints      = %8ld",numPoints) ;
         if( dbg == 1 )  bcdtmWrite_message(0,0,0,"recordNumber   = %8u  recordlength = %8u numPoints = %8ld startFilePos = %8u filePos = %8u",recordNumber,recordLength,numPoints,startFilePos,filePos) ;
         if( numPoints > pointsSize )
           {
            bcdtmWrite_message(1,0,0,"Number Of Esri Record Points Exceeds Setting") ;
            goto errexit ;
           } 
/*
**       Copy Cache Points To Points Array
*/
         cord1P = (double *) (cacheP+40) ;
         cord2P = (double *) (cacheP+40+16+numPoints*16) ;
         for( p3dP = pointsP ; p3dP < pointsP + numPoints ; ++p3dP)
           {
            p3dP->x = *cord1P ;
            ++cord1P ;
            p3dP->y = *cord1P ;
            ++cord1P ; 
            p3dP->z = *cord2P ;
            ++cord2P ;
            if( dbg == 2 ) bcdtmWrite_message(0,0,0,"%12.3lf %12.3lf %10.4lf",p3dP->x,p3dP->y,p3dP->z) ;
           }
/*
**       Store Points Array In DTM
*/
         if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,dtmFeatureType,DTM_NULL_USER_TAG,1,&nullFeatureId,pointsP,numPoints)) goto errexit ;
      break    ;

      case 21  :  // Point M
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;

      case 23  :  // Polyline M
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;

      case 25 :  // Polygon M
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;

      case 28 :  // Multipoint M
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;

      case 31 :  // MultiPatch
         bcdtmWrite_message(1,0,0,"Import Of Esri Shape Type %2ld Not Yet Implemented",esriFileHeader.shapeType) ; 
      break    ;

      default  :  
         bcdtmWrite_message(1,0,0,"Unknown Esri Shape Type %2ld",esriFileHeader.shapeType) ; 
      break    ;
    }
/*
**  Increment Number Of Records
*/
   ++numRecords ;
  }
/*
** Write Number Of Records Read
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Number Of Shape File Records Read = %10u",numRecords) ;
    bcdtmWrite_message(0,0,0,"Number Of Dtm Points              = %10ld",dtmP->numPoints) ;
   }
/*
** Read In Index Records
*/
 if( cdbg )
   {
    if( esriShapeFP != NULL ) { fclose(esriShapeFP) ; esriShapeFP = NULL ; }
    *(esriShapeFileP+wcslen(esriShapeFileP)-1) = L'x' ;
/*
**  Open Esri File
*/
    esriShapeFP = bcdtmFile_open(esriShapeFileP,L"rb") ;
    if( esriShapeFP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Failed To Open Esri Shape File %s",esriShapeFileP) ;
       goto errexit ;
      }
/*
**  Read File Header
*/
    if( fread(&esriFileHeader,sizeof(EsriFileHeader),1,esriShapeFP) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Error Reading Esri File %s",esriShapeFileP) ;
       goto errexit ;
      } 
/*
**  Do Big Endian Conversions
*/
    dtmCnv_swapByteArray((char *)&esriFileHeader.fileCode,2) ;
    dtmCnv_swapWordArray((short*)&esriFileHeader.fileCode,1) ;
    dtmCnv_swapByteArray((char *)&esriFileHeader.fileLength,2) ;
    dtmCnv_swapWordArray((short*)&esriFileHeader.fileLength,1) ;
/*
** Write Shape Type
*/
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"File Code      = %8ld",esriFileHeader.fileCode) ;
       bcdtmWrite_message(0,0,0,"File Length    = %8ld",esriFileHeader.fileLength*2) ;
       bcdtmWrite_message(0,0,0,"Shape Type     = %8ld",esriFileHeader.shapeType) ;
       bcdtmWrite_message(0,0,0,"xMin = %12.5lf yMin = %12.5lf zMin = %10.4lf",esriFileHeader.xMin,esriFileHeader.yMin,esriFileHeader.zMin ) ;
       bcdtmWrite_message(0,0,0,"xMax = %12.5lf yMax = %12.5lf zMax = %10.4lf",esriFileHeader.xMax,esriFileHeader.yMax,esriFileHeader.zMax ) ;
       bcdtmWrite_message(0,0,0,"mMin = %12.5lf mMax = %12.5lf",esriFileHeader.mMin,esriFileHeader.mMax) ;
      }
/*
**  Read The Index Records
*/
    recordNumber = 1 ;
    while( fread(cacheP,8,1,esriShapeFP) == 1 )
      {
       memcpy(&recordOfs,cacheP,4) ;
       memcpy(&recordLength,cacheP+4,4) ;
       recordOfs    = dtmCnv_byteSwap4(recordOfs) ;
       recordLength = dtmCnv_byteSwap4(recordLength) ;
       bcdtmWrite_message(0,0,0,"recordNumber = %8ld  recordOfs = %8ld recordLength = %8ld",recordNumber,recordOfs*2,recordLength*2) ;
       ++recordNumber ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( esriShapeFP  != NULL ) { fclose(esriShapeFP) ; esriShapeFP  = NULL ; }
 if( cacheP  != NULL ) { free(cacheP)   ; cacheP  = NULL ; }
 if( pointsP != NULL ) { free(pointsP)  ; pointsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing Esri Shape File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing Esri Shape File Error") ;
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
BENTLEYDTM_Private int bcdtmFormatEsri_closeEsriShapeFiles(EsriFileInfo *esriFileInfoP) 
{
 int  ret=DTM_SUCCESS ;
 DVector3d  boundingCube ;
 UInt16 mask = ESRI_HDR_MASK_FILE_LENGTH | ESRI_HDR_MASK_BOUNDING_BOX ;
/*
** Reset File Length In Shape File Header
*/
 if( esriFileInfoP->shp != -1 )
   {
/*
**  Set Bounding Cube
*/
    boundingCube.org.x = esriFileInfoP->xMin ;
    boundingCube.org.y = esriFileInfoP->yMin ;
    boundingCube.org.z = esriFileInfoP->zMin ;
    boundingCube.end.x = esriFileInfoP->xMax ;
    boundingCube.end.y = esriFileInfoP->yMax ;
    boundingCube.end.z = esriFileInfoP->zMax ;
/*
** Set File Length And Bounding Cube In Shape File
*/
    if( bcdtmFormatEsri_writeFileHeader(esriFileInfoP->shp,1,&boundingCube,mask) == -1 ) 
      {
       bcdtmWrite_message(1,0,0,"Error Writing Esri File Length In Shp Header") ;
       goto errexit ;
      }
/*
** Set File Length And Bounding Cube In Index File
*/
    if( bcdtmFormatEsri_writeFileHeader(esriFileInfoP->shx,1,&boundingCube,mask) == -1 ) 
      {
       bcdtmWrite_message(1,0,0,"Error Writing Esri File Length In Shx Header") ;
       goto errexit ;
      }
   } 
/*
** Close Files
*/
 if( esriFileInfoP->shp != -1 ) _close (esriFileInfoP->shp);
 if( esriFileInfoP->shx != -1 ) _close (esriFileInfoP->shx);
 if( esriFileInfoP->dbf != -1 ) _close (esriFileInfoP->dbf);
/*
** Initialise Esri File Info Structure
*/
 esriFileInfoP->shp = -1 ; 
 esriFileInfoP->shx = -1 ; 
 esriFileInfoP->dbf = -1 ;
 esriFileInfoP->recordNumber = 0 ;
 esriFileInfoP->fileLength = 100 ;
 esriFileInfoP->xMin = esriFileInfoP->xMax = 0.0 ;
 esriFileInfoP->yMin = esriFileInfoP->yMax = 0.0 ;
 esriFileInfoP->zMin = esriFileInfoP->zMax = 0.0 ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
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
BENTLEYDTM_Private int bcdtmFormatEsri_openEsriShapeFilesForExport(EsriFileInfo *esriFileInfoP,wchar_t *dtmTypeP,UInt32 shapeType) 
{
 int ret=DTM_SUCCESS,dbg=0 ;
 wchar_t fileName[256] ;
 UInt16  mask=ESRI_HDR_MASK_UPDATE_ALL ;
 DVector3d boundingCube ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Opening Esri Shape Files For Export") ;
/*
** Initialise
*/
 esriFileInfoP->shp = -1 ; 
 esriFileInfoP->shx = -1 ; 
 esriFileInfoP->dbf = -1 ; 
 esriFileInfoP->recordNumber = 0 ;
 esriFileInfoP->fileLength = 100 ;
 esriFileInfoP->xMin = esriFileInfoP->xMax = 0.0 ;
 esriFileInfoP->yMin = esriFileInfoP->yMax = 0.0 ;
 esriFileInfoP->zMin = esriFileInfoP->zMax = 0.0 ;
/*
** Set Bounding Cube
*/
 boundingCube.org.x = esriFileInfoP->xMin ;
 boundingCube.org.y = esriFileInfoP->yMin ;
 boundingCube.org.z = esriFileInfoP->zMin ;
 boundingCube.end.x = esriFileInfoP->xMax ;
 boundingCube.end.y = esriFileInfoP->yMax ;
 boundingCube.end.z = esriFileInfoP->zMax ;
/*
** Open Shape File And Write Header
*/
 wcscpy(fileName,esriFileInfoP->esriFilePrefix) ;
 wcscat(fileName,dtmTypeP) ;
 wcscat(fileName,L".shp") ;
 esriFileInfoP->shp = _wopen (fileName , _O_BINARY | _O_WRONLY | _O_CREAT | _O_TRUNC , _S_IWRITE ) ; 
 if( esriFileInfoP->shp == -1 )
   {
    bcdtmWrite_message(1,0,0,"Error Opening Esri Shape File %s",fileName) ;
    goto errexit ;
   }
 if( bcdtmFormatEsri_writeFileHeader(esriFileInfoP->shp,shapeType,&boundingCube,mask) == -1 ) 
   {
    bcdtmWrite_message(1,0,0,"Error Writing Esri Shape File Header %s",fileName) ;
    goto errexit ;
   }
/*
** Open Index File And Write Header
*/
 wcscpy(fileName,esriFileInfoP->esriFilePrefix) ;
 wcscat(fileName,dtmTypeP) ;
 wcscat(fileName,L".shx") ;
 esriFileInfoP->shx = _wopen (fileName , _O_BINARY | _O_WRONLY | _O_CREAT | _O_TRUNC , _S_IWRITE ) ; 
 if( esriFileInfoP->shx == -1 )
   {
    bcdtmWrite_message(1,0,0,"Error Opening Esri Index File %s",fileName) ;
    goto errexit ;
   }
 if( bcdtmFormatEsri_writeFileHeader(esriFileInfoP->shx,shapeType,&boundingCube,mask) == -1 ) 
   {
    bcdtmWrite_message(1,0,0,"Error Writing Esri Index File Header %s",fileName) ;
    goto errexit ;
   }
/*
** Open dBase File And Write Header
*/
 wcscpy(fileName,esriFileInfoP->esriFilePrefix) ;
 wcscat(fileName,dtmTypeP) ;
 wcscat(fileName,L".dbf") ;
 esriFileInfoP->dbf = _wopen (fileName , _O_BINARY |_O_RDWR | _O_CREAT | _O_TRUNC , _S_IWRITE ) ; 
 if( esriFileInfoP->dbf == -1 )
   {
    bcdtmWrite_message(1,0,0,"Error Opening Esri dBase File %s",fileName) ;
    goto errexit ;
   }
 if( bcdtmFormatEsri_writeDBaseHeader(esriFileInfoP->dbf) == -1 )
   {
    bcdtmWrite_message(1,0,0,"Error Writing Esri dBase File Header %s",fileName) ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Opening Esri Shape Files For Export Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Opening Esri Shape Files For Export Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 bcdtmFormatEsri_closeEsriShapeFiles(esriFileInfoP)  ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTMFORMATS_EXPORT int bcdtmFormatEsri_exportEsriShapeFileDtmObject(BC_DTM_OBJ *dtmP,wchar_t *esriShapeFilePrefixP) 
{
 int ret=DTM_SUCCESS,dbg=0 ;
 long maxSpots=5000 ;
 EsriFileInfo esriFileInfo ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Exporting To Esri Shape File") ;
    bcdtmWrite_message(0,0,0,"dtmP                 = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"esriShapeFilePrefixP = %s",esriShapeFilePrefixP) ;
   } 
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Initialise Esri File Info Structure
*/
 esriFileInfo.shp = -1 ; 
 esriFileInfo.shx = -1 ; 
 esriFileInfo.dbf = -1 ; 
 esriFileInfo.recordNumber = 0 ;
 esriFileInfo.fileLength   = 100 ;
 esriFileInfo.xMin = esriFileInfo.xMax = 0.0 ;
 esriFileInfo.yMin = esriFileInfo.yMax = 0.0 ;
 esriFileInfo.zMin = esriFileInfo.zMax = 0.0 ;
 wcscpy(esriFileInfo.esriFilePrefix,esriShapeFilePrefixP) ;
/*
** Export Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Exporting Points") ;
 if( bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject(dtmP,DTMFeatureType::RandomSpots,maxSpots,bcdtmFormatEsri_callBackFunction,FALSE,DTMFenceType::Block, DTMFenceOption::Inside,NULL,0,(void *) &esriFileInfo )) goto errexit ; 
 if (bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (dtmP, DTMFeatureType::GroupSpots, maxSpots, bcdtmFormatEsri_callBackFunction, FALSE, DTMFenceType::Block, DTMFenceOption::Inside, NULL, 0, (void *)&esriFileInfo)) goto errexit;
 bcdtmFormatEsri_closeEsriShapeFiles(&esriFileInfo) ;
 /*
** Export Break Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Exporting Break Lines") ;
 if (bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (dtmP, DTMFeatureType::Breakline, maxSpots, bcdtmFormatEsri_callBackFunction, FALSE, DTMFenceType::Block, DTMFenceOption::Inside, NULL, 0, (void *)&esriFileInfo)) goto errexit;
 bcdtmFormatEsri_closeEsriShapeFiles(&esriFileInfo) ;
/*
** Export Soft Break Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Exporting Soft Break Lines") ;
 if (bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (dtmP, DTMFeatureType::SoftBreakline, maxSpots, bcdtmFormatEsri_callBackFunction, FALSE, DTMFenceType::Block, DTMFenceOption::Inside, NULL, 0, (void *)&esriFileInfo)) goto errexit;
 if (bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (dtmP, DTMFeatureType::GraphicBreak, maxSpots, bcdtmFormatEsri_callBackFunction, FALSE, DTMFenceType::Block, DTMFenceOption::Inside, NULL, 0, (void *)&esriFileInfo)) goto errexit;
 bcdtmFormatEsri_closeEsriShapeFiles(&esriFileInfo) ;
/*
** Export Contour Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Exporting Contour Lines") ;
 if (bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (dtmP, DTMFeatureType::ContourLine, maxSpots, bcdtmFormatEsri_callBackFunction, FALSE, DTMFenceType::Block, DTMFenceOption::Inside, NULL, 0, (void *)&esriFileInfo)) goto errexit;
 bcdtmFormatEsri_closeEsriShapeFiles(&esriFileInfo) ;
/*
** Export Voids
*/
 if (bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (dtmP, DTMFeatureType::Void, maxSpots, bcdtmFormatEsri_callBackFunction, FALSE, DTMFenceType::Block, DTMFenceOption::Inside, NULL, 0, (void *)&esriFileInfo)) goto errexit;
 if (bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (dtmP, DTMFeatureType::BreakVoid, maxSpots, bcdtmFormatEsri_callBackFunction, FALSE, DTMFenceType::Block, DTMFenceOption::Inside, NULL, 0, (void *)&esriFileInfo)) goto errexit;
 bcdtmFormatEsri_closeEsriShapeFiles(&esriFileInfo) ;
/*
** Export Islands
*/
 if (bcdtmInterruptLoad_dtmFeatureTypeFromDtmObject (dtmP, DTMFeatureType::Island, maxSpots, bcdtmFormatEsri_callBackFunction, FALSE, DTMFenceType::Block, DTMFenceOption::Inside, NULL, 0, (void *)&esriFileInfo)) goto errexit;
 bcdtmFormatEsri_closeEsriShapeFiles(&esriFileInfo) ;
/*
** Clean Up
*/
 cleanup :
 bcdtmFormatEsri_closeEsriShapeFiles(&esriFileInfo) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Exporting To Esri Shape File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Importing To Esri Shape File Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
#ifdef NOTNEEDED
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmFormatEsri_initialiseEsriFileStructureDtmObject(EsriFileIo *esriFileIoP,wchar_t *esriShapeFilePrefixP) 
{
 esriFileIoP->esriShapeFP  = NULL ;
 esriFileIoP->esriIndexFP  = NULL ;
 esriFileIoP->filePosition = 0 ;
 esriFileIoP->indexFilePosition = 0 ;
 esriFileIoP->shapeType    = 0 ;
 esriFileIoP->recordNumber = 1 ;
 esriFileIoP->xMin         = 0.0 ; 
 esriFileIoP->xMax         = 0.0 ; 
 esriFileIoP->yMin         = 0.0 ; 
 esriFileIoP->xMax         = 0.0 ; 
 esriFileIoP->zMin         = 0.0 ; 
 esriFileIoP->zMax         = 0.0 ; 
 wcscpy(esriFileIoP->filePrefix,esriShapeFilePrefixP) ;
 return(DTM_SUCCESS);
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmFormatEsri_finaliseEsriFileStructureDtmObject(EsriFileIo *esriFileIoP) 
{
/*
** Finalise Shape File
*/
 esriFileIoP->filePosition = esriFileIoP->filePosition / 2 ;
 dtmCnv_swapWordArray((short*)&esriFileIoP->filePosition,1) ;
 dtmCnv_swapByteArray((char *)&esriFileIoP->filePosition,2) ;
 fseek(esriFileIoP->esriShapeFP,24,SEEK_SET) ;
 fwrite(&esriFileIoP->filePosition,4,1,esriFileIoP->esriShapeFP) ;
 fseek(esriFileIoP->esriShapeFP,36,SEEK_SET) ;
 fwrite(&esriFileIoP->xMin,4,1,esriFileIoP->esriShapeFP) ;
 fwrite(&esriFileIoP->yMin,4,1,esriFileIoP->esriShapeFP) ;
 fwrite(&esriFileIoP->xMax,4,1,esriFileIoP->esriShapeFP) ;
 fwrite(&esriFileIoP->yMax,4,1,esriFileIoP->esriShapeFP) ;
 fwrite(&esriFileIoP->zMin,4,1,esriFileIoP->esriShapeFP) ;
 fwrite(&esriFileIoP->zMax,4,1,esriFileIoP->esriShapeFP) ;
 fclose(esriFileIoP->esriShapeFP) ;
 esriFileIoP->esriShapeFP = NULL ;
/*
** Finalise Index File
*/
 esriFileIoP->indexFilePosition = esriFileIoP->indexFilePosition / 2 ;
 dtmCnv_swapWordArray((short*)&esriFileIoP->indexFilePosition,1) ;
 dtmCnv_swapByteArray((char *)&esriFileIoP->indexFilePosition,2) ;
 fseek(esriFileIoP->esriIndexFP,24,SEEK_SET) ;
 fwrite(&esriFileIoP->indexFilePosition,4,1,esriFileIoP->esriIndexFP) ;
 fseek(esriFileIoP->esriIndexFP,36,SEEK_SET) ;
 fwrite(&esriFileIoP->xMin,4,1,esriFileIoP->esriIndexFP) ;
 fwrite(&esriFileIoP->yMin,4,1,esriFileIoP->esriIndexFP) ;
 fwrite(&esriFileIoP->xMax,4,1,esriFileIoP->esriIndexFP) ;
 fwrite(&esriFileIoP->yMax,4,1,esriFileIoP->esriIndexFP) ;
 fwrite(&esriFileIoP->zMin,4,1,esriFileIoP->esriIndexFP) ;
 fwrite(&esriFileIoP->zMax,4,1,esriFileIoP->esriIndexFP) ;
 fclose(esriFileIoP->esriIndexFP) ;
 esriFileIoP->esriIndexFP = NULL ;
/*
** Return
*/
 return(DTM_SUCCESS);
}
#endif
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int  bcdtmFormatEsri_callBackFunction(DTMFeatureType dtmFeatureType,DTMUserTag userTag, DTMFeatureId dtmFeatureId,DPoint3d *featurePtsP,size_t numFeaturePts,void *userP)
/*
** Call Back Function 
*/
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 int  bytesWritten=0 ;
 EsriFileInfo  *esriFileInfoP=NULL ;
 double xMin,yMin,zMin,xMax,yMax,zMax ;
 char   dbRecord[256] ;
 DVector3d boundingCube ;
 DPoint3d  *p3dP ;
 long numParts=1;
 UInt32 parts[10] ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Esri Call Back Function ** dtmFeatureType = %4ld numFeaturePts = %8ld",dtmFeatureType,numFeaturePts) ;
/*
** Write Different Features To Different Shape Files 
*/
 esriFileInfoP = ( EsriFileInfo * ) userP ;
/*
** Check For New DTM Feature Type
*/
 if( esriFileInfoP->shp == -1 )
   {
/*
**  Open ESRI Files
*/  
    if( dbg ) bcdtmWrite_message(0,0,0,"Opening Esri Files") ; 
    switch(dtmFeatureType)
      {
       case DTMFeatureType::RandomSpots :
       case DTMFeatureType::GroupSpots :
         if( bcdtmFormatEsri_openEsriShapeFilesForExport(esriFileInfoP,L"_points",SHAPEFILE_TYPE_MultiPointZ)) goto errexit ;
       break ;

       case DTMFeatureType::Breakline :
         if( bcdtmFormatEsri_openEsriShapeFilesForExport(esriFileInfoP,L"_breakLines",SHAPEFILE_TYPE_PolyLineZ)) goto errexit ;
       break ;
 
       case DTMFeatureType::SoftBreakline    :
       case DTMFeatureType::GraphicBreak :
         if( bcdtmFormatEsri_openEsriShapeFilesForExport(esriFileInfoP,L"_softBreakLines",SHAPEFILE_TYPE_PolyLineZ)) goto errexit ;
       break ;
 
       case DTMFeatureType::ContourLine :
         if( bcdtmFormatEsri_openEsriShapeFilesForExport(esriFileInfoP,L"_contourLines",SHAPEFILE_TYPE_PolyLineZ)) goto errexit ;
       break ;
 
       case DTMFeatureType::Void  :
       case DTMFeatureType::BreakVoid :
         if( bcdtmFormatEsri_openEsriShapeFilesForExport(esriFileInfoP,L"_voids",SHAPEFILE_TYPE_PolygonZ)) goto errexit ;
       break ;
 
       case DTMFeatureType::Island :
         if( bcdtmFormatEsri_openEsriShapeFilesForExport(esriFileInfoP,L"_islands",SHAPEFILE_TYPE_PolygonZ)) goto errexit ;
       break ;

       default :
         bcdtmWrite_message(0,0,0,"Unexpected Dtm Feature Type") ;
         goto errexit ;
       break ;
     } ;
/*
**  Initialise Bounding Cube
*/
    esriFileInfoP->xMin = esriFileInfoP->xMax = featurePtsP->x ;
    esriFileInfoP->yMin = esriFileInfoP->yMax = featurePtsP->y ;
    esriFileInfoP->zMin = esriFileInfoP->zMax = featurePtsP->z ;
/*
**  Set Record Number
*/
    esriFileInfoP->recordNumber = 1 ;
  }
/*
** Get Bounding Cube For Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Bounding Cube") ; 
 xMin = xMax = featurePtsP->x ;
 yMin = yMax = featurePtsP->y ;
 zMin = zMax = featurePtsP->z ;
 for( p3dP = featurePtsP + 1 ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
   {
    if( p3dP->x < xMin ) xMin = p3dP->x ;
    if( p3dP->x > xMax ) xMax = p3dP->x ;
    if( p3dP->y < yMin ) yMin = p3dP->y ;
    if( p3dP->y > yMax ) yMax = p3dP->y ;
    if( p3dP->z < zMin ) zMin = p3dP->z ;
    if( p3dP->z > zMax ) zMax = p3dP->z ;
   }
/*
** Set Bounding Cube For All Features
*/
 if( xMin < esriFileInfoP->xMin ) esriFileInfoP->xMin = xMin ;
 if( xMax > esriFileInfoP->xMax ) esriFileInfoP->xMax = xMax ; 
 if( yMin < esriFileInfoP->yMin ) esriFileInfoP->yMin = yMin ;
 if( yMax > esriFileInfoP->yMax ) esriFileInfoP->yMax = yMax ; 
 if( zMin < esriFileInfoP->zMin ) esriFileInfoP->zMin = zMin ;
 if( zMax > esriFileInfoP->zMax ) esriFileInfoP->zMax = zMax ; 
/*
** Set Bounding Cube
*/
 boundingCube.org.x = xMin ;
 boundingCube.org.y = yMin ;
 boundingCube.org.z = zMin ;
 boundingCube.end.x = xMax ;
 boundingCube.end.y = yMax ;
 boundingCube.end.z = zMax ;
/*
** Initialise dbRecord
*/
 dbRecord[0] = 0 ;
/*
** Append Record To Esri File
*/
 switch(dtmFeatureType)
   {
    case DTMFeatureType::RandomSpots :
    case DTMFeatureType::GroupSpots :
      sprintf(dbRecord,"Point") ;
      if( dbg ) bcdtmWrite_message(0,0,0,"Appending MultiPointZ") ;
      bytesWritten = bcdtmFormatEsri_appendMultiPointZ( esriFileInfoP->shp,esriFileInfoP->shx,esriFileInfoP->dbf,esriFileInfoP->recordNumber,&boundingCube,(UInt32)numFeaturePts,( DPoint3d *) featurePtsP,dbRecord) ;
      if( bytesWritten == -1 )
        {
         bcdtmWrite_message(1,0,0,"Error Appending ESRI MultiPointZ Record") ;
         goto errexit ;
        }  
    break ;

    case DTMFeatureType::Breakline :
      if( dbg ) bcdtmWrite_message(0,0,0,"Appending PolyLineZ") ; 
      numParts=1 ;
      parts[0] = 0 ;
      bytesWritten = bcdtmFormatEsri_appendPolyLineZ( esriFileInfoP->shp, esriFileInfoP->shx, esriFileInfoP->dbf, esriFileInfoP->recordNumber,&boundingCube,numParts,(UInt32)numFeaturePts,parts,( DPoint3d *) featurePtsP,dbRecord) ;
      if( bytesWritten == -1 )
        {
         bcdtmWrite_message(1,0,0,"Error Appending ESRI PolyLineZ Record") ;
         goto errexit ;
        } 
    break ;
 
    case DTMFeatureType::SoftBreakline    :
    case DTMFeatureType::GraphicBreak :
      numParts=1 ;
      parts[0] = 0 ;
      bytesWritten = bcdtmFormatEsri_appendPolyLineZ( esriFileInfoP->shp, esriFileInfoP->shx, esriFileInfoP->dbf, esriFileInfoP->recordNumber,&boundingCube,numParts,(UInt32)numFeaturePts,parts,( DPoint3d *) featurePtsP,dbRecord) ;
      if( bytesWritten == -1 )
        {
         bcdtmWrite_message(1,0,0,"Error Appending ESRI PolyLineZ Record") ;
         goto errexit ;
        } 
    break ;
 
    case DTMFeatureType::ContourLine :
      numParts=1 ;
      parts[0] = 0 ;
      bytesWritten = bcdtmFormatEsri_appendPolyLineZ( esriFileInfoP->shp, esriFileInfoP->shx, esriFileInfoP->dbf, esriFileInfoP->recordNumber,&boundingCube,numParts,(UInt32)numFeaturePts,parts,( DPoint3d *) featurePtsP,dbRecord) ;
      if( bytesWritten == -1 )
        {
         bcdtmWrite_message(1,0,0,"Error Appending ESRI PolyLineZ Record") ;
         goto errexit ;
        } 
    break ;
 
    case DTMFeatureType::Void  :
    case DTMFeatureType::BreakVoid :
      numParts=1 ;
      parts[0] = 0 ;
      bytesWritten = bcdtmFormatEsri_appendPolyLineZ( esriFileInfoP->shp, esriFileInfoP->shx, esriFileInfoP->dbf, esriFileInfoP->recordNumber,&boundingCube,numParts,(UInt32)numFeaturePts,parts,( DPoint3d *) featurePtsP,dbRecord) ;
      if( bytesWritten == -1 )
        {
         bcdtmWrite_message(1,0,0,"Error Appending ESRI PolyLineZ Record") ;
         goto errexit ;
        } 
    break ;
 
    case DTMFeatureType::Island :
      numParts=1 ;
      parts[0] = 0 ;
      bytesWritten = bcdtmFormatEsri_appendPolyLineZ( esriFileInfoP->shp, esriFileInfoP->shx, esriFileInfoP->dbf, esriFileInfoP->recordNumber,&boundingCube,numParts,(UInt32)numFeaturePts,parts,( DPoint3d *) featurePtsP,dbRecord) ;
      if( bytesWritten == -1 )
        {
         bcdtmWrite_message(1,0,0,"Error Appending ESRI PolyLineZ Record") ;
         goto errexit ;
        } 
    break ;

    default :
      bcdtmWrite_message(0,0,0,"Unexpected Dtm Feature Type") ;
      goto errexit ;
    break ;
  } ;
/*
** Update File Stats
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Updating File Stats") ;
 ++esriFileInfoP->recordNumber ;  
 esriFileInfoP->fileLength = esriFileInfoP->fileLength + bytesWritten ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Esri Call Back Function Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Esri Call Back  Function Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}

#ifdef NOTNEEDED
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int  bcdtmFormatEsri_callBackFunctionOld(DTMFeatureType dtmFeatureType,DTMUserTag userTag, DTMFeatureId dtmFeatureId,DPoint3d *featurePtsP,long numFeaturePts,void *userP)
/*
** Call Back Function 
*/
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long recordSize=0,recordNumber=0,contentSize=0,indexFilePosition=0 ;
 EsriFileIo  *esriFileIoP=NULL ;
 FILE *esriShapeFP=NULL,*esriIndexFP=NULL ;
 wchar_t esriShapeFileName[256] ;   
 wchar_t esriIndexFileName[256] ;   
 double xMin,yMin,zMin,xMax,yMax,zMax ;
 EsriFileHeader esriFileHeader ;
 DPoint3d  *p3dP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Esri Call Back Function ** dtmFeatureType = %4ld numFeaturePts = %8ld",dtmFeatureType,numFeaturePts) ;
/*
** Write Different Features To Different Shape Files 
*/
 esriFileIoP = ( EsriFileIo * ) userP ;
 esriShapeFP =  esriFileIoP->esriShapeFP ;
 esriIndexFP =  esriFileIoP->esriIndexFP ;
/*
** Check For New Header Record
*/
 if( esriFileIoP->esriShapeFP == NULL )
   {
/*
**  Set Header Variables
*/
    esriFileHeader.fileCode   = 9994 ;
    esriFileHeader.fileLength = 0  ;
    esriFileHeader.unUsed1    = 0  ;
    esriFileHeader.unUsed2    = 0 ;
    esriFileHeader.unUsed3    = 0 ; 
    esriFileHeader.unUsed4    = 0 ;
    esriFileHeader.unUsed5    = 0 ;
    esriFileHeader.fileLength = 0 ;
    esriFileHeader.version    = 1000 ;
    esriFileHeader.shapeType  = 0 ;
    esriFileHeader.xMin       = featurePtsP->x ;
    esriFileHeader.yMin       = featurePtsP->y ;
    esriFileHeader.xMax       = featurePtsP->x ;
    esriFileHeader.yMax       = featurePtsP->y ; 
    esriFileHeader.zMin       = featurePtsP->z ;
    esriFileHeader.zMax       = featurePtsP->z ;
    esriFileHeader.mMin       = 0.0 ;
    esriFileHeader.mMax       = 0.0 ;
/*
**  Do BIG_ENDIAN Conversions
*/
    dtmCnv_swapWordArray((short*)&esriFileHeader.fileCode,1) ;
    dtmCnv_swapByteArray((char *)&esriFileHeader.fileCode,2) ;
/*
**  Set File Name From Feature Type
*/   
    esriFileHeader.shapeType  = 13 ;   // ESRI PolyLineZ 
    esriFileIoP->shapeType    = 18 ;
    wcscpy(esriShapeFileName,esriFileIoP->filePrefix) ;
    wcscpy(esriIndexFileName,esriFileIoP->filePrefix) ;
    switch(dtmFeatureType)
      {
       case DTMFeatureType::RandomSpots :
       case DTMFeatureType::GroupSpots :
         wcscat(esriShapeFileName,L"_points.shp") ;
         wcscat(esriIndexFileName,L"_points.shx") ;
         esriFileHeader.shapeType  = 18 ;  // ESRI Multipoint z 
         esriFileIoP->shapeType    = 18 ;
       break ;

       case DTMFeatureType::Breakline :
         wcscat(esriShapeFileName,L"_hardBreakLines.shp") ;
         wcscat(esriIndexFileName,L"_hardBreakLines.shx") ;
       break ;
 
       case DTMFeatureType::SoftBreakline    :
       case DTMFeatureType::GraphicBreak :
         wcscat(esriShapeFileName,L"_softBreakLines.shp") ;
         wcscat(esriIndexFileName,L"_softBreakLines.shx") ;
       break ;
 
       case DTMFeatureType::ContourLine :
         wcscat(esriShapeFileName,L"_contourLines.shp") ;
         wcscat(esriIndexFileName,L"_contourLines.shx") ;
       break ;
 
       case DTMFeatureType::Void  :
       case DTMFeatureType::BreakVoid :
         wcscat(esriShapeFileName,L"_voids.shp") ;
         wcscat(esriIndexFileName,L"_voids.shx") ;
       break ;
 
       case DTMFeatureType::Island :
         wcscat(esriShapeFileName,L"_islands.shp") ;
         wcscat(esriIndexFileName,L"_islands.shx") ;
       break ;

       default :
         bcdtmWrite_message(0,0,0,"Unexpected Dtm Feature Type") ;
         goto errexit ;
       break ;
     } ;
/*
**  Open Esri Shape File
*/
    esriFileIoP->esriShapeFP = bcdtmFile_open(esriShapeFileName,L"wb") ;
    if( esriFileIoP->esriShapeFP == NULL )
      {
       bcdtmWrite_message(0,0,0,"Error Opening Esri Shape File %s",esriShapeFileName) ; 
       goto errexit ;
      }
    esriFileIoP->esriIndexFP = bcdtmFile_open(esriIndexFileName,L"wb") ;
    if( esriFileIoP->esriShapeFP == NULL )
      {
       bcdtmWrite_message(0,0,0,"Error Opening Esri Index File %s",esriIndexFileName) ; 
       goto errexit ;
      }
/*
**  Write ESRI Shape File Header
*/
    if( fwrite(&esriFileHeader,sizeof(EsriFileHeader),1,esriFileIoP->esriShapeFP) != 1 ) 
      {
       bcdtmWrite_message(0,0,0,"Error Writing Esri Shape File Header") ; 
       goto errexit ;
      }
/*
**  Write ESRI Index File Header
*/
    if( fwrite(&esriFileHeader,sizeof(EsriFileHeader),1,esriFileIoP->esriIndexFP) != 1 ) 
      {
       bcdtmWrite_message(0,0,0,"Error Writing Esri Index File Header") ; 
       goto errexit ;
      }
/*
**  Inc File Length
*/
   esriFileIoP->filePosition = esriFileIoP->filePosition + sizeof(EsriFileHeader) ;
   esriFileIoP->indexFilePosition = sizeof(EsriFileHeader) ;
   if( dbg ) bcdtmWrite_message(0,0,0,"**** esriFileIoP->filePosition = %8ld",esriFileIoP->filePosition) ;
  }
/*
** Get Bounding Cube For Feature
*/
 xMin = xMax = featurePtsP->x ;
 yMin = yMax = featurePtsP->y ;
 zMin = zMax = featurePtsP->z ;
 for( p3dP = featurePtsP + 1 ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
   {
    if( p3dP->x < xMin ) xMin = p3dP->x ;
    if( p3dP->x > xMax ) xMax = p3dP->x ;
    if( p3dP->y < yMin ) yMin = p3dP->y ;
    if( p3dP->y > yMax ) yMax = p3dP->y ;
    if( p3dP->z < zMin ) zMin = p3dP->y ;
    if( p3dP->z > zMax ) zMax = p3dP->z ;
   }
/*
** Set Bounding Cube For All Features
*/
 if( xMin < esriFileIoP->xMin ) esriFileIoP->xMin = xMin ;
 if( xMax > esriFileIoP->xMax ) esriFileIoP->xMax = xMax ; 
 if( yMin < esriFileIoP->yMin ) esriFileIoP->yMin = yMin ;
 if( yMax > esriFileIoP->yMax ) esriFileIoP->yMax = yMax ; 
 if( zMin < esriFileIoP->zMin ) esriFileIoP->zMin = zMin ;
 if( zMax > esriFileIoP->zMax ) esriFileIoP->zMax = zMax ; 
/*
**  Switch On Feature Type And Write Shape And Index Records
*/
 switch( dtmFeatureType )
   {
    case DTMFeatureType::RandomSpots :      // ESRI MultiPointz Records
    case DTMFeatureType::GroupSpots  :
/*
**    Set Content Size
*/
      contentSize  = 56 + 24 * numFeaturePts ;
      recordSize   = contentSize / 2   ;        // Expressed In 16 Bit Words
      indexFilePosition = esriFileIoP->filePosition / 2 ;   // Expressed In 16 Bit Words
      recordNumber = esriFileIoP->recordNumber ;
bcdtmWrite_message(0,0,0,"indexFilePosition = %8ld recordSize = %8ld",indexFilePosition,recordSize) ;
/*
**    Do Big Endian Conversions
*/
      dtmCnv_swapWordArray((short*)&recordSize,1) ;
      dtmCnv_swapByteArray((char *)&recordSize,2) ;
      dtmCnv_swapWordArray((short*)&recordNumber,1) ;
      dtmCnv_swapByteArray((char *)&recordNumber,2) ;
      dtmCnv_swapWordArray((short*)&indexFilePosition,1) ;
      dtmCnv_swapByteArray((char *)&indexFilePosition,2) ;
/*
**    Write Index File Record
*/
      if( fwrite(&indexFilePosition,4,1,esriFileIoP->esriIndexFP)         != 1 ) goto errexit ; 
      if( fwrite(&recordSize,4,1,esriFileIoP->esriIndexFP)                != 1 ) goto errexit ; 
      esriFileIoP->indexFilePosition = esriFileIoP->indexFilePosition + 8 ;
/*
**    Write Record Header
*/
      if( fwrite(&recordNumber,4,1,esriFileIoP->esriShapeFP)              != 1 ) goto errexit ; 
      if( fwrite(&recordSize,4,1,esriFileIoP->esriShapeFP)                != 1 ) goto errexit ; 
      esriFileIoP->filePosition = esriFileIoP->filePosition + 8   ;
/*
**    Write Record Content
*/
      if( fwrite(&esriFileIoP->shapeType,4,1,esriFileIoP->esriShapeFP)    != 1 ) goto errexit ; 
      if( fwrite(&xMin,8,1,esriFileIoP->esriShapeFP)                      != 1 ) goto errexit ; 
      if( fwrite(&yMin,8,1,esriFileIoP->esriShapeFP)                      != 1 ) goto errexit ; 
      if( fwrite(&xMax,8,1,esriFileIoP->esriShapeFP)                      != 1 ) goto errexit ; 
      if( fwrite(&yMax,8,1,esriFileIoP->esriShapeFP)                      != 1 ) goto errexit ; 
      if( fwrite(&numFeaturePts,4,1,esriFileIoP->esriShapeFP)             != 1 ) goto errexit ; 
      for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
        {
         if( fwrite(&p3dP->x,8,1,esriFileIoP->esriShapeFP)                != 1 ) goto errexit ; 
         if( fwrite(&p3dP->y,8,1,esriFileIoP->esriShapeFP)                != 1 ) goto errexit ; 
        }
      if( fwrite(&zMin,8,1,esriFileIoP->esriShapeFP)                      != 1 ) goto errexit ; 
      if( fwrite(&zMax,8,1,esriFileIoP->esriShapeFP)                      != 1 ) goto errexit ; 
      for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
        {
         if( fwrite(&p3dP->z,8,1,esriFileIoP->esriShapeFP)                != 1 ) goto errexit ; 
        } 
     ++esriFileIoP->recordNumber ;
     esriFileIoP->filePosition = esriFileIoP->filePosition + contentSize   ;
 
    break ;

    case DTMFeatureType::Breakline    :    // ESRI PolyLineZ Records
    case DTMFeatureType::SoftBreakline    :
    case DTMFeatureType::GraphicBreak :
    case DTMFeatureType::ContourLine  :
    case DTMFeatureType::Void          :
    case DTMFeatureType::BreakVoid    :
    case DTMFeatureType::Island        :
    break ;

    default :
      bcdtmWrite_message(0,0,0,"Unexpected Dtm Feature Type") ;
      goto errexit ;
    break ;
   } ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Esri Call Back Function Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Esri Call Back  Function Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( esriFileIoP->esriShapeFP != NULL ) { fclose(esriFileIoP->esriShapeFP) ; esriFileIoP->esriShapeFP = NULL ; }
 goto cleanup ;
}
#endif
/*-----------------------------------------------------------------------------

function :                        bcdtmFormatEsri_appendDBaseRecord
contents :                        Miscellaneous Functions
description :                     Appends a dBase record to an open dbf file
return :                          int;number of bytes written to the file or -1 if an error is encountered
param :                           int;dbf;IN;file descriptor for open file
param :                           char*;pRecord;IN;pointer to the record to be written

-----------------------------------------------------------------------------*/
BENTLEYDTM_Private int        bcdtmFormatEsri_appendDBaseRecord
(
 int                              dbf,
 char                             *pRecord
 )
{
       int                        byteswritten = 0;
       long                       pos;

       // move pointer to start of file
       if ((pos = _lseek (dbf, 0x00, SEEK_SET)) != -1)
       {
              int                        nRead;
              DBASEHeader                header;



              // read header and move file pointer to end of file
              if ((nRead = _read (dbf, &header, 0x20)) > 0 && (pos = _lseek (dbf, 0L, SEEK_END)) != -1)
              {
                     // output record
                    if ((byteswritten = _write (dbf, pRecord, header.rLength)) != -1)
                     {
                           // increment number of records in file
                           header.nRecords++;

                           // move pointer to start of file
                           if ((pos = _lseek (dbf, 0x04, SEEK_SET)) != -1L) byteswritten = _write (dbf, &header.nRecords, 0x04);
                           else byteswritten = -1;
                     }

              } else byteswritten = -1;

       } else byteswritten = -1;

       return (byteswritten);
}

/*-----------------------------------------------------------------------------

function :                        bcdtmFormatEsri_appendIndex
contents :                        Miscellaneous Functions
description :              Appends an index record to an open shx file
return :                          int;number of bytes written to the file or -1 if an error is encountered
param :                                  int;shx;IN;file descriptor for open file
param :                                  UInt32;offset;IN;the number of 16-bit words to the record in the main file
param :                                  UInt32;length;IN;the length of the record in the main file in 16-bit words

-----------------------------------------------------------------------------*/
BENTLEYDTM_Private int        bcdtmFormatEsri_appendIndex
(
 int                              shx,
 UInt32                           offset,
 UInt32                           length
 )
{
       int                               byteswritten = 0;
       long                       pos;
       ESRIIndexRecord            record = 
       {                                        // Position          Field                Value                Type          Byte Order
              dtmCnv_byteSwap4(offset / 2),            // Byte 0*           Offset               Offset               Integer              Big
              dtmCnv_byteSwap4(length / 2)             // Byte 4            Content Length       Content Length       Integer              Big
       };                                       // * offset is the number of 16-bit words from the start of the file

       // move file pointer to end of file
       if ((pos = _lseek (shx, 0L, SEEK_END)) != -1)
       {
              // output record
              byteswritten = _write (shx, &record, 0x08);
       }
       else byteswritten = -1;
       return (byteswritten);
}

#ifdef NOTNEEDED
/*-----------------------------------------------------------------------------

function :                        bcdtmFormatEsri_appendPoint
contents :                        Miscellaneous Functions
description :              Append a 2d point record to an open shp file
return :                          int;number of bytes written to the file or -1 if an error is encountered
param :                                  int;shp;IN;file descriptor for open shp file
param :                                  int;shx;IN;file descriptor for open shx file
param :                                  int;dbf;IN;file descriptor for open dbf file
param :                                  UInt32;record;IN;pointer to the record to be written
param :                                  DPoint3d*;pPoint;IN;pointer to the point to be appended
param :                                  char*;pRecord;IN;pointer to the record to be written

-----------------------------------------------------------------------------*/
BENTLEYDTM_Private int        bcdtmFormatEsri_appendPoint
(
 int                              shp,
 int                              shx,
 int                              dbf,
 UInt32                                  record,
 DPoint3d                         *pPoint,
 char                             *pRecord
 )
{
       int                               byteswritten = 0;
       int                               length = 4 + 8 * 2;
       long                       pos;
       ESRIRecordHeader     header =
       {                                                      // Position          Field                Value                Type          Byte Order
              dtmCnv_byteSwap4(record),                // Byte 0            Record Number Record Number Integer              Big
              dtmCnv_byteSwap4(length / 2)             // Byte 4*           Content Length       Content Length       Integer              Big
       };                                                     // * length of the record contents section measured in 16-bit words

       ESRIPoint                  point =
       {                                                      // Position          Field                Value                Type          Byte Order
              SHAPEFILE_TYPE_Point,             // Byte 0            Shape Type           1                           Integer              Little
              pPoint->x,                               // Byte 4            x                          x                           Double        Little
              pPoint->y,                               // Byte 12           y                          y                           Double        Little
       };

       // move file pointer to end of file
       if ((pos = _lseek (shp, 0L, SEEK_END)) != -1)
       {
              // output file header
              if (byteswritten != -1) byteswritten = _write (shp, &header.id, 0x04);
              if (byteswritten != -1) byteswritten = _write (shp, &header.length, 0x04);
              if (byteswritten != -1) byteswritten = _write (shp, &point.type, 0x04);
              if (byteswritten != -1) byteswritten = _write (shp, &point.x, 0x08);
              if (byteswritten != -1) byteswritten = _write (shp, &point.y, 0x08);

              // append index record
              if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendIndex (shx, pos, length);
              if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendDBaseRecord (dbf, pRecord);
       }
       else byteswritten = -1;
       return (byteswritten);
}

/*-----------------------------------------------------------------------------

function :                        bcdtmFormatEsri_appendPointZ
contents :                        Miscellaneous Functions
description :                     Append a 3d point record to an open shp file
return :                          int;number of bytes written to the file or -1 if an error is encountered
param :                                  int;shp;IN;file descriptor for open shp file
param :                                  int;shx;IN;file descriptor for open shx file
param :                                  int;dbf;IN;file descriptor for open dbf file
param :                                  UInt32;record;IN;pointer to the record to be written
param :                                  DPoint3d*;pPoint;IN;pointer to the point to be appended
param :                                  char*;pRecord;IN;pointer to the record to be written

-----------------------------------------------------------------------------*/
BENTLEYDTM_Private int        bcdtmFormatEsri_appendPointZ
(
 int                              shp,
 int                              shx,
 int                              dbf,
 UInt32                           record,
 DPoint3d                         *pPoint,
 char                             *pRecord
 )
{
       int                        byteswritten = 0;
       int                        length = 4 + 8 * 4;
       long                       pos;
       ESRIRecordHeader     header =
       {                                               // Position          Field                Value                Type          Byte Order
              dtmCnv_byteSwap4(record),                // Byte 0            Record Number Record Number               Integer       Big
              dtmCnv_byteSwap4(length / 2)             // Byte 4*           Content Length       Content Length       Integer       Big
       };                                              // * length of the record contents section measured in 16-bit words

       ESRIPointZ                 point =
       {                                               // Position          Field                Value                Type          Byte Order
              SHAPEFILE_TYPE_PointZ,                   // Byte 0            Shape Type           11                   Integer       Little
              pPoint->x,                               // Byte 4            x                    x                    Double        Little
              pPoint->y,                               // Byte 12           y                    y                    Double        Little
              pPoint->z,                               // Byte 20           z                    z                    Double        Little
              0x00                                     // Byte 28           Measure              M                    Double        Little
       };

       // move file pointer to end of file
       if ((pos = _lseek (shp, 0L, SEEK_END)) != -1)
       {
              // output file header
              if (byteswritten != -1) byteswritten = _write (shp, &header.id, 0x04);
              if (byteswritten != -1) byteswritten = _write (shp, &header.length, 0x04);
              if (byteswritten != -1) byteswritten = _write (shp, &point.type, 0x04);
              if (byteswritten != -1) byteswritten = _write (shp, &point.x, 0x08);
              if (byteswritten != -1) byteswritten = _write (shp, &point.y, 0x08);
              if (byteswritten != -1) byteswritten = _write (shp, &point.z, 0x08);
              if (byteswritten != -1) byteswritten = _write (shp, &point.M, 0x08);

              // append index record
              if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendIndex (shx, pos, length);
              if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendDBaseRecord (dbf, pRecord);
       }
       else byteswritten = -1;
       return (byteswritten);
}

/*-----------------------------------------------------------------------------

function :                        bcdtmFormatEsri_appendPolyLine
contents :                        Miscellaneous Functions
description :                     Append a 2d polyline record to an open shp file
return :                          int;number of bytes written to the file or -1 if an error is encountered
param :                                  int;shp;IN;file descriptor for open shp file
param :                                  int;shx;IN;file descriptor for open shx file
param :                                  int;dbf;IN;file descriptor for open dbf file
param :                                  UInt32;record;IN;pointer to the record to be written
param :                                  DVector3d*;pRange;IN;pointer to the element range
param :                                  UInt32;nPoints;IN;number of points in record
param :                                  DPoint3d*;pPoints;IN;pointer to the points to be appended
param :                                  char*;pRecord;IN;pointer to the record to be written

-----------------------------------------------------------------------------*/
BENTLEYDTM_Private int        bcdtmFormatEsri_appendPolyLine
(
 int                              shp,
 int                              shx,
 int                              dbf,
 UInt32                           record,
 DVector3d                        *pRange,
 UInt32                           nParts,
 UInt32                           nPoints,
 UInt32                           *pParts,
 DPoint3d                         *pPoints,
 char                             *pRecord
 )
{
       int                               byteswritten = 0;
       int                               length = 4 + 8 * 4 + 4 * 3 + nPoints * 16;
       ESRIRecordHeader     header =
       {                                        // Position          Field                Value                Type          Byte Order
              dtmCnv_byteSwap4(record),                // Byte 0            Record Number Record Number Integer              Big
              dtmCnv_byteSwap4(length / 2)             // Byte 4*           Content Length       Content Length       Integer              Big
       };                                       // * length of the record contents section measured in 16-bit words

       ESRIPolyLine         line =
       {                                                      // Position          Field                Value                Type          Byte Order
              SHAPEFILE_TYPE_PolyLine,   // Byte 0            Shape Type           3                           Integer              Little
              pRange->org.x,                           // Byte 4            Xmin                 Xmin                 Double        Little
              pRange->org.y,                           // Byte 12           Ymin                 Ymin                 Double        Little
              pRange->end.x,                           // Byte 20           Xmax                 Xmax                 Double        Little
              pRange->end.y,                           // Byte 28           Ymax                 Ymax                 Double        Little
              nParts,                                         // Byte 36           NumParts             nParts               Integer              Little
              nPoints,                                 // Byte 40           NumPoints            nPoints                     Integer              Little
              pParts,                                         // Byte 44           Parts                pParts               Integer              Little
              NULL,                                    // Byte x (48)       Points               pPoints(x,y)  Double        Little
       };                                                     // x = 44 + (4 * NumParts)

       // allocate memory for XY coordinates
 //      if ((line.pPoints = (DPoint2d *) calloc (line.nPoints, sizeof (DPoint2d))) != NULL)
       if ((line.pPoints = (DPoint2d *) calloc (line.nPoints, sizeof (DPoint2d))) != NULL)
       {
              long                       pos;
              UInt32                     i;

              // loop through all points
              for (i = 0; i < line.nPoints; i++)
              {
                     // record coordinate values
                     line.pPoints[i].x = pPoints[i].x;
                     line.pPoints[i].y = pPoints[i].y;
              }

              // move file pointer to end of file
              if ((pos = _lseek (shp, 0L, SEEK_END)) != -1)
              {
                     // output file header
                     if (byteswritten != -1) byteswritten = _write (shp, &header.id, 0x04);
                     if (byteswritten != -1) byteswritten = _write (shp, &header.length, 0x04);
                     if (byteswritten != -1) byteswritten = _write (shp, &line.type, 0x04);
                     if (byteswritten != -1) byteswritten = _write (shp, &line.Xmin, 0x08);
                     if (byteswritten != -1) byteswritten = _write (shp, &line.Ymin, 0x08);
                     if (byteswritten != -1) byteswritten = _write (shp, &line.Xmax, 0x08);
                     if (byteswritten != -1) byteswritten = _write (shp, &line.Ymax, 0x08);
                     if (byteswritten != -1) byteswritten = _write (shp, &line.nParts, 0x04);
                     if (byteswritten != -1) byteswritten = _write (shp, &line.nPoints, 0x04);
                     if (byteswritten != -1) byteswritten = _write (shp, line.pParts, 0x04 * line.nParts);
                     if (byteswritten != -1) byteswritten = _write (shp, line.pPoints, 0x10 * line.nPoints);

                     // append index record
                     if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendIndex (shx, pos, length);
                     if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendDBaseRecord (dbf, pRecord);
              }
              else byteswritten = -1;

              // free allocated memory
              free (line.pPoints);
       }
       else byteswritten = -1;
       return (byteswritten);
}
#endif
/*-----------------------------------------------------------------------------

function :                        bcdtmFormatEsri_appendPolyLine
contents :                        Miscellaneous Functions
description :                     Append a 3d polyline record to an open shp file
return :                          int;number of bytes written to the file or -1 if an error is encountered
param :                           int;shp;IN;file descriptor for open shp file
param :                           int;shx;IN;file descriptor for open shx file
param :                           int;dbf;IN;file descriptor for open dbf file
param :                           UInt32;record;IN;pointer to the record to be written
param :                           DVector3d*;pRange;IN;pointer to the element range
param :                           UInt32;nPoints;IN;number of points in record
param :                           DPoint3d*;pPoints;IN;pointer to the points to be appended
param :                           char*;pRecord;IN;pointer to the record to be written

-----------------------------------------------------------------------------*/
BENTLEYDTM_Private int        bcdtmFormatEsri_appendPolyLineZ
(
 int                              shp,
 int                              shx,
 int                              dbf,
 UInt32                           record,
 DVector3d                        *pRange,
 UInt32                           nParts,
 UInt32                           nPoints,
 UInt32                           *pParts,
 DPoint3d                         *pPoints,
 char                             *pRecord
 )
{
       int                               byteswritten = 0;
       int                               length = 4 + 8 * 4 + 4 * 3 + nPoints * 16 + 8 * 2 + nPoints * 8;
       ESRIRecordHeader     header =
       {                                               // Position          Field                Value                Type          Byte Order
              dtmCnv_byteSwap4(record),                // Byte 0            Record Number        Record Number        Integer       Big
              dtmCnv_byteSwap4(length / 2)             // Byte 4*           Content Length       Content Length       Integer       Big
       };                                              // * length of the record contents section measured in 16-bit words

       ESRIPolyLineZ        line =
       {                                               // Position          Field                Value                Type          Byte Order
              SHAPEFILE_TYPE_PolyLineZ,                // Byte 0            Shape Type           13                   Integer       Little
              pRange->org.x,                           // Byte 4            Xmin                 Xmin                 Double        Little
              pRange->org.y,                           // Byte 12           Ymin                 Ymin                 Double        Little
              pRange->end.x,                           // Byte 20           Xmax                 Xmax                 Double        Little
              pRange->end.y,                           // Byte 28           Ymax                 Ymax                 Double        Little
              nParts,                                  // Byte 36           NumParts             nParts               Integer       Little
              nPoints,                                 // Byte 40           NumPoints            nPoints              Integer       Little
              pParts,                                  // Byte 44           Parts                pParts               Integer       Little
              NULL,                                    // Byte x (48)       Points               pPoints(x,y)         Double        Little
              pRange->org.z,                           // Byte y            Zmin                 Zmin                 Double        Little
              pRange->end.z,                           // Byte y + 8        Zmax                 Zmax                 Double        Little
              NULL                                     // Byte y + 16       Zarray               Zarray               Double        Little
       };                                              // x = 44 + (4 * NumParts), y = x + (16 * NumPoints)

       // allocate memory for XY coordinates
       if ((line.pPoints = (DPoint2d *) calloc (line.nPoints, sizeof (DPoint2d))) != NULL)
       {
              // allocate memory for z coordinates
              if ((line.pPointsZ = (double *) calloc (line.nPoints, sizeof (double))) != NULL)
              {
                     long                       pos;
                     UInt32                     i;

                     // loop through all points
                     for (i = 0; i < line.nPoints; i++)
                     {
                           // record coordinate values
                           line.pPoints[i].x = pPoints[i].x;
                           line.pPoints[i].y = pPoints[i].y;
                           line.pPointsZ[i] = pPoints[i].z;
                     }

                     // move file pointer to end of file
                     if ((pos = _lseek (shp, 0L, SEEK_END)) != -1)
                     {
                           // output file header
                           if (byteswritten != -1) byteswritten = _write (shp, &header.id, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, &header.length, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, &line.type, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, &line.Xmin, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &line.Ymin, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &line.Xmax, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &line.Ymax, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &line.nParts, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, &line.nPoints, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, line.pParts, 0x04 * line.nParts);
                           if (byteswritten != -1) byteswritten = _write (shp, line.pPoints, 0x10 * line.nPoints);
                           if (byteswritten != -1) byteswritten = _write (shp, &line.Zmin, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &line.Zmax, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, line.pPointsZ, 0x08 * line.nPoints);

                           // append index record
                           if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendIndex (shx, pos, length);
                           if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendDBaseRecord (dbf, pRecord);
                     }
                     else byteswritten = -1;

                     // free allocated memory
                     free (line.pPointsZ);
              }
              else byteswritten = -1;

              // free allocated memory
              free (line.pPoints);
       }
       else byteswritten = -1;
       return (byteswritten);
}
#ifdef NOTDEFINED
/*-----------------------------------------------------------------------------

function :                        bcdtmFormatEsri_appendPolygon
contents :                        Miscellaneous Functions
description :                     Append a 2d polygon record to an open shp file
return :                           int;number of bytes written to the file or -1 if an error is encountered
param :                                  int;shp;IN;file descriptor for open shp file
param :                                  int;shx;IN;file descriptor for open shx file
param :                                  int;dbf;IN;file descriptor for open dbf file
param :                                   UInt32;record;IN;pointer to the record to be written
param :                                  DVector3d*;pRange;IN;pointer to the element range
param :                                  UInt32;nParts;IN;number of parts in record
param :                                  UInt32;nPoints;IN;number of points in record
param :                                  UInt32*;pParts;IN;pointer to the parts indexes to be appended
param :                                  DPoint3d*;pPoints;IN;pointer to the points to be appended
param :                                  char*;pRecord;IN;pointer to the record to be written

-----------------------------------------------------------------------------*/
BENTLEYDTM_Private int        bcdtmFormatEsri_appendPolygon
(
 int                              shp,
 int                              shx,
 int                              dbf,
 UInt32                           record,
 DVector3d                        *pRange,
 UInt32                           nParts,
 UInt32                           nPoints,
 UInt32                           *pParts,
 DPoint3d                         *pPoints,
 char                             *pRecord
 )
{
       int                               byteswritten = 0;
       int                               length = 4 + 8 * 4 + 4 + 4 + nParts * 4 + nPoints * 16;
       ESRIRecordHeader     header =
       {                                        // Position          Field                Value                Type          Byte Order
              dtmCnv_byteSwap4(record),                // Byte 0            Record Number Record Number Integer              Big
              dtmCnv_byteSwap4(length / 2)             // Byte 4*           Content Length       Content Length       Integer              Big
       };                                       // * length of the record contents section measured in 16-bit words

       ESRIPolygon                polygon =
       {                                                      // Position          Field                Value                Type          Byte Order
              SHAPEFILE_TYPE_Polygon,           // Byte 0            Shape Type           3                           Integer              Little
              pRange->org.x,                           // Byte 4            Xmin                 Xmin                 Double        Little
              pRange->org.y,                           // Byte 12           Ymin                 Ymin                 Double        Little
              pRange->end.x,                           // Byte 20           Xmax                 Xmax                 Double        Little
              pRange->end.y,                           // Byte 28           Ymax                 Ymax                 Double        Little
              nParts,                                         // Byte 36           NumParts             nParts               Integer              Little
              nPoints,                                 // Byte 40           NumPoints            nPoints                     Integer              Little
              pParts,                                         // Byte 44           Parts                pParts               Integer              Little
              NULL,                                    // Byte x            Points               pPoints(x,y)  Double        Little
       };                                                     // x = 44 + (4 * NumParts)

       // allocate memory for XY coordinates
       if ((polygon.pPoints = (DPoint2d *) calloc (polygon.nPoints, sizeof (DPoint2d))) != NULL)
       {
              long                       pos;
              UInt32                     i;

              // loop through all points
              for (i = 0; i < polygon.nPoints; i++)
              {
                     // record coordinate values
                     polygon.pPoints[i].x = pPoints[i].x;
                     polygon.pPoints[i].y = pPoints[i].y;
              }

              // move file pointer to end of file
              if ((pos = _lseek (shp, 0L, SEEK_END)) != -1)
              {
                     // output file header
                     if (byteswritten != -1) byteswritten = _write (shp, &header.id, 0x04);
                     if (byteswritten != -1) byteswritten = _write (shp, &header.length, 0x04);
                     if (byteswritten != -1) byteswritten = _write (shp, &polygon.type, 0x04);
                     if (byteswritten != -1) byteswritten = _write (shp, &polygon.Xmin, 0x08);
                     if (byteswritten != -1) byteswritten = _write (shp, &polygon.Ymin, 0x08);
                     if (byteswritten != -1) byteswritten = _write (shp, &polygon.Xmax, 0x08);
                     if (byteswritten != -1) byteswritten = _write (shp, &polygon.Ymax, 0x08);
                     if (byteswritten != -1) byteswritten = _write (shp, &polygon.nParts, 0x04);
                     if (byteswritten != -1) byteswritten = _write (shp, &polygon.nPoints, 0x04);
                     if (byteswritten != -1) byteswritten = _write (shp, polygon.pParts, 0x04 * polygon.nParts);
                     if (byteswritten != -1) byteswritten = _write (shp, polygon.pPoints, 0x10 * polygon.nPoints);

                     // append index record
                     if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendIndex (shx, pos, length);
                     if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendDBaseRecord (dbf, pRecord);
              }
              else byteswritten = -1;

              // free allocated memory
              free (polygon.pPoints);
       }
       else byteswritten = -1;
       return (byteswritten);
}

/*-----------------------------------------------------------------------------

function :                        bcdtmFormatEsri_appendPolygonZ
contents :                        Miscellaneous Functions
description :                     Append a 3d polygon record to an open shp file
return :                          int;number of bytes written to the file or -1 if an error is encountered
param :                                  int;shp;IN;file descriptor for open shp file
param :                                  int;shx;IN;file descriptor for open shx file
param :                                  int;dbf;IN;file descriptor for open dbf file
param :                                  UInt32;record;IN;pointer to the record to be written
param :                                  DVector3d*;pRange;IN;pointer to the element range
param :                                  UInt32;nParts;IN;number of parts in record
param :                                  UInt32;nPoints;IN;number of points in record
param :                                  UInt32*;pParts;IN;pointer to the parts indexes to be appended
param :                                  DPoint3d*;pPoints;IN;pointer to the points to be appended
param :                                  char*;pRecord;IN;pointer to the record to be written

-----------------------------------------------------------------------------*/
BENTLEYDTM_Private int        bcdtmFormatEsri_appendPolygonZ
(
 int                              shp,
 int                              shx,
 int                              dbf,
 UInt32                           record,
 DVector3d                        *pRange,
 UInt32                           nParts,
 UInt32                           nPoints,
 UInt32                           *pParts,
 DPoint3d                         *pPoints,
 char                             *pRecord
 )
{
       int                               byteswritten = 0;
       int                               length = 4 + 8 * 4 + 4 + 4 + nParts * 4 + nPoints * 16 + 8 * 2 + nPoints * 8;
       ESRIRecordHeader     header =
       {                                               // Position          Field                Value                Type          Byte Order
              dtmCnv_byteSwap4(record),                // Byte 0            Record Number Record Number Integer              Big
              dtmCnv_byteSwap4(length / 2)             // Byte 4*           Content Length       Content Length       Integer              Big
       };                                              // * length of the record contents section measured in 16-bit words

       ESRIPolygonZ         polygon =
       {                                               // Position          Field                Value                Type          Byte Order
              SHAPEFILE_TYPE_PolygonZ,                 // Byte 0            Shape Type           13                   Integer       Little
              pRange->org.x,                           // Byte 4            Xmin                 Xmin                 Double        Little
              pRange->org.y,                           // Byte 12           Ymin                 Ymin                 Double        Little
              pRange->end.x,                           // Byte 20           Xmax                 Xmax                 Double        Little
              pRange->end.y,                           // Byte 28           Ymax                 Ymax                 Double        Little
              nParts,                                  // Byte 36           NumParts             nParts               Integer       Little
              nPoints,                                 // Byte 40           NumPoints            nPoints              Integer       Little
              pParts,                                  // Byte 44           Parts                pParts               Integer       Little
              NULL,                                    // Byte x            Points               pPoints(x,y)         Double        Little
              pRange->org.z,                           // Byte y            Zmin                 Zmin                 Double        Little
              pRange->end.z,                           // Byte y + 8        Zmax                 Zmax                 Double        Little
              NULL                                     // Byte y + 16       Zarray               Zarray               Double        Little
       };                                              // x = 44 + (4 * NumParts), y = x + (16 * NumPoints)

       // allocate memory for XY coordinates
       if ((polygon.pPoints = (DPoint2d *) calloc (polygon.nPoints, sizeof (DPoint2d))) != NULL)
       {
              // allocate memory for z coordinates
              if ((polygon.pPointsZ = (double *) calloc (polygon.nPoints, sizeof (double))) != NULL)
              {
                     long                       pos;
                     UInt32                     i;

                     // loop through all points
                     for (i = 0; i < polygon.nPoints; i++)
                     {
                           // record coordinate values
                           polygon.pPoints[i].x = pPoints[i].x;
                           polygon.pPoints[i].y = pPoints[i].y;
                           polygon.pPointsZ[i] = pPoints[i].z;
                     }

                     // move file pointer to end of file
                     if ((pos = _lseek (shp, 0L, SEEK_END)) != -1)
                     {
                           // output file header
                           if (byteswritten != -1) byteswritten = _write (shp, &header.id, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, &header.length, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, &polygon.type, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, &polygon.Xmin, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &polygon.Ymin, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &polygon.Xmax, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &polygon.Ymax, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &polygon.nParts, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, &polygon.nPoints, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, polygon.pParts, 0x04 * polygon.nParts);
                           if (byteswritten != -1) byteswritten = _write (shp, polygon.pPoints, 0x10 * polygon.nPoints);
                           if (byteswritten != -1) byteswritten = _write (shp, &polygon.Zmin, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &polygon.Zmax, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, polygon.pPointsZ, 0x08 * polygon.nPoints);

                           // append index and Xbase record
                           if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendIndex (shx, pos, length);
                           if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendDBaseRecord (dbf, pRecord);
                     }
                     else byteswritten = -1;

                     // free allocated memory
                     free (polygon.pPointsZ);
              }
              else byteswritten = -1;

              // free allocated memory
              free (polygon.pPoints);
       }
       else byteswritten = -1;
       return (byteswritten);
}
#endif
/*-----------------------------------------------------------------------------

function :                        bcdtmFormatEsri_appendMultiPointZ
contents :                        Miscellaneous Functions
description :                     Appends a multi point array to an open shp file
return :                          int;number of bytes written to the file or -1 if an error is encountered
param :                                  int;shp;IN;file descriptor for open shp file
param :                                  int;shx;IN;file descriptor for open shx file
param :                                  int;dbf;IN;file descriptor for open dbf file
param :                                  UInt32;record;IN;pointer to the record to be written
param :                                  DVector3d*;pRange;IN;pointer to the element range
param :                                  UInt32;nParts;IN;number of parts in record
param :                                  UInt32;nPoints;IN;number of points in record
param :                                  UInt32*;pParts;IN;pointer to the parts indexes to be appended
param :                                  DPoint3d*;pPoints;IN;pointer to the points to be appended
param :                                  char*;pRecord;IN;pointer to the record to be written

-----------------------------------------------------------------------------*/
BENTLEYDTM_Private int        bcdtmFormatEsri_appendMultiPointZ
(
 int                              shp,
 int                              shx,
 int                              dbf,
 UInt32                           record,
 DVector3d                        *pRange,
 UInt32                           nPoints,
 DPoint3d                         *pPoints,
 char                             *pRecord
 )
{
       int                               byteswritten = 0;
       int                               length = 4 + 8 * 4 + 4 + nPoints * 16 + 8 * 2 + nPoints * 8  ;
       ESRIRecordHeader     header =
       {                                               // Position          Field                Value                Type          Byte Order
              dtmCnv_byteSwap4(record),                // Byte 0            Record Number        Record Number        Integer              Big
              dtmCnv_byteSwap4(length / 2)             // Byte 4*           Content Length       Content Length       Integer              Big
       };                                              // * length of the record contents section measured in 16-bit words

       ESRIMultiPointZ        multiPoint =
       {                                               // Position          Field                Value                Type          Byte Order
              SHAPEFILE_TYPE_MultiPointZ,              // Byte 0            Shape Type           18                   Integer       Little
              pRange->org.x,                           // Byte 4            Xmin                 Xmin                 Double        Little
              pRange->org.y,                           // Byte 12           Ymin                 Ymin                 Double        Little
              pRange->end.x,                           // Byte 20           Xmax                 Xmax                 Double        Little
              pRange->end.y,                           // Byte 28           Ymax                 Ymax                 Double        Little
              nPoints,                                 // Byte 36           NumPoints            nPoints              Integer       Little
              NULL,                                    // Byte x            Points               pPoints(x,y)         Double        Little
              pRange->org.z,                           // Byte y            Zmin                 Zmin                 Double        Little
              pRange->end.z,                           // Byte y + 8        Zmax                 Zmax                 Double        Little
              NULL                                     // Byte y + 16       Zarray               Zarray               Double        Little
       };                                              // x = 44 + (4 * NumParts), y = x + (16 * NumPoints)

       // allocate memory for XY coordinates
       if (( multiPoint.pPoints = (DPoint2d *) calloc (multiPoint.nPoints, sizeof (DPoint2d))) != NULL)
       {
              // allocate memory for z coordinates
              if (( multiPoint.pPointsZ = (double *) calloc (multiPoint.nPoints, sizeof (double))) != NULL)
              {
                     long                       pos;
                     UInt32                     i;

                     // loop through all points
                     for (i = 0; i < multiPoint.nPoints; i++)
                     {
                           // record coordinate values
                           multiPoint.pPoints[i].x = pPoints[i].x;
                           multiPoint.pPoints[i].y = pPoints[i].y;
                           multiPoint.pPointsZ[i]  = pPoints[i].z;
                     }

                     // move file pointer to end of file
                     if ((pos = _lseek (shp, 0L, SEEK_END)) != -1)
                     {
                           // output file header
                           if (byteswritten != -1) byteswritten = _write (shp, &header.id, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, &header.length, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, &multiPoint.type, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, &multiPoint.Xmin, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &multiPoint.Ymin, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &multiPoint.Xmax, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &multiPoint.Ymax, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &multiPoint.nPoints, 0x04);
                           if (byteswritten != -1) byteswritten = _write (shp, multiPoint.pPoints, 0x10 * multiPoint.nPoints);
                           if (byteswritten != -1) byteswritten = _write (shp, &multiPoint.Zmin, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, &multiPoint.Zmax, 0x08);
                           if (byteswritten != -1) byteswritten = _write (shp, multiPoint.pPointsZ, 0x08 * multiPoint.nPoints);

                           // append index and Xbase record
                           if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendIndex (shx, pos, length);
                           if (byteswritten != -1) byteswritten = bcdtmFormatEsri_appendDBaseRecord (dbf, pRecord);
                     }  else byteswritten = -1;

                     // free allocated memory
                     free (multiPoint.pPointsZ);
              }  else byteswritten = -1;

              // free allocated memory
              free (multiPoint.pPoints);
       }  else byteswritten = -1;
       return (byteswritten);
}

/*-----------------------------------------------------------------------------

function :                        bcdtmFormatEsri_writeDBaseHeader
contents :                         Miscellaneous Functions
description :              Writes the file header information for a newly created dbf file to be used with all record types except text records
return :                          int;number of bytes written to the file or -1 if an error is encountered
param :                                  int;fh;IN;file descriptor for open file

-----------------------------------------------------------------------------*/
BENTLEYDTM_Private int        bcdtmFormatEsri_writeDBaseHeader
(
 int                              fh
 )
{
       int                               i;
       int                               byteswritten = 0;
       byte                        fields[NUM_DBF_FIELDS][32] =
       {
              {'E','N','T','I','T','y',0,0,0,0,0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},             // element type
              {'L','A','y','E','R',0,0,0,0,0,0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},               // level name
              {'I','G','D','S','_','L','E','V','E','L',0,'N',0,0,0,0,0x05,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     // level number
              {'I','G','D','S','_','C','O','L','O','R',0,'N',0,0,0,0,0x05,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     // color (0 - 255)
              {'I','G','D','S','_','S','T','y','L','E',0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     // linestyle name
              {'W','E','I','G','H','T',0,0,0,0,0,'N',0,0,0,0,0x05,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},             // weight (0 - 31)
              {'G','R','O','U','P',0,0,0,0,0,0,'N',0,0,0,0,0x05,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},               // graphic group (0 - 65535)
              {'F','E','A','T','C','O','D','E',0,0,0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},         // feature code (remapped)
              {'C','A','T','E','G','O','R','y',0,0,0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},         // feature category (original)
              {'D','E','S','C','R','I','P','T',0,0,0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},         // feature description (original)
              {'H','E','I','G','H','T',0,0,0,0,0,'N',0,0,0,0,0x14,0x03,0,0,0,0,0,0,0,0,0,0,0,0,0,0},          // element elevation (points and contours)
       };
       long                       pos;
       UInt16                     terminator = 0x200D;
       UInt16                     hLength = 32 + NUM_DBF_FIELDS * 32 + 1;
       UInt16                     rLength = 0x20 + 0x20 + 0x05 + 0x05 + 0x20 + 0x05 + 0x05 + 0x20 + 0x20 + 0x20 + 0x14 + 1;
       time_t                     current = time (NULL);
       struct tm                  today = *localtime (&current);

       DBASEHeader                header =
       {                                               // Position          Field                Value                Type          Byte Order
              0x03,                                    // Byte 0            Version                    3                           Byte          Little
              (byte)today.tm_year,                           // Byte 1            Year                 Year - 1900          Byte          Little
              (byte)today.tm_mon + 1,                        // Byte 2            Month                1 to 12                     Byte          Little
              (byte)today.tm_mday,                           // Byte 3            Day                        1 to 31                   Byte          Little
              0,                                       // Byte 4            Number Records       nRecords             Integer              Little
              hLength,                                 // Byte 8            Header Length hLength                     Short         Little
              rLength,                                 // Byte 10           Record Length rLength                     Short         Little
              0,                                       // Byte 12           Reserved             0                           Short         Little
              0,                                       // Byte 14           Incomplete           0                           Byte          Little
              0,                                       // Byte 15           Encryption           0                           Byte          Little
              0,                                       // Byte 16           Free Record          0                           Integer              Little
              0,                                       // Byte 20           Multi User           0                           Integer              Little
              0,                                       // Byte 24           Multi User           0                           Integer              Little
              0,                                       // Byte 28           MDX Flag             0                           Byte          Little
              0,                                       // Byte 29           Language Driver      0                           Byte          Little
              0,                                       // Byte 30           Reserved             0                           Short         Little
       };

       // output file header
       if ((pos = _lseek (fh, 0x00, SEEK_SET)) != -1L) byteswritten = _write (fh, &header, 0x20);
       for (i = 0; byteswritten != -1 && i < NUM_DBF_FIELDS; i++) byteswritten = _write (fh, fields[i], 0x20);
       if (byteswritten != -1) byteswritten = _write (fh, &terminator, 0x02);
       return (byteswritten);
}

/*-----------------------------------------------------------------------------

function :                        bcdtmFormatEsri_writeDBaseTextHeader
contents :                        Miscellaneous Functions
description :              Writes the file header information for a newly created dbf file to be used with text records
return :                          int;number of bytes written to the file or -1 if an error is encountered
param :                                  int;fh;IN;file descriptor for open file

-----------------------------------------------------------------------------*/
#ifdef NOTNEEDED
BENTLEYDTM_Private int        bcdtmFormatEsri_writeDBaseTextHeader
(
 int                              fh
 )
{
       int                               i;
       int                               byteswritten = 0;
       byte                       fields[NUM_DBF_TEXT_FIELDS][32] =
       {
              {'E','N','T','I','T','y',0,0,0,0,0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},                    // element type
              {'L','A','y','E','R',0,0,0,0,0,0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},               // level name
              {'I','G','D','S','_','L','E','V','E','L',0,'N',0,0,0,0,0x05,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},       // level number
              {'I','G','D','S','_','C','O','L','O','R',0,'N',0,0,0,0,0x05,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},       // color (0 - 255)
              {'I','G','D','S','_','S','T','y','L','E',0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},       // linestyle name
              {'W','E','I','G','H','T',0,0,0,0,0,'N',0,0,0,0,0x05,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},                    // weight (0 - 31)
              {'G','R','O','U','P',0,0,0,0,0,0,'N',0,0,0,0,0x05,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},               // graphic group (0 - 65535)
              {'F','E','A','T','C','O','D','E',0,0,0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},         // feature code (remapped)
              {'C','A','T','E','G','O','R','y',0,0,0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},         // feature category (original)
              {'D','E','S','C','R','I','P','T',0,0,0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},         // feature description (original)
              {'H','E','I','G','H','T',0,0,0,0,0,'N',0,0,0,0,0x14,0x03,0,0,0,0,0,0,0,0,0,0,0,0,0,0},          // element elevation (points and contours)
              {'T','E','x','T','S','T','R','I','N','G',0,'C',0,0,0,0,0xFF,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},       // miscellaneous text
              {'T','E','x','T','F','O','N','T',0,0,0,'C',0,0,0,0,0x20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},         // font name
              {'T','E','x','T','S','I','z','E',0,0,0,'N',0,0,0,0,0x14,0X03,0,0,0,0,0,0,0,0,0,0,0,0,0,0},       // text size
              {'T','E','x','T','A','N','G','L','E',0,0,'N',0,0,0,0,0x14,0X03,0,0,0,0,0,0,0,0,0,0,0,0,0,0},// text rotation angle (degrees)
       };
       long                       pos;
       UInt16                      terminator = 0x200D;
       UInt16                     hLength = 32 + NUM_DBF_TEXT_FIELDS * 32 + 1;
       UInt16                     rLength = 0x20 + 0x20 + 0x05 + 0x05 + 0x20 + 0x05 + 0x05 + 0x20 + 0x20 + 0x20 + 0x14 + 0xFF + 0x20 + 0x14 + 0x14 + 1;
       time_t                     current = time (NULL);
       struct tm                  today = *localtime (&current);

       DBASEHeader                header =
       {                                                      // Position          Field                Value                Type          Byte Order
              0x03,                                    // Byte 0            Version                    3                           Byte          Little
              (byte)today.tm_year,                           // Byte 1            Year                 Year - 1900          Byte          Little
              today.tm_mon + 1,                 // Byte 2            Month                1 to 12                     Byte          Little
              (byte)today.tm_mday,                           // Byte 3            Day                        1 to 31                   Byte          Little
              0,                                              // Byte 4            Number Records       nRecords             Integer              Little
              hLength,                                 // Byte 8            Header Length hLength                     Short         Little
              rLength,                                 // Byte 10           Record Length rLength                     Short         Little
              0,                                              // Byte 12           Reserved             0                           Short         Little
              0,                                              // Byte 14           Incomplete           0                           Byte          Little
              0,                                              // Byte 15           Encryption           0                           Byte          Little
              0,                                              // Byte 16           Free Record          0                           Integer              Little
              0,                                              // Byte 20           Multi User           0                           Integer              Little
              0,                                              // Byte 24           Multi User           0                           Integer              Little
              0,                                              // Byte 28           MDX Flag             0                           Byte          Little
              0,                                              // Byte 29           Language Driver      0                           Byte          Little
              0,                                              // Byte 30           Reserved             0                           Short         Little
       };

       // output file header
       if ((pos = _lseek (fh, 0x00, SEEK_SET)) != -1L) byteswritten = _write (fh, &header, 0x20);
       for (i = 0; byteswritten != -1 && i < NUM_DBF_TEXT_FIELDS; i++) byteswritten = _write (fh, fields[i], 0x20);
       if (byteswritten != -1) byteswritten = _write (fh, &terminator, 0x02);
       return (byteswritten);
}
#endif
/*-----------------------------------------------------------------------------

function :                        bcdtmFormatEsri_writeFileHeader
contents :                        Miscellaneous Functions
description :                     Writes the file header information for a newly created shp file
return :                          int;number of bytes written to the file or -1 if an error is encountered
param :                                  int;fh;IN;file descriptor for open file
param :                                  UInt32;shapeType;IN;shape file type
param :                                  DVector3d*;pRange;IN;range of elements in file
param :                                  UInt16;mask;IN;bitmask used to selectively write portions of the header

-----------------------------------------------------------------------------*/
BENTLEYDTM_Private int        bcdtmFormatEsri_writeFileHeader
(
 int                              fh,
 UInt32                           shapeType,
 DVector3d                        *pRange,
 UInt16                           mask
 )
{
       int                        byteswritten = 0;
       long                       pos;
       ESRIHeader                 header =
       {                                               // Position          Field                Value                Type          Byte Order
              dtmCnv_byteSwap4(9994),                  // Byte 0            File Code            9994                 Integer       Big
              0,                                       // Byte 4            Unused               0                    Integer       Big
              0,                                       // Byte 8            Unused               0                    Integer       Big
              0,                                       // Byte 12           Unused               0                    Integer       Big
              0,                                       // Byte 16           Unused               0                    Integer       Big
              0,                                       // Byte 20           Unused               0                    Integer       Big
              0,                                       // Byte 24           File Length          File Length          Integer       Big
              1000,                                    // Byte 28           Version              1000                 Integer       Little
              shapeType,                               // Byte 32           Shape Type           Shape Type           Integer       Little
              fc_zero,                                 // Byte 36           Bounding Box         Xmin                 Double        Little
              fc_zero,                                 // Byte 44           Bounding Box         Ymin                 Double        Little
              fc_zero,                                 // Byte 52           Bounding Box         Xmax                 Double        Little
              fc_zero,                                 // Byte 60           Bounding Box         Ymax                 Double        Little
              fc_zero,                                 // Byte 68*          Bounding Box         Zmin                 Double        Little
              fc_zero,                                 // Byte 76*          Bounding Box         Zmax                 Double        Little
              fc_zero,                                 // Byte 84*          Bounding Box         Mmin                 Double        Little
              fc_zero                                  // Byte 92*          Bounding Box         Mmax                 Double        Little
       };                                              // * Unused, with value 0.0, if not Measured or z type

       // check if range is required

       if (mask & ESRI_HDR_MASK_BOUNDING_BOX && pRange != NULL)
       {
              // record bounding box values

              header.Xmin = pRange->org.x;
              header.Ymin = pRange->org.y;
              header.Zmin = pRange->org.z;
              header.Xmax = pRange->end.x;
              header.Ymax = pRange->end.y;
              header.Zmax = pRange->end.z;
       }

       // output file header

       if (mask & ESRI_HDR_MASK_SIGNATURE    && byteswritten != -1 && (pos = _lseek (fh, ESRI_HDR_ADDR_SIGNATURE, SEEK_SET))    != -1L) byteswritten = _write (fh, &header.code, ESRI_HDR_SIZE_SIGNATURE);
       if (mask & ESRI_HDR_MASK_FILE_LENGTH  && byteswritten != -1 && (pos = _lseek (fh, ESRI_HDR_ADDR_FILE_LENGTH, SEEK_SET))  != -1L) byteswritten = _write (fh, &header.length, ESRI_HDR_SIZE_FILE_LENGTH);
       if (mask & ESRI_HDR_MASK_FILE_VERSION && byteswritten != -1 && (pos = _lseek (fh, ESRI_HDR_ADDR_FILE_VERSION, SEEK_SET)) != -1L) byteswritten = _write (fh, &header.version, ESRI_HDR_SIZE_FILE_VERSION);
       if (mask & ESRI_HDR_MASK_SHAPE_TYPE   && byteswritten != -1 && (pos = _lseek (fh, ESRI_HDR_ADDR_SHAPE_TYPE, SEEK_SET))   != -1L) byteswritten = _write (fh, &header.type, ESRI_HDR_SIZE_SHAPE_TYPE);
       if (mask & ESRI_HDR_MASK_BOUNDING_BOX && byteswritten != -1 && (pos = _lseek (fh, ESRI_HDR_ADDR_BOUNDING_BOX, SEEK_SET)) != -1L) byteswritten = _write (fh, &header.Xmin, ESRI_HDR_SIZE_BOUNDING_BOX);

       // check if length is being set
       if (mask & ESRI_HDR_MASK_FILE_LENGTH && byteswritten != -1)
       {
              // calculate length of file
              header.length = dtmCnv_byteSwap4(_lseek (fh, 0L, SEEK_END) / 2);

              // output length of file
              if ((pos = _lseek (fh, ESRI_HDR_ADDR_FILE_LENGTH, SEEK_SET)) != -1L) byteswritten = _write (fh, &header.length, ESRI_HDR_SIZE_FILE_LENGTH);
       }
       return (byteswritten);
}

