/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmLattice.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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
BENTLEYDTM_EXPORT int bcdtmLattice_createLatticeFromDtmFile
(
 WCharCP dtmFileNameP,
 WCharCP latticeFileP,
 long        polynomialOption,
 long        latticeOption,
 long        numLatticePts,
 double      xinc,
 double      xreg,
 double      xmin,
 double      xmax,
 double      yinc,
 double      yreg,
 double      ymin,
 double      ymax 
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ  *dtmP=NULL ;
 DTM_LAT_OBJ *latticeP=NULL ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Lattice From Dtm File %s",dtmFileNameP) ;
/*
** Test If Requested Dtm File Is Current Dtm Object
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileNameP)) goto errexit ;
/*
** Delete Current Lattice Object If It Exists
*/
// if( DTM_CLOBJ != NULL )  if( bcdtmObject_deleteLatticeObject(&DTM_CLOBJ)) goto errexit ;
/*
**  Build Lattice
*/
 if( bcdtmLattice_createLatticeFromDtmObject(dtmP,&latticeP,polynomialOption,latticeOption,numLatticePts,xinc,xreg,xmin,xmax,yinc,yreg,ymin,ymax)) goto errexit ;
/*
** Delete DTM Object
*/
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Set Currency
*/
// if( bcdtmUtl_setCurrentLatticeObject(latticeP,latticeFileP)) goto errexit ; 
/*
** Write Lattice File
*/
 if( latticeFileP != 0 ) if( bcdtmWrite_toFileLatticeObject(latticeP,latticeFileP)) goto errexit  ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Lattice From Dtm File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Lattice From Dtm File Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( latticeP != NULL )  bcdtmObject_deleteLatticeObject(&latticeP) ;      
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLattice_createLatticeFromDtmObject
(
 BC_DTM_OBJ  *dtmP,
 DTM_LAT_OBJ **latticePP,
 long        polynomialOption,
 long        latticeOption,
 long        numLatticePts,
 double      xinc,
 double      xreg,
 double      xmin,
 double      xmax,
 double      yinc,
 double      yreg,
 double      ymin,
 double      ymax
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long   startTime ;
 long   nxl,nyl ;
 double *partialDerivP=NULL ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Lattice From Dtm Object %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"latticePP        = %p",*latticePP) ;
    bcdtmWrite_message(0,0,0,"polynomialOption = %8ld",polynomialOption) ;
    bcdtmWrite_message(0,0,0,"latticeOption    = %8ld",latticeOption) ;
    bcdtmWrite_message(0,0,0,"numLatticePts    = %8ld",numLatticePts) ;
    bcdtmWrite_message(0,0,0,"xinc             = %8.3lf",xinc) ;  
    bcdtmWrite_message(0,0,0,"xreg             = %8.3lf",xreg) ;  
    bcdtmWrite_message(0,0,0,"xmin             = %8.3lf",xmin) ;  
    bcdtmWrite_message(0,0,0,"xmax             = %8.3lf",xmax) ;  
    bcdtmWrite_message(0,0,0,"yinc             = %8.3lf",yinc) ;  
    bcdtmWrite_message(0,0,0,"yreg             = %8.3lf",yreg) ;  
    bcdtmWrite_message(0,0,0,"ymin             = %8.3lf",ymin) ;  
    bcdtmWrite_message(0,0,0,"ymax             = %8.3lf",ymax) ;  
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check For None Null Lattice Object
*/
 if( *latticePP != NULL ) if( bcdtmObject_deleteLatticeObject(latticePP)) goto errexit ;
/*
** Validate Lattice Arguements
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Validating Lattice Options") ;
 if( ( latticeOption == 1 || latticeOption == 2 ) && numLatticePts <= 10 )
   {
    bcdtmWrite_message(1,0,0,"Number Of Lattice Points Must Be Grater Than 10") ;
    goto errexit  ;
   }
 if( (latticeOption == 2 || latticeOption == 4 ) && ( xmax <= xmin || ymax <= ymin ))
   {
    bcdtmWrite_message(1,0,0,"Lattice Window Coordinates Incorrect") ;
    goto errexit  ;
   }
 if( (latticeOption == 3 || latticeOption == 4 ) && ( xinc <= 0.0 || yinc <= 0.0 ))
   {
    bcdtmWrite_message(1,0,0,"Lattice Spacing Incorrect") ;
    goto errexit  ;
   }
/*
** Create The Lattice Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Lattice Object") ;
 if( bcdtmObject_createLatticeObject(latticePP)) goto errexit  ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Lattice Object %p Created",*latticePP) ;
/*
** Calculate Partial Derivatives For DTMFeatureState::Tin
*/
 if( polynomialOption )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Polynomial Coefficients For Dtm Object") ;
    if( bcdtmMath_calculatePartialDerivativesDtmObject(dtmP,&partialDerivP )) goto errexit ;
    (*latticePP)->INTMODE = 1 ;
   }
/*
** Build Lattice
*/
 startTime = bcdtmClock() ;
 switch(latticeOption)
   {
    case   1 :
      if( dbg ) bcdtmWrite_message(0,0,0,"Setting Lattice Option 1 Parameters") ;
      (*latticePP)->LXMIN = dtmP->xMin ; (*latticePP)->LXMAX = dtmP->xMax ; (*latticePP)->LXDIF = dtmP->xRange ;
      (*latticePP)->LYMIN = dtmP->yMin ; (*latticePP)->LYMAX = dtmP->yMax ; (*latticePP)->LYDIF = dtmP->yRange ;
      (*latticePP)->LZMIN = dtmP->zMin ; (*latticePP)->LZMAX = dtmP->zMax ; (*latticePP)->LZDIF = dtmP->zRange ;
      bcdtmLattice_calculateNumberOfLatticeLines((*latticePP)->LXDIF,(*latticePP)->LYDIF,&nxl,&nyl,numLatticePts) ;
	  (*latticePP)->NXL = nxl ;
	  (*latticePP)->NYL = nyl ;
      if( dbg ) bcdtmWrite_message(0,0,0,"Creating %ld Point Lattice",nxl*nyl) ;
      if( bcdtmLattice_populateLatticeDtmObject(dtmP,*latticePP,polynomialOption,partialDerivP) ) goto errexit ;
    break  ;

    case   2 :
      (*latticePP)->LXMIN = xmin  ; (*latticePP)->LXMAX = xmax  ; (*latticePP)->LXDIF = xmax - xmin ;
      (*latticePP)->LYMIN = ymin  ; (*latticePP)->LYMAX = ymax  ; (*latticePP)->LYDIF = ymax - ymin ;
      bcdtmLattice_calculateNumberOfLatticeLines((*latticePP)->LXDIF,(*latticePP)->LYDIF,&nxl,&nyl,numLatticePts) ;
	  (*latticePP)->NXL = nxl ;
	  (*latticePP)->NYL = nyl ;
      (*latticePP)->LZMIN = dtmP->zMin ; (*latticePP)->LZMAX = dtmP->zMax ; (*latticePP)->LZDIF = dtmP->zRange ;
      if( dbg ) bcdtmWrite_message(0,0,0,"Creating %ld Point Lattice",nxl*nyl) ;
      if( bcdtmLattice_populateLatticeDtmObject(dtmP,*latticePP,polynomialOption,partialDerivP) ) goto errexit ;
    break  ;

    case   3 :
      (*latticePP)->LXMIN = bcdtmUtl_adjustValueDown(dtmP->xMin,xreg,xinc) ;
      (*latticePP)->LXMAX = bcdtmUtl_adjustValueUp (dtmP->xMax,xreg,xinc) ;
      (*latticePP)->LXDIF = (*latticePP)->LXMAX - (*latticePP)->LXMIN ;
      nyl   = (long) ( (*latticePP)->LXDIF / xinc ) + 1  ;
      (*latticePP)->LYMIN = bcdtmUtl_adjustValueDown(dtmP->yMin,yreg,yinc) ;
      (*latticePP)->LYMAX = bcdtmUtl_adjustValueUp (dtmP->yMax,yreg,yinc) ;
      (*latticePP)->LYDIF = (*latticePP)->LYMAX - (*latticePP)->LYMIN ;
      nxl   = (long) ( (*latticePP)->LYDIF / yinc ) + 1  ;
	  (*latticePP)->NXL = nxl ;
	  (*latticePP)->NYL = nyl ;
      (*latticePP)->LXMAX = (*latticePP)->LXMIN + xinc * ( nyl - 1 ) ;
      (*latticePP)->LYMAX = (*latticePP)->LYMIN + yinc * ( nxl - 1 ) ;
      (*latticePP)->LXDIF = (*latticePP)->LXMAX - (*latticePP)->LXMIN ;
      (*latticePP)->LYDIF = (*latticePP)->LYMAX - (*latticePP)->LYMIN ;
      (*latticePP)->LZMIN = dtmP->zMin ; 
      (*latticePP)->LZMAX = dtmP->zMax ; 
      (*latticePP)->LZDIF = dtmP->zRange ;
      if( dbg ) bcdtmWrite_message(0,0,0,"Creating %ld Point Lattice",nxl*nyl) ;
      if( bcdtmLattice_populateLatticeDtmObject(dtmP,*latticePP,polynomialOption,partialDerivP) ) goto errexit ;
    break  ;

    case   4 :
      (*latticePP)->LXMIN = bcdtmUtl_adjustValueDown(xmin,xreg,xinc) ;
      (*latticePP)->LXMAX = bcdtmUtl_adjustValueUp (xmax,xreg,xinc) ;
      (*latticePP)->LXDIF = (*latticePP)->LXMAX - (*latticePP)->LXMIN ;
      nyl   = (long) ( (*latticePP)->LXDIF / xinc ) + 1 ;
      (*latticePP)->LYMIN = bcdtmUtl_adjustValueDown(ymin,yreg,yinc) ;
      (*latticePP)->LYMAX = bcdtmUtl_adjustValueUp (ymax,yreg,yinc) ;
      (*latticePP)->LYDIF = (*latticePP)->LYMAX - (*latticePP)->LYMIN ;
      nxl   = (long) ( (*latticePP)->LYDIF / yinc ) + 1  ;
	  (*latticePP)->NXL = nxl ;
	  (*latticePP)->NYL = nyl ;
      (*latticePP)->LXMAX = (*latticePP)->LXMIN + xinc * ( nyl - 1 ) ;
      (*latticePP)->LYMAX = (*latticePP)->LYMIN + yinc * ( nxl - 1 ) ;
      (*latticePP)->LXDIF = (*latticePP)->LXMAX - (*latticePP)->LXMIN ;
      (*latticePP)->LYDIF = (*latticePP)->LYMAX - (*latticePP)->LYMIN ;
      (*latticePP)->LZMIN = dtmP->zMin ; 
      (*latticePP)->LZMAX = dtmP->zMax ; 
      (*latticePP)->LZDIF = dtmP->zRange ;
      if( dbg ) bcdtmWrite_message(0,0,0,"Creating %ld Point Lattice",nxl*nyl) ;
      if( bcdtmLattice_populateLatticeDtmObject(dtmP,*latticePP,polynomialOption,partialDerivP) ) goto errexit ;
    break  ;
   } ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Create %8ld Point Lattice  = %8.3lf Seconds",(*latticePP)->NOLATPTS,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
**  DevNote - Development Purposes Only
*/
 if( dbg ) bcdtmUtl_writeStatisticsLatticeObject(*latticePP) ;
/*
** Clean Up
*/
 cleanup :
 if( partialDerivP != NULL ) free(partialDerivP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Build Lattice From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Build Lattice From Dtm Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *latticePP != NULL ) bcdtmObject_deleteLatticeObject(latticePP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLattice_populateLatticeDtmObject(BC_DTM_OBJ *dtmP,DTM_LAT_OBJ *latticeP,long polynomialOption,double *partialDerivP)
/*
** This Function Interpolates The Lattice Values
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long    i,j,p1,p2,p3,xs,xf,ys,yf,iofs,newTriangle,voidTriangle,clPtr ;
 double  xl,yl,zl,trgXmin,trgXmax,trgYmin,trgYmax,x[3],y[3],z[3],partialDeriv[15] ;
 float   nullValue,*latPointP,latZmin,latZmax ;
 DTM_TIN_POINT *p1P,*p2P,*p3P ;  
 DTM_TIN_NODE  *nodeP ; 
 DTM_CIR_LIST  *clistP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Lattice ** DTM Object = %p ** Lattice Object = %p",dtmP,latticeP) ;
/*
** Set Lattice Null Value
*/
 nullValue = (float) (dtmP->zMin - 100.0 ) ;
/*
** Allocate Memory for Lattice
*/
 if( bcdtmObject_allocateLatticeMemoryLatticeObject(latticeP,latticeP->NXL,latticeP->NYL,nullValue)) goto errexit ;
/*
** Initialise Lattice values
*/
 latticeP->DX = (latticeP->LXMAX-latticeP->LXMIN) / (double) (latticeP->NYL - 1) ;
 latticeP->DY = (latticeP->LYMAX-latticeP->LYMIN) / (double) (latticeP->NXL - 1) ;
/*
** Process Each Triangle :-
** 1. Determine Lattice Points in Triangle
** 2. Interpolate z value for Each Lattice Point
*/
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    nodeP = nodeAddrP(dtmP,p1) ;
    if( ( clPtr = nodeP->cPtr ) != dtmP->nullPtr )
      {
       p1P = pointAddrP(dtmP,p1) ;
       x[0] = p1P->x ;
       y[0] = p1P->y ;
       z[0] = p1P->z ;
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum )) < 0 ) goto errexit ;
       while( clPtr != dtmP->nullPtr )
	     {
          clistP = clistAddrP(dtmP,clPtr) ;
          p3     = clistP->pntNum ;
          clPtr  = clistP->nextPtr ;
	      if( p3 > p1 && p2 > p1 && nodeP->hPtr != p2 )
	        {
             if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
			 if( ! voidTriangle )
		       {
                p2P = pointAddrP(dtmP,p2) ;
                p3P = pointAddrP(dtmP,p3) ;
                x[1] = p2P->x ;
                y[1] = p2P->y ;
                z[1] = p2P->z ;
                x[2] = p3P->x ;
                y[2] = p3P->y ;
                z[2] = p3P->z ;
		        trgXmin = bcdtmUtl_getTrgMin(x) ;
                trgXmax = bcdtmUtl_getTrgMax(x) ;
		        trgYmin = bcdtmUtl_getTrgMin(y) ;
                trgYmax = bcdtmUtl_getTrgMax(y) ;
		        if( trgXmin <= latticeP->LXMAX && trgXmax >= latticeP->LXMIN && trgYmin <= latticeP->LYMAX && trgYmax >= latticeP->LYMIN )
		          {
		           newTriangle = 1 ;
		           if( polynomialOption ) bcdtmLattice_getPartialDerivatives(p1,p2,p3,partialDerivP,partialDeriv) ;
		           xs = (long)((trgXmin-latticeP->LXMIN) / latticeP->DX) ;
		           xf = (long)((trgXmax-latticeP->LXMIN) / latticeP->DX) ;
		           ys = (long)((trgYmin-latticeP->LYMIN) / latticeP->DY) ;
		           yf = (long)((trgYmax-latticeP->LYMIN) / latticeP->DY) ;
		           if( xs < 0 ) xs = 0 ; if( xf >= latticeP->NYL ) xf = latticeP->NYL - 1 ;
		           if( ys < 0 ) ys = 0 ; if( yf >= latticeP->NXL ) yf = latticeP->NXL - 1 ;
		           for( i = xs ; i <= xf ; ++i)
		             {
		              for( j = ys ; j <= yf ; ++j )
		                {
		                 xl = (double) i * latticeP->DX + latticeP->LXMIN ;
		                 yl = (double) j * latticeP->DY + latticeP->LYMIN ;
		                 iofs = latticeP->NYL * j + i ;
		                 if( *(latticeP->LAT+iofs) == nullValue )
		                   {
		                    if(bcdtmMath_sideOf(x[0],y[0],x[1],y[1],xl,yl) <= 0 )
		  	                if(bcdtmMath_sideOf(x[1],y[1],x[2],y[2],xl,yl) <= 0 )
				            if(bcdtmMath_sideOf(x[2],y[2],x[0],y[0],xl,yl) <= 0 )
				              {
				               if( ! polynomialOption ) bcdtmMath_interpolatePointOnTriangle(xl,yl,&zl,x,y,z) ;
				               else                     bcdtmMath_interpolatePointOnPolynomial(newTriangle,xl,yl,&zl,x,y,z,partialDeriv) ;
				               *(latticeP->LAT+iofs) = (float) zl   ;
				               newTriangle = 0 ;
				              }
			               }
			            }
		             }
		          }
	           }
	        }
	      p2 = p3 ; 
	     } 
      }
   }
