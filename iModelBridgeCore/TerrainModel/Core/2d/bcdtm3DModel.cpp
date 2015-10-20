/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtm3DModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtm3DModel_volumesDtm
(
 BC_DTM_OBJLIST *designTinObjectsP,
 long             numDesignTinObjects,
 BC_DTM_OBJLIST *groundTinObjectP,
 long             numGroundTinObjects,
 long             loadFlag,
 MODEL_VOLUME    **designVolumesPP,
 long            *numDesignVolumesP,
 MODEL_VOLUME    **groundVolumesPP,
 long            *numGroundVolumesP
) 
/*
** This Function Calculates The Volumes between the Tins In The Tin 
** Object List
**
** ArgueMents
**
** designTinObjectsP    ==> List Of Tin Object Pointers To Design Surface
** numDesignTinObjects  ==> Number Of Design Tin Objects
** GrdTinObjects        ==> List Of Ground Tin Objects
** NumGrdTinObjects     ==> Number Of Ground Tin Objects
** loadFlag             ==> Volume Polygons Load Flag
**                      == 1 Load The Volume Polygons
**                      == 0 Do Not Load The Volume Polygons
** designVolumesPP      <== Volumes Between Design Surfaces Are Passed Back In This Structure
** numDesignVolumesP    <== Number Of Design Volumes
** groundVolumesPP      <== Volumes Between Ground Surfaces Are Passed Back In This Structure
** numGroundVolumesP    <== Number Of Ground Volumes 
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected 
**
** Author :  Rob Cormack
** Date   :  6th February 2002
**
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    NumIntPolys=0,nd ;
 TAG_POLYGON *IntPolys=nullptr,*pip ;
/*
** Write Status message
*/
 bcdtmWrite_message(0,0,0,"Calculating 3D Model Volumes") ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Design Tin Objects List      = %p",designTinObjectsP) ;
    bcdtmWrite_message(0,0,0,"Number Of Design Tin Objects = %6ld",numDesignTinObjects) ;
    bcdtmWrite_message(0,0,0,"Ground Tin Objects List      = %p",groundTinObjectP) ;
    bcdtmWrite_message(0,0,0,"Number Of Ground Tin Objects = %6ld",numGroundTinObjects) ;
    bcdtmWrite_message(0,0,0,"Load Flag                    = %1ld",loadFlag) ;
    bcdtmWrite_message(0,0,0,"Design Volumes               = %p",*designVolumesPP) ;
    bcdtmWrite_message(0,0,0,"Number Of Design Volumes     = %6ld",*numDesignVolumesP) ;
    bcdtmWrite_message(0,0,0,"Ground Volumes               = %p",*groundVolumesPP) ;
    bcdtmWrite_message(0,0,0,"Number Of Ground Volumes     = %6ld",*numGroundVolumesP) ;
   }
/*
** Initialise
*/
 *numDesignVolumesP = 0 ;
 *numGroundVolumesP = 0 ;
/*
** Validate
*/
 if( numDesignTinObjects <= 0 ) { bcdtmWrite_message(2,0,0,"Invalid Value For numDesignTinObjects") ; goto errexit ; }
 if( numGroundTinObjects <  0 ) numGroundTinObjects = 0 ;
 if( designTinObjectsP == nullptr ) { bcdtmWrite_message(2,0,0,"designTinObjectsP Initialised To nullptr") ; goto errexit ; }
 if( numGroundTinObjects > 0 && groundTinObjectP == nullptr ) { bcdtmWrite_message(2,0,0,"groundTinObjectP Initialised To nullptr") ; goto errexit ; }
 if( *designVolumesPP !=   nullptr ) { bcdtmWrite_message(2,0,0,"designVolumesPP Not Initialised To nullptr") ; goto errexit ; }
 if( *groundVolumesPP !=   nullptr ) { bcdtmWrite_message(2,0,0,"groundVolumesPP Not Initialised To nullptr") ; goto errexit ; }
/*
** Intersect All The Design DTMFeatureState::Tin Hulls
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Design Tin Hulls") ;
 if( bcdtm3DModel_intersectDesignTinHullsDtm(designTinObjectsP,numDesignTinObjects,&IntPolys,&NumIntPolys)) goto errexit ; 
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersect Polygons = %6ld",NumIntPolys) ;
/*
** Calculate Design Surface Volumes
*/
 *numDesignVolumesP = 0 ;
 *designVolumesPP = nullptr ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Design Surface Volumes") ;
 if( bcdtm3DModel_calculateDesignSurfaceVolumesDtm(designTinObjectsP,numDesignTinObjects,loadFlag,IntPolys,NumIntPolys,designVolumesPP,numDesignVolumesP)) goto errexit ; 
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Design Volumes = %6ld",*numDesignVolumesP) ;
    for( nd = 0 ; nd < *numDesignVolumesP ; ++nd ) 
      {
       bcdtmWrite_message(0,0,0,"Design Volume[%4ld] ** B = %4I64d T = %4I64d Cut = %12.3lf Fill = %12.3lf Area = %12.3lf",nd,(*designVolumesPP+nd)->BottomIndex,(*designVolumesPP+nd)->TopIndex,(*designVolumesPP+nd)->Cut,(*designVolumesPP+nd)->Fill,(*designVolumesPP+nd)->Area) ;
      }
   }