/*
** Set Lattice Min & Max z Values 
*/
 latZmin  = (float) latticeP->LZMAX ;
 latZmax  = (float) latticeP->LZMIN ;
 latticeP->NOACTPTS = 0 ;
 for( latPointP = latticeP->LAT ; latPointP < latticeP->LAT + latticeP->NOLATPTS ; ++latPointP )
   {
    if( *latPointP != nullValue ) 
	  { 
	   ++latticeP->NOACTPTS ; 
	   if( *latPointP < latZmin ) latZmin = *latPointP ;
	   if( *latPointP > latZmax ) latZmax = *latPointP ;
	  }
   }
 latticeP->LZMIN = latZmin  ;
 latticeP->LZMAX = latZmax  ;
 latticeP->LZDIF = latticeP->LZMAX - latticeP->LZMIN ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS) bcdtmWrite_message(0,0,0,"Creating Lattice Completed") ;
 if( dbg && ret != DTM_SUCCESS) bcdtmWrite_message(0,0,0,"Creating Lattice Error") ;
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
BENTLEYDTM_EXPORT int bcdtmLattice_createIsopachLatticeFromDtmFile
(
 WCharCP dtmFileNameP,
 WCharCP latticeFileP,
 long        polynomialOption,
 long        latticeOption,
 long        numLatticePts,
 double      elevation ,
 double      xinc,
 double      xreg,
 double      xmin,
 double      xmax,
 double      yinc,
 double      yreg,
 double      ymin,
 double      ymax 
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ  *dtmP=NULL ;
 DTM_LAT_OBJ *latticeP=NULL ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Isopach Lattice From Dtm File %s",dtmFileNameP) ;
/*
** Test If Requested Dtm File Is Current Dtm Object
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileNameP)) goto errexit ;
/*
** Delete Current Lattice Object If It Exists
*/
// if( DTM_CLOBJ != NULL )  if( bcdtmObject_deleteLatticeObject(&DTM_CLOBJ)) goto errexit ;
/*
**  Build Lattice
*/
 if( bcdtmLattice_createIsopachLatticeFromDtmObject(dtmP,&latticeP,polynomialOption,latticeOption,numLatticePts,elevation,xinc,xreg,xmin,xmax,yinc,yreg,ymin,ymax)) goto errexit ;
/*
** Delete DTM Object
*/
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Set Currency
*/
 if( bcdtmUtl_setCurrentLatticeObject(latticeP,latticeFileP)) goto errexit ; 
/*
** Write Lattice File
*/
 if( latticeFileP != 0 ) if( bcdtmWrite_toFileLatticeObject(latticeP,latticeFileP)) goto errexit  ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Lattice From Dtm File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Lattice From Dtm File Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( latticeP != NULL )  bcdtmObject_deleteLatticeObject(&latticeP) ;      
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLattice_createIsopachLatticeFromDtmObject
(
 BC_DTM_OBJ  *dtmP,
 DTM_LAT_OBJ **latticePP,
 long        polynomialOption,
 long        latticeOption,
 long        numLatticePts,
 double      elevation,
 double      xinc,
 double      xreg,
 double      xmin,
 double      xmax,
 double      yinc,
 double      yreg,
 double      ymin,
 double      ymax
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Isopach Lattice From Dtm Object %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"latticePP        = %p",*latticePP) ;
    bcdtmWrite_message(0,0,0,"polynomialOption = %8ld",polynomialOption) ;
    bcdtmWrite_message(0,0,0,"latticeOption    = %8ld",latticeOption) ;
    bcdtmWrite_message(0,0,0,"elevation        = %8.2lf",elevation) ;
    bcdtmWrite_message(0,0,0,"xinc             = %8.3lf",xinc) ;  
    bcdtmWrite_message(0,0,0,"xreg             = %8.3lf",xreg) ;  
    bcdtmWrite_message(0,0,0,"xmin             = %8.3lf",xmin) ;  
    bcdtmWrite_message(0,0,0,"xmax             = %8.3lf",xmax) ;  
    bcdtmWrite_message(0,0,0,"yinc             = %8.3lf",yinc) ;  
    bcdtmWrite_message(0,0,0,"yreg             = %8.3lf",yreg) ;  
    bcdtmWrite_message(0,0,0,"ymin             = %8.3lf",ymin) ;  
    bcdtmWrite_message(0,0,0,"ymax             = %8.3lf",ymax) ;  
   }
/*
**  Create Lattice
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Lattice") ;
 if( bcdtmLattice_createLatticeFromDtmObject(dtmP,latticePP,polynomialOption,latticeOption,numLatticePts,xinc,xreg,xmin,xmax,yinc,yreg,ymin,ymax)) goto errexit ;
/*
**  Subtract Elevation From Lattice
*/
 if( bcdtmLattice_moveZLatticeObject(*latticePP,elevation,2) ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Isopach Lattice From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Isopach Lattice From Dtm Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *latticePP != NULL ) bcdtmObject_deleteLatticeObject(latticePP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLattice_moveZLatticeObject(DTM_LAT_OBJ *latticeP,double moveValue,long moveOption) 
/*
** This Function Moves The z Value Of The Lattice Object
**
** moveOption == 0  Set z to moveValue
**            == 1  Add moveValue To z
**            == 2  Subtract moveValue From z  
**            == 3  Subtract z From moveValue  
*/
{
 int  ret=DTM_SUCCESS ;
 long i,j ;
/*
** Test For Valid Lattice Object
*/
 if( bcdtmObject_testForValidLatticeObject(latticeP)) goto errexit ;
/*
** Scan Lattice Array And Move z Zalues
*/
 for( j = 0 ; j < latticeP->NXL  ; ++j )
   {
    for( i = 0 ; i < latticeP->NYL  ; ++i )
      {
       if( *(latticeP->LAT + latticeP->NYL *  j + i)  != latticeP->NULLVAL )
         {
	      if( moveOption == 0 ) *(latticeP->LAT + latticeP->NYL *  j + i) = (float) moveValue ;
	      if( moveOption == 1 ) *(latticeP->LAT + latticeP->NYL *  j + i) = *(latticeP->LAT + latticeP->NYL *  j + i) + (float) moveValue ;
	      if( moveOption == 2 ) *(latticeP->LAT + latticeP->NYL *  j + i) = *(latticeP->LAT + latticeP->NYL *  j + i) - (float) moveValue ;
	      if( moveOption == 3 ) *(latticeP->LAT + latticeP->NYL *  j + i) = (float) moveValue - *(latticeP->LAT + latticeP->NYL *  j + i) ;
         }
      }
   } 
/*
** Adjust Lattice z Ranges
*/
 if( moveOption == 0 ) { latticeP->LZMIN = latticeP->LZMAX = (float) moveValue ; latticeP->LZDIF = 0.0 ; }
 if( moveOption == 1 ) { latticeP->LZMIN += (float) moveValue ; latticeP->LZMAX += (float) moveValue ; }
 if( moveOption == 2 ) { latticeP->LZMIN -= (float) moveValue ; latticeP->LZMAX -= (float) moveValue ; }
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLattice_createIsopachLatticeFromDtmFiles
(
 WCharCP fromDtmFileP,
 WCharCP toDtmFileP,
 WCharCP latticeFileP,
 long   latticeOption,
 long   numLatticePts,
 double xinc,
 double xreg,
 double xmin,
 double xmax,
 double yinc,
 double yreg,
 double ymin,
 double ymax
) 
{
 int         ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ  *fromDtmP=NULL,*toDtmP=NULL ;
 DTM_LAT_OBJ *latticeP=NULL ;
/*
** Write Status Message
*/ 
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Isopach Lattice From Dtm Files") ;
/*
** Check For Different Files
*/
 if( wcscmp(fromDtmFileP,toDtmFileP) == 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Dtm Files The Same") ;
    goto errexit ;
   } 
/*
** Read From Dtm File As Current DTM Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading From Dtm File %s",fromDtmFileP) ;
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&fromDtmP,fromDtmFileP)) goto errexit ; 
/*
** Read To Dtm File Into Memory
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading To   Dtm File %s",toDtmFileP) ;
 if( bcdtmRead_fromFileDtmObject(&toDtmP,toDtmFileP))  goto errexit ; 
/*
** Delete Current Lattice Object If It Exists
*/
/*
 if( DTM_CLOBJ != NULL ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Destroying Current DTM Lattice Object %p",DTM_CLOBJ ) ;
    if( bcdtmObject_deleteLatticeObject(&DTM_CLOBJ)) goto errexit ;
    wcscpy(DTM_CLOBJ_FILE,L"") ;
   } 
*/   
/*
**  Create Isopach Lattice
*/
 if( bcdtmLattice_createIsopachLatticeFromDtmObjects(fromDtmP,toDtmP,&latticeP,latticeOption,numLatticePts,xinc,xreg,xmin,xmax,yinc,yreg,ymin,ymax) ) goto errexit ;
/*
** Delete Dtm Objects
*/
 if( fromDtmP != NULL ) bcdtmObject_destroyDtmObject(&fromDtmP) ; 
 if( toDtmP   != NULL ) bcdtmObject_destroyDtmObject(&toDtmP) ; 
/*
** Set Currency
*/
// DTM_CLOBJ = latticeP ;
// if( *latticeFileP != 0 ) wcscpy(DTM_CLOBJ_FILE,latticeFileP) ;
/*
** Write Lattice File
*/
 if( *latticeFileP != 0 ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Lattice File %s",latticeFileP) ;
    if( bcdtmWrite_toFileLatticeObject(latticeP,latticeFileP)) goto errexit ;
   }
/*
** Cleanup
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( fromDtmP != NULL ) bcdtmObject_destroyDtmObject(&fromDtmP) ; 
 if( toDtmP   != NULL ) bcdtmObject_destroyDtmObject(&toDtmP) ; 
 if( latticeP != NULL ) bcdtmObject_deleteLatticeObject(&latticeP) ;
/* 
 if( DTM_CDTM != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Destroying Current DTM Object %p",DTM_CDTM ) ;
    if( ! bcdtmObject_testForValidDtmObject(DTM_CDTM) )
      {
       bcdtmObject_destroyDtmObject(&DTM_CDTM) ;  
       wcscpy(DTM_CDTM_FILE,L"") ;
      }  
   }
 if( DTM_CLOBJ != NULL ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Destroying Current DTM Lattice Object %p",DTM_CLOBJ ) ;
    if( bcdtmObject_deleteLatticeObject(&DTM_CLOBJ)) goto errexit ;
    wcscpy(DTM_CLOBJ_FILE,L"") ;
   } 
*/   
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLattice_createIsopachLatticeFromDtmObjects
(
 BC_DTM_OBJ *fromDtmP,
 BC_DTM_OBJ *toDtmP,
 DTM_LAT_OBJ **latticePP,
 long latticeOption,
 long numLatticePts,
 double xinc,
 double xreg,
 double xmin,
 double xmax,
 double yinc,
 double yreg,
 double ymin,
 double ymax
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   nxl,nyl,intersectResult ;
 double bxMin,bxMax,byMin,byMax ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Creating Isopach Lattice From Dtm Objects") ;
    bcdtmWrite_message(0,0,0,"fromDtmP        = %p",fromDtmP) ;
    bcdtmWrite_message(0,0,0,"toDtmP          = %p",toDtmP) ;
    bcdtmWrite_message(0,0,0,"*latticePP      = %p",*latticePP) ;
    bcdtmWrite_message(0,0,0,"latticeOption   = %8ld",latticeOption) ;
    bcdtmWrite_message(0,0,0,"numLatticePts   = %8ld",numLatticePts) ;
    bcdtmWrite_message(0,0,0,"xreg            = %8.2lf",xreg) ;
    bcdtmWrite_message(0,0,0,"yreg            = %8.2lf",yreg) ;
    bcdtmWrite_message(0,0,0,"xinc            = %8.2lf",xinc) ;
    bcdtmWrite_message(0,0,0,"yinc            = %8.2lf",yinc) ;
    bcdtmWrite_message(0,0,0,"xmin            = %8.2lf",xmin) ;
    bcdtmWrite_message(0,0,0,"xmax            = %8.2lf",xmax) ;
    bcdtmWrite_message(0,0,0,"ymin            = %8.2lf",ymin) ;
    bcdtmWrite_message(0,0,0,"ymax            = %8.2lf",ymax) ;
   }
/*
** Check For Valid Objects
*/
 if( bcdtmObject_testForValidDtmObject(fromDtmP)) goto errexit ;
 if( bcdtmObject_testForValidDtmObject(toDtmP)) goto errexit ;
 if( *latticePP != NULL ) bcdtmObject_deleteLatticeObject(latticePP) ;
/*
** Check Dtm Objects Are In Dtm Tin State
*/
 if( fromDtmP->dtmState != DTMState::Tin || toDtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"DTM Object(s) Not Triangulated") ;
    goto errexit ;
   }
/*
** Validate Lattice Arguements
*/
 if( latticeOption < 1 || latticeOption > 4 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Lattice Option") ;
    goto errexit ;
   } 
 if( ( latticeOption == 1 || latticeOption == 2 ) && numLatticePts <= 10 )
   {
    bcdtmWrite_message(1,0,0,"Number Of Lattice Points Must Be Grater Than 10") ;
    goto errexit ;
   }
 if( (latticeOption == 2 || latticeOption == 4 ) && ( xmax <= xmin || ymax <= ymin ))
   {
    bcdtmWrite_message(1,0,0,"Lattice Window Coordinates Incorrect") ;
    goto errexit ;
   }
 if( (latticeOption == 3 || latticeOption == 4 ) && ( xinc <= 0.0 || yinc <= 0.0 ))
   {
    bcdtmWrite_message(1,0,0,"Lattice Spacing Incorrect") ;
    goto errexit ;
   }
/*
** Set Minimum Bounding Rectangle To Covering Both Hulls
*/
 bxMin = fromDtmP->xMin ; bxMax = fromDtmP->xMax ;
 byMin = fromDtmP->yMin ; byMax = fromDtmP->yMax ;
 if( bxMin < toDtmP->xMin ) bxMin = toDtmP->xMin ;
 if( bxMax > toDtmP->xMax ) bxMax = toDtmP->xMax ;
 if( byMin < toDtmP->yMin ) byMin = toDtmP->yMin ;
 if( byMax > toDtmP->yMax ) byMax = toDtmP->yMax ;
 if( xmin < bxMin || xmin > bxMax ) xmin = bxMin ;
 if( xmax < bxMin || xmax > bxMax ) xmax = bxMax ;
 if( ymin < byMin || ymin > byMax ) ymin = byMin ;
 if( ymax < byMin || ymax > byMax ) ymax = byMax ;
 if( latticeOption == 1 ) latticeOption = 2 ;
 if( latticeOption == 3 ) latticeOption = 4 ;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Minimum Bounding Rectange Covering Both Tin Hulls") ;
    bcdtmWrite_message(0,0,0,"xmin = %12.4lf ymin = %12.4lf xmax = %12.4lf ymax = %12.4lf",xmin,ymin,xmax,ymax) ;
   }
/*
** Check Tin Hulls Overlap
*/
  if( bcdtmClip_checkTinHullsIntersectDtmObject(fromDtmP,toDtmP,&intersectResult)) goto errexit ;
  if( dbg ) bcdtmWrite_message(0,0,0,"intersectResult = %2ld",intersectResult) ; 
  if( ! intersectResult )
    {
     bcdtmWrite_message(1,0,0,"Tin Hulls Do Not Overlap") ;
     goto errexit ;
    } 
/*
** Create A Lattice Object
*/
 if( bcdtmObject_createLatticeObject(latticePP)) goto errexit ;
/*
** Build Lattice
*/
 switch(latticeOption)
   {
    case   1 :
      if( fromDtmP->xMin <= toDtmP->xMin ) (*latticePP)->LXMIN = toDtmP->xMin ;
      else                                 (*latticePP)->LXMIN = fromDtmP->xMin ;
      if( fromDtmP->xMax <= toDtmP->xMax ) (*latticePP)->LXMAX = fromDtmP->xMax ;
      else                                 (*latticePP)->LXMAX = toDtmP->xMax ;
      if( fromDtmP->yMin <= toDtmP->yMin ) (*latticePP)->LYMIN = toDtmP->yMin ;
      else                                 (*latticePP)->LYMIN = fromDtmP->yMin ;
      if( fromDtmP->yMax <= toDtmP->yMax ) (*latticePP)->LYMAX = fromDtmP->yMax ;
      else                                 (*latticePP)->LYMAX = toDtmP->yMax ;
      (*latticePP)->LXDIF = (*latticePP)->LXMAX - (*latticePP)->LXMIN ;
      (*latticePP)->LYDIF = (*latticePP)->LYMAX - (*latticePP)->LYMIN ;
      bcdtmLattice_calculateNumberOfLatticeLines((*latticePP)->LXDIF,(*latticePP)->LYDIF,&nxl,&nyl,numLatticePts) ;
	  (*latticePP)->NXL = nxl ;
	  (*latticePP)->NYL = nyl ;
    break  ;

    case   2 :
      (*latticePP)->LXMIN = xmin  ; (*latticePP)->LXMAX = xmax  ; (*latticePP)->LXDIF = xmax - xmin ;
      (*latticePP)->LYMIN = ymin  ; (*latticePP)->LYMAX = ymax  ; (*latticePP)->LYDIF = ymax - ymin ;
      bcdtmLattice_calculateNumberOfLatticeLines((*latticePP)->LXDIF,(*latticePP)->LYDIF,&nxl,&nyl,numLatticePts) ;
	  (*latticePP)->NXL = nxl ;
	  (*latticePP)->NYL = nyl ;
    break  ;

    case   3 :
      if( fromDtmP->xMin <= toDtmP->xMin ) (*latticePP)->LXMIN = toDtmP->xMin ;
      else                                 (*latticePP)->LXMIN = fromDtmP->xMin ;
      if( fromDtmP->xMax <= toDtmP->xMax ) (*latticePP)->LXMAX = fromDtmP->xMax ;
      else                                 (*latticePP)->LXMAX = toDtmP->xMax ;
      if( fromDtmP->yMin <= toDtmP->yMin ) (*latticePP)->LYMIN = toDtmP->yMin ;
      else                                 (*latticePP)->LYMIN = fromDtmP->yMin ;
      if( fromDtmP->yMax <= toDtmP->yMax ) (*latticePP)->LYMAX = fromDtmP->yMax ;
      else                                 (*latticePP)->LYMAX = toDtmP->yMax ;

      (*latticePP)->LXMIN = bcdtmUtl_adjustValueDown((*latticePP)->LXMIN,xreg,xinc) ;
      (*latticePP)->LXMAX = bcdtmUtl_adjustValueUp((*latticePP)->LXMAX,xreg,xinc) ;
      (*latticePP)->LXDIF = (*latticePP)->LXMAX - (*latticePP)->LXMIN ;
      nyl   = (long) ( (*latticePP)->LXDIF / xinc ) + 1  ;
      (*latticePP)->LYMIN = bcdtmUtl_adjustValueDown((*latticePP)->LYMIN,yreg,yinc) ;
      (*latticePP)->LYMAX = bcdtmUtl_adjustValueUp ((*latticePP)->LYMAX,yreg,yinc) ;
      (*latticePP)->LYDIF = (*latticePP)->LYMAX - (*latticePP)->LYMIN ;
      nxl   = (long) ( (*latticePP)->LYDIF / yinc ) + 1  ;
	  (*latticePP)->NXL = nxl ;
	  (*latticePP)->NYL = nyl ;
    break  ;

    case   4 :
      (*latticePP)->LXMIN = bcdtmUtl_adjustValueDown(xmin,xreg,xinc) ;
      (*latticePP)->LXMAX = bcdtmUtl_adjustValueUp (xmax,xreg,xinc) ;
      (*latticePP)->LXDIF = (*latticePP)->LXMAX - (*latticePP)->LXMIN ;
      nyl   = (long) ( (*latticePP)->LXDIF / xinc ) + 1 ;
      (*latticePP)->LYMIN = bcdtmUtl_adjustValueDown(ymin,yreg,yinc) ;
      (*latticePP)->LYMAX = bcdtmUtl_adjustValueUp (ymax,yreg,yinc) ;
      (*latticePP)->LYDIF = (*latticePP)->LYMAX - (*latticePP)->LYMIN ;
      nxl   = (long) ( (*latticePP)->LYDIF / yinc ) + 1  ;
	  (*latticePP)->NXL = nxl ;
	  (*latticePP)->NYL = nyl ;
    break  ;
   } ;
/*
** Populate Isopach Lattice
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Isopach Lattice") ;
 if( bcdtmLattice_populateIsopachLatticeFromDtmObjects(fromDtmP,toDtmP,*latticePP)) goto errexit ;
/*
**  Write Isopach Lattice Stats
*/
 if( dbg ) bcdtmUtl_writeStatisticsLatticeObject(*latticePP) ;
/*
** Cleanup
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLattice_populateIsopachLatticeFromDtmObjects
(
 BC_DTM_OBJ *fromDtmP,           /* ==> Pointer To From_DTM Object   */
 BC_DTM_OBJ *toDtmP,             /* ==> Pointer To To_DTM Object     */
 DTM_LAT_OBJ *latticeP           /* ==> Pointer To Isopach Lattice   */
)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    i,j,p1,p2,p3,xs,xf,ys,yf,iofs,clPtr,numLatPts,newTriangle,voidTriangle ;
 double  xl,yl,zl,trgXmin,trgXmax,trgYmin,trgYmax,x[3],y[3],z[3] ;
 float   *lat,zmin,zmax ;
 unsigned char    *subFlagP=NULL,*cp ;
 DTM_TIN_POINT *p1P,*p2P,*p3P ;  
 DTM_TIN_NODE  *nodeP ; 
 DTM_CIR_LIST  *clistP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Isopach Lattice") ;
/*
** Allocate Storage for lattice
*/
 if( bcdtmObject_allocateLatticeMemoryLatticeObject(latticeP,latticeP->NXL,latticeP->NYL,-989898.0)) goto errexit ;
/*
** Initialise Lattice values
*/
 latticeP->DX = (latticeP->LXMAX-latticeP->LXMIN) / (double) (latticeP->NYL - 1) ;
 latticeP->DY = (latticeP->LYMAX-latticeP->LYMIN) / (double) (latticeP->NXL - 1) ;
/*
** Write Lattice Parameters 
/*
** Allocate Memory To Flag Subtraction
*/
 subFlagP = ( unsigned char * ) malloc( (latticeP->NOLATPTS/8+1)*sizeof(char)) ;
 if( subFlagP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
 for( cp = subFlagP ; cp < subFlagP + latticeP->NOLATPTS/8+1 ; ++cp ) *cp = 0 ;
/*
**  Build Lattice For From Dtm Element
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Lattice For From Dtm %p",fromDtmP) ;
 numLatPts = 0 ;
 for( p1 = 0 ; p1 < fromDtmP->numPoints ; ++p1 )
   {
    nodeP = nodeAddrP(fromDtmP,p1) ;
    if( ( clPtr = nodeP->cPtr ) != fromDtmP->nullPtr )
      {
       p1P = pointAddrP(fromDtmP,p1) ;
       x[0] = p1P->x ;
       y[0] = p1P->y ;
       z[0] = p1P->z ;
       if( ( p2 = bcdtmList_nextAntDtmObject(fromDtmP,p1,clistAddrP(fromDtmP,clPtr)->pntNum )) < 0 ) goto errexit ;
       while( clPtr != fromDtmP->nullPtr )
	     {
          clistP = clistAddrP(fromDtmP,clPtr) ;
          p3     = clistP->pntNum ;
          clPtr  = clistP->nextPtr ;
	      if( p3 > p1 && p2 > p1 && nodeP->hPtr != p2 )
	        {
             if( bcdtmList_testForVoidTriangleDtmObject(fromDtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
			 if( ! voidTriangle )
		       {
                p2P = pointAddrP(fromDtmP,p2) ;
                p3P = pointAddrP(fromDtmP,p3) ;
                x[1] = p2P->x ;
                y[1] = p2P->y ;
                z[1] = p2P->z ;
                x[2] = p3P->x ;
                y[2] = p3P->y ;
                z[2] = p3P->z ;
		        trgXmin = bcdtmUtl_getTrgMin(x) ;
                trgXmax = bcdtmUtl_getTrgMax(x) ;
		        trgYmin = bcdtmUtl_getTrgMin(y) ;
                trgYmax = bcdtmUtl_getTrgMax(y) ;
		        if( trgXmin <= latticeP->LXMAX && trgXmax >= latticeP->LXMIN && trgYmin <= latticeP->LYMAX && trgYmax >= latticeP->LYMIN )
		          {
		           newTriangle = 1 ;
		           xs = (long)((trgXmin-latticeP->LXMIN) / latticeP->DX) ;
		           xf = (long)((trgXmax-latticeP->LXMIN) / latticeP->DX) ;
		           ys = (long)((trgYmin-latticeP->LYMIN) / latticeP->DY) ;
		           yf = (long)((trgYmax-latticeP->LYMIN) / latticeP->DY) ;
		           if( xs < 0 ) xs = 0 ; if( xf >= latticeP->NYL ) xf = latticeP->NYL - 1 ;
		           if( ys < 0 ) ys = 0 ; if( yf >= latticeP->NXL ) yf = latticeP->NXL - 1 ;
		           for( i = xs ; i <= xf ; ++i)
		             {
		              for( j = ys ; j <= yf ; ++j )
		                {
		                 xl = (double) i * latticeP->DX + latticeP->LXMIN ;
		                 yl = (double) j * latticeP->DY + latticeP->LYMIN ;
		                 iofs = latticeP->NYL * j + i ;
		                 if( *(latticeP->LAT+iofs) == latticeP->NULLVAL )
		                   {
		                    if(bcdtmMath_sideOf(x[0],y[0],x[1],y[1],xl,yl) <= 0 )
		  	                if(bcdtmMath_sideOf(x[1],y[1],x[2],y[2],xl,yl) <= 0 )
				            if(bcdtmMath_sideOf(x[2],y[2],x[0],y[0],xl,yl) <= 0 )
				              {
                               bcdtmMath_interpolatePointOnTrianglePlane(newTriangle,xl,yl,&zl,x,y,z) ;
				               *(latticeP->LAT+iofs) = (float) zl   ;
				               newTriangle = 0 ;
                               ++numLatPts ;
				              }
			               }
			            }
		             }
		          }
	           }
	        }
	      p2 = p3 ; 
	     } 
      }
   }
/*
** Write Number Of Lattice Points Set
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Lattice Points Set = %9ld",numLatPts) ;
/*
**  Build Lattice For To Dtm And Subtract  
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Lattice For To   Dtm %p",toDtmP) ;
 numLatPts = 0 ;
 for( p1 = 0 ; p1 < toDtmP->numPoints ; ++p1 )
   {
    nodeP = nodeAddrP(toDtmP,p1) ;
    if( ( clPtr = nodeP->cPtr ) != toDtmP->nullPtr )
      {
       p1P = pointAddrP(toDtmP,p1) ;
       x[0] = p1P->x ;
       y[0] = p1P->y ;
       z[0] = p1P->z ;
       if( ( p2 = bcdtmList_nextAntDtmObject(toDtmP,p1,clistAddrP(toDtmP,clPtr)->pntNum )) < 0 ) goto errexit ;
       while( clPtr != toDtmP->nullPtr )
	     {
          clistP = clistAddrP(toDtmP,clPtr) ;
          p3     = clistP->pntNum ;
          clPtr  = clistP->nextPtr ;
	      if( p3 > p1 && p2 > p1 && nodeP->hPtr != p2 )
	        {
             if( bcdtmList_testForVoidTriangleDtmObject(toDtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
			 if( ! voidTriangle )
		       {
                p2P = pointAddrP(toDtmP,p2) ;
                p3P = pointAddrP(toDtmP,p3) ;
                x[1] = p2P->x ;
                y[1] = p2P->y ;
                z[1] = p2P->z ;
                x[2] = p3P->x ;
                y[2] = p3P->y ;
                z[2] = p3P->z ;
		        trgXmin = bcdtmUtl_getTrgMin(x) ;
                trgXmax = bcdtmUtl_getTrgMax(x) ;
		        trgYmin = bcdtmUtl_getTrgMin(y) ;
                trgYmax = bcdtmUtl_getTrgMax(y) ;
		        if( trgXmin <= latticeP->LXMAX && trgXmax >= latticeP->LXMIN && trgYmin <= latticeP->LYMAX && trgYmax >= latticeP->LYMIN )
		          {
		           newTriangle = 1 ;
		           xs = (long)((trgXmin-latticeP->LXMIN) / latticeP->DX) ;
		           xf = (long)((trgXmax-latticeP->LXMIN) / latticeP->DX) ;
		           ys = (long)((trgYmin-latticeP->LYMIN) / latticeP->DY) ;
		           yf = (long)((trgYmax-latticeP->LYMIN) / latticeP->DY) ;
		           if( xs < 0 ) xs = 0 ; if( xf >= latticeP->NYL ) xf = latticeP->NYL - 1 ;
		           if( ys < 0 ) ys = 0 ; if( yf >= latticeP->NXL ) yf = latticeP->NXL - 1 ;
		           for( i = xs ; i <= xf ; ++i)
		             {
		              for( j = ys ; j <= yf ; ++j )
		                {
		                 xl = (double) i * latticeP->DX + latticeP->LXMIN ;
		                 yl = (double) j * latticeP->DY + latticeP->LYMIN ;
		                 iofs = latticeP->NYL * j + i ;
                         if( *(latticeP->LAT+iofs) != latticeP->NULLVAL )
		                   {
                            if( ! bcdtmFlag_testFlag(subFlagP,iofs))
                              {
		                       if(bcdtmMath_sideOf(x[0],y[0],x[1],y[1],xl,yl) <= 0 )
		  	                   if(bcdtmMath_sideOf(x[1],y[1],x[2],y[2],xl,yl) <= 0 )
				               if(bcdtmMath_sideOf(x[2],y[2],x[0],y[0],xl,yl) <= 0 )
				                 {
                                  bcdtmMath_interpolatePointOnTrianglePlane(newTriangle,xl,yl,&zl,x,y,z) ;
				                  *(latticeP->LAT+iofs) -= (float) zl   ;
                                  bcdtmFlag_setFlag(subFlagP,iofs) ;
				                  newTriangle = 0 ;
                                  ++numLatPts ;
                                 }
				              }
			               }
			            }
		             }
		          }
	           }
	        }
	      p2 = p3 ; 
	     } 
      }
   }
/*
** Write Number Of Lattice Points Set
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Lattice Points Set = %9ld",numLatPts) ;
/*
** Null Out Lattice Points Where Overlap Did Not Occur
*/
 for( iofs=0 ; iofs < latticeP->NOLATPTS ; ++iofs )
   {
    if( ! bcdtmFlag_testFlag(subFlagP,iofs) ) *(latticeP->LAT+iofs) = latticeP->NULLVAL ;
   } 
/*
** Set Lattice Min & Max Values 
*/
 p1 = 0 ;   
 zmin  =  zmax  = 0.0 ;
 latticeP->NOACTPTS = 0 ;
 for( lat = latticeP->LAT ; lat < latticeP->LAT + latticeP->NOLATPTS ; ++lat )
   {
    if( *lat != latticeP->NULLVAL ) 
	  { 
	   ++latticeP->NOACTPTS ; 
       if( ! p1 ) { zmin = zmax = *lat ; p1 = 1 ; }
	   if( *lat < zmin ) zmin = *lat ;
	   if( *lat > zmax ) zmax = *lat ;
	  }
   }
 latticeP->LZMIN = zmin  ;
 latticeP->LZMAX = zmax  ;
 latticeP->LZDIF = latticeP->LZMAX - latticeP->LZMIN ;
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( subFlagP != NULL ) { free(subFlagP) ; subFlagP = NULL ; }
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
BENTLEYDTM_EXPORT int bcdtmLattice_createReferenceAndIsopachLatticesFromDtmObjects
(
 BC_DTM_OBJ *fromDtmP,
 BC_DTM_OBJ *toDtmP,
 DTM_LAT_OBJ **refLatticePP,
 DTM_LAT_OBJ **isoLatticePP,
 long latticeOption,
 long numLatticePts,
 double xinc,
 double xreg,
 double xmin,
 double xmax,
 double yinc,
 double yreg,
 double ymin,
 double ymax
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   nxl,nyl,intersectResult ;
 double bxMin,bxMax,byMin,byMax ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Creating Isopach Lattice From Dtm Objects") ;
    bcdtmWrite_message(0,0,0,"fromDtmP        = %p",fromDtmP) ;
    bcdtmWrite_message(0,0,0,"toDtmP          = %p",toDtmP) ;
    bcdtmWrite_message(0,0,0,"refLatticePP    = %p",*refLatticePP) ;
    bcdtmWrite_message(0,0,0,"isoLatticePP    = %p",*isoLatticePP) ;
    bcdtmWrite_message(0,0,0,"latticeOption   = %8ld",latticeOption) ;
    bcdtmWrite_message(0,0,0,"numLatticePts   = %8ld",numLatticePts) ;
    bcdtmWrite_message(0,0,0,"xreg            = %8.2lf",xreg) ;
    bcdtmWrite_message(0,0,0,"yreg            = %8.2lf",yreg) ;
    bcdtmWrite_message(0,0,0,"xinc            = %8.2lf",xinc) ;
    bcdtmWrite_message(0,0,0,"yinc            = %8.2lf",yinc) ;
    bcdtmWrite_message(0,0,0,"xmin            = %8.2lf",xmin) ;
    bcdtmWrite_message(0,0,0,"xmax            = %8.2lf",xmax) ;
    bcdtmWrite_message(0,0,0,"ymin            = %8.2lf",ymin) ;
    bcdtmWrite_message(0,0,0,"ymax            = %8.2lf",ymax) ;
   }
/*
** Validate
*/
 if( *refLatticePP != NULL ) bcdtmObject_deleteLatticeObject(refLatticePP) ;
 if( *isoLatticePP  != NULL ) bcdtmObject_deleteLatticeObject(isoLatticePP) ;
/*
** Check For Valid Objects
*/
 if( bcdtmObject_testForValidDtmObject(fromDtmP)) goto errexit ;
 if( bcdtmObject_testForValidDtmObject(toDtmP)) goto errexit ;
/*
** Check Dtm Objects Are In Dtm Tin State
*/
 if( fromDtmP->dtmState != DTMState::Tin || toDtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM's") ;
    goto errexit ;
   }
/*
** Validate Lattice Arguements
*/
 if( latticeOption < 1 || latticeOption > 4 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Lattice Option") ;
    goto errexit ;
   } 
 if( ( latticeOption == 1 || latticeOption == 2 ) && numLatticePts <= 10 )
   {
    bcdtmWrite_message(1,0,0,"Number Of Lattice Points Must Be Grater Than 10") ;
    goto errexit ;
   }
 if( (latticeOption == 2 || latticeOption == 4 ) && ( xmax <= xmin || ymax <= ymin ))
   {
    bcdtmWrite_message(1,0,0,"Lattice Window Coordinates Incorrect") ;
    goto errexit ;
   }
 if( (latticeOption == 3 || latticeOption == 4 ) && ( xinc <= 0.0 || yinc <= 0.0 ))
   {
    bcdtmWrite_message(1,0,0,"Lattice Spacing Incorrect") ;
    goto errexit ;
   }
/*
** Set Minimum Bounding Rectangle To Covering Both Hulls
*/
 bxMin = fromDtmP->xMin ; bxMax = fromDtmP->xMax ;
 byMin = fromDtmP->yMin ; byMax = fromDtmP->yMax ;
 if( bxMin < toDtmP->xMin ) bxMin = toDtmP->xMin ;
 if( bxMax > toDtmP->xMax ) bxMax = toDtmP->xMax ;
 if( byMin < toDtmP->yMin ) byMin = toDtmP->yMin ;
 if( byMax > toDtmP->yMax ) byMax = toDtmP->yMax ;
 if( xmin < bxMin || xmin > bxMax ) xmin = bxMin ;
 if( xmax < bxMin || xmax > bxMax ) xmax = bxMax ;
 if( ymin < byMin || ymin > byMax ) ymin = byMin ;
 if( ymax < byMin || ymax > byMax ) ymax = byMax ;
 if( latticeOption == 1 ) latticeOption = 2 ;
 if( latticeOption == 3 ) latticeOption = 4 ;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Minimum Bounding Rectange Covering Both Tin Hulls") ;
    bcdtmWrite_message(0,0,0,"xmin = %12.4lf ymin = %12.4lf xmax = %12.4lf ymax = %12.4lf",xmin,ymin,xmax,ymax) ;
   }