/*
** Calculate Ground Surface Volumes
*/
 loadFlag = 0 ;
 *numGroundVolumesP = 0 ;
 *groundVolumesPP = nullptr ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Ground Surface Volumes") ;
 if( bcdtm3DModel_calculateGroundSurfaceVolumesDtm(groundTinObjectP,numGroundTinObjects,designTinObjectsP,numDesignTinObjects,loadFlag,IntPolys,NumIntPolys,groundVolumesPP,numGroundVolumesP)) goto errexit ; 
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Ground Volumes = %6ld",*numGroundVolumesP) ;
    for( nd = 0 ; nd < *numGroundVolumesP ; ++nd ) 
      {
       bcdtmWrite_message(0,0,0,"Ground Volume[%4ld] ** B = %4I64d T = %4I64d Cut = %12.3lf Fill = %12.3lf Area = %12.3lf",nd,(*groundVolumesPP+nd)->BottomIndex,(*groundVolumesPP+nd)->TopIndex,(*groundVolumesPP+nd)->Cut,(*groundVolumesPP+nd)->Fill,(*groundVolumesPP+nd)->Area) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( IntPolys != nullptr && NumIntPolys > 0 )
   {
    for( pip = IntPolys ; pip < IntPolys + NumIntPolys ; ++pip )
      {
       if( pip->PolyPts    != nullptr ) { free(pip->PolyPts)    ; pip->PolyPts    = nullptr ; }
       if( pip->TinIndexes != nullptr ) { free(pip->TinIndexes) ; pip->TinIndexes = nullptr ; }
       if( pip->IntPolys   != nullptr ) { free(pip->IntPolys)   ; pip->IntPolys   = nullptr ; }
      }
   }
 if( IntPolys != nullptr ) { free(IntPolys) ; IntPolys = nullptr ; }
/*
** Blank Out Non Fatal Error Messages
*/
 bcdtmWrite_message(10,0,0,"") ;
/*
** Job Completed
*/
 if( dbg &&  ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating 3D Model Volumes Completed") ;
 if( dbg &&  ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating 3D Model Volumes Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit  :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtm3DModel_intersectDesignTinHullsDtm(BC_DTM_OBJLIST *DesignTins,long NumDesignTins,TAG_POLYGON **IntPolys,long *NumIntPolys) 
/*
**
** ArgueMents
**
** TinObjects    ==> List Of Data Object Pointers To Be Joined
** NumTinObjects ==> Number Of Objects To Be Scanned
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected 
**
** Author :  Rob Cormack
** Date   :  6th February 2002
**
*/
{
 int              ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long             NumHullPts;
 DTMDirection Direction;
 double           Area,Pptol=0.001,Pltol=0.001 ;
 DPoint3d              *p3d,*HullPts=nullptr ;
 BC_DTM_OBJLIST  *olP ;
 BC_DTM_OBJ       *dtmP=nullptr ;
 TAG_POLYGON      *pip ; 
 VOLTININDEX      *pind ;
 DTMFeatureId   dtmFeatureId ;
 DTM_DAT_OBJ      *dataP=nullptr ;
 DTM_GUID nullGuid=DTM_NULL_GUID ;
/*
** Write Status message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Design Tin Hulls") ;
/*
** Create Dtm Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
/*
** Scan Design Tins And Write Tin Hulls To Data Object
*/
 for( olP = DesignTins ; olP < DesignTins + NumDesignTins ; ++olP )
   {
/*
** Extract Tin Hull
*/
    if( bcdtmList_extractHullDtmObject(olP->Tin,&HullPts,&NumHullPts)) goto errexit ;
    Pptol = olP->Tin->ppTol ;
    Pltol = olP->Tin->plTol ;
/*
**  Store Hull In DTM As Break Line
*/
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,3,&dtmFeatureId,HullPts,NumHullPts) ) goto errexit ;
/*
** Free HullPts Memory
*/
    NumHullPts = 0 ;
    if( HullPts != nullptr ) { free(HullPts) ; HullPts = nullptr ; }
   }
/*
** Write Intersect Data - Development Only
*/
 if( dbg ) bcdtmWrite_geopakDatFileFromDtmObject(dtmP,L"intersect.dat") ;
/*
** Triangulate Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Dtm Object") ;
 dtmP->ppTol = dtmP->plTol = 0.0 ;  
 if( bcdtmObject_createTinDtmObject(dtmP,1,0.0, false, false)) goto errexit ;
/*
** Remove None Feature Hull Lines
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
/*
** Write Hull Area For Valiadtion Check ** Development Only
*/
 if( dbg )
   {
    bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtmP,dtmP->hullPoint,&Area,&Direction) ;
    bcdtmWrite_message(0,0,0,"Hull Area = %12.3lf Direction = %1ld",Area,Direction) ;
   }
/*
** Write Intersect Tin - Development Only
*/
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"intersect.tin") ;
/*
** Extract Polygons From Intersected DTMFeatureState::Tin Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Intersected Polygons") ;
 if( bcdtm3DModel_extractIntersectedTinHullPolygonsDtmObject(dtmP,IntPolys,NumIntPolys)) goto errexit ;
/*
** Assign Tin Indexes To Polygons
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Tin Indexes To Intersect Polygons") ;
 if( bcdtm3DModel_assignTinIndexesToIntersectPolygonsDtm(DesignTins,NumDesignTins,*IntPolys,*NumIntPolys) ) goto errexit ;
/*
** Write Polygon Points And Associated Tin Indexes ** Development Only
*/
 if(dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Intersect Polygons = %6ld",*NumIntPolys) ;
    for( pip = *IntPolys ; pip < *IntPolys + *NumIntPolys ; ++pip )
      {
       bcdtmWrite_message(0,0,0,"Intersect Polygon = %5ld",(long)(pip-*IntPolys)) ; 
       bcdtmWrite_message(0,0,0,"Number Of Points  = %6ld",pip->NumPolyPts) ;
       for( p3d = pip->PolyPts  ; p3d < pip->PolyPts + pip->NumPolyPts ; ++p3d )
         {
          bcdtmWrite_message(0,0,0,"Point[%4ld] = %10.4lf %10.4lf %10.4lf",(long)(p3d-pip->PolyPts),p3d->x,p3d->y,p3d->z) ;
         }
       bcdtmWrite_message(0,0,0,"Number Of Tin Indexes    = %6ld",pip->NumTinIndexes) ;
       for( pind = pip->TinIndexes  ; pind < pip->TinIndexes + pip->NumTinIndexes ; ++pind )
         {
          bcdtmWrite_message(0,0,0,"TinIndex[%4ld] ** Tin = %p TinIndex = %6I64d z = %10.4lf",(long)(pind-pip->TinIndexes),pind->Tin,pind->Index,pind->z) ;
         }
      }
   } 