/*
** Check Tin Hulls Overlap
*/
  if( bcdtmClip_checkTinHullsIntersectDtmObject(fromDtmP,toDtmP,&intersectResult)) goto errexit ;
  if( dbg ) bcdtmWrite_message(0,0,0,"intersectResult = %2ld",intersectResult) ; 
  if( ! intersectResult )
    {
     bcdtmWrite_message(1,0,0,"Tin Hulls Do Not Overlap") ;
     goto errexit ;
    } 
/*
** Create A Lattice Object
*/
 if( bcdtmObject_createLatticeObject(refLatticePP)) goto errexit ;
 if( bcdtmObject_createLatticeObject(isoLatticePP))  goto errexit ;
/*
** Build Lattice
*/
 switch(latticeOption)
   {
    case   1 :
      if( fromDtmP->xMin <= toDtmP->xMin ) (*isoLatticePP)->LXMIN = toDtmP->xMin ;
      else                                 (*isoLatticePP)->LXMIN = fromDtmP->xMin ;
      if( fromDtmP->xMax <= toDtmP->xMax ) (*isoLatticePP)->LXMAX = fromDtmP->xMax ;
      else                                 (*isoLatticePP)->LXMAX = toDtmP->xMax ;
      if( fromDtmP->yMin <= toDtmP->yMin ) (*isoLatticePP)->LYMIN = toDtmP->yMin ;
      else                                 (*isoLatticePP)->LYMIN = fromDtmP->yMin ;
      if( fromDtmP->yMax <= toDtmP->yMax ) (*isoLatticePP)->LYMAX = fromDtmP->yMax ;
      else                                 (*isoLatticePP)->LYMAX = toDtmP->yMax ;
      (*isoLatticePP)->LXDIF = (*isoLatticePP)->LXMAX - (*isoLatticePP)->LXMIN ;
      (*isoLatticePP)->LYDIF = (*isoLatticePP)->LYMAX - (*isoLatticePP)->LYMIN ;
      bcdtmLattice_calculateNumberOfLatticeLines((*isoLatticePP)->LXDIF,(*isoLatticePP)->LYDIF,&nxl,&nyl,numLatticePts) ;
	  (*isoLatticePP)->NXL = nxl ;
	  (*isoLatticePP)->NYL = nyl ;
    break  ;

    case   2 :
      (*isoLatticePP)->LXMIN = xmin  ; (*isoLatticePP)->LXMAX = xmax  ; (*isoLatticePP)->LXDIF = xmax - xmin ;
      (*isoLatticePP)->LYMIN = ymin  ; (*isoLatticePP)->LYMAX = ymax  ; (*isoLatticePP)->LYDIF = ymax - ymin ;
      bcdtmLattice_calculateNumberOfLatticeLines((*isoLatticePP)->LXDIF,(*isoLatticePP)->LYDIF,&nxl,&nyl,numLatticePts) ;
	  (*isoLatticePP)->NXL = nxl ;
	  (*isoLatticePP)->NYL = nyl ;
    break  ;

    case   3 :
      if( fromDtmP->xMin <= toDtmP->xMin ) (*isoLatticePP)->LXMIN = toDtmP->xMin ;
      else                                 (*isoLatticePP)->LXMIN = fromDtmP->xMin ;
      if( fromDtmP->xMax <= toDtmP->xMax ) (*isoLatticePP)->LXMAX = fromDtmP->xMax ;
      else                                 (*isoLatticePP)->LXMAX = toDtmP->xMax ;
      if( fromDtmP->yMin <= toDtmP->yMin ) (*isoLatticePP)->LYMIN = toDtmP->yMin ;
      else                                 (*isoLatticePP)->LYMIN = fromDtmP->yMin ;
      if( fromDtmP->yMax <= toDtmP->yMax ) (*isoLatticePP)->LYMAX = fromDtmP->yMax ;
      else                                 (*isoLatticePP)->LYMAX = toDtmP->yMax ;

      (*isoLatticePP)->LXMIN = bcdtmUtl_adjustValueDown((*isoLatticePP)->LXMIN,xreg,xinc) ;
      (*isoLatticePP)->LXMAX = bcdtmUtl_adjustValueUp((*isoLatticePP)->LXMAX,xreg,xinc) ;
      (*isoLatticePP)->LXDIF = (*isoLatticePP)->LXMAX - (*isoLatticePP)->LXMIN ;
      nyl   = (long) ( (*isoLatticePP)->LXDIF / xinc ) + 1  ;
      (*isoLatticePP)->LYMIN = bcdtmUtl_adjustValueDown((*isoLatticePP)->LYMIN,yreg,yinc) ;
      (*isoLatticePP)->LYMAX = bcdtmUtl_adjustValueUp ((*isoLatticePP)->LYMAX,yreg,yinc) ;
      (*isoLatticePP)->LYDIF = (*isoLatticePP)->LYMAX - (*isoLatticePP)->LYMIN ;
      nxl   = (long) ( (*isoLatticePP)->LYDIF / yinc ) + 1  ;
	  (*isoLatticePP)->NXL = nxl ;
	  (*isoLatticePP)->NYL = nyl ;
    break  ;

    case   4 :
      (*isoLatticePP)->LXMIN = bcdtmUtl_adjustValueDown(xmin,xreg,xinc) ;
      (*isoLatticePP)->LXMAX = bcdtmUtl_adjustValueUp (xmax,xreg,xinc) ;
      (*isoLatticePP)->LXDIF = (*isoLatticePP)->LXMAX - (*isoLatticePP)->LXMIN ;
      nyl   = (long) ( (*isoLatticePP)->LXDIF / xinc ) + 1 ;
      (*isoLatticePP)->LYMIN = bcdtmUtl_adjustValueDown(ymin,yreg,yinc) ;
      (*isoLatticePP)->LYMAX = bcdtmUtl_adjustValueUp (ymax,yreg,yinc) ;
      (*isoLatticePP)->LYDIF = (*isoLatticePP)->LYMAX - (*isoLatticePP)->LYMIN ;
      nxl   = (long) ( (*isoLatticePP)->LYDIF / yinc ) + 1  ;
	  (*isoLatticePP)->NXL = nxl ;
	  (*isoLatticePP)->NYL = nyl ;
    break  ;
   } ;
/*
** Populate Isopach Lattice
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Isopach Lattice") ;
 if( bcdtmLattice_populateReferenceAndIsopachLatticesFromDtmObjects(fromDtmP,toDtmP,*refLatticePP,*isoLatticePP)) goto errexit ;
/*
**  Write Isopach Lattice Stats
*/
 if( dbg ) bcdtmUtl_writeStatisticsLatticeObject(*isoLatticePP) ;
/*
** Cleanup
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLattice_populateReferenceAndIsopachLatticesFromDtmObjects
(
 BC_DTM_OBJ  *fromDtmP,           /* ==> Pointer To From_DTM Object     */
 BC_DTM_OBJ  *toDtmP,             /* ==> Pointer To To_DTM Object       */
 DTM_LAT_OBJ *refLatticeP,        /* ==> Pointer To Reference Lattice   */
 DTM_LAT_OBJ *isoLatticeP         /* ==> Pointer To Isopach Lattice     */
)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    i,j,p1,p2,p3,xs,xf,ys,yf,iofs,clPtr,numLatPts,newTriangle,voidTriangle ;
 double  xl,yl,zl,trgXmin,trgXmax,trgYmin,trgYmax,x[3],y[3],z[3] ;
 float   *lat,zmin,zmax ;
 unsigned char    *subFlagP=NULL,*cp ;
 DTM_TIN_POINT *p1P,*p2P,*p3P ;  
 DTM_TIN_NODE  *nodeP ; 
 DTM_CIR_LIST  *clistP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Populating Reference And Isopach Lattices") ;
    bcdtmWrite_message(0,0,0,"fromDtmP     = %p",fromDtmP) ;
    bcdtmWrite_message(0,0,0,"toDtmP       = %p",toDtmP) ;
    bcdtmWrite_message(0,0,0,"refLatticeP  = %p",refLatticeP) ;
    bcdtmWrite_message(0,0,0,"isoLatticeP  = %p",isoLatticeP) ;
   }
/*
** Allocate Storage for lattice
*/
 if( bcdtmObject_allocateLatticeMemoryLatticeObject(isoLatticeP,isoLatticeP->NXL,isoLatticeP->NYL,-989898.0)) goto errexit ;
/*
** Initialise Lattice values
*/
 isoLatticeP->DX = (isoLatticeP->LXMAX-isoLatticeP->LXMIN) / (double) (isoLatticeP->NYL - 1) ;
 isoLatticeP->DY = (isoLatticeP->LYMAX-isoLatticeP->LYMIN) / (double) (isoLatticeP->NXL - 1) ;
/*
** Write Lattice Parameters 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"xlmin = %12.5lf xlmax = %12.5lf ** ylmin = %12.5lf ylmax = %12.5lf",isoLatticeP->LXMIN,isoLatticeP->LXMAX,isoLatticeP->LYMIN,isoLatticeP->LYMAX) ;
/*
** Allocate Memory To Flag Subtraction
*/
 subFlagP = ( unsigned char * ) malloc( (isoLatticeP->NOLATPTS/8+1)*sizeof(char)) ;
 if( subFlagP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
 for( cp = subFlagP ; cp < subFlagP + isoLatticeP->NOLATPTS/8+1 ; ++cp ) *cp = 0 ;
/*
**  Build Lattice For From Dtm Element
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Lattice For fromDtm %p",fromDtmP) ;
 numLatPts = 0 ;
 for( p1 = 0 ; p1 < fromDtmP->numPoints ; ++p1 )
   {
    nodeP = nodeAddrP(fromDtmP,p1) ;
    if( ( clPtr = nodeP->cPtr ) != fromDtmP->nullPtr )
      {
       p1P = pointAddrP(fromDtmP,p1) ;
       x[0] = p1P->x ;
       y[0] = p1P->y ;
       z[0] = p1P->z ;
       if( ( p2 = bcdtmList_nextAntDtmObject(fromDtmP,p1,clistAddrP(fromDtmP,clPtr)->pntNum )) < 0 ) goto errexit ;
       while( clPtr != fromDtmP->nullPtr )
	     {
          clistP = clistAddrP(fromDtmP,clPtr) ;
          p3     = clistP->pntNum ;
          clPtr  = clistP->nextPtr ;
	      if( p3 > p1 && p2 > p1 && nodeP->hPtr != p2 )
	        {
             if( bcdtmList_testForVoidTriangleDtmObject(fromDtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
			 if( ! voidTriangle )
		       {
                p2P = pointAddrP(fromDtmP,p2) ;
                p3P = pointAddrP(fromDtmP,p3) ;
                x[1] = p2P->x ;
                y[1] = p2P->y ;
                z[1] = p2P->z ;
                x[2] = p3P->x ;
                y[2] = p3P->y ;
                z[2] = p3P->z ;
		        trgXmin = bcdtmUtl_getTrgMin(x) ;
                trgXmax = bcdtmUtl_getTrgMax(x) ;
		        trgYmin = bcdtmUtl_getTrgMin(y) ;
                trgYmax = bcdtmUtl_getTrgMax(y) ;
		        if( trgXmin <= isoLatticeP->LXMAX && trgXmax >= isoLatticeP->LXMIN && trgYmin <= isoLatticeP->LYMAX && trgYmax >= isoLatticeP->LYMIN )
		          {
		           newTriangle = 1 ;
		           xs = (long)((trgXmin-isoLatticeP->LXMIN) / isoLatticeP->DX) ;
		           xf = (long)((trgXmax-isoLatticeP->LXMIN) / isoLatticeP->DX) ;
		           ys = (long)((trgYmin-isoLatticeP->LYMIN) / isoLatticeP->DY) ;
		           yf = (long)((trgYmax-isoLatticeP->LYMIN) / isoLatticeP->DY) ;
		           if( xs < 0 ) xs = 0 ; if( xf >= isoLatticeP->NYL ) xf = isoLatticeP->NYL - 1 ;
		           if( ys < 0 ) ys = 0 ; if( yf >= isoLatticeP->NXL ) yf = isoLatticeP->NXL - 1 ;
		           for( i = xs ; i <= xf ; ++i)
		             {
		              for( j = ys ; j <= yf ; ++j )
		                {
		                 xl = (double) i * isoLatticeP->DX + isoLatticeP->LXMIN ;
		                 yl = (double) j * isoLatticeP->DY + isoLatticeP->LYMIN ;
		                 iofs = isoLatticeP->NYL * j + i ;
		                 if( *(isoLatticeP->LAT+iofs) == isoLatticeP->NULLVAL )
		                   {
		                    if(bcdtmMath_sideOf(x[0],y[0],x[1],y[1],xl,yl) <= 0 )
		  	                if(bcdtmMath_sideOf(x[1],y[1],x[2],y[2],xl,yl) <= 0 )
				            if(bcdtmMath_sideOf(x[2],y[2],x[0],y[0],xl,yl) <= 0 )
				              {
                               bcdtmMath_interpolatePointOnTrianglePlane(newTriangle,xl,yl,&zl,x,y,z) ;
				               *(isoLatticeP->LAT+iofs) = (float) zl   ;
				               newTriangle = 0 ;
                               ++numLatPts ;
				              }
			               }
			            }
		             }
		          }
	           }
	        }
	      p2 = p3 ; 
	     } 
      }
   }
/*
** Set Lattice Ranges
*/
 
/*
** Write Number Of Lattice Points Set
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Lattice Points Set = %9ld",numLatPts) ;
/*
** Copy To Reference Lattice
*/
 if( bcdtmObject_copyLatticeObjectToLatticeObject(isoLatticeP,refLatticeP)) goto errexit ;
/*
**  Build Lattice For To Dtm And Subtract  
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Lattice For toDtm %p",toDtmP) ;
 numLatPts = 0 ;
 for( p1 = 0 ; p1 < toDtmP->numPoints ; ++p1 )
   {
    nodeP = nodeAddrP(toDtmP,p1) ;
    if( ( clPtr = nodeP->cPtr ) != toDtmP->nullPtr )
      {
       p1P = pointAddrP(toDtmP,p1) ;
       x[0] = p1P->x ;
       y[0] = p1P->y ;
       z[0] = p1P->z ;
       if( ( p2 = bcdtmList_nextAntDtmObject(toDtmP,p1,clistAddrP(toDtmP,clPtr)->pntNum )) < 0 ) goto errexit ;
       while( clPtr != toDtmP->nullPtr )
	     {
          clistP = clistAddrP(toDtmP,clPtr) ;
          p3     = clistP->pntNum ;
          clPtr  = clistP->nextPtr ;
	      if( p3 > p1 && p2 > p1 && nodeP->hPtr != p2 )
	        {
             if( bcdtmList_testForVoidTriangleDtmObject(toDtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
			 if( ! voidTriangle )
		       {
                p2P = pointAddrP(toDtmP,p2) ;
                p3P = pointAddrP(toDtmP,p3) ;
                x[1] = p2P->x ;
                y[1] = p2P->y ;
                z[1] = p2P->z ;
                x[2] = p3P->x ;
                y[2] = p3P->y ;
                z[2] = p3P->z ;
		        trgXmin = bcdtmUtl_getTrgMin(x) ;
                trgXmax = bcdtmUtl_getTrgMax(x) ;
		        trgYmin = bcdtmUtl_getTrgMin(y) ;
                trgYmax = bcdtmUtl_getTrgMax(y) ;
		        if( trgXmin <= isoLatticeP->LXMAX && trgXmax >= isoLatticeP->LXMIN && trgYmin <= isoLatticeP->LYMAX && trgYmax >= isoLatticeP->LYMIN )
		          {
		           newTriangle = 1 ;
		           xs = (long)((trgXmin-isoLatticeP->LXMIN) / isoLatticeP->DX) ;
		           xf = (long)((trgXmax-isoLatticeP->LXMIN) / isoLatticeP->DX) ;
		           ys = (long)((trgYmin-isoLatticeP->LYMIN) / isoLatticeP->DY) ;
		           yf = (long)((trgYmax-isoLatticeP->LYMIN) / isoLatticeP->DY) ;
		           if( xs < 0 ) xs = 0 ; if( xf >= isoLatticeP->NYL ) xf = isoLatticeP->NYL - 1 ;
		           if( ys < 0 ) ys = 0 ; if( yf >= isoLatticeP->NXL ) yf = isoLatticeP->NXL - 1 ;
		           for( i = xs ; i <= xf ; ++i)
		             {
		              for( j = ys ; j <= yf ; ++j )
		                {
		                 xl = (double) i * isoLatticeP->DX + isoLatticeP->LXMIN ;
		                 yl = (double) j * isoLatticeP->DY + isoLatticeP->LYMIN ;
		                 iofs = isoLatticeP->NYL * j + i ;
                         if( *(isoLatticeP->LAT+iofs) != isoLatticeP->NULLVAL )
		                   {
                            if( ! bcdtmFlag_testFlag(subFlagP,iofs))
                              {
		                       if(bcdtmMath_sideOf(x[0],y[0],x[1],y[1],xl,yl) <= 0 )
		  	                   if(bcdtmMath_sideOf(x[1],y[1],x[2],y[2],xl,yl) <= 0 )
				               if(bcdtmMath_sideOf(x[2],y[2],x[0],y[0],xl,yl) <= 0 )
				                 {
                                  bcdtmMath_interpolatePointOnTrianglePlane(newTriangle,xl,yl,&zl,x,y,z) ;
				                  *(isoLatticeP->LAT+iofs) -= (float) zl   ;
                                  bcdtmFlag_setFlag(subFlagP,iofs) ;
				                  newTriangle = 0 ;
                                  ++numLatPts ;
                                 }
				              }
			               }
			            }
		             }
		          }
	           }
	        }
	      p2 = p3 ; 
	     } 
      }
   }
/*
** Write Number Of Lattice Points Set
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Lattice Points Set = %9ld",numLatPts) ;
/*
** Null Out Lattice Points Where Overlap Did Not Occur
*/
 for( iofs=0 ; iofs < isoLatticeP->NOLATPTS ; ++iofs )
   {
    if( ! bcdtmFlag_testFlag(subFlagP,iofs) ) *(isoLatticeP->LAT+iofs) = isoLatticeP->NULLVAL ;
   } 
/*
** Set Lattice Min & Max Values 
*/
 p1 = 0 ;   
 zmin  =  zmax  = 0.0 ;
 isoLatticeP->NOACTPTS = 0 ;
 for( lat = isoLatticeP->LAT ; lat < isoLatticeP->LAT + isoLatticeP->NOLATPTS ; ++lat )
   {
    if( *lat != isoLatticeP->NULLVAL ) 
	  { 
	   ++isoLatticeP->NOACTPTS ; 
       if( ! p1 ) { zmin = zmax = *lat ; p1 = 1 ; }
	   if( *lat < zmin ) zmin = *lat ;
	   if( *lat > zmax ) zmax = *lat ;
	  }
   }
 isoLatticeP->LZMIN = zmin  ;
 isoLatticeP->LZMAX = zmax  ;
 isoLatticeP->LZDIF = isoLatticeP->LZMAX - isoLatticeP->LZMIN ;
/*
** Write Bounding Cube
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"isoLatticeP->LXMIN = %10.4lf isoLatticeP->LXMAX = %10.4lf",isoLatticeP->LXMIN,isoLatticeP->LXMAX) ;
    bcdtmWrite_message(0,0,0,"isoLatticeP->LYMIN = %10.4lf isoLatticeP->LYMAX = %10.4lf",isoLatticeP->LYMIN,isoLatticeP->LYMAX) ;
    bcdtmWrite_message(0,0,0,"isoLatticeP->LZMIN = %10.4lf isoLatticeP->LZMAX = %10.4lf",isoLatticeP->LZMIN,isoLatticeP->LZMAX) ;
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( subFlagP != NULL ) { free(subFlagP) ; subFlagP = NULL ; }
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
BENTLEYDTM_EXPORT int bcdtmLattice_createLatticeThemeFromDtmFiles
(
 WCharCP fromDtmFileP,
 WCharCP toDtmFileP,
 WCharCP latticeFileP,
 long   themeOption,
 long   latticeOption,
 long   numLatticePts,
 double xinc,
 double xreg,
 double xmin,
 double xmax,
 double yinc,
 double yreg,
 double ymin,
 double ymax
) 
{
 int         ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ  *fromDtmP=NULL,*toDtmP=NULL ;
 DTM_LAT_OBJ *latticeP=NULL ;
/*
** Write Status Message
*/ 
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Theme Lattice From Dtm Files") ;
/*
** Check For Different Files
*/
 if( wcscmp(fromDtmFileP,toDtmFileP) == 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Dtm Files The Same") ;
    goto errexit ;
   } 
/*
** Read From Dtm File As Current DTM Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading From Dtm File %s",fromDtmFileP) ;
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&fromDtmP,fromDtmFileP)) goto errexit ; 
/*
** Read To Dtm File Into Memory
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading To   Dtm File %s",toDtmFileP) ;
 if( bcdtmRead_fromFileDtmObject(&toDtmP,toDtmFileP))  goto errexit ; 
/*
** Delete Current Lattice Object If It Exists
*/
/*
 if( DTM_CLOBJ != NULL ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Destroying Current DTM Lattice Object %p",DTM_CLOBJ ) ;
    if( bcdtmObject_deleteLatticeObject(&DTM_CLOBJ)) goto errexit ;
    wcscpy(DTM_CLOBJ_FILE,L"") ;
   } 
*/   
/*
**  Create Lattice Theme
*/
 if( bcdtmLattice_createLatticeThemeFromDtmObjects(fromDtmP,toDtmP,&latticeP,themeOption,latticeOption,numLatticePts,xinc,xreg,xmin,xmax,yinc,yreg,ymin,ymax) ) goto errexit ;
/*
** Delete Dtm Objects
*/
 if( fromDtmP != NULL ) bcdtmObject_destroyDtmObject(&fromDtmP) ; 
 if( toDtmP   != NULL ) bcdtmObject_destroyDtmObject(&toDtmP) ; 
/*
** Set Currency
*/
// DTM_CLOBJ = latticeP ;
// if( *latticeFileP != 0 ) wcscpy(DTM_CLOBJ_FILE,latticeFileP) ;
/*
** Write Lattice File
*/
 if( *latticeFileP != 0 ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Lattice File %s",latticeFileP) ;
    if( bcdtmWrite_toFileLatticeObject(latticeP,latticeFileP)) goto errexit ;
   }
/*
** Cleanup
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( fromDtmP != NULL ) bcdtmObject_destroyDtmObject(&fromDtmP) ; 
 if( toDtmP   != NULL ) bcdtmObject_destroyDtmObject(&toDtmP) ; 
 if( latticeP != NULL ) bcdtmObject_deleteLatticeObject(&latticeP) ;
 /*
 if( DTM_CDTM != NULL )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Destroying Current DTM Object %p",DTM_CDTM ) ;
    if( ! bcdtmObject_testForValidDtmObject(DTM_CDTM) )
      {
       bcdtmObject_destroyDtmObject(&DTM_CDTM) ;  
       wcscpy(DTM_CDTM_FILE,L"") ;
      }  
   }
 if( DTM_CLOBJ != NULL ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Destroying Current DTM Lattice Object %p",DTM_CLOBJ ) ;
    if( bcdtmObject_deleteLatticeObject(&DTM_CLOBJ)) goto errexit ;
    wcscpy(DTM_CLOBJ_FILE,L"") ;
   } 
*/   
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLattice_createLatticeThemeFromDtmObjects
(
 BC_DTM_OBJ  *fromDtmP,
 BC_DTM_OBJ  *toDtmP,
 DTM_LAT_OBJ **latticePP,
 long   themeOption,
 long   latticeOption,
 long   numLatticePts,
 double xinc,
 double xreg,
 double xmin,
 double xmax,
 double yinc,
 double yreg,
 double ymin,
 double ymax
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0)  ;
 long   nxl,nyl,intersectResult  ;
 double bxMin,bxMax,byMin,byMax ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Creating Lattice Theme From Dtm Objects") ;
    bcdtmWrite_message(0,0,0,"fromDtmP        = %p",fromDtmP) ;
    bcdtmWrite_message(0,0,0,"toDtmP          = %p",toDtmP) ;
    bcdtmWrite_message(0,0,0,"*latticePP      = %p",*latticePP) ;
    bcdtmWrite_message(0,0,0,"themeOption     = %8ld",themeOption) ;
    bcdtmWrite_message(0,0,0,"latticeOption   = %8ld",latticeOption) ;
    bcdtmWrite_message(0,0,0,"numLatticePts   = %8ld",numLatticePts) ;
    bcdtmWrite_message(0,0,0,"xreg            = %8.2lf",xreg) ;
    bcdtmWrite_message(0,0,0,"yreg            = %8.2lf",yreg) ;
    bcdtmWrite_message(0,0,0,"xinc            = %8.2lf",xinc) ;
    bcdtmWrite_message(0,0,0,"yinc            = %8.2lf",yinc) ;
    bcdtmWrite_message(0,0,0,"xmin            = %8.2lf",xmin) ;
    bcdtmWrite_message(0,0,0,"xmax            = %8.2lf",xmax) ;
    bcdtmWrite_message(0,0,0,"ymin            = %8.2lf",ymin) ;
    bcdtmWrite_message(0,0,0,"ymax            = %8.2lf",ymax) ;
   }
/*
** Check For Valid Objects
*/
 if( bcdtmObject_testForValidDtmObject(fromDtmP)) goto errexit ;
 if( bcdtmObject_testForValidDtmObject(toDtmP)) goto errexit ;
 if( *latticePP != NULL ) bcdtmObject_deleteLatticeObject(latticePP) ;
/*
** Check Dtm Objects Are In Dtm Tin State
*/
 if( fromDtmP->dtmState != DTMState::Tin || toDtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"DTM Object(s) Not Triangulated") ;
    goto errexit ;
   }
/*
** Validate Lattice Arguements
*/
 if( themeOption < 1 || themeOption > 3 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Theme Option") ;
    goto errexit ;
   } 
 if( latticeOption < 1 || latticeOption > 4 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Lattice Option") ;
    goto errexit ;
   } 
 if( ( latticeOption == 1 || latticeOption == 2 ) && numLatticePts <= 10 )
   {
    bcdtmWrite_message(1,0,0,"Number Of Lattice Points Must Be Grater Than 10") ;
    goto errexit ;
   }
 if( (latticeOption == 2 || latticeOption == 4 ) && ( xmax <= xmin || ymax <= ymin ))
   {
    bcdtmWrite_message(1,0,0,"Lattice Window Coordinates Incorrect") ;
    goto errexit ;
   }
 if( (latticeOption == 3 || latticeOption == 4 ) && ( xinc <= 0.0 || yinc <= 0.0 ))
   {
    bcdtmWrite_message(1,0,0,"Lattice Spacing Incorrect") ;
    goto errexit ;
   }
/*
** Set Minimum Bounding Rectangle Covering Both Hulls
*/
 bxMin = fromDtmP->xMin ; bxMax = fromDtmP->xMax ;
 byMin = fromDtmP->yMin ; byMax = fromDtmP->yMax ;
 if( bxMin < toDtmP->xMin ) bxMin = toDtmP->xMin ;
 if( bxMax > toDtmP->xMax ) bxMax = toDtmP->xMax ;
 if( byMin < toDtmP->yMin ) byMin = toDtmP->yMin ;
 if( byMax > toDtmP->yMax ) byMax = toDtmP->yMax ;
 if( xmin < bxMin || xmin > bxMax ) xmin = bxMin ;
 if( xmax < bxMin || xmax > bxMax ) xmax = bxMax ;
 if( ymin < byMin || ymin > byMax ) ymin = byMin ;
 if( ymax < byMin || ymax > byMax ) ymax = byMax ;
 if( latticeOption == 1 ) latticeOption = 2 ;
 if( latticeOption == 3 ) latticeOption = 4 ;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Minimum Bounding Rectange Covering Both Tin Hulls") ;
    bcdtmWrite_message(0,0,0,"xmin = %12.4lf ymin = %12.4lf xmax = %12.4lf ymax = %12.4lf",xmin,ymin,xmax,ymax) ;
   }
/*
** Check Tin Hulls Overlap
*/
  if( bcdtmClip_checkTinHullsIntersectDtmObject(fromDtmP,toDtmP,&intersectResult)) goto errexit ;
  if( dbg ) bcdtmWrite_message(0,0,0,"intersectResult = %2ld",intersectResult) ; 
  if( ! intersectResult )
    {
     bcdtmWrite_message(1,0,0,"Tin Hulls Do Not Overlap") ;
     goto errexit ;
    } 
/*
** Create A Lattice Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Lattice Object") ;
 if( bcdtmObject_createLatticeObject(latticePP))  goto errexit ;
/*
** Build Lattice
*/
 switch(latticeOption)
   {
    case   1 :
      if( fromDtmP->xMin <= toDtmP->xMin ) (*latticePP)->LXMIN = toDtmP->xMin ;
      else                             (*latticePP)->LXMIN = fromDtmP->xMin ;
      if( fromDtmP->xMax <= toDtmP->xMax ) (*latticePP)->LXMAX = fromDtmP->xMax ;
      else                             (*latticePP)->LXMAX = toDtmP->xMax ;
      if( fromDtmP->yMin <= toDtmP->yMin ) (*latticePP)->LYMIN = toDtmP->yMin ;
      else                             (*latticePP)->LYMIN = fromDtmP->yMin ;
      if( fromDtmP->yMax <= toDtmP->yMax ) (*latticePP)->LYMAX = fromDtmP->yMax ;
      else                             (*latticePP)->LYMAX = toDtmP->yMax ;
      (*latticePP)->LXDIF = (*latticePP)->LXMAX - (*latticePP)->LXMIN ;
      (*latticePP)->LYDIF = (*latticePP)->LYMAX - (*latticePP)->LYMIN ;
      bcdtmLattice_calculateNumberOfLatticeLines((*latticePP)->LXDIF,(*latticePP)->LYDIF,&nxl,&nyl,numLatticePts) ;
	  (*latticePP)->NXL = nxl ;
	  (*latticePP)->NYL = nyl ;
    break  ;

    case   2 :
      (*latticePP)->LXMIN = xmin  ; (*latticePP)->LXMAX = xmax  ; (*latticePP)->LXDIF = xmax - xmin ;
      (*latticePP)->LYMIN = ymin  ; (*latticePP)->LYMAX = ymax  ; (*latticePP)->LYDIF = ymax - ymin ;
      bcdtmLattice_calculateNumberOfLatticeLines((*latticePP)->LXDIF,(*latticePP)->LYDIF,&nxl,&nyl,numLatticePts) ;
	  (*latticePP)->NXL = nxl ;
	  (*latticePP)->NYL = nyl ;
    break  ;

    case   3 :
      if( fromDtmP->xMin <= toDtmP->xMin ) (*latticePP)->LXMIN = toDtmP->xMin ;
      else                             (*latticePP)->LXMIN = fromDtmP->xMin ;
      if( fromDtmP->xMax <= toDtmP->xMax ) (*latticePP)->LXMAX = fromDtmP->xMax ;
      else                             (*latticePP)->LXMAX = toDtmP->xMax ;
      if( fromDtmP->yMin <= toDtmP->yMin ) (*latticePP)->LYMIN = toDtmP->yMin ;
      else                             (*latticePP)->LYMIN = fromDtmP->yMin ;
      if( fromDtmP->yMax <= toDtmP->yMax ) (*latticePP)->LYMAX = fromDtmP->yMax ;
      else                             (*latticePP)->LYMAX = toDtmP->yMax ;

      (*latticePP)->LXMIN = bcdtmUtl_adjustValueDown((*latticePP)->LXMIN,xreg,xinc) ;
      (*latticePP)->LXMAX = bcdtmUtl_adjustValueUp((*latticePP)->LXMAX,xreg,xinc) ;
      (*latticePP)->LXDIF = (*latticePP)->LXMAX - (*latticePP)->LXMIN ;
      nyl   = (long) ( (*latticePP)->LXDIF / xinc ) + 1  ;
      (*latticePP)->LYMIN = bcdtmUtl_adjustValueDown((*latticePP)->LYMIN,yreg,yinc) ;
      (*latticePP)->LYMAX = bcdtmUtl_adjustValueUp ((*latticePP)->LYMAX,yreg,yinc) ;
      (*latticePP)->LYDIF = (*latticePP)->LYMAX - (*latticePP)->LYMIN ;
      nxl   = (long) ( (*latticePP)->LYDIF / yinc ) + 1  ;
	  (*latticePP)->NXL = nxl ;
	  (*latticePP)->NYL = nyl ;
    break  ;

    case   4 :
      (*latticePP)->LXMIN = bcdtmUtl_adjustValueDown(xmin,xreg,xinc) ;
      (*latticePP)->LXMAX = bcdtmUtl_adjustValueUp (xmax,xreg,xinc) ;
      (*latticePP)->LXDIF = (*latticePP)->LXMAX - (*latticePP)->LXMIN ;
      nyl   = (long) ( (*latticePP)->LXDIF / xinc ) + 1 ;
      (*latticePP)->LYMIN = bcdtmUtl_adjustValueDown(ymin,yreg,yinc) ;
      (*latticePP)->LYMAX = bcdtmUtl_adjustValueUp (ymax,yreg,yinc) ;
      (*latticePP)->LYDIF = (*latticePP)->LYMAX - (*latticePP)->LYMIN ;
      nxl   = (long) ( (*latticePP)->LYDIF / yinc ) + 1  ;
	  (*latticePP)->NXL = nxl ;
	  (*latticePP)->NYL = nyl ;
    break  ;
   } ;
/*
** Populate Theme Lattice
*/
 if( bcdtmLattice_populateLatticeThemeFromDtmObjects(fromDtmP,toDtmP,*latticePP,themeOption) ) goto errexit ;
/*
** DevNote - Development Purposes Only
*/
 if( dbg ) bcdtmUtl_writeStatisticsLatticeObject(*latticePP) ;