/*
** Write Intersect Polygons To Data Object ** Development Only
*/
 if( dbg )
   {
    if( bcdtmObject_createDataObject(&dataP)) goto errexit ;
    for( pip = *IntPolys ; pip < *IntPolys + *NumIntPolys ; ++pip )
      {
       for( p3d = pip->PolyPts  ; p3d < pip->PolyPts + pip->NumPolyPts -1 ; ++p3d )
         {
          bcdtmObject_storePointInDataObject(dataP,2,DTM_NULL_USER_TAG,nullGuid,p3d->x,p3d->y,p3d->z) ;
          bcdtmObject_storePointInDataObject(dataP,3,DTM_NULL_USER_TAG,nullGuid,(p3d+1)->x,(p3d+1)->y,(p3d+1)->z) ;
         }
      }
    bcdtmWrite_dataFileFromDataObject(dataP,L"intpoly.dat") ;
    bcdtmObject_deleteDataObject(&dataP) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( HullPts != nullptr ) { free(HullPts) ; HullPts = nullptr ; }
 if( dtmP    != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( dataP   != nullptr ) bcdtmObject_deleteDataObject(&dataP) ;
/*
** Job Completed
*/
 if( dbg &&  ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Tin Hulls Completed") ;
 if( dbg &&  ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Intersecting Tin Hulls Error") ;
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
BENTLEYDTM_Public int bcdtm3DModel_extractIntersectedTinHullPolygonsDtmObject(BC_DTM_OBJ *dtmP,TAG_POLYGON **IntPolys,long *NumIntPolys) 
/*
** This Function Extracts The Intersected Tin Hull Polygons
** 
** Return Values
**
** 0 Succesfull
** 1 Error Detected 
**
** Author :  Rob Cormack
** Date   :  6th February 2002
**
*/
{
 int         ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long        p1,p2,sp,np,pp,spnt,clc,Offset ;
 DTMDirection Direction;
 long        NumPolyPts=0,TagPolyNe=0,TagPolyMe=0,TagPolyMemInc=1000 ;
 double      PolyArea,Ipx,Ipy ;
 unsigned char        *Line=nullptr,*pc ;
 DTM_TIN_NODE       *pd ;
 DPoint3d         *PolyPts=nullptr ;
 TAG_POLYGON *TagPolygons=nullptr,*ppoly,*ppoly1 ;
/*
** Write Status Message ** Development Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Tin Hull Polygons") ;
/*
** Allocate Memory
*/
 Offset = dtmP->cListPtr / 8  + 1 ;
 Line = ( unsigned char * ) malloc ( Offset * sizeof(char)) ;
 if( Line == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
 for( pc = Line ; pc < Line + Offset ; ++pc ) *pc = (char) 0 ;
/*
** Scan Tin Lines 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Tin Lines") ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    pd = nodeAddrP(dtmP,p1) ;
    clc = pd->cPtr ;
    while( clc != dtmP->nullPtr )
      {
       p2  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( p2 > p1 )
         {
/*
**        Check If Line P1 P2 Is A Feature Line
*/
          sp = np = dtmP->nullPnt ;
          if     ( bcdtmList_testForBreakLineDirectionDtmObject(dtmP,p1,p2)) { sp = p1 ; np = p2 ; }
          else if( bcdtmList_testForBreakLineDirectionDtmObject(dtmP,p2,p1)) { sp = p2 ; np = p1 ; }
/*
**        Extract Polygon
*/
          if( sp != dtmP->nullPnt )
            {
/*
**           Check Line Has Not Been Previously Processed
*/
             bcdtmTheme_getLineOffsetDtmObject(dtmP,&Offset,sp,np) ;
             if( ! bcdtmFlag_testFlag(Line,Offset) )
               { 
/*
**             Scan Back To Start Point
*/
                NumPolyPts = 0 ;
                if( PolyPts != nullptr ) { free(PolyPts) ; PolyPts = nullptr ; }
                spnt = sp ;
                do
                  { 
                   ++NumPolyPts ;
                   nodeAddrP(dtmP,sp)->tPtr = np ;
                   pp = sp ; sp = np ; np = pp ;
                   if( (np = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) return(1) ;
                   while( ! bcdtmList_testForLineOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,sp,np))
                     { if( (np = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) return(1) ; }
                  } while ( sp != spnt ) ;
                ++NumPolyPts ;
/*
**              Write Polygon ** Development Only
*/
                if( dbg )
                  { 
                   bcdtmWrite_message(0,0,0,"Intersect Polygon = %6ld ** Number Of Points = %6ld",TagPolyNe,NumPolyPts) ;
                   np = 0 ;
                   sp = spnt ;
                   do
                     {
                      bcdtmWrite_message(0,0,0,"Poly Point[%4ld] Tin Point = %6ld ** %10.4lf %10.4lf %10.4lf",np,sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                      sp = nodeAddrP(dtmP,sp)->tPtr ;
                      ++np ;
                     } while ( sp != spnt ) ;
                   bcdtmWrite_message(0,0,0,"Poly Point[%4ld] Tin Point = %6ld ** %10.4lf %10.4lf %10.4lf",np,sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                  }
/*
**              Calculate Tptr Polygon Area
*/
                if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,spnt,&PolyArea,&Direction)) goto errexit ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Polygon Area = %10.4lf Direction = %1ld",PolyArea,Direction) ;
/*
**              Copy Tptr List To Polygon Points
*/
                if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,spnt,&PolyPts,&NumPolyPts) ) goto errexit ;
/*
**              Find Internal Point For Polygon
*/
                if( NumPolyPts <  4 ) { bcdtmWrite_message(1,0,0,"Not Enough Points In Polygon") ; goto errexit ; }
                if( NumPolyPts == 4 )
                  {
                   Ipx = ((PolyPts+1)->x + (PolyPts+2)->x) / 2.0 ;
                   Ipy = ((PolyPts+1)->y + (PolyPts+2)->y) / 2.0 ;
                   Ipx = ( Ipx + PolyPts->x ) / 2.0 ;
                   Ipy = ( Ipy + PolyPts->y ) / 2.0 ;
                  }
                else
                  {
                   sp = spnt ;
                   if(( np = bcdtmList_nextAntDtmObject(dtmP,sp,nodeAddrP(dtmP,sp)->tPtr)) < 0 ) goto errexit ;
                   while ( nodeAddrP(dtmP,np)->tPtr == sp )
                     {
                      sp = nodeAddrP(dtmP,sp)->tPtr ;
                      if(( np = bcdtmList_nextAntDtmObject(dtmP,sp,nodeAddrP(dtmP,sp)->tPtr)) < 0 ) goto errexit ;
                     }
                   Ipx = (pointAddrP(dtmP,sp)->x + pointAddrP(dtmP,np)->x ) / 2.0 ;
                   Ipy = (pointAddrP(dtmP,sp)->y + pointAddrP(dtmP,np)->y ) / 2.0 ;
                  }
                if( dbg ) bcdtmWrite_message(0,0,0,"Polygon Internal Point = %10.4lf %10.4lf",Ipx,Ipy) ;
/*
**              Mark Tptr Polygon Lines As Scanned
*/
                sp = spnt ;
                do
                  {
                   np = nodeAddrP(dtmP,sp)->tPtr ;
                   bcdtmTheme_getLineOffsetDtmObject(dtmP,&Offset,sp,np) ;
                   bcdtmFlag_setFlag(Line,Offset) ;
                   sp = np ;
                  } while ( sp != spnt ) ;
/*
**              Store Polygon Points In Tag Polygon Data Structure
*/
                if( TagPolyNe == TagPolyMe )
                  {
                   TagPolyMe = TagPolyMe + TagPolyMemInc ;
                   if( TagPolygons == nullptr ) TagPolygons = ( TAG_POLYGON * ) malloc  ( TagPolyMe * sizeof(TAG_POLYGON)) ;
                   else                   TagPolygons = ( TAG_POLYGON * ) realloc ( TagPolygons , TagPolyMe * sizeof(TAG_POLYGON)) ;
                   if( TagPolygons == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
                  }
                (TagPolygons+TagPolyNe)->PolyId        = TagPolyNe ;
                (TagPolygons+TagPolyNe)->PolyArea      = PolyArea  ;
                (TagPolygons+TagPolyNe)->IntToPoly     =  -1  ;
                (TagPolygons+TagPolyNe)->IntPointX     =  Ipx ;
                (TagPolygons+TagPolyNe)->IntPointY     =  Ipy ;
                (TagPolygons+TagPolyNe)->PolyPts       = PolyPts ;
                (TagPolygons+TagPolyNe)->NumPolyPts    = NumPolyPts ;
                (TagPolygons+TagPolyNe)->TinIndexes    = nullptr ;
                (TagPolygons+TagPolyNe)->NumTinIndexes = 0 ;
                (TagPolygons+TagPolyNe)->IntPolys      = nullptr ;
                (TagPolygons+TagPolyNe)->NumIntPolys   = 0 ;
                ++TagPolyNe ;
/*
**              Free Memory
*/
                NumPolyPts =  0 ;
                PolyPts  = nullptr ;
/*
**              Null Out Tptr Polygon
*/         
                bcdtmList_nullTptrListDtmObject(dtmP,spnt) ;
               } 
            }
         }
      }
   }
/*
** Determine Internal Polygons
*/ 
 if( dbg ) bcdtmWrite_message(0,0,0,"Determing Internal Polygons") ;
 if( bcdtm3DModel_determineInternalIntersectedTinHullPolygonsDtmObject(dtmP,TagPolygons,TagPolyNe) ) goto errexit ;
/*
** Add Internal Polygons Offests To Intersect Polygons
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adding Internal Polygon Offsets To Intersect Polygons") ;
 for( ppoly = TagPolygons ; ppoly < TagPolygons + TagPolyNe ; ++ppoly )
   {
    if( ppoly->IntToPoly >= 0 )
      {
       ppoly1 = TagPolygons + ppoly->IntToPoly ;
       if( ppoly1->IntPolys == nullptr ) ppoly1->IntPolys = (long * ) malloc( sizeof(long)) ;
       else                           ppoly1->IntPolys = (long * ) realloc( ppoly1->IntPolys,(ppoly1->NumIntPolys+1) * sizeof(long)) ;
       if( ppoly1->IntPolys == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
       *(ppoly1->IntPolys+ppoly1->NumIntPolys) = ppoly->PolyId ;
       ++ppoly1->NumIntPolys ;
      } 
   }
/*
** Write Internal Polygons ** Developemnt Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"List Of Polygons Internal To Polygons") ;
    PolyArea = 0.0 ;
    for( ppoly = TagPolygons ; ppoly < TagPolygons + TagPolyNe ; ++ppoly )
      {
       bcdtmWrite_message(0,0,0,"Poly = %6ld Number Of Internal Polygons = %6ld",ppoly->PolyId,ppoly->NumIntPolys) ;
       PolyArea = PolyArea + ppoly->PolyArea ;
       for( p1 = 0 ; p1 < ppoly->NumIntPolys ; ++p1 )
         {
          bcdtmWrite_message(0,0,0,"****  Internal Poly = %6ld  ** Internal Poly Internal To Poly = %6ld",(TagPolygons+*(ppoly->IntPolys+p1))->PolyId,(TagPolygons+*(ppoly->IntPolys+p1))->IntToPoly) ;
          PolyArea = PolyArea - (TagPolygons+*(ppoly->IntPolys+p1))->PolyArea ;
         }
      } 
    bcdtmWrite_message(0,0,0,"Total Area Of Intersected Polygons = %12.3lf",PolyArea) ;
   }
/*
** Set Return Values
*/
 *IntPolys    = TagPolygons ;
 *NumIntPolys = TagPolyNe ;
 TagPolygons  = nullptr ;
/*
** Clean Up
*/
 cleanup :
 if( Line != nullptr ) { free(Line) ; Line = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Extracting Tin Hull Polygons Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Extracting Tin Hull Polygons Error") ;
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
BENTLEYDTM_Public int bcdtm3DModel_assignTinIndexesToIntersectPolygonsDtm(BC_DTM_OBJLIST *DesignTins,long NumDesignTins,TAG_POLYGON *IntPolys,long NumIntPolys) 
/*
** This Function Assigns The Tin Indexes To The Intersect Polygons For
** The Purpose Of Volume Calculations
** 
** Return Values
**
** 0 Succesfull
** 1 Error Detected 
**
** Author :  Rob Cormack
** Date   :  21th February 2002
**
*/
{
 int         ret=DTM_SUCCESS ;
 long        dbg=DTM_TRACE_VALUE(0),isw,P1,P2,P3,Ptype,NoIndexes,NumTinIndexes,MemTinIndexes,MemTinIndexesInc=10 ;
 double      z ;
 BC_DTM_OBJLIST  *ptin  ;
 TAG_POLYGON *ppoly ;
 VOLTININDEX *TinIndexes=nullptr,*pind,pindv,*pi ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Tin Indexes To Intersect Polygons") ;
/*
** Scan Intersection Polygons And Drape Internal Points On Design Tins
*/
 NumTinIndexes = MemTinIndexes = 0 ;
 for( ppoly = IntPolys ; ppoly < IntPolys + NumIntPolys ; ++ppoly)
   {
/*
** Scan Design Tins And Drape Polygon Internal Point
*/
    for( ptin = DesignTins ; ptin < DesignTins + NumDesignTins ; ++ptin )
      {
/*
**  Find Triangle
*/
       if( bcdtmFind_triangleForPointDtmObject(ptin->Tin,ppoly->IntPointX,ppoly->IntPointY,&z,&Ptype,&P1,&P2,&P3 ) ) return(1) ;
       if( Ptype )
         {
          if( NumTinIndexes == MemTinIndexes )
            {
             MemTinIndexes = MemTinIndexes + MemTinIndexesInc ;
             if( TinIndexes == nullptr ) TinIndexes = ( VOLTININDEX * ) malloc(MemTinIndexes*sizeof(VOLTININDEX)) ;
             else                     TinIndexes = ( VOLTININDEX * ) realloc(TinIndexes,MemTinIndexes*sizeof(VOLTININDEX)) ;
             if( TinIndexes == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
            }
          (TinIndexes+NumTinIndexes)->Tin      = ptin->Tin ;
          (TinIndexes+NumTinIndexes)->Index    = ptin->TinIndex ;
          (TinIndexes+NumTinIndexes)->z        = z ;
          ++NumTinIndexes ;
         }
      } 
/*
** Reallocate Tin Indexes Memory
*/
    TinIndexes = ( VOLTININDEX * ) realloc(TinIndexes,NumTinIndexes*sizeof(VOLTININDEX)) ;
/*
** Write Tin Index Array Before Depth Sorting
*/
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Tin Indexes Before Depth Sort = %6ld",NumTinIndexes) ;
       for( pind = TinIndexes ; pind < TinIndexes + NumTinIndexes ; ++pind )
         { 
          bcdtmWrite_message(0,0,0,"Tin = %p Index = %6I64d z = %10.4lf",pind->Tin,pind->Index,pind->z) ;
         } 
      } 
/*
** Bubble Sort Tin Indexes On Increasing z
*/
    pi = &pindv ;
    isw = 1 ;
    NoIndexes = NumTinIndexes ;
    while ( NoIndexes > 1 && isw )
      {
       isw = 0 ;
       for( pind = TinIndexes ; pind < TinIndexes + NoIndexes - 1 ; ++pind )
         {
          if( pind->z > (pind+1)->z )  
            {
             *pi = *pind ;
             *pind = *(pind+1) ;
             *(pind+1) = *pi ;
             isw = 1 ;
            } 
         }
       --NoIndexes ; 
      }
/*
** Write Tin Index Array After Depth Sorting
*/
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"Tin Indexes After  Depth Sort = %6ld",NumTinIndexes) ;
       for( pind = TinIndexes ; pind < TinIndexes + NumTinIndexes ; ++pind )
         { 
          bcdtmWrite_message(0,0,0,"Tin = %p Index = %6I64d z = %10.4lf",pind->Tin,pind->Index,pind->z) ;
         } 
      } 
/*
** Assign Tin Indexes To Intersection Polygons
*/
    ppoly->TinIndexes = TinIndexes ;
    ppoly->NumTinIndexes = NumTinIndexes ;
/*
** Reset For Next Intersection Polygon
*/
    NumTinIndexes = MemTinIndexes = 0 ;
    TinIndexes = nullptr ;
   }
/*
** Clean Up
*/
 cleanup :
 if( TinIndexes != nullptr ) { free(TinIndexes) ; TinIndexes = nullptr ; }
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Assigning Tin Indexes To Intersect Polygons Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Assigning Tin Indexes To Intersect Polygons Error") ;
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
BENTLEYDTM_Public int bcdtm3DModel_determineInternalIntersectedTinHullPolygonsDtmObject(BC_DTM_OBJ *dtmP,TAG_POLYGON *IntPolys,long NumIntPolys) 
/*
** This Function Determines The Intersected Tin Hull Polygons Internal To
** Other Intersected Tin Hull Polygons
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected 
**
** Author :  Rob Cormack
** Date   :  20th February 2002
*/
{
 int  ret=DTM_SUCCESS ;
 long dbg=DTM_TRACE_VALUE(0),sp,np,cln,Spnt,nfeat,polytag,intpolytag ;
 TAG_POLYGON *ppol ;
/*
** Write Status Message ** Developement Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determing Internal Intersected Hull Polygons") ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"IntPolys     = %p",IntPolys) ;
    bcdtmWrite_message(0,0,0,"NumIntPolys  = %6ld",NumIntPolys) ;
   }
/*
** Remove All DTM Features From Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing All Dtm Features From Tin ** Number Of Features = %6ld",dtmP->numFeatures) ;
 if( bcdtmData_deleteAllDtmFeaturesDtmObject(dtmP) ) goto errexit ;
 if( bcdtmTin_compactFeatureTableDtmObject(dtmP)) goto errexit ;
 if( bcdtmTin_compactFeatureListDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing All Dtm Features From Tin Completed ** Number Of Features = %6ld",dtmP->numFeatures) ;
/*
** Store All Polygons Into Tin As Dtm Feature Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Polygons In Tin As Dtm Feature Polygon") ;
 for( ppol = IntPolys ; ppol < IntPolys + NumIntPolys ; ++ppol )
   {
    if( bcdtmInsert_internalStringIntoDtmObject(dtmP,1,2,ppol->PolyPts,ppol->NumPolyPts,&Spnt)) goto errexit ;
    if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,nullptr,0,DTMFeatureType::Polygon,ppol->PolyId,DTM_NULL_FEATURE_ID,Spnt,1)) goto errexit ; 
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Polygons In Tin As Dtm Feature Polygon Completed") ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Polygons = %6ld",dtmP->numFeatures) ;
/*
** Scan Feature List And Look For Internal Polygons
*/
 for( nfeat = 0 ; nfeat < dtmP->numFeatures ; ++nfeat )
   {
    polytag = (long) ftableAddrP(dtmP,nfeat)->dtmUserTag ;
/*
**  Copy Feature To Tptr Array
*/
    if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,nfeat,&Spnt)) goto errexit ;
/*
**  Scan Tptr List And Look For Internal Polygons
*/
    sp = Spnt ;
    do
      {
       np = nodeAddrP(dtmP,sp)->tPtr ;
       while ( nodeAddrP(dtmP,np)->tPtr != sp )
         {
          if( nodeAddrP(dtmP,np)->tPtr == dtmP->nullPnt )
            {
             cln = nodeAddrP(dtmP,np)->fPtr ; 
             while( cln != dtmP->nullPtr )
               {
                intpolytag = (long) ftableAddrP(dtmP,flistAddrP(dtmP,cln)->dtmFeature)->dtmUserTag ;
                if( dbg && (IntPolys+intpolytag)->IntToPoly == -1 ) bcdtmWrite_message(0,0,0,"Polygon %6ld Internal To Polygon %6ld",intpolytag,polytag) ;
                (IntPolys+intpolytag)->IntToPoly = polytag ;
                cln = flistAddrP(dtmP,cln)->nextPtr ; 
               }
            }
          if(( np = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ; 
         } 
       sp = nodeAddrP(dtmP,sp)->tPtr ;
      } while ( sp != Spnt ) ; 
/*
** Null Out Tptr List
*/
    bcdtmList_nullTptrListDtmObject(dtmP,Spnt) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ! ret )  bcdtmWrite_message(0,0,0,"Determing Internal Intersected Hull Polygons Completed") ;
 if( dbg &&   ret )  bcdtmWrite_message(0,0,0,"Determing Internal Intersected Hull Polygons Error") ;
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
BENTLEYDTM_Public int bcdtm3DModel_getBreakLineUserTagsForTinLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,DTMUserTag **UserTags,long *NumOfTags)
/*
** This Function Retrieves All The DTM Features And User Tags For A Tin Line
**
** Arguements
**
** Tin              ==> Tin Object
** P1               ==> Tin Point Number For Line End Point
** P2               ==> Tin Point Number For Line End Point
** UserTags         <== List Of Usertags
** NumberOfFeatures <== Size Of Feature and UserTag Lists
**
** Arguement Validation  
**
** 1. No Validity Checking On The Tin Object 
** 2. Tin Point Range Is Checked
** 3. Features Must Be Set To Null
** 4. UserTags Must Be Set To Null
** 5. No Validity Checking On NumberOfFeatures
**
** Return Values
**
** 0 Succesfull
** 1 Error Detected 
**
** Author :  Rob Cormack
** Date   :  7th February 2002
**
*/
{
 long dbg=DTM_TRACE_VALUE(0),cln,nfeat ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Break Line User Tags For Line %6ld %6ld",P1,P2) ;
/*
** Initialise
*/
 *NumOfTags = 0 ; 
/*
** Validate
*/
 if( P1 < 0 || P1 >= dtmP->numPoints ) { bcdtmWrite_message(2,0,0,"Tin Point Range Error") ; goto errexit ; }
 if( P2 < 0 || P2 >= dtmP->numPoints ) { bcdtmWrite_message(2,0,0,"Tin Point Range Error") ; goto errexit ; }
 if( *UserTags != nullptr ) { bcdtmWrite_message(1,0,0,"UserTags Not Initialised To Null") ; goto errexit ; }
/*
** Scan Feature List For Point P1 And Count Features That Connect To P2
*/
 cln = nodeAddrP(dtmP,P1)->fPtr ;
 while( cln != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,cln)->nextPnt == P2 )  ++*NumOfTags ;
    cln = flistAddrP(dtmP,cln)->nextPtr ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Tags = %6ld",*NumOfTags) ;
/*
** Store User Tags
*/
 if( *NumOfTags > 0 )
   {
/*
** Allocate Memory For Features And UserTags
*/
    *UserTags = ( DTMUserTag *) malloc(*NumOfTags * sizeof(DTMUserTag)) ;
    if( *UserTags == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Copy Features And UserTags To User Arrays
*/
    nfeat = 0 ;
/*
** Copy UserTags For P1
*/
    cln = nodeAddrP(dtmP,P1)->fPtr ;
    while( cln != dtmP->nullPtr )
      {
       if(flistAddrP(dtmP,cln)->nextPnt == P2 ) 
         {
          *(*UserTags+nfeat) = ftableAddrP(dtmP,flistAddrP(dtmP,cln)->dtmFeature)->dtmUserTag ;
          ++nfeat ;
         } 
       cln = flistAddrP(dtmP,cln)->nextPtr ;
      }
   }
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Break Line User Tags For Line Completed") ;
 return(0) ;
/*
** Error Exit
*/
 errexit :
 *NumOfTags = 0 ; 
 if( *UserTags != nullptr ) { free(*UserTags) ; *UserTags = nullptr ; }
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Break Line User Tags For Line Error") ;
 return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtm3DModel_calculateDesignSurfaceVolumesDtm
(
 BC_DTM_OBJLIST *designTinObjectsP,
 long            numDesignTinObjects,
 long            loadFlag,
 TAG_POLYGON     *IntPolys,
 long            NumIntPolys,
 MODEL_VOLUME    **designVolumesPP,
 long            *numDesignVolumesP
 ) 
/*
** This Function Calculates The Design Surface Volumes
** 
** Return Values
**
** 0 Succesfull
** 1 Error Detected 
**
** Author :  Rob Cormack
** Date   :  11th February 2002
**
*/
{
 int     ret=DTM_SUCCESS ;
 long    dbg=DTM_TRACE_VALUE(0),nvoid,MemdesignVolumesPP=0,MemdesignVolumesPPInc=1000,Spnt ;
 double  Cut,Fill,Balance,Area ;
 DPoint3d     *p3d  ; 
 TAG_POLYGON *pip,*ipoly ;
 VOLTININDEX *pind  ;
 VOLRANGETAB *RangeTable=nullptr ;
 BC_DTM_OBJ  *dtmP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Design Surface Volumes ** designTinObjectsP = %p numDesignTinObjects = %4ld",designTinObjectsP,numDesignTinObjects) ;
/*
** Initialise
*/
 *numDesignVolumesP=0 ;
/*
** Process All The Design Surface Intersect Polygons
*/
 for( pip = IntPolys ; pip < IntPolys + NumIntPolys ; ++pip )
   {
/*
** Calculate Polygon Areas For ** Development Only
*/
    if( dbg )
      {
       Area = pip->PolyArea ;
       for( nvoid = 0 ; nvoid < pip->NumIntPolys ; ++nvoid )
         {
          ipoly = IntPolys + *(pip->IntPolys + nvoid) ;
          Area = Area - ipoly->PolyArea ;
         }
       bcdtmWrite_message(0,0,0,"**************Processing Design Surface Intersect Polygon %4ld ** Area = %12.3lf",(long)(pip-IntPolys),Area) ; 
      } 
/*
** Load The Polygons If Required
*/
    if( loadFlag )
      {
       for( p3d = pip->PolyPts ; p3d < pip->PolyPts + pip->NumPolyPts ; ++p3d )
         {
          if( bcdtmLoad_storePointInCache(p3d->x,p3d->y,p3d->z)) goto errexit ; 
         }
//       if( bcdtmLoad_writeDtmFeature(DTMFeatureType::Polygon,DTM_NULL_PNT) ) goto errexit ; 
      }
/*
**  Calculate Volumes
*/
    for( pind = pip->TinIndexes ; pind < pip->TinIndexes + pip->NumTinIndexes - 1 ; ++pind )
      {
/*
**     Copy Tin
*/
       if( bcdtmObject_cloneDtmObject((BC_DTM_OBJ *)(pind+1)->Tin,&dtmP)) goto errexit ;
/*
**     Insert Polygons Internal To Intersect Polygon As Voids
*/
       if( pip->NumIntPolys )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Inserting %2ld Voids",pip->NumIntPolys) ;
          for( nvoid = 0 ; nvoid < pip->NumIntPolys ; ++nvoid )
            {
             ipoly = IntPolys + *(pip->IntPolys + nvoid) ;
             if( bcdtmInsert_internalStringIntoDtmObject(dtmP,1,2,ipoly->PolyPts,ipoly->NumPolyPts,&Spnt)) goto errexit ;
             if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,nullptr,0,DTMFeatureType::Void,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,Spnt,1)) goto errexit ;
            }
          bcdtmTin_markInternalVoidPointsDtmObject(dtmP) ;
          if( pip->NumIntPolys > 1 ) if( bcdtmClip_resolveAdjoiningPolygonalFeaturesDtmObject(dtmP)) goto errexit ;
         }
/*
**     Calculate Surface To Surface Volume
*/
       if( bcdtmTinVolume_surfaceToSurfaceDtmObjects((BC_DTM_OBJ *) pind->Tin,dtmP,RangeTable,0,pip->PolyPts,pip->NumPolyPts,nullptr,nullptr,Cut,Fill,Balance,Area ) ) goto err01 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"BI = %6I64d TI = %6I64d ** Cut = %10.4lf Fill = %10.4lf Area = %10.4lf",pind->Index,(pind+1)->Index,Cut,Fill,Area) ;
/*
**     Store In Design Volumes Structure
*/
       if( *numDesignVolumesP == MemdesignVolumesPP )          
         {
          MemdesignVolumesPP = MemdesignVolumesPP + MemdesignVolumesPPInc ;
          if( *designVolumesPP == nullptr ) *designVolumesPP = ( MODEL_VOLUME * ) malloc ( MemdesignVolumesPP * sizeof(MODEL_VOLUME)) ;
          else                         *designVolumesPP = ( MODEL_VOLUME * ) realloc( *designVolumesPP , MemdesignVolumesPP * sizeof(MODEL_VOLUME)) ;  
          if( *designVolumesPP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
         }
       (*designVolumesPP + *numDesignVolumesP)->BottomIndex = pind->Index ;
       (*designVolumesPP + *numDesignVolumesP)->TopIndex    = (pind+1)->Index ;
       (*designVolumesPP + *numDesignVolumesP)->Cut  = Cut  ;
       (*designVolumesPP + *numDesignVolumesP)->Fill = Fill ;
       (*designVolumesPP + *numDesignVolumesP)->Area = Area ;
       ++*numDesignVolumesP ;
/*
** Delete Tin Object
*/
err01 : 
       if( dtmP != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
      }
   }
/*
** Realloacte Design Volumes Array
*/
 *designVolumesPP = ( MODEL_VOLUME * ) realloc( *designVolumesPP,*numDesignVolumesP  * sizeof(MODEL_VOLUME)) ;  
/*
** Clean Up
*/
 cleanup :
 if( dtmP != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Calculating Design Surface Volumes Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Calculating Design Surface Volumes Error") ;
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
BENTLEYDTM_Public int bcdtm3DModel_calculateGroundSurfaceVolumesDtm(BC_DTM_OBJLIST *groundTinObjectP,long numGroundTinObjects,BC_DTM_OBJLIST *designTinObjectsP,long numDesignTinObjects,long loadFlag,TAG_POLYGON *IntPolys,long NumIntPolys,MODEL_VOLUME **groundVolumesPP,long *numGroundVolumesP) 
/*
** This Function Calculates The Ground Surface Volumes
** 
** Return Values
**
** 0 Succesfull
** 1 Error Detected 
**
** Author :  Rob Cormack
** Date   :  11th February 2002
**
*/
{
 int         ret=DTM_SUCCESS ;
 long        dbg=DTM_TRACE_VALUE(0),nvoid,Spnt,MemgroundVolumesPP=0,MemgroundVolumesPPInc=1000 ;
 double      Cut,Fill,Balance,Area ;
 DPoint3d         *p3d  ; 
 TAG_POLYGON *pip,*ipoly ;
 VOLTININDEX *pind ;
 VOLRANGETAB *RangeTable=nullptr ;
 BC_DTM_OBJ  *dtmP=nullptr ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Gound Surface Volumes ** designTinObjectsP = %p numDesignTinObjects = %4ld",designTinObjectsP,numDesignTinObjects) ;
/*
** Initialise
*/
 *numGroundVolumesP=0 ;
 if( numGroundTinObjects <= 0 ) goto cleanup ;
/*
** Process All The Design Surface Intersect Polygons Against The Ground Tins
*/
 for( pip = IntPolys ; pip < IntPolys + NumIntPolys ; ++pip )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Processing Ground Surface Intersect Polygon %4ld",(long)(pip-IntPolys)) ; 
/*
** Load The Polygons If Required
*/
    if( loadFlag )
      {
       for( p3d = pip->PolyPts ; p3d < pip->PolyPts + pip->NumPolyPts ; ++p3d )
         {
          if( bcdtmLoad_storePointInCache(p3d->x,p3d->y,p3d->z)) goto errexit ; 
         }
//       if( bcdtmLoad_writeDtmFeature(DTMFeatureType::Polygon,DTM_NULL_PNT) ) goto errexit ; 
      }
/*
**  Set Pointer To Lowest Design Surface
*/
    pind = pip->TinIndexes ;
/*
** Copy Tin
*/
//    if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
    if( bcdtmObject_cloneDtmObject(( BC_DTM_OBJ *) pind->Tin,&dtmP)) goto errexit ;
/*
**  Insert Polygons Internal To Intersect Polygon As Voids
*/
    if( pip->NumIntPolys )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting %2ld Voids",pip->NumIntPolys) ;
       for( nvoid = 0 ; nvoid < pip->NumIntPolys ; ++nvoid )
         {
          ipoly = IntPolys + *(pip->IntPolys + nvoid) ;
          if( bcdtmInsert_internalStringIntoDtmObject(dtmP,1,2,ipoly->PolyPts,ipoly->NumPolyPts,&Spnt)) goto errexit ;
          if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,nullptr,0,DTMFeatureType::Void,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,Spnt,1)) goto errexit ;
         }
       bcdtmTin_markInternalVoidPointsDtmObject(dtmP) ;
       if( pip->NumIntPolys > 1 ) if( bcdtmClip_resolveAdjoiningPolygonalFeaturesDtmObject(dtmP)) goto errexit ;
      }
/*
**  Calculate Volumes
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Volumes") ;
    if( bcdtmTinVolume_surfaceToSurfaceDtmObjects((BC_DTM_OBJ *)groundTinObjectP->Tin,dtmP,RangeTable,0,pip->PolyPts,pip->NumPolyPts,nullptr,nullptr,Cut,Fill,Balance,Area ) ) goto err01 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Index = %6I64d ** Cut = %10.4lf Fill = %10.4lf Area = %10.4lf",pind->Index,Cut,Fill,Area) ;
/*
**  Store In Ground Volumes Structure
*/
    if( *numGroundVolumesP == MemgroundVolumesPP )          
      {
       MemgroundVolumesPP = MemgroundVolumesPP + MemgroundVolumesPPInc ;
       if( *groundVolumesPP == nullptr ) *groundVolumesPP = ( MODEL_VOLUME * ) malloc ( MemgroundVolumesPP * sizeof(MODEL_VOLUME)) ;
       else                           *groundVolumesPP = ( MODEL_VOLUME * ) realloc( *groundVolumesPP , MemgroundVolumesPP * sizeof(MODEL_VOLUME)) ;  
       if( *groundVolumesPP == nullptr ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
      }
    (*groundVolumesPP + *numGroundVolumesP)->BottomIndex = pind->Index ;
    (*groundVolumesPP + *numGroundVolumesP)->TopIndex    = groundTinObjectP->TinIndex ;
    (*groundVolumesPP + *numGroundVolumesP)->Cut  = Cut  ;
    (*groundVolumesPP + *numGroundVolumesP)->Fill = Fill ;
    (*groundVolumesPP + *numGroundVolumesP)->Area = Area ;
    ++*numGroundVolumesP ;
err01 : 
   if( dtmP != nullptr ) bcdtmObject_destroyDtmObject(&dtmP) ;
   }
/*
** Realloacte Ground Volumes Array
*/
 *groundVolumesPP = ( MODEL_VOLUME * ) realloc( *groundVolumesPP,*numGroundVolumesP  * sizeof(MODEL_VOLUME)) ;  
/*
** Clean Up
*/
 cleanup :
 if( dtmP != nullptr ) bcdtmObject_destroyDtmObject(&dtmP)  ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Calculating Ground Surface Volumes Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Calculating Ground Surface Volumes Error") ;
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
BENTLEYDTM_EXPORT int bcdtm3DModel_insertMissingHullBreakLinesDtmObject(BC_DTM_OBJ *dtmP ) 
/*
** This Function Inserts Break Lines Around The Tin Hull
** Written At request Of Jay Vose For 3D Model Purposes
**
**  Tin  => Tin For Hull Break Lines To Be Inserted
**
** Author : Rob Cormack
** Date   : 12/12/2002
*/
 {
  int  ret=DTM_SUCCESS ;
  long sp,np,breakLineInserted ; 
/*
** Check For Valid Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Initialise
*/
 breakLineInserted = 0 ;
/*
** Scan Tin Hull
*/
 sp = dtmP->hullPoint ;
 do
   {
    np = nodeAddrP(dtmP,sp)->hPtr ;
/*
**  Check For Break Line
*/
    if( ! bcdtmList_testForBreakLineDtmObject(dtmP,sp,np) )
      {
/*
**  Insert As Break Line In Tin
*/
       nodeAddrP(dtmP,sp)->tPtr = np ;
       if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,nullptr,0,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,sp,1)) goto errexit ; 
       breakLineInserted = 1 ;
      } 
/*
**  Get Next Line
*/
    sp = np ;
   } while( sp != dtmP->hullPoint ) ;
/*
** Compact Feature Table And List If Break Lines Were Inserted
*/
 if( breakLineInserted )
   {
    if( bcdtmTin_compactFeatureTableDtmObject(dtmP)) goto errexit ;
    if( bcdtmTin_compactFeatureListDtmObject(dtmP)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup : ;
/*
** Job Completed
*/
 return(0) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
  