/*
** Job Completed
*/
 cleanup :
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Lattice Theme From Dtm Objects Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Lattice Theme From Dtm Objects Error") ;
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
BENTLEYDTM_Private int bcdtmLattice_populateLatticeThemeFromDtmObjects
(
 BC_DTM_OBJ  *fromDtmP,           /* ==> Pointer To From_DTM Object   */
 BC_DTM_OBJ  *toDtmP,             /* ==> Pointer To To_DTM Object     */
 DTM_LAT_OBJ *latticeP,           /* ==> Pointer To Isopach Lattice   */
 long        themeOption          /* ==> Theme Option                 */
)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    i,j,p1,p2,p3,xs,xf,ys,yf,iofs,clPtr,newTriangle,voidTriangle ;
 double  xl,yl,zl,trgXmin,trgXmax,trgYmin,trgYmax,x[3],y[3],z[3] ;
 float   *lat,zmin,zmax ;
 unsigned char    *subFlagP=NULL,*cp ;
 DTM_TIN_POINT *p1P,*p2P,*p3P ;  
 DTM_TIN_NODE  *nodeP ; 
 DTM_CIR_LIST  *clistP ;
/*
** Allocate Storage for lattice
*/
 if( bcdtmObject_allocateLatticeMemoryLatticeObject(latticeP,latticeP->NXL,latticeP->NYL,-989898.0)) goto errexit ;
/*
** Initialise Lattice values
*/
 latticeP->DX = (latticeP->LXMAX-latticeP->LXMIN) / (double) (latticeP->NYL - 1) ;
 latticeP->DY = (latticeP->LYMAX-latticeP->LYMIN) / (double) (latticeP->NXL - 1) ;
/*
** Allocate Memory To Flag Subtraction
*/
 subFlagP = ( unsigned char * ) malloc( (latticeP->NOLATPTS/8+1)*sizeof(char)) ;
 if( subFlagP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
 for( cp = subFlagP ; cp < subFlagP + latticeP->NOLATPTS/8+1 ; ++cp ) *cp = 0 ;
/*
**  Build Lattice For From Dtm Element
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Lattice For From Dtm Element %p",fromDtmP) ;
 for( p1 = 0 ; p1 < fromDtmP->numPoints ; ++p1 )
   {
    nodeP = nodeAddrP(fromDtmP,p1) ;
    if( ( clPtr = nodeP->cPtr ) != fromDtmP->nullPtr )
      {
       p1P = pointAddrP(fromDtmP,p1) ;
       x[0] = p1P->x ;
       y[0] = p1P->y ;
       z[0] = p1P->z ;
       if( ( p2 = bcdtmList_nextAntDtmObject(fromDtmP,p1,clistAddrP(fromDtmP,clPtr)->pntNum )) < 0 ) goto errexit ;
       while( clPtr != fromDtmP->nullPtr )
	     {
          clistP = clistAddrP(fromDtmP,clPtr) ;
          p3     = clistP->pntNum ;
          clPtr  = clistP->nextPtr ;
	      if( p3 > p1 && p2 > p1 && nodeP->hPtr != p2 )
	        {
             if( bcdtmList_testForVoidTriangleDtmObject(fromDtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
			 if( ! voidTriangle )
		       {
                p2P = pointAddrP(fromDtmP,p2) ;
                p3P = pointAddrP(fromDtmP,p3) ;
                x[1] = p2P->x ;
                y[1] = p2P->y ;
                z[1] = p2P->z ;
                x[2] = p3P->x ;
                y[2] = p3P->y ;
                z[2] = p3P->z ;
		        trgXmin = bcdtmUtl_getTrgMin(x) ;
                trgXmax = bcdtmUtl_getTrgMax(x) ;
		        trgYmin = bcdtmUtl_getTrgMin(y) ;
                trgYmax = bcdtmUtl_getTrgMax(y) ;
		        if( trgXmin <= latticeP->LXMAX && trgXmax >= latticeP->LXMIN && trgYmin <= latticeP->LYMAX && trgYmax >= latticeP->LYMIN )
		          {
		           newTriangle = 1 ;
		           xs = (long)((trgXmin-latticeP->LXMIN) / latticeP->DX) ;
		           xf = (long)((trgXmax-latticeP->LXMIN) / latticeP->DX) ;
		           ys = (long)((trgYmin-latticeP->LYMIN) / latticeP->DY) ;
		           yf = (long)((trgYmax-latticeP->LYMIN) / latticeP->DY) ;
		           if( xs < 0 ) xs = 0 ; if( xf >= latticeP->NYL ) xf = latticeP->NYL - 1 ;
		           if( ys < 0 ) ys = 0 ; if( yf >= latticeP->NXL ) yf = latticeP->NXL - 1 ;
		           for( i = xs ; i <= xf ; ++i)
		             {
		              for( j = ys ; j <= yf ; ++j )
		                {
		                 xl = (double) i * latticeP->DX + latticeP->LXMIN ;
		                 yl = (double) j * latticeP->DY + latticeP->LYMIN ;
		                 iofs = latticeP->NYL * j + i ;
		                 if( *(latticeP->LAT+iofs) == latticeP->NULLVAL )
		                   {
		                    if(bcdtmMath_sideOf(x[0],y[0],x[1],y[1],xl,yl) <= 0 )
		  	                if(bcdtmMath_sideOf(x[1],y[1],x[2],y[2],xl,yl) <= 0 )
				            if(bcdtmMath_sideOf(x[2],y[2],x[0],y[0],xl,yl) <= 0 )
				              {
          		               bcdtmMath_interpolatePointOnTriangle(xl,yl,&zl,x,y,z) ;
				               *(latticeP->LAT+iofs) = (float) zl   ;
				               newTriangle = 0 ;
				              }
			               }
			            }
		             }
		          }
	           }
	        }
	      p2 = p3 ; 
	     } 
      }
   }
/*
**  Build Lattice For To Dtm And Subtract  
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Lattice For To Dtm %p",toDtmP) ;
 for( p1 = 0 ; p1 < toDtmP->numPoints ; ++p1 )
   {
    nodeP = nodeAddrP(toDtmP,p1) ;
    if( ( clPtr = nodeP->cPtr ) != toDtmP->nullPtr )
      {
       p1P = pointAddrP(toDtmP,p1) ;
       x[0] = p1P->x ;
       y[0] = p1P->y ;
       z[0] = p1P->z ;
       if( ( p2 = bcdtmList_nextAntDtmObject(toDtmP,p1,clistAddrP(toDtmP,clPtr)->pntNum )) < 0 ) goto errexit ;
       while( clPtr != toDtmP->nullPtr )
	     {
          clistP = clistAddrP(toDtmP,clPtr) ;
          p3     = clistP->pntNum ;
          clPtr  = clistP->nextPtr ;
	      if( p3 > p1 && p2 > p1 && nodeP->hPtr != p2 )
	        {
             if( bcdtmList_testForVoidTriangleDtmObject(toDtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
			 if( ! voidTriangle )
		       {
                p2P = pointAddrP(toDtmP,p2) ;
                p3P = pointAddrP(toDtmP,p3) ;
                x[1] = p2P->x ;
                y[1] = p2P->y ;
                z[1] = p2P->z ;
                x[2] = p3P->x ;
                y[2] = p3P->y ;
                z[2] = p3P->z ;
		        trgXmin = bcdtmUtl_getTrgMin(x) ;
                trgXmax = bcdtmUtl_getTrgMax(x) ;
		        trgYmin = bcdtmUtl_getTrgMin(y) ;
                trgYmax = bcdtmUtl_getTrgMax(y) ;
		        if( trgXmin <= latticeP->LXMAX && trgXmax >= latticeP->LXMIN && trgYmin <= latticeP->LYMAX && trgYmax >= latticeP->LYMIN )
		          {
		           newTriangle = 1 ;
		           xs = (long)((trgXmin-latticeP->LXMIN) / latticeP->DX) ;
		           xf = (long)((trgXmax-latticeP->LXMIN) / latticeP->DX) ;
		           ys = (long)((trgYmin-latticeP->LYMIN) / latticeP->DY) ;
		           yf = (long)((trgYmax-latticeP->LYMIN) / latticeP->DY) ;
		           if( xs < 0 ) xs = 0 ; if( xf >= latticeP->NYL ) xf = latticeP->NYL - 1 ;
		           if( ys < 0 ) ys = 0 ; if( yf >= latticeP->NXL ) yf = latticeP->NXL - 1 ;
		           for( i = xs ; i <= xf ; ++i)
		             {
		              for( j = ys ; j <= yf ; ++j )
		                {
		                 xl = (double) i * latticeP->DX + latticeP->LXMIN ;
		                 yl = (double) j * latticeP->DY + latticeP->LYMIN ;
		                 iofs = latticeP->NYL * j + i ;
		                 if( *(latticeP->LAT+iofs) != latticeP->NULLVAL )
		                   {
                            if( ! bcdtmFlag_testFlag(subFlagP,iofs))
                              {
		                       if(bcdtmMath_sideOf(x[0],y[0],x[1],y[1],xl,yl) <= 0 )
		  	                   if(bcdtmMath_sideOf(x[1],y[1],x[2],y[2],xl,yl) <= 0 )
				               if(bcdtmMath_sideOf(x[2],y[2],x[0],y[0],xl,yl) <= 0 )
				                 {
				                  bcdtmMath_interpolatePointOnTriangle(xl,yl,&zl,x,y,z) ;
                                  switch( themeOption )
                                    {
                                     case 1 :   /* Maximum Lattice */
                                       if( *(latticeP->LAT+iofs) < (float) zl )  *(latticeP->LAT+iofs) = (float) zl   ; 
                                     break  ;

                                     case 2 :   /* Minmum Lattice */
                                       if( *(latticeP->LAT+iofs) > (float) zl )  *(latticeP->LAT+iofs) = (float) zl   ; 
                                     break  ;

                                     case 3 :   /* Difference Lattice */
				                       *(latticeP->LAT+iofs) -= (float) zl   ;
                                     break  ;

                                    } ;
                                  bcdtmFlag_setFlag(subFlagP,iofs) ;
				                  newTriangle = 0 ;
                                 }
				              }
			               }
			            }
		             }
		          }
	           }
	        }
	      p2 = p3 ; 
	     } 
      }
   }
/*
** Null Out Lattice Points Where Overlapm Did Not Occur
*/
 for( iofs=0 ; iofs < latticeP->NOLATPTS ; ++iofs )
   {
    if( ! bcdtmFlag_testFlag(subFlagP,iofs) ) *(latticeP->LAT+iofs) = latticeP->NULLVAL ;
   } 
/*
** Set Lattice Min & Max Values 
*/
 p1 = 0 ;   
 zmin  =  zmax  = 0.0 ;
 latticeP->NOACTPTS = 0 ;
 for( lat = latticeP->LAT ; lat < latticeP->LAT + latticeP->NOLATPTS ; ++lat )
   {
    if( *lat != latticeP->NULLVAL ) 
	  { 
	   ++latticeP->NOACTPTS ; 
       if( ! p1 ) { zmin = zmax = *lat ; p1 = 1 ; }
	   if( *lat < zmin ) zmin = *lat ;
	   if( *lat > zmax ) zmax = *lat ;
	  }
   }
 latticeP->LZMIN = zmin  ;
 latticeP->LZMAX = zmax  ;
 latticeP->LZDIF = latticeP->LZMAX - latticeP->LZMIN ;
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( subFlagP != NULL ) { free(subFlagP) ; subFlagP = NULL ; }
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
BENTLEYDTM_EXPORT int bcdtmLattice_createDtmFileFromLatticeFile
(
 WCharCP latticeFileP,
 WCharCP dtmFileP
) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTM_LAT_OBJ *latticeP=NULL ;
 BC_DTM_OBJ  *dtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Dtm File From Lattice File") ;
/*
** Read Lattice File
*/
 if( bcdtmUtl_testForAndSetCurrentLatticeObject(&latticeP,latticeFileP)) goto errexit ;
/*
** Create Dtm Object From Lattice Object
*/
 if( bcdtmLattice_createDtmObjectFromLatticeObject(latticeP,&dtmP)) goto errexit ;
/*
** Write Dtm File
*/
 if( bcdtmWrite_toFileDtmObject(dtmP,dtmFileP)) goto errexit ;
/*
** Set Currency
*/
 if( bcdtmUtility_setCurrentDtmObject(dtmP,dtmFileP)) goto errexit ;  
/*
** Clean Up
*/
 cleanup:
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Dtm File From Lattice File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Dtm File From Lattice File Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLattice_createDtmObjectFromLatticeObject
(
 DTM_LAT_OBJ *latticeP,
 BC_DTM_OBJ  **dtmPP
) 
/*
**
** Notes :-
** 
** 1. Xo & Yo are the bottom left hand corner of the DEM
** 2. xinc and yinc are the x and y increment values
** 3. nrows = number of rows    ( parallel to the x axis )
** 4. ncols = number of columns ( parallel to the y axis ) 
** 5. The DEM Coordinates are written to the Data Object A column at a time,
**    from the bottom up and from left to right   
** 6. Null DEM Points Are set to Voids
**
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   i,j,nrows,ncols ;
 double xo,yo,xinc,yinc,nullVal;
 DPoint3d    demPoint ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Dtm Object From Lattice Object") ;
/*
** Test For Valid Lattice Object
*/
 if( bcdtmObject_testForValidLatticeObject(latticeP)) goto errexit ;
/*
** Initialise DEM Triangulation Parameters From Lattice
*/
 xo      = latticeP->LXMIN ;
 yo      = latticeP->LYMIN ;
 xinc    = latticeP->DX ;
 yinc    = latticeP->DY ;
 nrows   = latticeP->NXL ;
 ncols   = latticeP->NYL ;
 nullVal = latticeP->NULLVAL ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Lattice Points = %7ld",nrows*ncols) ;
/*
** Create Dtm Object
*/
 if( bcdtmObject_createDtmObject(dtmPP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(*dtmPP,nrows*ncols,1000) ;
/*
** Populate DTM Object From Lattice Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating DTM Object From Lattice Object") ;
 for( i = 0 ; i < latticeP->NYL ; ++i )
   {
    demPoint.x =  latticeP->LXMIN + latticeP->DX * i ; 
    for( j = 0 ; j < latticeP->NXL ; ++j )
      {
       demPoint.y = latticeP->LYMIN + latticeP->DY * j ;
       demPoint.z = *(latticeP->LAT + j*latticeP->NYL  + i)  ;   
       if( bcdtmObject_storeDtmFeatureInDtmObject(*dtmPP,DTMFeatureType::RandomSpots,(*dtmPP)->nullUserTag,1,&(*dtmPP)->nullFeatureId,&demPoint,1)) goto errexit ;
      } 
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Dtm Points = %7ld",(*dtmPP)->numPoints ) ;
/*
** Triangulate DEM DTM Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating DEM") ;
 if( bcdtmLattice_createDemTinDtmObject(*dtmPP,nrows,ncols,nullVal) ) goto errexit ;
/*
** Clean Up
*/
 cleanup:
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Dtm Object From Lattice Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Dtm Object From Lattice Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( *dtmPP != NULL ) bcdtmObject_destroyDtmObject(dtmPP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmLattice_createDemTinDtmObject(BC_DTM_OBJ *dtmP,long numRows,long numColumns,double nullValue)
/*
** This Function Creates A Tin From DEM ( Digital Elevation Model ) Data 
** Notes :-
** 
** 1. numRows    = number of rows    ( parallel to the x axis )
** 2. numColumns = number of columns ( parallel to the y axis ) 
** 3. nullValue  = Null Value Assigned For Missing Points In The DEM Data Set
** 4. The DEM Coordinates must Be written to the Dtm Object A Column at a time,
**    from the bottom up and from left to right 
**  
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long   numPoints,startTime   ;
/*
** Initialise
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Tin From %8ld Point DEM",numRows*numColumns) ;
/*
** Check Number of Dtm Points
*/
 numPoints = numRows * numColumns ;
 if( numPoints != dtmP->numPoints ){ bcdtmWrite_message(1,0,0,"Incorrect Number Of DEM Data Points") ; goto errexit ; }
/*
**  Triangulate Dem Dtm Object
*/
 if( bcdtmLattice_triangulateDemDtmObject(dtmP,numRows,numColumns,nullValue)) goto errexit ;
/*
**  Check Tin
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP) )
      {
       bcdtmWrite_message(2,0,0,"Tin Invalid") ;
       goto errexit ;
      } 
    bcdtmWrite_message(0,0,0,"Tin Valid") ;
   }
/*
** Write Dtm Statistics
*/
 //TODO if( dbg ) bcdtmUtility_writeStatisticsDtmObject(dtmP) ;
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Create Tin = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Tin From DEM Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Tin From DEM Error") ;
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
BENTLEYDTM_Private int bcdtmLattice_triangulateDemDtmObject(BC_DTM_OBJ *dtmP,long numRows,long numColumns,double nullValue )
/*
** This Is The Controlling Routine For Triangulating A DEM Dtm Object
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
/*
** Set Dtm State To Tin
*/
 dtmP->dtmState = DTMState::Tin  ;
 dtmP->numSortedPoints = dtmP->numPoints ;
/*
** Calculate Machine Precision For Dtm Object
*/
 bcdtmMath_calculateMachinePrecisionForDtmObject(dtmP) ;
 if( dtmP->ppTol <= dtmP->mppTol ) dtmP->ppTol = dtmP->mppTol * 1000.0  ;
 if( dtmP->plTol <= dtmP->mppTol ) dtmP->plTol = dtmP->mppTol * 1000.0  ;
/*
** Alocate Nodes Memory
*/
 if( bcdtmObject_allocateNodesMemoryDtmObject(dtmP) ) goto errexit ;
/*
** Allocate Circular List Memory
*/
 if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP) ) goto errexit ;
/*
** Triangulate Data Set
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating %8ld Point DEM",numRows*numColumns) ;
 if( bcdtmLattice_triangulateGridDtmObject(dtmP,numRows,numColumns,nullValue) ) goto errexit ;
/*
** Resort Tin
*/
 if( dtmP->numSortedPoints != dtmP->numPoints )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Resorting Tin Structure") ;
    if( bcdtmTin_resortTinStructureDtmObject(dtmP)) goto errexit ;
   }
/*
** Scan Dtm Hull And Remove Null Valued Hull Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Null Valued Hull Points") ;
 if( bcdtmLattice_removeNullPointsOnTinHullDtmObject(dtmP,nullValue)) goto errexit ;
/*
** Scan Dtm And Void Null Valued Internal Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Voiding Null Valued Internal Points") ;
 if( bcdtmLattice_voidInternalNullPointsDtmObject(dtmP,nullValue)) goto errexit ;
/*
** Compact Dtm Structure
*/
 if( dbg)  bcdtmWrite_message(0,0,0,"Compacting Dtm Structure") ;
 if( bcdtmTin_compactCircularListDtmObject(dtmP))       goto errexit ;
 if( bcdtmTin_compactFeatureTableDtmObject(dtmP))       goto errexit ;
 if( bcdtmTin_compactFeatureListDtmObject(dtmP))        goto errexit ;
 if( bcdtmTin_compactPointAndNodeTablesDtmObject(dtmP)) goto errexit ;
/*
** Resize Dtm Memory
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resizing Dtm Memory")  ;
 if( bcdtmObject_resizeMemoryDtmObject(dtmP) ) goto errexit ;
/*
** Set Number Of Dtm Points For Binary Searching
*/
 dtmP->numSortedPoints = dtmP->numPoints ;
/*
** Reset Bounding Cube For Dtm Object
*/
 bcdtmMath_setBoundingCubeDtmObject(dtmP) ;
/*
** Count Number Of Triangles and Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Counting Triangles And Lines") ;
 bcdtmList_countTrianglesAndLinesDtmObject(dtmP,&dtmP->numTriangles,&dtmP->numLines) ;
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLattice_triangulateGridDtmObject(BC_DTM_OBJ *dtmP,long numRows,long numColumns,double nullValue )
/*
** This Function Triangulates A DEM Dtm Object
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long i,j,p1,p2,p3,p4 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Grid ** numRows = %6ld numColumns = %6ld nullValue = %lf",numRows,numColumns,nullValue) ;
/*
** Join Rows
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Joining Rows") ;
 for( i = 0 ; i < numRows ; ++i )
   {
    p1 = i ;
    for ( j = 0 ; j < numColumns - 1 ; ++j )
      {
       p2 = p1 + numRows ;
       p3 = p1 - numRows ;
       if( j == 0 ) p3 = dtmP->nullPnt ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p1,p2,p3)) goto errexit ; 
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,p1,dtmP->nullPnt)) goto errexit ;
       p1 = p2 ;
      }
   }
/*
** Join Columns
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Joining Columns") ;
 for( j = 0 ; j < numColumns - 1 ; ++j )
   {
    p1 = j * numRows ;
    for ( i = 0 ; i < numRows - 1 ; ++i )
      {
       p2 = p1 + 1 ;
       p3 = p2 + numRows ;
       p4 = p1 + numRows ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p1,p2,p4)) goto errexit ; 
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,p1,p3))  goto errexit ;
       if( j == numColumns - 2 )
         {
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,p3,p1))  goto errexit ; 
          if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p3,p4,p2)) goto errexit ;
         }
       p1 = p2 ;
      }
   }
/*
** Join Diagonals
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Joining Diagonals") ;
 for( j = 0 ; j < numColumns - 1 ; ++j )
   {
    p1 = j * numRows ; 
    for ( i = 0 ; i < numRows - 1 ; ++i )
      {
       p2 = p1 + 1 ;
       p3 = p2 + numRows ;
       p4 = p1 + numRows ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,p4,p3))  goto errexit ; 
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,p2,p1))  goto errexit ;
       p1 = p2 ;
      }
   }
/*
** Set Convex Hull
*/
 if( dbg) bcdtmWrite_message(0,0,0,"Setting Convex Hull") ;
 dtmP->hullPoint = 0 ;
 dtmP->nextHullPoint = numRows ;
 bcdtmTin_setConvexHullDtmObject(dtmP,dtmP->hullPoint,dtmP->nextHullPoint) ;
/*
** Cleanup
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
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLattice_removeNullPointsOnTinHullDtmObject(BC_DTM_OBJ *dtmP ,double nullValue )
/*
** This Function Removes Null Valued Points On The Dtm HULL
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),sdof ;
 long pnt,ap,sp,np,mp,clc,process ;
 DTM_TIN_POINT *pntP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Null Valued Points On Dtm Hull") ;
/*
** Iteratively Scan Dtm Hull Until no more Null Points Can Be Removed
*/
 process = 1 ;
 while ( process )
   {
    process = 0 ;
    sp = dtmP->hullPoint ;
    if( ( dtmP->hullPoint = bcdtmList_nextClkDtmObject(dtmP,dtmP->hullPoint,nodeAddrP(dtmP,dtmP->hullPoint)->hPtr)) < 0 ) goto errexit ;
    do
      { 
       np = nodeAddrP(dtmP,sp)->hPtr ;
       if(pointAddrP(dtmP,sp)->z == nullValue )
         {
/*
**        Scan Hull Point For Connecting Hull Points
*/
          if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
          while ( nodeAddrP(dtmP,ap)->hPtr == dtmP->nullPnt ) 
            {
             if( ( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ;
            } 
/*
**        Check If Hull Point Can Be Deleted
*/
          if(nodeAddrP(dtmP,ap)->hPtr == sp )
            {
             process = 1 ;
/*
**           Reset Hull Pointers
*/
             mp = np ; 
             if(( ap = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit  ;
             do
               {
                nodeAddrP(dtmP,ap)->hPtr = mp ;
                 mp = ap ;  
                 if(( ap = bcdtmList_nextAntDtmObject(dtmP,sp,ap)) < 0 ) goto errexit ; ;
               } while ( ap != np ) ;
/*
**           Delete Circular List For Hull Point
*/
             clc = nodeAddrP(dtmP,sp)->cPtr ;
             while( clc != dtmP->nullPtr )
               {
                ap  = clistAddrP(dtmP,clc)->pntNum ;
                clc = clistAddrP(dtmP,clc)->nextPtr ;
                if( bcdtmList_deleteLineDtmObject(dtmP,sp,ap) ) goto errexit ;
               }
             nodeAddrP(dtmP,sp)->cPtr = dtmP->nullPtr ;
             nodeAddrP(dtmP,sp)->hPtr = dtmP->nullPnt ;
             nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
             nodeAddrP(dtmP,sp)->fPtr = dtmP->nullPtr ;
            }  
         }
/*
**     Reset For Next Hull Point
*/
       sp = np ;
      } while ( sp != dtmP->hullPoint ) ;
   }
/*
** Remove Non Contiguous Null Valued Hull Points
*/
 process = 1 ;
 while( process )
   {
    process = 0 ;
    sp = dtmP->hullPoint ;
    if( ( mp = bcdtmList_nextClkDtmObject(dtmP,dtmP->hullPoint,nodeAddrP(dtmP,dtmP->hullPoint)->hPtr)) < 0 ) goto errexit ;
    do
      { 
       np = nodeAddrP(dtmP,sp)->hPtr ;
       if( pointAddrP(dtmP,mp)->z != nullValue &&  pointAddrP(dtmP,sp)->z == nullValue &&  pointAddrP(dtmP,np)->z != nullValue )
         {
          sdof = bcdtmMath_pointSideOfDtmObject(dtmP,mp,np,sp) ;
/*
**        Null Valued Point To Left Of Mp-Np
*/
          if( sdof ==  1 ) 
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Null Value Point %6ld To Left Of Mp-Np",sp) ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtmP,mp,np,sp)) goto errexit ;
             if( bcdtmList_insertLineBeforePointDtmObject(dtmP,np,mp,sp)) goto errexit ;
             nodeAddrP(dtmP,sp)->hPtr = dtmP->nullPnt ;
             nodeAddrP(dtmP,mp)->hPtr = np ;
             if( dtmP->hullPoint == sp ) dtmP->hullPoint = mp ;
             sp = mp ; 
             process = 1 ; 
            }
/*
**        Null Valued Point To Right Of Mp-Np
*/
          if( sdof ==  -1 ) 
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Null Value Point %6ld To Right Of Mp-Np",sp) ;
             clc = nodeAddrP(dtmP,sp)->cPtr ;
             while( clc != dtmP->nullPtr )
               {
                ap  = clistAddrP(dtmP,clc)->pntNum ;
                clc = clistAddrP(dtmP,clc)->nextPtr ;
                if( bcdtmList_deleteLineDtmObject(dtmP,sp,ap) ) goto errexit ;
               }
             nodeAddrP(dtmP,sp)->cPtr = dtmP->nullPtr ;
             nodeAddrP(dtmP,sp)->hPtr = dtmP->nullPnt ;
             nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
             nodeAddrP(dtmP,sp)->fPtr = dtmP->nullPtr ;
/*
**           If Mp Np Not Connected Insert Line Mp-Np 
*/
             if( ! bcdtmList_testLineDtmObject(dtmP,mp,np)) if( bcdtmList_insertLineDtmObject(dtmP,mp,np)) goto errexit ;
/*
**           Reset For Next Line
*/
             nodeAddrP(dtmP,mp)->hPtr = np ;
             if( dtmP->hullPoint == sp ) dtmP->hullPoint = mp ;
             sp = mp ; 
             process = 1 ; 
            }
         }  
       mp = sp ;
       sp = np ;
      } while ( sp != dtmP->hullPoint ) ;
   } 
/*
** Reset Dtm z Minimun,Maximum And Range Values
*/
  for( pnt = 0 ; pnt < dtmP->numPoints ; ++pnt )
    {
    if( nodeAddrP(dtmP,pnt)->cPtr != dtmP->nullPtr )
      {
       pntP = pointAddrP(dtmP,pnt) ;
       if( ! pnt ) { dtmP->zMin = dtmP->zMax = pntP->z ; }
       else
         {
          if( pntP->z < dtmP->zMin ) dtmP->zMin = pntP->z ;
          if( pntP->z > dtmP->zMax ) dtmP->zMax = pntP->z ;
         }
      }
   }
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
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLattice_voidInternalNullPointsDtmObject(BC_DTM_OBJ *dtmP ,double nullValue )
/*
** This Function Removes Null Valued Points On The Dtm HULL
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    p1,p2,p3,clc ;
 double  lastZ ;
 DPoint3d     linePts[2] ;
 BC_DTM_OBJ *voidDtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Voiding Internal Null Valued Points") ;
/*
** Create Data Object
*/
 if( bcdtmObject_createDtmObject(&voidDtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(voidDtmP,1000,1000) ;
/*
** Scan Dtm Hull And Write Void Lines As Break Lines
*/
 p1 = dtmP->hullPoint ;
 do
   {
    p2 = nodeAddrP(dtmP,p1)->hPtr ; 
    if( pointAddrP(dtmP,p1)->z == nullValue || pointAddrP(dtmP,p2)->z == nullValue )
      {
       memcpy(&linePts[0],pointAddrP(dtmP,p1),sizeof(DPoint3d)) ;
       memcpy(&linePts[1],pointAddrP(dtmP,p2),sizeof(DPoint3d)) ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(voidDtmP,DTMFeatureType::Breakline,voidDtmP->nullUserTag,1,&voidDtmP->nullFeatureId,linePts,2) ) goto errexit ;
       if( dbg ) 
         {
          bcdtmWrite_message(0,0,0,"Hull Void Line ** %8ld  %10.4lf %10.4lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
          bcdtmWrite_message(0,0,0,"               ** %8ld  %10.4lf %10.4lf %10.4lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
         } 
      }
    p1 = p2 ;
   } while ( p1 != dtmP->hullPoint ) ;
/*
** Scan Internal Dtm Points For Null Valued Points
*/
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    if( nodeAddrP(dtmP,p1)->cPtr != dtmP->nullPtr )
      {
       if( pointAddrP(dtmP,p1)->z == nullValue )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Void Point ** 8ld Fptr = %9ld ** %8ld  %10.4lf %10.4lf %10.4lf",p1,nodeAddrP(dtmP,p1)->hPtr,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z) ;
          clc = nodeAddrP(dtmP,p1)->cPtr ;
          if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ; 
          while ( clc != dtmP->nullPtr )
            {
             p3  = clistAddrP(dtmP,clc)->pntNum ;
             clc = clistAddrP(dtmP,clc)->nextPtr ;
             if( nodeAddrP(dtmP,p3)->hPtr != p1 )
               {
                if( pointAddrP(dtmP,p2)->z != nullValue &&  pointAddrP(dtmP,p3)->z != nullValue )
                  {
                   memcpy(&linePts[0],pointAddrP(dtmP,p2),sizeof(DPoint3d)) ;
                   memcpy(&linePts[1],pointAddrP(dtmP,p3),sizeof(DPoint3d)) ;
                   if( bcdtmObject_storeDtmFeatureInDtmObject(voidDtmP,DTMFeatureType::Breakline,voidDtmP->nullUserTag,1,&voidDtmP->nullFeatureId,linePts,2) ) goto errexit ;
                   if( dbg ) 
                     {
                      bcdtmWrite_message(0,0,0,"Internal Void Line ** %8ld  %10.4lf %10.4lf %10.4lf",p2,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z) ;
                      bcdtmWrite_message(0,0,0,"                   ** %8ld  %10.4lf %10.4lf %10.4lf",p3,pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p3)->z) ;
                     } 
                  } 
               }
             p2 = p3 ;
            } 
         }
      }
   } 
/*
** Process Internal Voids If Detected
*/
 if( voidDtmP->numPoints > 0 )
   { 
/*
**  Write Data Object Developement Only
*/
//    if( dbg ) bcdtmWrite_dataFileFromDataObject(data,"voidhulls.dat") ;
/*
**  Resolve Voids Into Voids And Islands
*/
    if( bcdtmLattice_resolveVoidsFromBreakLinesDtmObject(voidDtmP)) goto errexit ;
/*
**  Write Data Object Developement Only
*/
//    if( dbg ) bcdtmWrite_dataFileFromDataObject(data,"void&islandhulls.dat") ;
/*
**  Insert Voids And Islands IntoDtm Element
*/
    if( bcdtmLattice_insertVoidsAndIslandsIntoDtmObject(dtmP,voidDtmP) ) goto errexit ;
/*
**  Scan Dtm Hull And Replace Null Hull Point Values With Last Non Null Value
*/
    p1 = dtmP->nullPnt ;
    p2 = dtmP->hullPoint ;
    do
      {
       if(pointAddrP(dtmP,p2)->z != nullValue ) p1 = p2 ;
       p2 = nodeAddrP(dtmP,p2)->hPtr ;
      } while ( p2 != dtmP->hullPoint && p1 == dtmP->nullPnt ) ;
    if( p1 != dtmP->nullPnt )
      {
       lastZ = pointAddrP(dtmP,p1)->z ;
       p2 = p1 ;
       do
         {
          if(pointAddrP(dtmP,p2)->z == nullValue ) pointAddrP(dtmP,p2)->z = lastZ ;
          else                              lastZ = pointAddrP(dtmP,p2)->z ;  
          p2 = nodeAddrP(dtmP,p2)->hPtr ;
         } while ( p2 != p1 ) ; 
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( voidDtmP != NULL ) bcdtmObject_destroyDtmObject(&voidDtmP) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLattice_resolveVoidsFromBreakLinesDtmObject(BC_DTM_OBJ *voidDtmP) 
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    sp,np,p1,p2,p3,clc,numStartFeatures,numIslands,numVoids ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Voids From Break Lines") ;
/*
** Triangulate DTM Object
*/
 if( bcdtmObject_createTinDtmObject(voidDtmP,1,0.0)) goto errexit ;
 numStartFeatures = voidDtmP->numFeatures ;
/*
** Remove None Feature Hull Lines
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(voidDtmP)) goto errexit ;
/*
** Null Sptr List
*/
 bcdtmList_nullSptrValuesDtmObject(voidDtmP) ; 
/*
** Scan Void Hull And Look For Start Of Void Polygon
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Scanning Hull For Void Polygons") ;
 numVoids = 0 ;
 sp = voidDtmP->hullPoint ;
 do
   {
    np = nodeAddrP(voidDtmP,sp)->hPtr ;
/*
**  Test For Hull Break Line
*/
    if( bcdtmList_testForBreakLineDtmObject(voidDtmP,sp,np) )
      { 
/*
**    Test For Start Of New Void Polygon
*/
       if( nodeAddrP(voidDtmP,sp)->sPtr == voidDtmP->nullPnt && nodeAddrP(voidDtmP,np)->sPtr == voidDtmP->nullPnt )
         {
/*
**        Extract Void Polygon
*/
          nodeAddrP(voidDtmP,sp)->tPtr = np ;
          nodeAddrP(voidDtmP,sp)->sPtr = np ;
          p1 = sp ;
          p2 = np ;
/*
**        Scan Around Exterior Of Break Lines
*/
          do
            {
             p3 = p1 ;
             do
               {
                if( ( p3 = bcdtmList_nextAntDtmObject(voidDtmP,p2,p3)) < 0 ) goto errexit ; 
               }  while ( ! bcdtmList_testForBreakLineDtmObject(voidDtmP,p2,p3) ) ;
             nodeAddrP(voidDtmP,p2)->tPtr = p3 ;
             nodeAddrP(voidDtmP,p2)->sPtr = p3 ;
             p1 = p2 ;
             p2 = p3 ;
            } while ( p3 != sp  ) ;
/*
**        Store Void In Dtm Object
*/
          if( bcdtmInsert_addDtmFeatureToDtmObject(voidDtmP,NULL,0,DTMFeatureType::Void,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,sp,1)) goto errexit ;
          ++numVoids ;
         }
      } 
/*
**  Reset For Next Hull Line
*/
    sp = np ;
   } while ( sp != voidDtmP->hullPoint ) ;
/*
** Scan Internal Lines And Extract Islands
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Scanning Internal Dtm Lines For Island Polygons") ;
 numIslands = 0 ;
 for( sp = 0 ; sp < voidDtmP->numPoints ; ++sp )
   {
    clc = nodeAddrP(voidDtmP,sp)->cPtr ;
    while( clc != voidDtmP->nullPtr )
      {
       np  = clistAddrP(voidDtmP,clc)->pntNum ;
       clc = clistAddrP(voidDtmP,clc)->nextPtr ;
       if( sp < np  )
         {
/*
**        Test For Internal Line
*/
          if( nodeAddrP(voidDtmP,sp)->hPtr != np && nodeAddrP(voidDtmP,np)->hPtr != sp )
            {
/*
**          Test For Not A Break Line
*/
             if( ! bcdtmList_testForBreakLineDtmObject(voidDtmP,sp,np) )
               { 
/*
**              Scan To Start Line Of Island
*/
                p1 = np ;
                do
                  {
                   if( ( p1 = bcdtmList_nextClkDtmObject(voidDtmP,sp,p1)) < 0 ) goto errexit ;
                  } while ( p1 != np && ! bcdtmList_testForBreakLineDtmObject(voidDtmP,sp,p1) ) ; 
                np = p1 ;
/*
**             Test For Start Of New Island Polygon
*/
                if( bcdtmList_testForBreakLineDtmObject(voidDtmP,sp,np) )
                  { 
                   if( nodeAddrP(voidDtmP,sp)->sPtr == voidDtmP->nullPnt && nodeAddrP(voidDtmP,np)->sPtr == voidDtmP->nullPnt )
                     {
/*
**                    Extract Island Polygon
*/
                      nodeAddrP(voidDtmP,sp)->sPtr = np ;
                      nodeAddrP(voidDtmP,sp)->tPtr = np ;
/*
**                    Scan Around Interior Of Break Lines
*/
                      p1 = sp ;
                      p2 = np ;
                      do
                        {
                         p3 = p1 ;
                         do
                           {
                            if( ( p3 = bcdtmList_nextClkDtmObject(voidDtmP,p2,p3)) < 0 ) goto errexit ; 
                           }  while ( ! bcdtmList_testForBreakLineDtmObject(voidDtmP,p2,p3) ) ;
                         nodeAddrP(voidDtmP,p2)->sPtr = p3 ;
                         nodeAddrP(voidDtmP,p2)->tPtr = p3 ;
                         p1 = p2 ;
                         p2 = p3 ;
                        } while ( p3 != sp  ) ;
/*
**                   Store Feature As Polygon
*/
                      if( bcdtmInsert_addDtmFeatureToDtmObject(voidDtmP,NULL,0,DTMFeatureType::Island,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,sp,1)) goto errexit ;
                      ++numIslands ;
                     }
                  }
               }
            }
         } 
      } 
   }
/*
** Write Number Of Detected Features
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Number Of Voids   Detected = %6ld",numVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Islands Detected = %6ld",numIslands) ;
   }
/*
** Write Void Polygons ** Development Only
*/
// if( dbg ) bcdtmWrite_dataFileFromDataObject(*voidDtmP,"voidDtmP.dat") ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Voids From Break Lines Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Resolving Voids From Break Lines Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmLattice_insertVoidsAndIslandsIntoDtmObject
(
 BC_DTM_OBJ *dtmP,
 BC_DTM_OBJ *voidDtmP
) 
/*
** Note : Assumes Any Islands Are Internal To Voids
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    pnt,loop,lpnt,spnt,numVoid,numIsland ;
 DTMDirection direction;
 long    dtmFeature,numFeaturePts,firstCall ;
 DTMFeatureType dtmFeatureType;
 double  area ;
 DPoint3d     *p3dP,*featurePtsP=NULL ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Voids And Islands Into Dtm Object") ;
/*
** Scan And Insert Voids And Islands
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Voids And Islands") ;
 numVoid = numIsland = 0 ;
 for( loop = 0 ; loop < 2 ; ++loop )
   {
    firstCall = TRUE ;
    if( loop == 0 ) dtmFeatureType = DTMFeatureType::Void ;
    else            dtmFeatureType = DTMFeatureType::Island ;
    if( bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(voidDtmP,dtmFeatureType,firstCall,&dtmFeatureP,&dtmFeature) ) goto errexit ;
    while( dtmFeatureP != NULL )
      {
       firstCall = FALSE ;
/*
**     Get Points For Dtm Feature
*/
       if( bcdtmObject_getPointsForDtmFeatureDtmObject(dtmP,dtmFeature,( DTM_TIN_POINT **)&featurePtsP,&numFeaturePts)) goto errexit ; 
/*
**     Store Polygon As Sptr Polygon
*/
        spnt = lpnt = dtmP->nullPnt ;
        for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
          {
           bcdtmFind_closestPointDtmObject(dtmP,p3dP->x,p3dP->y,&pnt) ;
           if( lpnt != dtmP->nullPnt ) nodeAddrP(dtmP,lpnt)->sPtr = pnt ; 
           if( spnt == dtmP->nullPnt ) spnt = pnt ;
           lpnt = pnt ;
          }
/*
**     Check Direction Of Sptr Polygon
*/
       bcdtmMath_calculateAreaAndDirectionSptrPolygonDtmObject(dtmP,spnt,&area,&direction) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Sptr Polygon Area = %10.4lf ** Direction = %1ld",area,direction) ;
/*
**     If Sptr Polygon Direction Clock Wise Reverse Direction
*/  
       if( direction == DTMDirection::Clockwise ) bcdtmList_reverseSptrPolygonDtmObject(dtmP,spnt) ;
/*
**     Insert Sptr Polygon Into Dtm 
*/
       pnt = spnt ;
       do
         {
          if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,pnt,nodeAddrP(dtmP,pnt)->sPtr,1,2) ) goto errexit ;
          pnt = nodeAddrP(dtmP,pnt)->sPtr ;
         } while ( pnt != spnt ) ; 
/*
**     Null Sptr Values
*/ 
       if( bcdtmList_nullSptrListDtmObject(dtmP,spnt)) goto errexit ;
/*
**     Insert Tptr Polygon As Void Or Island Feature
*/
       if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,spnt,1)) goto errexit ;
       if( dtmFeatureType == DTMFeatureType::Void   ) ++numVoid ;
       if( dtmFeatureType == DTMFeatureType::Island ) ++numIsland ;
/*
**     Get Next Feature
*/
       if( bcdtmList_getPointerAndOffsetToNextDtmFeatureTypeOccurrenceDtmObject(voidDtmP,dtmFeatureType,firstCall,&dtmFeatureP,&dtmFeature) ) goto errexit ;
      }
   }
/*
** Number Of Voids And Islands ** Development Only
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Voids   Inserted = %2ld",numVoid) ;
    bcdtmWrite_message(0,0,0,"Number Of Islands Inserted = %2ld",numIsland) ;
   }
/*
** Void Points Internal To Voids And External To Islands
*/
 if( bcdtmMark_voidPointsDtmObject(dtmP)) goto errexit  ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Voids And Islands Into Dtm Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Inserting Voids And Islands Into Dtm Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmObject_deleteAllLatticeObjects(void)
/*
** This Function Deletes a LAT Object
*/
{
/*
** Scan Lattice Object Pointer List For Lattice Object Entries
*/
/*
 for ( i = 0 ; i < DTM_MAX_LAT_OBJS ; ++i )
   {
    if ( latObjPtrs[i] != NULL )
      {
       if( bcdtmObject_freeMemoryLatticeObject(latObjPtrs[i])) return(1) ;
       free(latObjPtrs[i]) ; latObjPtrs[i] = NULL ;
      }
   }
*/   
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmWrite_toFileLatticeObject(DTM_LAT_OBJ *Lattice,WCharCP LatticeFileName)
{
/*
** Test For Valid Lattice Object
*/
 if( bcdtmObject_testForValidLatticeObject(Lattice)) return(1) ;
/*
** Set File Name In Lattice Object
*/
// if( strlen(LatticeFileName) > 0 && strlen(LatticeFileName) < DTM_FILE_SIZE ) 
//   {
//    strcpy(Lattice->LatticeObjectFile,LatticeFileName) ;
//   }
/*
** Write Lattice Object To File
*/
 if( bcdtmWrite_latticeObject(Lattice,LatticeFileName)) return(1) ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   bcdtmWrite_latticeObject()                                       |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmWrite_latticeObject(DTM_LAT_OBJ *Lattice,WCharCP latticeFileName)
{
 FILE  *fpLAT=NULL ;
/*
** Check Lattice Object File Name Set
*/
// if( strlen(Lattice->LatticeObjectFile) == 0 )
//   { 
//    bcdtmWrite_message(1,0,0,"Lattice File Name Not Set In Lattice Object") ; return(1) ; 
//   }
/*
** Open Lattice File For Writing
*/
 bcdtmWrite_message(0,0,0,"Writing Lattice File %ws",latticeFileName) ;
 fpLAT = bcdtmFile_open(latticeFileName,L"wb") ;
 if( fpLAT == NULL )
   {
    bcdtmWrite_message(1,0,0,"Error Opening Lattice File %s For Writing",Lattice->LatticeObjectFile) ;
    return(1) ; 
   }
/*
** Write Lattice Header
*/

#ifdef BIG_ENDIAN
    bcdtmCnv_swapByteArray((char*) &Lattice->dtmFileType,104) ;
    bcdtmCnv_swapWordArray((short*)&Lattice->dtmFileType, 52) ;
    bcdtmCnv_swapLongArray((long*) &Lattice->DX,    19) ;
#endif

 if( bcdtmFwrite(Lattice,sizeof(DTM_LAT_OBJ),1,fpLAT) != 1 )
   {
    fclose(fpLAT) ;
    bcdtmWrite_message(1,0,0,"Error Writing Lattice File %s ",Lattice->LatticeObjectFile) ;
    return(1) ; 
   }

#ifdef BIG_ENDIAN
    bcdtmCnv_swapByteArray((char*) &Lattice->dtmFileType,104) ;
    bcdtmCnv_swapWordArray((short*)&Lattice->dtmFileType, 52) ;
    bcdtmCnv_swapLongArray((long*) &Lattice->DX,    19) ;
#endif

/*
** Write Lattice Points
*/

#ifdef BIG_ENDIAN
    bcdtmCnv_swapByteArray((char*) Lattice->LAT,Lattice->NXL*Lattice->NYL * 2) ;
    bcdtmCnv_swapWordArray((short*)Lattice->LAT,Lattice->NXL*Lattice->NYL) ;
#endif

 if( bcdtmFwrite(Lattice->LAT,sizeof(float)*Lattice->NOLATPTS,1,fpLAT) != 1 )
   {
    fclose(fpLAT) ;
    bcdtmWrite_message(1,0,0,"Error Writing Lattice File %s ",Lattice->LatticeObjectFile) ;
    return(1) ; 
   }

#ifdef BIG_ENDIAN
    bcdtmCnv_swapByteArray((char*) Lattice->LAT,Lattice->NXL*Lattice->NYL * 2) ;
    bcdtmCnv_swapWordArray((short*)Lattice->LAT,Lattice->NXL*Lattice->NYL) ;
#endif
/*
** Close Lattice File
*/
 fclose(fpLAT) ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmUtl_setCurrentLatticeObject()                                  |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtl_setCurrentLatticeObject(DTM_LAT_OBJ *Lattice,WCharCP LatticeFile)
/*
**
** This function sets the current Lattice Object
**
*/
{
/*
** Test For Valid Lattice Object
*/
 if( bcdtmObject_testForValidLatticeObject(Lattice)) return(1) ;
/*
** Delete Current Lattice Object If It Exists
*/
// if( DTM_CLOBJ != NULL ) if( bcdtmObject_deleteLatticeObject(&DTM_CLOBJ)) return(1) ;
/*
** Set Current Lattice Object
*/
// DTM_CLOBJ = Lattice ;
 if( *LatticeFile != 0 )
   {
//    wcscpy(DTM_CLOBJ_FILE,LatticeFile) ;
//    bcdtmUtl_setCurrentLatticeFileName(LatticeFile);
   } 
// else wcscpy(DTM_CLOBJ_FILE,L"MEMORY.LAT") ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmUtl_testForAndSetCurrentLatticeObject()                        |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmUtl_testForAndSetCurrentLatticeObject(DTM_LAT_OBJ **Lattice,WCharCP LatticeFile)
/*
**
** This function tests if the current Lattice Object is the requested Lattice
** If not the requested tin is read into memory.
**
*/
{
/*
** If Lattice File Not In Memory Read Into Current Lattice Object
*/
/*
 if( wcscmp(LatticeFile,DTM_CLOBJ_FILE) != 0 || DTM_CLOBJ == NULL )
   {
    if( DTM_CLOBJ != NULL ) bcdtmObject_deleteLatticeObject(&DTM_CLOBJ) ;
    DTM_CLOBJ_FILE[0] = 0 ;
    if( bcdtmRead_fromFileLatticeObject(Lattice,LatticeFile) )
      { if( *Lattice != NULL ) bcdtmObject_deleteLatticeObject(Lattice) ; return(1) ; }
    bcdtmWrite_message(0,0,0,"") ;
    wcscpy(DTM_CLOBJ_FILE,LatticeFile) ;
    DTM_CLOBJ = *Lattice ;
   }
 else *Lattice = DTM_CLOBJ ;
*/ 
/*
** Set Global Lattice File Name
*/
// bcdtmUtl_setCurrentLatticeFileName(LatticeFile);
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|  bcdtmLattice_getPartialDerivatives()                                |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLattice_getPartialDerivatives(long p1,long p2,long p3,double *Pdv,double pd[] )
{
 double  *fp,*dp ;
 for( fp=Pdv+5*p1, dp = &pd[0]  ; dp < &pd[0]  + 5 ; ++fp, ++dp ) *dp = *fp ;
 for( fp=Pdv+5*p2, dp = &pd[5]  ; dp < &pd[5]  + 5 ; ++fp, ++dp ) *dp = *fp ;
 for( fp=Pdv+5*p3, dp = &pd[10] ; dp < &pd[10] + 5 ; ++fp, ++dp ) *dp = *fp ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   int bcdtmObject_allocateLatticeMemoryLatticeObject()               |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_allocateLatticeMemoryLatticeObject(DTM_LAT_OBJ *Lattice,long Nxl,long Nyl,float NullVal)
/*
** This Routine Allocates The Memory Ror The Lattice
*/
{
 long  NxlNyl ;
 float *pf ;
/*
** Test For Valid Lattice Object
*/
 if( bcdtmObject_testForValidLatticeObject(Lattice) ) return(1) ;
/*
** Free Any Existing Memory
*/
 if( Lattice->LAT != NULL ) { free(Lattice->LAT) ; Lattice->LAT = NULL ; }
 Lattice->NOLATPTS = 0 ;
/*
** Allocate Memory
*/
 NxlNyl = Nxl * Nyl ;
 if( NxlNyl < 520000000 ) Lattice->LAT = ( float * ) malloc ( NxlNyl * sizeof(float)) ;
 if( Lattice->LAT == NULL ) { bcdtmWrite_message(1,0,0,"Too Many Grid/Lattice Points - Check Grid/Lattice Interval") ; return(1) ; }
/*
** Initialise Data Values
*/
 Lattice->NXL = Nxl ;
 Lattice->NYL = Nyl ;
 Lattice->NOLATPTS = NxlNyl ;
 Lattice->NOACTPTS = 0 ;
 Lattice->NULLVAL = NullVal ;
 for( pf = Lattice->LAT ; pf < Lattice->LAT + Lattice->NOLATPTS ; ++pf ) *pf = NullVal ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmObject_copyLatticeObjectToLatticeObject(DTM_LAT_OBJ *Lattice1, DTM_LAT_OBJ *Lattice2)
/*
** This Function Copies A Lattice Object To A Lattice Object
*/
{
/*
** Test For Valid Lattice Objects
*/
 if( bcdtmObject_testForValidLatticeObject(Lattice1)) return(1) ;
 if( bcdtmObject_testForValidLatticeObject(Lattice2)) return(1) ;
/*
** Release Any Memory Assigned To Lattice2 Object
*/
 if( Lattice2->LAT != NULL ) { free(Lattice2->LAT) ; Lattice2->LAT = NULL ; }
/*
** Copy Lattice Variables
*/
 *Lattice2 = *Lattice1 ;
/*
** Copy Lattice Structure
*/
 Lattice2->LAT = ( float * ) malloc ( Lattice2->NOLATPTS * sizeof(float)) ;
 if( Lattice2->LAT == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    return(1) ;
   }
/*
** Copy Lattice Values
*/
 memcpy(Lattice2->LAT,Lattice1->LAT,Lattice2->NOLATPTS*sizeof(float)) ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLattice_calculateNumberOfLatticeLines(double Xdif,double Ydif,long *Xll,long *Yll,long NoLatPts )
/*
** This Function Calculates The Number Of Lattice
*/
{
 double ratio,znl ;
 if( Xdif > Ydif )  ratio = Ydif / Xdif ;
 else               ratio = Xdif / Ydif ;
 znl = sqrt(ratio * (double)NoLatPts) ;
 if( Xdif >= Ydif ) { *Yll = (long) (znl / ratio) ; *Xll = NoLatPts / * Yll ; }
 if( Xdif <  Ydif ) { *Xll = (long) (znl / ratio) ; *Yll = NoLatPts / * Xll ; }
/*
** Job Completed
*/
 return(0) ;
}
