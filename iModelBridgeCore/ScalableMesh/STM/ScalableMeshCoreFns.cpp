/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshCoreFns.cpp $
|    $RCSfile: ScalableMeshCoreFns.cpp,v $
|   $Revision: 1.40 $
|       $Date: 2012/11/14 18:20:56 $
|     $Author: Daryl.Holmwood $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|   ScalableMeshFileCreator.cpp                        (C) Copyright 2001.     |
|                                               BCIVIL Corporation.     |
|                                               All Rights Reserved.    |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   General purpose memory functions.                                   |
|                                                                       |
|   SYSTEM:         BCIVIL API LIBRARY                                  |
|   DLL LIBRARY:    ScalableMesh                                               |
|   CATEGORY:       DIGITAL TERRAIN MODELING                            |
|                                                                       |
+----------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
/*----------------------------------------------------------------------+
| Include BCivil general header files                                   |
+----------------------------------------------------------------------*/

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "ScalableMeshCoreFns.h"

#include <ScalableMesh\ScalableMeshUtilityFunctions.h>


/*------------------------------------------------------------------+
| Include COGO definitions                                          |
+------------------------------------------------------------------*/

/*----------------------------------------------+
| Constant definitions                          |
+----------------------------------------------*/

/*----------------------------------------------+
| Private type definitions                      |
+----------------------------------------------*/

/*==================================================================*/
/*                                                                  */
/*          INTERNAL FUNCTIONS                                      */
/*                                                                  */
/*==================================================================*/

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmMultiResolution_getSampleCameraOri(double viewportRotMatrix[][3], int* camOriP)
{
    double Omega;
    double Phi;
          
    double PhiOmegaPossibleVals[5] = {0,45,90,270,345};
    long   PhiNearestPossibleValInd = 0;
    long   OmegaNearestPossibleValInd = 0;
    long   PossibleValInd;
            
    Phi = asin(viewportRotMatrix[2][0]);

    if (viewportRotMatrix[2][2] == 0)
        {
        Omega = 90;     
        }
    else
        {
        Omega = atan(-viewportRotMatrix[2][1] / viewportRotMatrix[2][2]);     
        }

    // To radiand
    Phi = msGeomConst_radiansPerDegree*Phi;
    Omega = msGeomConst_radiansPerDegree*Omega;

    if (Phi < 0) Phi += 360;
    if (Omega < 0) Omega += 360;
    
    for (PossibleValInd = 1; PossibleValInd < 5; PossibleValInd++)
        {        
        if (fabs(Phi - PhiOmegaPossibleVals[PossibleValInd]) < fabs(Phi - PhiOmegaPossibleVals[PhiNearestPossibleValInd]))
            {
            PhiNearestPossibleValInd = PossibleValInd;
            }

        if (fabs(Phi - PhiOmegaPossibleVals[PossibleValInd]) < fabs(Omega - PhiOmegaPossibleVals[OmegaNearestPossibleValInd]))
            {
            OmegaNearestPossibleValInd = PossibleValInd;
            }
        }

    //Which of Omega or Phi is nearest to a possible predefined camera orientation?
    if ((fabs(Omega - PhiOmegaPossibleVals[OmegaNearestPossibleValInd]) < fabs(Phi - PhiOmegaPossibleVals[PhiNearestPossibleValInd])) || 
        (PhiNearestPossibleValInd == 0))
        {
        switch (OmegaNearestPossibleValInd)
            {
            case 0: *camOriP = 0; break;
            case 1: *camOriP = 6; break;
            case 2: *camOriP = 2; break;
            case 3: *camOriP = 4; break;
            case 4: *camOriP = 8; break;
            default : assert(0);
            }
        }        
    else
        {
        switch (PhiNearestPossibleValInd)
            {
            case 0: *camOriP = 0; break;
            case 1: *camOriP = 5; break;
            case 2: *camOriP = 1; break;
            case 3: *camOriP = 3; break;
            case 4: *camOriP = 7; break;
            default : assert(0);
            }
        }
          
    return 0;
}

int bcdtmMultiResolution_getTileAreaInCurrentView(double rootToViewMatrix[][4], DPoint3d* tileShapePts, int nbTileShapePts, double* rootToViewScaleP)
{       
    HArrayAutoPtr<double> pointBuffer2D(new double[nbTileShapePts * 2]);
    double                div;

    for (int tilePtInd = 0; tilePtInd < nbTileShapePts; tilePtInd++)
        {            
        pointBuffer2D[tilePtInd * 2] = rootToViewMatrix[0][0] * tileShapePts[tilePtInd].x + rootToViewMatrix[0][1] * tileShapePts[tilePtInd].y + rootToViewMatrix[0][3];
        pointBuffer2D[tilePtInd * 2 + 1] = rootToViewMatrix[1][0] * tileShapePts[tilePtInd].x + rootToViewMatrix[1][1] * tileShapePts[tilePtInd].y + rootToViewMatrix[1][3];
        div = rootToViewMatrix[3][0] * tileShapePts[tilePtInd].x + rootToViewMatrix[3][1] * tileShapePts[tilePtInd].y + rootToViewMatrix[3][3];            

        pointBuffer2D[tilePtInd * 2] /= div;
        pointBuffer2D[tilePtInd * 2 + 1] /= div;       
        }                            

    HFCPtr<HGF2DCoordSys> pDummyCoordSys(new HGF2DCoordSys());

    HVE2DPolygonOfSegments ReprojectedTile(8, pointBuffer2D, pDummyCoordSys);       

    *rootToViewScaleP = ReprojectedTile.CalculateArea();
        
    return 0;
}


int bcdtmMultiResolution_getMaxFaceAreaInCurrentView(double rootToViewMatrix[][4], DPoint3d* facePts, int nbFacePts, double* rootToViewScaleP)
{   
    //Rectangular prism
    assert(nbFacePts == 8);

    HArrayAutoPtr<double> pointBuffer2D(new double[nbFacePts * 2]);
    double                div;

    for (int facePtInd = 0; facePtInd < nbFacePts; facePtInd++)
        {            
        pointBuffer2D[facePtInd * 2] = rootToViewMatrix[0][0] * facePts[facePtInd].x + rootToViewMatrix[0][1] * facePts[facePtInd].y + rootToViewMatrix[0][2] * facePts[facePtInd].z + rootToViewMatrix[0][3];
        pointBuffer2D[facePtInd * 2 + 1] = rootToViewMatrix[1][0] * facePts[facePtInd].x + rootToViewMatrix[1][1] * facePts[facePtInd].y + rootToViewMatrix[1][2] * facePts[facePtInd].z + rootToViewMatrix[1][3];
        div = rootToViewMatrix[3][0] * facePts[facePtInd].x + rootToViewMatrix[3][1] * facePts[facePtInd].y + rootToViewMatrix[3][2] * facePts[facePtInd].z + rootToViewMatrix[3][3];

        pointBuffer2D[facePtInd * 2] /= div;
        pointBuffer2D[facePtInd * 2 + 1] /= div;       
        }                            

    //NEEDS_WORK_SM : Could be more generic (no magic number)    
    *rootToViewScaleP = 0;

    HArrayAutoPtr<double> facePointBuffer2D(new double[4 * 2]);

    for (size_t faceInd = 0; faceInd < 6; faceInd++)
        {        
        switch (faceInd)        
            {
            case 0 :    
                facePointBuffer2D[0] = pointBuffer2D[0];
                facePointBuffer2D[1] = pointBuffer2D[1];
                facePointBuffer2D[2] = pointBuffer2D[2];
                facePointBuffer2D[3] = pointBuffer2D[3];
                facePointBuffer2D[4] = pointBuffer2D[6];
                facePointBuffer2D[5] = pointBuffer2D[7];
                facePointBuffer2D[6] = pointBuffer2D[4];
                facePointBuffer2D[7] = pointBuffer2D[5];
                break;

            case 1 :    
                facePointBuffer2D[0] = pointBuffer2D[8];
                facePointBuffer2D[1] = pointBuffer2D[9];
                facePointBuffer2D[2] = pointBuffer2D[10];
                facePointBuffer2D[3] = pointBuffer2D[11];
                facePointBuffer2D[4] = pointBuffer2D[14];
                facePointBuffer2D[5] = pointBuffer2D[15];
                facePointBuffer2D[6] = pointBuffer2D[12];
                facePointBuffer2D[7] = pointBuffer2D[13];
                break;

            case 2 :    
                facePointBuffer2D[0] = pointBuffer2D[0];
                facePointBuffer2D[1] = pointBuffer2D[1];
                facePointBuffer2D[2] = pointBuffer2D[4];
                facePointBuffer2D[3] = pointBuffer2D[5];
                facePointBuffer2D[4] = pointBuffer2D[12];
                facePointBuffer2D[5] = pointBuffer2D[13];
                facePointBuffer2D[6] = pointBuffer2D[8];
                facePointBuffer2D[7] = pointBuffer2D[9];
                break;

            case 3 :    
                facePointBuffer2D[0] = pointBuffer2D[8];
                facePointBuffer2D[1] = pointBuffer2D[9];
                facePointBuffer2D[2] = pointBuffer2D[10];
                facePointBuffer2D[3] = pointBuffer2D[11];
                facePointBuffer2D[4] = pointBuffer2D[2];
                facePointBuffer2D[5] = pointBuffer2D[3];
                facePointBuffer2D[6] = pointBuffer2D[0];
                facePointBuffer2D[7] = pointBuffer2D[1];
                break;

            case 4 :  
                facePointBuffer2D[0] = pointBuffer2D[10];
                facePointBuffer2D[1] = pointBuffer2D[11];
                facePointBuffer2D[2] = pointBuffer2D[2];
                facePointBuffer2D[3] = pointBuffer2D[3];
                facePointBuffer2D[4] = pointBuffer2D[6];
                facePointBuffer2D[5] = pointBuffer2D[7];
                facePointBuffer2D[6] = pointBuffer2D[14];
                facePointBuffer2D[7] = pointBuffer2D[15];
                break;

            case 5 :  
                facePointBuffer2D[0] = pointBuffer2D[4];
                facePointBuffer2D[1] = pointBuffer2D[5];
                facePointBuffer2D[2] = pointBuffer2D[6];
                facePointBuffer2D[3] = pointBuffer2D[7];
                facePointBuffer2D[4] = pointBuffer2D[14];
                facePointBuffer2D[5] = pointBuffer2D[15];
                facePointBuffer2D[6] = pointBuffer2D[12];
                facePointBuffer2D[7] = pointBuffer2D[13];
                break;
            }

        HFCPtr<HGF2DCoordSys> pDummyCoordSys(new HGF2DCoordSys());

        HVE2DPolygonOfSegments ReprojectedTile(8, facePointBuffer2D, pDummyCoordSys);       
        
        *rootToViewScaleP = max(*rootToViewScaleP, ReprojectedTile.CalculateArea());
        }
        
    return 0;
    }


int bcdtmMultiResolution_getMaxFaceAreaInCurrentViewCamOn(double& rootToViewScaleP, double& vanishingLineCutCorrectionFactor, double rootToViewMatrix[][4], DPoint3d* facePts, int nbFacePts)
    {   
    //NEEDS_WORK_SM : Query by level currently deactivated
    //assert((rootToViewMatrix[3][0] != 0.0) || (rootToViewMatrix[3][1] != 0.0) || (rootToViewMatrix[3][2] != 0.0));

    //Rectangular prism
    assert(nbFacePts == 8);
        
    rootToViewScaleP = 0;

    HArrayAutoPtr<double> facePointBuffer2D(new double[4 * 2]);

    //NEEDS_WORK_SM : Could be more generic (no magic 6 number)    
    for (size_t faceInd = 0; faceInd < 1; faceInd++)
        {        
        vector<DPoint3d> currentFacePts; 
        
        switch (faceInd)        
            {
            case 0 :     
                currentFacePts.push_back(facePts[0]);
                currentFacePts.push_back(facePts[1]);
                currentFacePts.push_back(facePts[3]);
                currentFacePts.push_back(facePts[2]);                
                break;

            case 1 :    
                currentFacePts.push_back(facePts[4]);
                currentFacePts.push_back(facePts[5]);
                currentFacePts.push_back(facePts[7]);
                currentFacePts.push_back(facePts[6]);                                
                break;

            case 2 :    
                currentFacePts.push_back(facePts[0]);
                currentFacePts.push_back(facePts[2]);
                currentFacePts.push_back(facePts[6]);
                currentFacePts.push_back(facePts[4]);                                
                break;

            case 3 :    
                currentFacePts.push_back(facePts[4]);
                currentFacePts.push_back(facePts[5]);
                currentFacePts.push_back(facePts[1]);
                currentFacePts.push_back(facePts[0]);                                                
                break;

            case 4 :  
                currentFacePts.push_back(facePts[5]);
                currentFacePts.push_back(facePts[1]);
                currentFacePts.push_back(facePts[3]);
                currentFacePts.push_back(facePts[7]);
                break;

            case 5 :  
                currentFacePts.push_back(facePts[2]);
                currentFacePts.push_back(facePts[3]);
                currentFacePts.push_back(facePts[7]);
                currentFacePts.push_back(facePts[6]);                
                break;
            }

        double currentFaceVanishingLineCutCorrectionFactor = 1.0;        
                
        vector<DPoint3d> shapeInFrontOfProjectivePlane;        

        int status = GetShapeInFrontOfProjectivePlane(shapeInFrontOfProjectivePlane, 
                                                      currentFaceVanishingLineCutCorrectionFactor,
                                                      currentFacePts, 
                                                      rootToViewMatrix);

        assert(status == SUCCESS);

        //The tile was cut by the projective plane.
        if (currentFaceVanishingLineCutCorrectionFactor != 1.0)
            {
            currentFacePts.clear();
            currentFacePts.insert(currentFacePts.begin(), shapeInFrontOfProjectivePlane.begin(), shapeInFrontOfProjectivePlane.end());          
            }

        HArrayAutoPtr<double> pointBuffer2D(new double[currentFacePts.size() * 2]);
        double                div;

        for (size_t facePtInd = 0; facePtInd < currentFacePts.size(); facePtInd++)
            {            
            pointBuffer2D[facePtInd * 2] = rootToViewMatrix[0][0] * currentFacePts[facePtInd].x + rootToViewMatrix[0][1] * currentFacePts[facePtInd].y + rootToViewMatrix[0][2] * currentFacePts[facePtInd].z + rootToViewMatrix[0][3];
            pointBuffer2D[facePtInd * 2 + 1] = rootToViewMatrix[1][0] * currentFacePts[facePtInd].x + rootToViewMatrix[1][1] * currentFacePts[facePtInd].y + rootToViewMatrix[1][2] * currentFacePts[facePtInd].z + rootToViewMatrix[1][3];
            div = rootToViewMatrix[3][0] * currentFacePts[facePtInd].x + rootToViewMatrix[3][1] * currentFacePts[facePtInd].y + rootToViewMatrix[3][2] * currentFacePts[facePtInd].z + rootToViewMatrix[3][3];

            pointBuffer2D[facePtInd * 2] /= div;
            pointBuffer2D[facePtInd * 2 + 1] /= div;       
            }                            
                                              
        HFCPtr<HGF2DCoordSys> pDummyCoordSys(new HGF2DCoordSys());

        HVE2DPolygonOfSegments ReprojectedTile(8, pointBuffer2D, pDummyCoordSys);       
        
        if (rootToViewScaleP <  ReprojectedTile.CalculateArea())
            {
            vanishingLineCutCorrectionFactor = currentFaceVanishingLineCutCorrectionFactor;
            rootToViewScaleP = ReprojectedTile.CalculateArea();
            }        
        }
        
    return 0;
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmMultiResolution_elevationDifferenceCompareFunction(const void *p1P,const void *p2P)
/*
** Compare Function For Qsort Of Void Points
*/
{
 struct ElevDifference { double elevation ; long point ; } *ed1P,*ed2P ;
 ed1P = ( struct ElevDifference * ) p1P ;
 ed2P = ( struct ElevDifference * ) p2P ;
 if     (  ed1P->elevation   <  ed2P->elevation  ) return(-1) ;
 else if(  ed1P->elevation   >  ed2P->elevation  ) return( 1)  ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmMultiResolution_tinDecimateRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object Containing points To Be Filtered       */
 long   filterOption,                  /* ==> < 1-Least Squares Plane , 2 - Average >                      */
 long   boundaryOption,                /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */
 long   numPointsRemove,               /* ==> Number Of Points To Remove                                   */ 
 long   *numFilteredPtsP,              /* <== Number Of Points After Filter                                */
 BC_DTM_OBJ *filteredPtsP              /* <== Pointer To DTM Containing The Filtered Points                */
)
{
#if 0
 int  ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long n,m,node1,node2,point,point1,clPtr,numElevationPoints,numPointDiffs ; 
 long firstPoint,lastPoint,saveLastPoint,usePlane=TRUE,excludeBoundary=TRUE ;
 double elevation ;
 DPoint3d dtmPoint ;
 DTM_TIN_NODE  *nodeP ;
 DTM_TIN_POINT *pointP ;
 DTM_CIR_LIST  *clistP ;
 BC_DTM_OBJ    *planePtsP=NULL ;
 DTM_PLANE plane ;
 struct ElevDifference { double elevation ; long point ; } *elevDiffP = NULL;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tin Decimating Random Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"filterOption      = %8ld",filterOption) ;
    bcdtmWrite_message(0,0,0,"boundaryOption    = %8ld",boundaryOption) ;
    bcdtmWrite_message(0,0,0,"numFilteredPtsP   = %8ld",*numFilteredPtsP) ;
    bcdtmWrite_message(0,0,0,"filteredPtsP      = %p",*filteredPtsP) ;
   }
/*
** Initialise
*/
 *numFilteredPtsP = 0 ;
/*
** Check Parameters
*/
 if( filterOption < 1 || filterOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Filter Option") ;
    goto errexit ;
   }
 if( boundaryOption < 1 || boundaryOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Boundary Option") ;
    goto errexit ;
   }
/*
** Set Program Options
*/
 if( filterOption == 2 ) usePlane = FALSE ;
 if( boundaryOption == 2 ) excludeBoundary = FALSE ;
/*
** Check For Valid DTM
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
 if( bcdtmObject_testForValidDtmObject(filteredPtsP)) goto errexit ;
/*
** Check For Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   } 
/*
** Initialise
*/
 if( bcdtmObject_initialiseDtmObject(filteredPtsP)) goto errexit ; 
/*
** Sort Points On X Axis
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"X Axis Sorting Random Spots") ;
    firstPoint = 0 ;
    saveLastPoint = lastPoint  = dtmP->numPoints - 1 ;
    if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
**  Remove Duplicate Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
    saveLastPoint = lastPoint ;
    if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
    if( saveLastPoint-lastPoint > 0 )
      {
       bcdtmWrite_message(1,0,0,"Duplicate points Found While Filtering") ;
       goto errexit ; 
      }
   }
/*
** Triangulate Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ;
 dtmP->edgeOption = 2 ;
 dtmP->ppTol = dtmP->plTol = 0.0  ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Create DTM To Store Plane Points
*/
 if( usePlane == TRUE )
   {
    if( bcdtmObject_createDtmObject(&planePtsP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(planePtsP,100,100) ;
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Scan All Internal Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 numPointDiffs = 0 ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    nodeP = nodeAddrP(dtmP,point) ;
/*
**  Least Squares Plane Filter Point
*/
    if( usePlane == TRUE )
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          planePtsP->numPoints = 0 ;
          clPtr = nodeP->cPtr ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
/*
**           Store Point In DTM Object
*/
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Point") ;
             pointP = pointAddrP(dtmP,point1) ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(planePtsP,DTMFeatureType::RandomSpots,planePtsP->nullUserTag,1,&planePtsP->nullFeatureId,(DPoint3d *)pointP,1)) goto errexit ;
            }
/*
**        Calculate Least Squares Plane Through Points
*/
          if( planePtsP->numPoints > 3 )
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Calculating Plane") ;
             if( bcdtmFilter_findPlaneDtmObject(planePtsP,0,planePtsP->numPoints,&plane) ) goto errexit ;
             pointP = pointAddrP(dtmP,point) ;
             elevation = pointP->z - ( plane.A * pointP->x + plane.B * pointP->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(elevation) ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            }
          else
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Setting To Max Elevation Difference") ;
             (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            } 
         }
       else
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Setting To Max Elevation Difference") ;
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
/*
**  Average Filter Point
*/
    else
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          clPtr = nodeP->cPtr ;
          elevation = 0.0 ;
          numElevationPoints = 0 ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             elevation = elevation + pointAddrP(dtmP,point1)->z ;
             ++numElevationPoints ;
            } 
          elevation = elevation / ( double ) numElevationPoints ;
          elevation = fabs(elevation - pointAddrP(dtmP,point)->z ) ;
          (elevDiffP+numPointDiffs)->elevation = elevation ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         }
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
   }
/*
** Check Consistency Of points Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointDiffs = %8ld",numPointDiffs) ;
 if( numPointDiffs != dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Inconsistent Number Of Point Differences") ;
    goto errexit ;
   }
/*
**  Quick Sort Elevation Difference Structure
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
 qsort(elevDiffP,numPointDiffs,sizeof(struct ElevDifference),bcdtmMultiResolution_elevationDifferenceCompareFunction) ;
/*
** Mark All Points That Can Be Removed And Copy All Filtered Points To Filtered Points DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 for( n = 0 ; n < numPointsRemove && n < numPointDiffs ; ++n )
   {
    nodeAddrP(dtmP,(elevDiffP+n)->point)->tPtr = 1 ;
    pointP = pointAddrP(dtmP,(elevDiffP+n)->point) ;
    dtmPoint.x = pointP->x ;
    dtmPoint.y = pointP->y ;
    dtmPoint.z = pointP->z ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(filteredPtsP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&dtmPoint,1)) goto errexit ;
   }
/*
**  Remove Deleted Points From DTM
*/
 for( node1 = node2 = 0 ; node2 < dtmP->numPoints ; ++node2 )
   {
    if( nodeAddrP(dtmP,node2)->tPtr == dtmP->nullPnt )
      {
       if( node2 != node1 )
         {
          *pointAddrP(dtmP,node1) = *pointAddrP(dtmP,node2) ;
         }
        ++node1 ;
       }
    } 
  dtmP->numPoints = node1 ;
  if( dbg ) bcdtmWrite_message(0,0,0,"After Removing Filtered Points ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Free Nodes memory
*/
 if( dtmP->nodesPP != NULL ) 
   { 
    for( m = 0 ; m < dtmP->numPointPartitions ; ++m ) free(dtmP->nodesPP[m]) ;
    free( dtmP->nodesPP) ;
    dtmP->nodesPP = NULL  ; 
    dtmP->numNodePartitions = 0 ;
    dtmP->numNodes = 0 ;
    dtmP->memNodes = 0 ;
   }
/*
** Free Circular List Memory
*/
 if( dtmP->cListPP != NULL ) 
   { 
    for( m = 0 ; m < dtmP->numClistPartitions ; ++m ) free(dtmP->cListPP[m]) ;
    free( dtmP->cListPP) ;
    dtmP->cListPP = NULL  ; 
    dtmP->numClistPartitions = 0 ;
    dtmP->numClist = 0 ;
    dtmP->memClist = 0 ;
    dtmP->cListPtr = 0 ;
    dtmP->cListDelPtr = dtmP->nullPtr ;
   }
/*
**  Reset DTM Header Values
*/
 dtmP->dtmState        = DTMState::Data ;
 dtmP->hullPoint       = dtmP->nullPnt ;
 dtmP->nextHullPoint   = dtmP->nullPnt ;
 dtmP->numSortedPoints = 0 ;
 dtmP->numLines        = 0 ;
 dtmP->numTriangles    = 0 ;
/*
**  Set Number Of Spots
*/
 *numFilteredPtsP = filteredPtsP->numPoints ;  
/*
** Clean Up
*/
 cleanup :
 if( planePtsP != NULL ) bcdtmObject_destroyDtmObject(&planePtsP) ;
 if( elevDiffP != NULL ) free(elevDiffP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Decimating Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Decimating Random Spots Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
 #else
 return DTM_ERROR;
 #endif
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmMultiResolution_tileDecimateRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object          */
 long   numPointsRemove,               /* ==> Number Of Points To Remove     */ 
 long   *numFilteredSpotsP,            /* <== Number Of Spots After Filter   */
 BC_DTM_OBJ *filteredDtmP              /* <== Pointer To DTM Object With The Filtered Points   */
)
{
#if 0
 int  ret=DTM_SUCCESS,dbg=0,tdbg=0 ;
 long n,node1,node2,numMarks,saveLastPoint,numPointDiffs ; 
 long pnt,firstPoint,lastPoint,maxTilePts,minTilePts,startTime ;
 long tile,numTiles;
 unsigned char *pointMarkP=NULL ;
 double dZ,xMin,yMin,zMin,xMax,yMax,zMax ;
 struct ElevDifference { double elevation ; long point ; } *eldP,*elevDiffP=NULL ;
 DTM_TIN_POINT *pntP ;
 DTM_PLANE plane ;
 DTM_POINT_TILE *pointTilesP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tile Decimating Random Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"numPointsRemove   = %8ld",numPointsRemove) ;
    bcdtmWrite_message(0,0,0,"numFilteredSpotsP = %8ld",*numFilteredSpotsP) ;
   }
/*
** Initialise
*/
 *numFilteredSpotsP = 0 ;
 firstPoint = 0 ;
 lastPoint  = dtmP->numPoints - 1 ;
/*
** Sort Points On X Axis
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"X Axis Sorting Random Spots") ;
 if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
** Remove Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
 saveLastPoint = lastPoint ;
 if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"firstPoint = %8ld ** lastPoint = %8ld",firstPoint,lastPoint) ;
 if( saveLastPoint-lastPoint > 0 )
   {
    bcdtmWrite_message(1,0,0,"Duplicate points Found While Filtering") ;
//    goto errexit ; 
   }
/*
**  Tile Dtm Points
*/
 startTime = bcdtmClock() ;
 maxTilePts = 10 ;
 minTilePts = 5 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tiling XYZ Points Into Tiles With A Maximum Of %8ld Points Per Tile",maxTilePts) ;
 if( bcdtmMedianTile_pointsDtmObject(dtmP,NULL,0,dtmP->numPoints,minTilePts,&pointTilesP,&numTiles)) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Tile %8ld Points Into %8ld Tiles = %8.3lf Secs",dtmP->numPoints,numTiles,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
** Write Tiles
*/
 if( dbg == 2 )
   {
    for( tile = 0 ; tile  < numTiles ; ++tile )
      {
       pntP = pointAddrP(dtmP,(pointTilesP+tile)->tileOffset) ;
       xMin = xMax = pntP->x ;
       yMin = yMax = pntP->y ;
       zMin = zMax = pntP->z ;
       for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
         {
          pntP = pointAddrP(dtmP,pnt) ;
          if( pntP->x < xMin ) xMin = pntP->x ;
          if( pntP->x > xMax ) xMax = pntP->x ;
          if( pntP->y < yMin ) yMin = pntP->y ;
          if( pntP->y > yMax ) yMax = pntP->y ;
          if( pntP->z < zMin ) zMin = pntP->z ;
          if( pntP->z > zMax ) zMax = pntP->z ;
         }
       bcdtmWrite_message(0,0,0,"tile[%4ld] ** startPoint = %8ld numPoints = %8ld ** xMin = %12.5lf xMax = %12.5lf ** yMin = %12.5lf yMax = %12.5lf ** zMin = %10.4lf zMax = %10.4lf",tile,(pointTilesP+tile)->tileOffset,(pointTilesP+tile)->numTilePts,xMin,xMax,yMin,yMax,zMin,zMax) ;
      }
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Filter Tiles
*/
 numPointDiffs = 0 ;
 for( tile = 0 ; tile  < numTiles ; ++tile )
   {
/*
**  Calculate Least Squares Plane Through The Tile Points
*/
    if( (pointTilesP+tile)->numTilePts > 3 )
      {
       plane.A = 0.0 ;
       plane.B = 0.0 ;
       plane.C = 0.0 ;  
       if( bcdtmFilter_findPlaneDtmObject(dtmP,(pointTilesP+tile)->tileOffset,(pointTilesP+tile)->numTilePts,&plane) == DTM_SUCCESS )
         {
          for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
            {
             dZ = pointAddrP(dtmP,pnt)->z - ( plane.A * pointAddrP(dtmP,pnt)->x + plane.B * pointAddrP(dtmP,pnt)->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(dZ) ;
             (elevDiffP+numPointDiffs)->point     = pnt ;
             ++numPointDiffs ;
            }
         }
      }
    else
      {
       for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = pnt ;
           ++numPointDiffs ;
         }
      }  
   }
/*
**  Quick Sort Elevation Difference Structure
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
 qsort(elevDiffP,numPointDiffs,sizeof(struct ElevDifference),bcdtmMultiResolution_elevationDifferenceCompareFunction) ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
    for( eldP = elevDiffP ; eldP < elevDiffP + numPointDiffs ; ++eldP )
      {
       bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** point = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->point) ;
      } 
   }
 if( dbg )  bcdtmWrite_message(0,0,0,"Median Elevation Point Difference = %10.4lf",(elevDiffP+numPointsRemove-1)->elevation) ;
/*
** Allocate Memory For Marking Points For Removal
*/
 numMarks = dtmP->numPoints / 8 + 1 ;
 pointMarkP = ( unsigned char *) malloc(numMarks*sizeof(unsigned char)) ;
 if( pointMarkP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for(unsigned char* cP = pointMarkP ; cP < pointMarkP + numMarks ; ++cP ) *cP = ( unsigned char ) 255 ;
/*
** Mark All Points That Can Be Removed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
 for( n = 0 ; n < numPointsRemove && n < numPointDiffs ; ++n )
   {
    bcdtmFlag_clearFlag(pointMarkP,(elevDiffP+n)->point) ;
   }
/*
** Remove Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Marked Points") ;
 for( node1 = node2 = 0 ; node2 < dtmP->numPoints ; ++node2 )
   {
    if( bcdtmFlag_testFlag(pointMarkP,node2) )
      {
       if( node2 != node1 )
         {
          *pointAddrP(dtmP,node1) = *pointAddrP(dtmP,node2) ;
         }
        ++node1 ;
       }
     else
       {
        if( bcdtmObject_storeDtmFeatureInDtmObject(filteredDtmP,DTMFeatureType::RandomSpots,filteredDtmP->nullUserTag,1,&filteredDtmP->nullFeatureId,(DPoint3d *)pointAddrP(dtmP,node2),1)) goto errexit ;
       } 
    } 
  dtmP->numPoints = node1 ;
  if( dbg ) bcdtmWrite_message(0,0,0,"After Removing Filtered Points ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Clean Up
*/
 cleanup :
 if( elevDiffP   != NULL ) free(elevDiffP) ;
 if( pointTilesP != NULL ) free(pointTilesP) ;
 if( pointMarkP  != NULL ) free(pointMarkP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Decimating Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Decimating Random Spots Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
 #else 
 return DTM_ERROR;
 #endif
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmMultiResolution_tileZToleranceFilterRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object          */
 double filterTolerance,               /* ==> Number Of Points To Remove     */ 
 long   *numFilteredSpotsP,            /* <== Number Of Spots After Filter   */
 BC_DTM_OBJ *filteredDtmP              /* <== Pointer To DTM Object With The Filtered Points   */
)
{
#if 0
 int  ret=DTM_SUCCESS,dbg=0,tdbg=0 ;
 long n,node1,node2,numMarks,saveLastPoint,numPointDiffs ; 
 long pnt,firstPoint,lastPoint,maxTilePts,minTilePts,startTime ;
 long tile,numTiles;
 unsigned char* pointMarkP=NULL ;
 double dZ,xMin,yMin,zMin,xMax,yMax,zMax ;
 struct ElevDifference { double elevation ; long point ; } *eldP,*elevDiffP=NULL ;
 DTM_TIN_POINT *pntP ;
 DTM_PLANE plane ;
 DTM_POINT_TILE *pointTilesP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tile Z Tolerance Filter Random Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"filterTolerance   = %8ld",filterTolerance) ;
    bcdtmWrite_message(0,0,0,"numFilteredSpotsP = %8ld",*numFilteredSpotsP) ;
   }
/*
** Initialise
*/
 *numFilteredSpotsP = 0 ;
 firstPoint = 0 ;
 lastPoint  = dtmP->numPoints - 1 ;
/*
** Sort Points On X Axis
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"X Axis Sorting Random Spots") ;
 if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
** Remove Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
 saveLastPoint = lastPoint ;
 if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"firstPoint = %8ld ** lastPoint = %8ld",firstPoint,lastPoint) ;
 if( saveLastPoint-lastPoint > 0 )
   {
    bcdtmWrite_message(1,0,0,"Duplicate points Found While Filtering") ;
//    goto errexit ; 
   }
/*
**  Tile Dtm Points
*/
 startTime = bcdtmClock() ;
 maxTilePts = 10 ;
 minTilePts = 5 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tiling XYZ Points Into Tiles With A Maximum Of %8ld Points Per Tile",maxTilePts) ;
 if( bcdtmMedianTile_pointsDtmObject(dtmP,NULL,0,dtmP->numPoints,minTilePts,&pointTilesP,&numTiles)) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Tile %8ld Points Into %8ld Tiles = %8.3lf Secs",dtmP->numPoints,numTiles,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
** Write Tiles
*/
 if( dbg == 2 )
   {
    for( tile = 0 ; tile  < numTiles ; ++tile )
      {
       pntP = pointAddrP(dtmP,(pointTilesP+tile)->tileOffset) ;
       xMin = xMax = pntP->x ;
       yMin = yMax = pntP->y ;
       zMin = zMax = pntP->z ;
       for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
         {
          pntP = pointAddrP(dtmP,pnt) ;
          if( pntP->x < xMin ) xMin = pntP->x ;
          if( pntP->x > xMax ) xMax = pntP->x ;
          if( pntP->y < yMin ) yMin = pntP->y ;
          if( pntP->y > yMax ) yMax = pntP->y ;
          if( pntP->z < zMin ) zMin = pntP->z ;
          if( pntP->z > zMax ) zMax = pntP->z ;
         }
       bcdtmWrite_message(0,0,0,"tile[%4ld] ** startPoint = %8ld numPoints = %8ld ** xMin = %12.5lf xMax = %12.5lf ** yMin = %12.5lf yMax = %12.5lf ** zMin = %10.4lf zMax = %10.4lf",tile,(pointTilesP+tile)->tileOffset,(pointTilesP+tile)->numTilePts,xMin,xMax,yMin,yMax,zMin,zMax) ;
      }
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Filter Tiles
*/
 numPointDiffs = 0 ;
 for( tile = 0 ; tile  < numTiles ; ++tile )
   {
/*
**  Calculate Least Squares Plane Through The Tile Points
*/
    if( (pointTilesP+tile)->numTilePts > 3 )
      {
       plane.A = 0.0 ;
       plane.B = 0.0 ;
       plane.C = 0.0 ;  
       if( bcdtmFilter_findPlaneDtmObject(dtmP,(pointTilesP+tile)->tileOffset,(pointTilesP+tile)->numTilePts,&plane) == DTM_SUCCESS )
         {
          for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
            {
             dZ = pointAddrP(dtmP,pnt)->z - ( plane.A * pointAddrP(dtmP,pnt)->x + plane.B * pointAddrP(dtmP,pnt)->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(dZ) ;
             (elevDiffP+numPointDiffs)->point     = pnt ;
             ++numPointDiffs ;
            }
         }
      }
    else
      {
       for( pnt = (pointTilesP+tile)->tileOffset ; pnt < (pointTilesP+tile)->tileOffset + (pointTilesP+tile)->numTilePts ; ++pnt )
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = pnt ;
           ++numPointDiffs ;
         }
      }  
   }
/*
**  Quick Sort Elevation Difference Structure
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
 qsort(elevDiffP,numPointDiffs,sizeof(struct ElevDifference),bcdtmMultiResolution_elevationDifferenceCompareFunction) ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
    for( eldP = elevDiffP ; eldP < elevDiffP + numPointDiffs ; ++eldP )
      {
       bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** point = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->point) ;
      } 
   }
/*
** Allocate Memory For Marking Points For Removal
*/
 numMarks = dtmP->numPoints / 8 + 1 ;
 pointMarkP = ( unsigned char *) malloc(numMarks*sizeof(unsigned char)) ;
 if( pointMarkP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for(unsigned char* cP = pointMarkP ; cP < pointMarkP + numMarks ; ++cP ) *cP = ( unsigned char ) 255 ;
/*
** Mark All Points That Can Be Removed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
 for( n = 0 ; n < numPointDiffs ; ++n )
   {
    if( (elevDiffP+n)->elevation <= filterTolerance )
      {
       bcdtmFlag_clearFlag(pointMarkP,(elevDiffP+n)->point) ;
      }  
   }
/*
** Remove Marked Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Marked Points") ;
 for( node1 = node2 = 0 ; node2 < dtmP->numPoints ; ++node2 )
   {
    if( bcdtmFlag_testFlag(pointMarkP,node2) )
      {
       if( node2 != node1 )
         {
          *pointAddrP(dtmP,node1) = *pointAddrP(dtmP,node2) ;
         }
        ++node1 ;
       }
     else
       {
        if( bcdtmObject_storeDtmFeatureInDtmObject(filteredDtmP,DTMFeatureType::RandomSpots,filteredDtmP->nullUserTag,1,&filteredDtmP->nullFeatureId,(DPoint3d *)pointAddrP(dtmP,node2),1)) goto errexit ;
       } 
    } 
  dtmP->numPoints = node1 ;
  if( dbg ) bcdtmWrite_message(0,0,0,"After Removing Filtered Points ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Clean Up
*/
 cleanup :
 if( elevDiffP   != NULL ) free(elevDiffP) ;
 if( pointTilesP != NULL ) free(pointTilesP) ;
 if( pointMarkP  != NULL ) free(pointMarkP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Z Tolerance Filter Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Z Tolerance Filter Random Spots Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
 #else
 return DTM_ERROR;
 #endif
}
#if 0
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmMultiResolution_tinZToleranceFilterRandomSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object Containing points To Be Filtered       */
 long   filterOption,                  /* ==> < 1-Least Squares Plane , 2 - Average >                      */
 long   boundaryOption,                /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */
 double filterTolerance,               /* ==> Filter Tolerance                                             */ 
 long   *numFilteredPtsP,              /* <== Number Of Points After Filter                                */
 BC_DTM_OBJ *filteredPtsP              /* <== Pointer To DTM Containing The Filtered Points                */
)
{
 int  ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long n,m,node1,node2,point,point1,clPtr,numElevationPoints,numPointDiffs ; 
 long firstPoint,lastPoint,saveLastPoint,usePlane=TRUE,excludeBoundary=TRUE ;
 double elevation ;
 DPoint3d dtmPoint ;
 DTM_TIN_NODE  *nodeP ;
 DTM_TIN_POINT *pointP ;
 DTM_CIR_LIST  *clistP ;
 BC_DTM_OBJ    *planePtsP=NULL ;
 DTM_PLANE plane ;
 struct ElevDifference { double elevation ; long point ; } *elevDiffP = NULL;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tin Z Tolerance Filter Random Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"filterOption      = %8ld",filterOption) ;
    bcdtmWrite_message(0,0,0,"boundaryOption    = %8ld",boundaryOption) ;
    bcdtmWrite_message(0,0,0,"filterTolerance   = %8.3lf",filterTolerance) ;
    bcdtmWrite_message(0,0,0,"numFilteredPtsP   = %8ld",*numFilteredPtsP) ;
    bcdtmWrite_message(0,0,0,"filteredPtsP      = %p",*filteredPtsP) ;
   }
/*
** Initialise
*/
 *numFilteredPtsP = 0 ;
 if( filterTolerance < 0.0 ) filterTolerance = 0.0 ;
/*
** Check Parameters
*/
 if( filterOption < 1 || filterOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Filter Option") ;
    goto errexit ;
   }
 if( boundaryOption < 1 || boundaryOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Boundary Option") ;
    goto errexit ;
   }
/*
** Set Program Options
*/
 if( filterOption == 2 ) usePlane = FALSE ;
 if( boundaryOption == 2 ) excludeBoundary = FALSE ;
/*
** Check For Valid DTM
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
 if( bcdtmObject_testForValidDtmObject(filteredPtsP)) goto errexit ;
/*
** Check For Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   } 
/*
** Initialise
*/
 if( bcdtmObject_initialiseDtmObject(filteredPtsP)) goto errexit ; 
/*
** Sort Points On X Axis
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"X Axis Sorting Random Spots") ;
    firstPoint = 0 ;
    saveLastPoint = lastPoint  = dtmP->numPoints - 1 ;
    if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
**  Remove Duplicate Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Random Spots") ;
    saveLastPoint = lastPoint ;
    if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
    if( saveLastPoint-lastPoint > 0 )
      {
       bcdtmWrite_message(1,0,0,"Duplicate points Found While Filtering") ;
       goto errexit ; 
      }
   }
/*
** Triangulate Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ;
 dtmP->edgeOption = 2 ;
 dtmP->ppTol = dtmP->plTol = 0.0  ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Create DTM To Store Plane Points
*/
 if( usePlane == TRUE )
   {
    if( bcdtmObject_createDtmObject(&planePtsP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(planePtsP,100,100) ;
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Scan All Internal Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 numPointDiffs = 0 ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    nodeP = nodeAddrP(dtmP,point) ;
/*
**  Least Squares Plane Filter Point
*/
    if( usePlane == TRUE )
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          planePtsP->numPoints = 0 ;
          clPtr = nodeP->cPtr ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
/*
**           Store Point In DTM Object
*/
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Point") ;
             pointP = pointAddrP(dtmP,point1) ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(planePtsP,DTMFeatureType::RandomSpots,planePtsP->nullUserTag,1,&planePtsP->nullFeatureId,(DPoint3d *)pointP,1)) goto errexit ;
            }
/*
**        Calculate Least Squares Plane Through Points
*/
          if( planePtsP->numPoints > 3 )
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Calculating Plane") ;
             if( bcdtmFilter_findPlaneDtmObject(planePtsP,0,planePtsP->numPoints,&plane) ) goto errexit ;
             pointP = pointAddrP(dtmP,point) ;
             elevation = pointP->z - ( plane.A * pointP->x + plane.B * pointP->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(elevation) ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            }
          else
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Setting To Max Elevation Difference") ;
             (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            } 
         }
       else
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Setting To Max Elevation Difference") ;
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
/*
**  Average Filter Point
*/
    else
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          clPtr = nodeP->cPtr ;
          elevation = 0.0 ;
          numElevationPoints = 0 ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             elevation = elevation + pointAddrP(dtmP,point1)->z ;
             ++numElevationPoints ;
            } 
          elevation = elevation / ( double ) numElevationPoints ;
          elevation = fabs(elevation - pointAddrP(dtmP,point)->z ) ;
          (elevDiffP+numPointDiffs)->elevation = elevation ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         }
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
   }
/*
** Check Consistency Of points Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointDiffs = %8ld",numPointDiffs) ;
 if( numPointDiffs != dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Inconsistent Number Of Point Differences") ;
    goto errexit ;
   }
/*
**  Quick Sort Elevation Difference Structure
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
 qsort(elevDiffP,numPointDiffs,sizeof(struct ElevDifference),bcdtmMultiResolution_elevationDifferenceCompareFunction) ;
/*
** Mark All Points That Can Be Removed And Copy All Filtered Points To Filtered Points DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 for( n = 0 ; n < numPointDiffs ; ++n )
   {
    if( (elevDiffP+n)->elevation <= filterTolerance )
      {
       nodeAddrP(dtmP,(elevDiffP+n)->point)->tPtr = 1 ;
       pointP = pointAddrP(dtmP,(elevDiffP+n)->point) ;
       dtmPoint.x = pointP->x ;
       dtmPoint.y = pointP->y ;
       dtmPoint.z = pointP->z ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(filteredPtsP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&dtmPoint,1)) goto errexit ;
      } 
   }
/*
**  Remove Deleted Points From DTM
*/
 for( node1 = node2 = 0 ; node2 < dtmP->numPoints ; ++node2 )
   {
    if( nodeAddrP(dtmP,node2)->tPtr == dtmP->nullPnt )
      {
       if( node2 != node1 )
         {
          *pointAddrP(dtmP,node1) = *pointAddrP(dtmP,node2) ;
         }
        ++node1 ;
       }
    } 
  dtmP->numPoints = node1 ;
  if( dbg ) bcdtmWrite_message(0,0,0,"After Removing Filtered Points ** dtmP->numPoints = %8ld",dtmP->numPoints) ;
/*
** Free Nodes memory
*/
 if( dtmP->nodesPP != NULL ) 
   { 
    for( m = 0 ; m < dtmP->numPointPartitions ; ++m ) free(dtmP->nodesPP[m]) ;
    free( dtmP->nodesPP) ;
    dtmP->nodesPP = NULL  ; 
    dtmP->numNodePartitions = 0 ;
    dtmP->numNodes = 0 ;
    dtmP->memNodes = 0 ;
   }
/*
** Free Circular List Memory
*/
 if( dtmP->cListPP != NULL ) 
   { 
    for( m = 0 ; m < dtmP->numClistPartitions ; ++m ) free(dtmP->cListPP[m]) ;
    free( dtmP->cListPP) ;
    dtmP->cListPP = NULL  ; 
    dtmP->numClistPartitions = 0 ;
    dtmP->numClist = 0 ;
    dtmP->memClist = 0 ;
    dtmP->cListPtr = 0 ;
    dtmP->cListDelPtr = dtmP->nullPtr ;
   }
/*
**  Reset DTM Header Values
*/
 dtmP->dtmState        = DTMState::Data ;
 dtmP->hullPoint       = dtmP->nullPnt ;
 dtmP->nextHullPoint   = dtmP->nullPnt ;
 dtmP->numSortedPoints = 0 ;
 dtmP->numLines        = 0 ;
 dtmP->numTriangles    = 0 ;
/*
**  Set Number Of Spots
*/
 *numFilteredPtsP = filteredPtsP->numPoints ;  
/*
** Clean Up
*/
 cleanup :
 if( planePtsP != NULL ) bcdtmObject_destroyDtmObject(&planePtsP) ;
 if( elevDiffP != NULL ) free(elevDiffP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Z Tolerance Filter Random Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Z Tolerance Filter Random Spots Error") ;
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
int bcdtmMultiResolution_tinZToleranceFilterGroupSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object           */
 long   filterOption,                  /* ==> < 1-Least Squares Plane , 2 - Average >                      */
 long   boundaryOption,                /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */
 double filterTolerance,               /* ==> Filter Tolerance                */ 
 long   *numFilteredPtsP,              /* <== Number Of Points After Filter   */
 BC_DTM_OBJ *filteredPtsP              /* <== Pointer To DTM Object With The Filtered Points   */
)
/*
** RobC - Note This Is A Special Purpose Multi Resolution Filtering Function And Should Not
** Be Used Out Of Its Current Context
*/
{
 int  ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long point,point1,tile,numPointDiffs ;
 long numPoints,memPoints=0,numFilteredPoints,numUnFilteredPoints ; 
 long firstPoint,dtmFeature,saveNumPoints,usePlane=TRUE,excludeBoundary=TRUE ; 
 long clPtr,numElevationPoints ;
 double elevation ;
 DPoint3d    *p3dP,*pointsP=NULL ;
 struct ElevDifference { double elevation ; long point ; } *eldP,*elevDiffP=NULL ;
 DTM_PLANE plane ;
 DTM_TIN_POINT *pointP ;
 DTM_TIN_NODE  *nodeP ;
 DTM_CIR_LIST  *clistP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 BC_DTM_OBJ *tempDtmP=NULL,*planePtsP=NULL ;
 DTMFeatureId dtmFeatureId ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tin Z Tolerance Filter Group Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;    
    bcdtmWrite_message(0,0,0,"dtmP->numPoints   = %8ld",dtmP->numPoints) ;    
    bcdtmWrite_message(0,0,0,"filterOption      = %8ld",filterOption) ;
    bcdtmWrite_message(0,0,0,"boundaryOption    = %8ld",boundaryOption) ;
    bcdtmWrite_message(0,0,0,"filterTolerance   = %8.3lf",filterTolerance) ;
    bcdtmWrite_message(0,0,0,"numFilteredPtsP   = %8ld",*numFilteredPtsP) ;
    bcdtmWrite_message(0,0,0,"filteredPtsP      = %p",filteredPtsP) ;    
   }
/*
** Check Parameters
*/
 if( filterTolerance < 0.0 ) filterTolerance = 0.0 ;
 if( filterOption < 1 || filterOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Filter Option") ;
    goto errexit ;
   }
 if( boundaryOption < 1 || boundaryOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Boundary Option") ;
    goto errexit ;
   }
/*
** Initialise
*/
 *numFilteredPtsP = 0 ;
 if( bcdtmObject_initialiseDtmObject(filteredPtsP)) goto errexit ;
 if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
 saveNumPoints = dtmP->numPoints ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numfeatures = %8ld",dtmP->numFeatures) ;
/*
** Check For Valid DTM
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   }  
/*
** Triangulate Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ;
 dtmP->edgeOption = 2 ;
 dtmP->ppTol = dtmP->plTol = 0.0  ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Check Triangulation Has Not Removed Duplicate Points
*/
 if( dtmP->numPoints != saveNumPoints )
   {
    bcdtmWrite_message(1,0,0,"Duplicate Points In Filter Data Set") ;
    goto errexit ;
   }
/*
** Create DTM To Store Plane Points
*/
 if( usePlane == TRUE )
   {
    if( bcdtmObject_createDtmObject(&planePtsP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(planePtsP,100,100) ;
   }
/*
** Allocate memory For Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Scan All Internal Tin Points And Determine Elevation Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 numPointDiffs = 0 ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    nodeP = nodeAddrP(dtmP,point) ;
/*
**  Least Squares Plane Filter Points
*/
    if( usePlane == TRUE )
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          planePtsP->numPoints = 0 ;
          clPtr = nodeP->cPtr ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
/*
**           Store Point In DTM Object
*/
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Point") ;
             pointP = pointAddrP(dtmP,point1) ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(planePtsP,DTMFeatureType::RandomSpots,planePtsP->nullUserTag,1,&planePtsP->nullFeatureId,(DPoint3d *)pointP,1)) goto errexit ;
            }
/*
**        Calculate Least Squares Plane Through Points
*/
          if( planePtsP->numPoints > 3 )
            {
             if( bcdtmFilter_findPlaneDtmObject(planePtsP,0,planePtsP->numPoints,&plane) ) goto errexit ;
             pointP = pointAddrP(dtmP,point) ;
             elevation = pointP->z - ( plane.A * pointP->x + plane.B * pointP->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(elevation) ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            }
          else
            {
             (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            } 
         }
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
/*
**  Average Filter points
*/
    else
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          clPtr = nodeP->cPtr ;
          elevation = 0.0 ;
          numElevationPoints = 0 ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             elevation = elevation + pointAddrP(dtmP,point1)->z ;
             ++numElevationPoints ;
            } 
          elevation = elevation / ( double ) numElevationPoints ;
          elevation = fabs(elevation - pointAddrP(dtmP,point)->z ) ;
          (elevDiffP+numPointDiffs)->elevation = elevation ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         }
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
   }
/*
** Check Consistency Of points Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointDiffs = %8ld",numPointDiffs) ;
 if( numPointDiffs != dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Inconsistent Number Of Point Differences") ;
//    goto errexit ;
   }
/*
** QSort Elevation Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Qsorting Elevation Differences") ;
 qsort(elevDiffP,numPointDiffs,sizeof(struct ElevDifference),bcdtmMultiResolution_elevationDifferenceCompareFunction) ;
/*
**  Mark All Points That Can Be Removed 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
 for( eldP = elevDiffP ; eldP < elevDiffP + numPointDiffs ; ++eldP )
   {
    if( eldP->elevation <= filterTolerance ) nodeAddrP(dtmP,eldP->point)->tPtr = eldP->point ;
   }
/*
** Count Number Of Filtered And Unfiltered Points
*/
 if( cdbg == 2 )
   {
    numPoints = numFilteredPoints = numUnFilteredPoints = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
         {
          firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
          do 
            {
             ++numPoints ;
             if( nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt ) ++numUnFilteredPoints ;
             else                                                    ++numFilteredPoints ;  
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
            } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
         }
       else
         {
          bcdtmWrite_message(1,0,0,"ERROR - Feature Not In Tin State") ;
         }
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPoints = %6ld ** numFilteredPoints = %8ld numUnfilteredPoints = %8ld",numPoints,numFilteredPoints,numUnFilteredPoints) ; 
   }
/*
** Copy Filtered And Unfiltered Points To Different DTMs
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Filtered And Unfiltered Points") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
/*
**     Count Number Of Filtered And Un Filtered Points 
*/      
       numPoints = numFilteredPoints = numUnFilteredPoints = 0 ;
       firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
       tile        = (long)dtmFeatureP->dtmUserTag ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Processing Tile %8ld",tile) ;  
       do
         {
          ++numPoints ;
          if( nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt ) ++numUnFilteredPoints ;
          else                                                    ++numFilteredPoints ;  
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
         } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
/*
**     Check memory
*/
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Checking memory") ;
       if( numPoints >= memPoints )
         {
          memPoints = numPoints * 2 ;
          if( pointsP == NULL ) pointsP = ( DPoint3d *) malloc( memPoints * sizeof(DPoint3d)) ;
          else                  pointsP = ( DPoint3d *) realloc(pointsP,memPoints * sizeof(DPoint3d)) ;
          if( pointsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            } 
         }   
/*
**     Store Filtered Points In Filtered Points In FilteredPts DTM
*/
       if( numFilteredPoints > 0 )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Filtered Points") ;
          p3dP = pointsP ;         
          firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
          do
            {
             if( nodeAddrP(dtmP,firstPoint)->tPtr != dtmP->nullPnt ) 
               {
                pointP = pointAddrP(dtmP,firstPoint) ;
                p3dP->x = pointP->x ;
                p3dP->y = pointP->y ;
                p3dP->z = pointP->z ;
                ++p3dP ;
               }
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
            } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(filteredPtsP,DTMFeatureType::GroupSpots,tile,3,&dtmFeatureId,pointsP,numFilteredPoints)) goto errexit ;
         }
/*
**     Store Un Filtered Points In Temp DTM
*/
       if( numUnFilteredPoints > 0 )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Un Filtered Points") ;
          p3dP = pointsP ;         
          firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
          tile        = (long)dtmFeatureP->dtmUserTag ;
          do
            {
             if( nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt ) 
               {
                pointP = pointAddrP(dtmP,firstPoint) ;
                p3dP->x = pointP->x ;
                p3dP->y = pointP->y ;
                p3dP->z = pointP->z ;
                ++p3dP ;
               }
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
            } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::GroupSpots,tile,3,&dtmFeatureId,pointsP,numUnFilteredPoints)) goto errexit ;
         }
      } 
   }
/*
**  Copy Temp DTM To DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Temp DTM") ;
 if( bcdtmObject_initialiseDtmObject(dtmP)) goto errexit ;
 if( tempDtmP->numPoints > 0 )
   {
    if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,tempDtmP->numPoints,tempDtmP->numPoints)) goto errexit ; 
    if( bcdtmObject_appendDtmObject(dtmP,tempDtmP)) goto errexit ;
   } 
/*
** Write Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Points Before Filter = %8ld",saveNumPoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Points After  Filter = %8ld",dtmP->numPoints+filteredPtsP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints         = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"filteredPtsP->numPoints = %8ld",filteredPtsP->numPoints) ;
    bcdtmWrite_toFileDtmObject(filteredPtsP,L"filteredPoints.dtm") ;
    bcdtmWrite_toFileDtmObject(dtmP,L"unFilteredPoints.dtm") ;
   }
/*
** Check That The Total Number Of Points Before And After Filter Are The Same
*/
 if( saveNumPoints != dtmP->numPoints+filteredPtsP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Warning ** Inconsistent Number Of Filter Points") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( elevDiffP   != NULL ) free(elevDiffP) ;
 if( pointsP     != NULL ) free(pointsP) ;
 if( tempDtmP    != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
 if( planePtsP   != NULL ) bcdtmObject_destroyDtmObject(&planePtsP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Z Tolerance Filter Group Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Z Tolerance Filter Group Spots Error") ;
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
int bcdtmMultiResolution_elevationDifferenceKeepCompareFunction(const void *p1P,const void *p2P)
/*
** Compare Function For Qsort Of Void Points
*/
{
 struct ElevDifference { double elevation ; long point ; long tile ; long keep ; } *ed1P,*ed2P ;
 ed1P = ( struct ElevDifference * ) p1P ;
 ed2P = ( struct ElevDifference * ) p2P ;
 if     (  ed1P->elevation < ed2P->elevation  ) return(-1) ;
 else if(  ed1P->elevation > ed2P->elevation  ) return( 1)  ;
 else if(  ed1P->tile < ed2P->tile  ) return(-1) ;
 else if(  ed1P->tile > ed2P->tile  ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmMultiResolution_elevationDifferenceTileCompareFunction(const void *p1P,const void *p2P)
/*
** Compare Function For Qsort Of Void Points
*/
{
 struct ElevDifference { double elevation ; long point ; long tile ; long keep ; } *ed1P,*ed2P ;
 ed1P = ( struct ElevDifference * ) p1P ;
 ed2P = ( struct ElevDifference * ) p2P ;
 if     (  ed1P->tile < ed2P->tile  ) return(-1) ;
 else if(  ed1P->tile > ed2P->tile  ) return( 1) ;
 else if(  ed1P->elevation < ed2P->elevation  ) return(-1) ;
 else if(  ed1P->elevation > ed2P->elevation  ) return( 1)  ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmMultiResolution_elevationDifferenceTileKeepCompareFunction(const void *p1P,const void *p2P)
/*
** Compare Function For Qsort Of Void Points
*/
{
 struct ElevDifference { double elevation ; long point ; long tile ; long keep ; } *ed1P,*ed2P ;
 ed1P = ( struct ElevDifference * ) p1P ;
 ed2P = ( struct ElevDifference * ) p2P ;
 if     (  ed1P->tile   <  ed2P->tile  ) return(-1) ;
 else if(  ed1P->tile   >  ed2P->tile  ) return( 1) ;
 else if(  ed1P->keep   <  ed2P->keep  ) return(-1) ;
 else if(  ed1P->keep   >  ed2P->keep  ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmMultiResolution_tinDecimateGroupSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object           */
 long   filterOption,                  /* ==> < 1-Least Squares Plane , 2 - Average >                      */
 long   boundaryOption,                /* ==> < 1-Do Not Filter Boundary Points 2-Filter Boundary Points > */
 long   numPointsRemove,               /* ==> Number Of Points To Remove      */ 
 long   *numFilteredPtsP,              /* <== Number Of Points After Filter   */
 BC_DTM_OBJ *filteredPtsP              /* <== Pointer To DTM Object With The Filtered Points   */
)
/*
** RobC - Note This Is A Special Purpose Multi Resolution Filtering Function And Should Not
** Be Used Out Of Its Current Context
*/
{
 int  ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 long point,point1,tile,numPointDiffs ;
 long numPoints,memPoints=0,numFilteredPoints,numUnFilteredPoints ; 
 long firstPoint,dtmFeature,saveNumPoints,usePlane=TRUE,excludeBoundary=TRUE ; 
 long clPtr,numElevationPoints ;
 double elevation ;
 DPoint3d    *p3dP,*pointsP=NULL ;
 struct ElevDifference { double elevation ; long point ; } *eldP,*elevDiffP=NULL ;
 DTM_PLANE plane ;
 DTM_TIN_POINT *pointP ;
 DTM_TIN_NODE  *nodeP ;
 DTM_CIR_LIST  *clistP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 BC_DTM_OBJ *tempDtmP=NULL,*planePtsP=NULL ;
 DTMFeatureId dtmFeatureId ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tin Decimating Group Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;    
    bcdtmWrite_message(0,0,0,"dtmP->numPoints   = %8ld",dtmP->numPoints) ;    
    bcdtmWrite_message(0,0,0,"filterOption      = %8ld",filterOption) ;
    bcdtmWrite_message(0,0,0,"boundaryOption    = %8ld",boundaryOption) ;
    bcdtmWrite_message(0,0,0,"numPointsRemove   = %8ld",numPointsRemove) ;
    bcdtmWrite_message(0,0,0,"numFilteredPtsP   = %8ld",*numFilteredPtsP) ;
    bcdtmWrite_message(0,0,0,"filteredPtsP      = %p",filteredPtsP) ;    
   }
/*
** Check Parameters
*/
 if( filterOption < 1 || filterOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Filter Option") ;
    goto errexit ;
   }
 if( boundaryOption < 1 || boundaryOption > 2 )
   {
    bcdtmWrite_message(1,0,0,"Illegal Boundary Option") ;
    goto errexit ;
   }
/*
** Initialise
*/
 *numFilteredPtsP = 0 ;
 if( bcdtmObject_initialiseDtmObject(filteredPtsP)) goto errexit ;
 if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
 saveNumPoints = dtmP->numPoints ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numfeatures = %8ld",dtmP->numFeatures) ;
/*
** Check For Valid DTM
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Data State
*/
 if( dtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   }  
/*
** Triangulate Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ;
 dtmP->edgeOption = 2 ;
 dtmP->ppTol = dtmP->plTol = 0.0  ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
/*
** Check Triangulation Has Not Removed Duplicate Points
*/
 if( dtmP->numPoints != saveNumPoints )
   {
    bcdtmWrite_message(1,0,0,"Duplicate Points In Filter Data Set") ;
    goto errexit ;
   }
/*
** Create DTM To Store Plane Points
*/
 if( usePlane == TRUE )
   {
    if( bcdtmObject_createDtmObject(&planePtsP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(planePtsP,100,100) ;
   }
/*
** Allocate memory For Elevation Differences
*/
 elevDiffP = ( struct ElevDifference * ) malloc( dtmP->numPoints * sizeof(struct ElevDifference)) ;
 if( elevDiffP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Scan All Internal Tin Points And Determine Elevation Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Points") ;
 numPointDiffs = 0 ;
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    nodeP = nodeAddrP(dtmP,point) ;
/*
**  Least Squares Plane Filter Points
*/
    if( usePlane == TRUE )
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          planePtsP->numPoints = 0 ;
          clPtr = nodeP->cPtr ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
/*
**           Store Point In DTM Object
*/
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Point") ;
             pointP = pointAddrP(dtmP,point1) ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(planePtsP,DTMFeatureType::RandomSpots,planePtsP->nullUserTag,1,&planePtsP->nullFeatureId,(DPoint3d *)pointP,1)) goto errexit ;
            }
/*
**        Calculate Least Squares Plane Through Points
*/
          if( planePtsP->numPoints > 3 )
            {
             if( bcdtmFilter_findPlaneDtmObject(planePtsP,0,planePtsP->numPoints,&plane) ) goto errexit ;
             pointP = pointAddrP(dtmP,point) ;
             elevation = pointP->z - ( plane.A * pointP->x + plane.B * pointP->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(elevation) ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            }
          else
            {
             (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
             (elevDiffP+numPointDiffs)->point     = point ;
             ++numPointDiffs ;
            } 
         }
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
/*
**  Average Filter points
*/
    else
      {
       if( nodeP->hPtr == dtmP->nullPnt || ( excludeBoundary == FALSE && nodeP->hPtr != dtmP->nullPnt ))
         {
          clPtr = nodeP->cPtr ;
          elevation = 0.0 ;
          numElevationPoints = 0 ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             point1 = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             elevation = elevation + pointAddrP(dtmP,point1)->z ;
             ++numElevationPoints ;
            } 
          elevation = elevation / ( double ) numElevationPoints ;
          elevation = fabs(elevation - pointAddrP(dtmP,point)->z ) ;
          (elevDiffP+numPointDiffs)->elevation = elevation ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         }
       else
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = point ;
          ++numPointDiffs ;
         } 
      }
   }
/*
** Check Consistency Of points Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointDiffs = %8ld",numPointDiffs) ;
 if( numPointDiffs != dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Inconsistent Number Of Point Differences") ;
//    goto errexit ;
   }
/*
** Adjust Number Of Points To Remove
*/
 if( numPointsRemove > numPointDiffs ) numPointsRemove = numPointDiffs ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointsRemove = %8ld",numPointsRemove) ;
/*
** QSort Elevation Differences
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Qsorting Elevation Differences") ;
 qsort(elevDiffP,numPointDiffs,sizeof(struct ElevDifference),bcdtmMultiResolution_elevationDifferenceCompareFunction) ;
/*
**  Mark All Points That Can Be Removed 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
 for( eldP = elevDiffP ; eldP < elevDiffP + numPointsRemove ; ++eldP )
   {
    nodeAddrP(dtmP,eldP->point)->tPtr = eldP->point ;
   }
/*
** Count Number Of Filtered And Unfiltered Points
*/
 if( cdbg == 2 )
   {
    numPoints = numFilteredPoints = numUnFilteredPoints = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
         {
          firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
          do 
            {
             ++numPoints ;
             if( nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt ) ++numUnFilteredPoints ;
             else                                                    ++numFilteredPoints ;  
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
            } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
         }
       else
         {
          bcdtmWrite_message(1,0,0,"ERROR - Feature Not In Tin State") ;
         }
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"numPoints = %6ld ** numFilteredPoints = %8ld numUnfilteredPoints = %8ld",numPoints,numFilteredPoints,numUnFilteredPoints) ; 
   }
/*
** Copy Filtered And Unfiltered Points To Different DTMs
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Filtered And Unfiltered Points") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
/*
**     Count Number Of Filtered And Un Filtered Points 
*/      
       numPoints = numFilteredPoints = numUnFilteredPoints = 0 ;
       firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
       tile        = (long)dtmFeatureP->dtmUserTag ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Processing Tile %8ld",tile) ;  
       do
         {
          ++numPoints ;
          if( nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt ) ++numUnFilteredPoints ;
          else                                                    ++numFilteredPoints ;  
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
         } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
/*
**     Check memory
*/
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Checking memory") ;
       if( numPoints >= memPoints )
         {
          memPoints = numPoints * 2 ;
          if( pointsP == NULL ) pointsP = ( DPoint3d *) malloc( memPoints * sizeof(DPoint3d)) ;
          else                  pointsP = ( DPoint3d *) realloc(pointsP,memPoints * sizeof(DPoint3d)) ;
          if( pointsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            } 
         }   
/*
**     Store Filtered Points In Filtered Points In FilteredPts DTM
*/
       if( numFilteredPoints > 0 )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Filtered Points") ;
          p3dP = pointsP ;         
          firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
          do
            {
             if( nodeAddrP(dtmP,firstPoint)->tPtr != dtmP->nullPnt ) 
               {
                pointP = pointAddrP(dtmP,firstPoint) ;
                p3dP->x = pointP->x ;
                p3dP->y = pointP->y ;
                p3dP->z = pointP->z ;
                ++p3dP ;
               }
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
            } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(filteredPtsP,DTMFeatureType::GroupSpots,tile,3,&dtmFeatureId,pointsP,numFilteredPoints)) goto errexit ;
         }
/*
**     Store Un Filtered Points In Temp DTM
*/
       if( numUnFilteredPoints > 0 )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Storing Un Filtered Points") ;
          p3dP = pointsP ;         
          firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
          tile        = (long)dtmFeatureP->dtmUserTag ;
          do
            {
             if( nodeAddrP(dtmP,firstPoint)->tPtr == dtmP->nullPnt ) 
               {
                pointP = pointAddrP(dtmP,firstPoint) ;
                p3dP->x = pointP->x ;
                p3dP->y = pointP->y ;
                p3dP->z = pointP->z ;
                ++p3dP ;
               }
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,firstPoint,&firstPoint)) goto errexit ;
            } while( firstPoint != dtmP->nullPnt && firstPoint != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::GroupSpots,tile,3,&dtmFeatureId,pointsP,numUnFilteredPoints)) goto errexit ;
         }
      } 
   }
/*
**  Copy Temp DTM To DTM
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Temp DTM") ;
 if( bcdtmObject_initialiseDtmObject(dtmP)) goto errexit ;
 if( tempDtmP->numPoints > 0 )
   {
    if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,tempDtmP->numPoints,tempDtmP->numPoints)) goto errexit ; 
    if( bcdtmObject_appendDtmObject(dtmP,tempDtmP)) goto errexit ;
   } 
/*
** Write Stats
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Points Before Filter = %8ld",saveNumPoints) ;
    bcdtmWrite_message(0,0,0,"Number Of Points After  Filter = %8ld",dtmP->numPoints+filteredPtsP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints         = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"filteredPtsP->numPoints = %8ld",filteredPtsP->numPoints) ;
    bcdtmWrite_toFileDtmObject(filteredPtsP,L"filteredPoints.dtm") ;
    bcdtmWrite_toFileDtmObject(dtmP,L"unFilteredPoints.dtm") ;
   }
/*
** Check That The Total Number Of Points Before And After Filter Are The Same
*/
 if( saveNumPoints != dtmP->numPoints+filteredPtsP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Warning ** Inconsistent Number Of Filter Points") ;
    goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 if( elevDiffP   != NULL ) free(elevDiffP) ;
 if( pointsP     != NULL ) free(pointsP) ;
 if( tempDtmP    != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
 if( planePtsP   != NULL ) bcdtmObject_destroyDtmObject(&planePtsP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Decimating Group Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tin Decimating Group Spots Error") ;
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
int bcdtmMultiResolution_tileDecimateGroupSpotsDtmObject
(
 BC_DTM_OBJ *dtmP,                     /* ==> Pointer To DTM Object          */
 long   numPointsRemove,               /* ==> Number Of Points To Remove     */ 
 long   *numFilteredSpotsP,            /* <== Number Of Spots After Filter   */
 BC_DTM_OBJ *filteredDtmP              /* <== Pointer To DTM Object With The Filtered Points   */
)
/*
** RobC - Note This Is A Special Multi Resolution Filtering Function And Should Not
** Be Used Out Of Its Current Context
*/
{
 int  ret=DTM_SUCCESS,dbg=0,tdbg=0,cdbg=0 ;
 long point,saveLastPoint,numPointDiffs,numPoints=0,memPoints=0,numRemovePoints ; 
 long pnt,firstPoint,lastPoint,maxTilePts,dtmFeature,startTime,lastTile,lastKeep ;
 long tile,numTiles,*tnumP,*tileNumberP=NULL,removeMethod=2 ;
 double dZ,xMin,yMin,zMin,xMax,yMax,zMax ;
 DPoint3d    *p3dP,*pointsP=NULL ;
 struct ElevDifferenceTile { double elevation ; long point ; long tile ; long keep ; } *eldP,*eld1P,*elevDiffP=NULL ;
 DTM_TIN_POINT *pntP ;
 DTM_PLANE plane ;
 BC_DTM_FEATURE *dtmFeatureP ;
 BC_DTM_OBJ *tempDtmP=NULL ;
 DTMFeatureId dtmFeatureId ;
 DTM_POINT_TILE *tilesP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Tile Decimate Group Spots") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;    
    bcdtmWrite_message(0,0,0,"numPointsRemove   = %8ld",numPointsRemove) ;
    bcdtmWrite_message(0,0,0,"numFilteredSpotsP = %8ld",*numFilteredSpotsP) ;
   }
/*
** Initialise
*/
 *numFilteredSpotsP = 0 ;
/*
** Allocate Memory To Store Point Tile Numbers
*/
 tileNumberP = ( long * ) malloc ( dtmP->numPoints * sizeof(long)) ;
 if( tileNumberP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
 if( cdbg ) for( tnumP = tileNumberP ; tnumP < tileNumberP + dtmP->numPoints ; ++tnumP ) *tnumP = -9999 ;
/*
** Populate Tile Number Array
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Populating Tile Number Array") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    firstPoint  = dtmFeatureP->dtmFeaturePts.firstPoint ;
    lastPoint   = firstPoint + dtmFeatureP->numDtmFeaturePts - 1 ;
    for( tnumP = tileNumberP + firstPoint ; tnumP <= tileNumberP + lastPoint ; ++tnumP )
      {
       *tnumP = (long ) dtmFeatureP->dtmUserTag ;
      } 
   }
/*
** Check All Points Have Been Allocated A Tile Number
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking All Points Have A Tile Number") ;
    for( tnumP = tileNumberP ; tnumP < tileNumberP + dtmP->numPoints ; ++tnumP )
      {
       if( *tnumP == -9999 ) 
         {
          bcdtmWrite_message(1,0,0,"Point %8ld Has Not Been Allocated A Tile Number",(long)(tnumP-tileNumberP)) ;
          goto errexit ;
         } 
      } 
   }
/*
** Sort Points On X Axis
*/
 firstPoint = 0 ;
 lastPoint  = dtmP->numPoints - 1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"X Axis Sorting Points") ;
 if( bcdtmFilter_sortTaggedPointRangeDtmObject(dtmP,tileNumberP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
** Remove Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Points") ;
 saveLastPoint = lastPoint ;
 if( bcdtmFilter_removeTaggedDuplicatePointsFromRangeDtmObject(dtmP,tileNumberP,firstPoint,&lastPoint)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
 if( saveLastPoint-lastPoint > 0 ) bcdtmWrite_message(0,0,0,"Warning **** Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ; 
/*
**  Tile Dtm Points
*/
 startTime = bcdtmClock() ;
 maxTilePts = 5 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tiling XYZ Points Into Tiles With A Maximum Of %8ld Points Per Tile",maxTilePts) ;
// if( bcdtmTile_pointsDtmObject(dtmP,tileNumberP,firstPoint,dtmP->numPoints,maxTilePts,&tilesP,&numTiles)) goto errexit ;
 if( bcdtmMedianTile_pointsDtmObject(dtmP,tileNumberP,firstPoint,dtmP->numPoints,maxTilePts,&tilesP,&numTiles)) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Tile %8ld Points Into %8ld Tiles = %8.3lf Secs",dtmP->numPoints,numTiles,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
** Write Tiles
*/
 if( dbg == 2 )
   {
    for( tile = 0 ; tile  < numTiles ; ++tile )
      {
       pntP = pointAddrP(dtmP,(tilesP+tile)->tileOffset) ;
       xMin = xMax = pntP->x ;
       yMin = yMax = pntP->y ;
       zMin = zMax = pntP->z ;
       for( pnt = (tilesP+tile)->tileOffset ; pnt < (tilesP+tile)->tileOffset + (tilesP+tile)->numTilePts ; ++pnt )
         {
          pntP = pointAddrP(dtmP,pnt) ;
          if( pntP->x < xMin ) xMin = pntP->x ;
          if( pntP->x > xMax ) xMax = pntP->x ;
          if( pntP->y < yMin ) yMin = pntP->y ;
          if( pntP->y > yMax ) yMax = pntP->y ;
          if( pntP->z < zMin ) zMin = pntP->z ;
          if( pntP->z > zMax ) zMax = pntP->z ;
         }
       bcdtmWrite_message(0,0,0,"tile[%4ld] ** startPoint = %8ld numPoints = %8ld ** xMin = %12.5lf xMax = %12.5lf ** yMin = %12.5lf yMax = %12.5lf ** zMin = %10.4lf zMax = %10.4lf",tile,(tilesP+tile)->tileOffset,(tilesP+tile)->numTilePts,xMin,xMax,yMin,yMax,zMin,zMax) ;
      }
   }
/*
** Allocate Memory To Store Point Elevation Differences
*/
 elevDiffP = ( struct ElevDifferenceTile * ) malloc ( dtmP->numPoints * sizeof(struct ElevDifferenceTile)) ;
 if( elevDiffP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Filter Tiles
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Least Square Filtering Tiles") ;
 numPointDiffs = 0 ;
 for( tile = 0 ; tile  < numTiles ; ++tile )
   {
/*
**  Calculate Least Squares Plane Through The Tile Points
*/
    if( (tilesP+tile)->numTilePts > 3 )
      {
       plane.A = 0.0 ;
       plane.B = 0.0 ;
       plane.C = 0.0 ;  
       if( bcdtmFilter_findPlaneDtmObject(dtmP,(tilesP+tile)->tileOffset,(tilesP+tile)->numTilePts,&plane) == DTM_SUCCESS )
         {
          for( pnt = (tilesP+tile)->tileOffset ; pnt < (tilesP+tile)->tileOffset + (tilesP+tile)->numTilePts ; ++pnt )
            {
             dZ = pointAddrP(dtmP,pnt)->z - ( plane.A * pointAddrP(dtmP,pnt)->x + plane.B * pointAddrP(dtmP,pnt)->y + plane.C );
             (elevDiffP+numPointDiffs)->elevation = fabs(dZ) ;
             (elevDiffP+numPointDiffs)->point     = pnt ;
             (elevDiffP+numPointDiffs)->tile      = *(tileNumberP+pnt) ;
             (elevDiffP+numPointDiffs)->keep      = 1 ;
             ++numPointDiffs ;
            }
         }
      }
    else
      {
       for( pnt = (tilesP+tile)->tileOffset ; pnt < (tilesP+tile)->tileOffset + (tilesP+tile)->numTilePts ; ++pnt )
         {
          (elevDiffP+numPointDiffs)->elevation = dtmP->zMax - dtmP->zMin ;
          (elevDiffP+numPointDiffs)->point     = pnt ;
          (elevDiffP+numPointDiffs)->tile      = *(tileNumberP+pnt) ;
          (elevDiffP+numPointDiffs)->keep      = 1 ;
           ++numPointDiffs ;
         }
      }  
   }
/*
** Adjust Number Of Points To Remove
*/
 if( numPointsRemove > numPointDiffs ) numPointsRemove = numPointDiffs ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numPointsRemove = %8ld",numPointsRemove) ;
/*
** Remove Method 1
*/
 if( removeMethod == 1 )
   {
/*
**  Quick Sort Elevation Difference Structure
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
    qsort(elevDiffP,numPointDiffs,sizeof(struct ElevDifferenceTile),bcdtmMultiResolution_elevationDifferenceTileCompareFunction) ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
       for( eldP = elevDiffP ; eldP < elevDiffP + 1000 ; ++eldP )
         {
          bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** point = %8ld tile = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->point,eldP->tile) ;
         } 
      }
    if( dbg )  bcdtmWrite_message(0,0,0,"Elevation Point Difference Of Last Remove Point = %10.4lf",(elevDiffP+numPointsRemove-1)->elevation) ;
/*
**  Mark All Points That Can Be Removed By Removing Equal Numbers From All Tiles
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
    eldP = elevDiffP ;
    lastTile = eldP->tile ;
    lastPoint = firstPoint = 0 ;
    while( eldP < elevDiffP + numPointDiffs )
      {
       while( eldP < elevDiffP + numPointDiffs && eldP->tile == lastTile ) ++eldP ;
       --eldP ;
       lastPoint = (long)(eldP-elevDiffP) ;
       numPoints = lastPoint - firstPoint + 1 ;
       numRemovePoints = firstPoint + numPoints / 2 ;
       for( eld1P = elevDiffP + firstPoint ; eld1P < elevDiffP + numRemovePoints ; ++eld1P ) eld1P->keep = 0 ;
       for( eld1P = elevDiffP + numRemovePoints + 1 ; eld1P < elevDiffP + numPoints ; ++eld1P ) eld1P->keep = 1 ;
/*
**     Reset For Next Tile
*/
       ++eldP ;
       if( eldP < elevDiffP + numPointDiffs )
         {
          lastTile = eldP->tile ;
          lastPoint = firstPoint = (long)(eldP-elevDiffP) ;
         }
      }
   }
/*
**  Remove Method Two
*/
 if( removeMethod == 2 )
   {
/*
**  Quick Sort Elevation Difference Structure
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Elevation Differences") ;
    qsort(elevDiffP,numPointDiffs,sizeof(struct ElevDifferenceTile),bcdtmMultiResolution_elevationDifferenceKeepCompareFunction) ;
    if( dbg == 2 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
       for( eldP = elevDiffP ; eldP < elevDiffP + 1000 ; ++eldP )
         {
          bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** point = %8ld tile = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->point,eldP->tile) ;
         } 
      }
    if( dbg )  bcdtmWrite_message(0,0,0,"Elevation Point Difference Of Last Remove Point = %10.4lf",(elevDiffP+numPointsRemove-1)->elevation) ;
/*
**  Mark All Points That Can Be Removed By Removing The Points With The Smallest Difference
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points For Removal") ;
    for( eldP = elevDiffP ; eldP < elevDiffP + numPointDiffs ; ++eldP )
      {
       if( (long)(eldP-elevDiffP) < numPointsRemove ) eldP->keep = 0 ;
       else                                           eldP->keep = 1 ;
      }
   }
/*
** Sort Points Into Tiles
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Points Tiles") ;
 qsort(elevDiffP,numPointDiffs,sizeof(struct ElevDifferenceTile),bcdtmMultiResolution_elevationDifferenceTileKeepCompareFunction) ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Elevation Difference = %8ld",numPointDiffs) ;
//    for( eldP = elevDiffP ; eldP < elevDiffP + numPointDiffs ; ++eldP )
    for( eldP = elevDiffP ; eldP < elevDiffP + 1000 ; ++eldP )
      {
       bcdtmWrite_message(0,0,0,"Point[%8ld] ** Diff = %10.4lf ** keep = %2ld point = %8ld tile = %8ld",(long)(eldP-elevDiffP),eldP->elevation,eldP->keep,eldP->point,eldP->tile) ;
      } 
   }
/*
** Copy Filtered Points To Filtered DTM
** Copy Unfiltered Points To Temp DTM
*/
 if( dbg )  bcdtmWrite_message(0,0,0,"Copying Filtered And Unfiltered Points") ;
 if( bcdtmObject_initialiseDtmObject(filteredDtmP)) goto errexit ;
 if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(filteredDtmP,numPointsRemove,numPointsRemove)) goto errexit ;
 if( numPointsRemove < numPointDiffs )
   { 
    if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
    if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,numPointDiffs-numPointsRemove,1000)) goto errexit ;
   }
 eldP = elevDiffP ;
 lastTile = eldP->tile ;
 lastKeep = eldP->keep ;
 lastPoint = firstPoint = 0 ;
 while( eldP < elevDiffP + numPointDiffs )
   {
    while( eldP < elevDiffP + numPointDiffs && eldP->keep == lastKeep && eldP->tile == lastTile ) ++eldP ;
    --eldP ;
    lastPoint = (long)(eldP-elevDiffP) ;
    numPoints = lastPoint - firstPoint + 1 ;
    if( numPoints > memPoints )
      {
       memPoints = numPoints * 2 ;
       if( pointsP == NULL ) pointsP = (DPoint3d *) malloc( memPoints * sizeof(DPoint3d)) ;
       else                  pointsP = (DPoint3d *) realloc( pointsP , memPoints * sizeof(DPoint3d)) ;
       if( pointsP == NULL )
         {
          bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         } 
      }
/*
**  Accumulate Points In Point Array
*/
    p3dP = pointsP ;
    for( point = firstPoint ; point <= lastPoint ; ++point )
      {
       pntP = pointAddrP(dtmP,point) ;
       p3dP->x = pntP->x ;
       p3dP->y = pntP->y ;
       p3dP->z = pntP->z ;
       ++p3dP ;
      } 
/*
**  Store Points As Point Feature 
*/
    if( lastKeep == 0 )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"lastKeep = %2ld lastTile = %8ld ** numPoints = %8ld",lastKeep,lastTile,numPoints) ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(filteredDtmP,DTMFeatureType::GroupSpots,lastTile,3,&dtmFeatureId,pointsP,numPoints)) goto errexit ;
      }
    else
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"lastKeep = %2ld lastTile = %8ld ** numPoints = %8ld",lastKeep,lastTile,numPoints) ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::GroupSpots,lastTile,3,&dtmFeatureId,pointsP,numPoints)) goto errexit ;
      }
/*
**  Reset For Next Tile
*/
    ++eldP ;
    if( eldP < elevDiffP + numPointDiffs )
      {
       lastTile = eldP->tile ;
       lastKeep = eldP->keep ;
       lastPoint = firstPoint = (long)(eldP-elevDiffP) ;
      }
   }
/*
**  Copy Temp DTM To DTM
*/
 if( bcdtmObject_initialiseDtmObject(dtmP)) goto errexit ;
 if( tempDtmP->numPoints > 0 )
   {
    if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,tempDtmP->numPoints,tempDtmP->numPoints)) goto errexit ; 
    if( bcdtmObject_appendDtmObject(dtmP,tempDtmP)) goto errexit ;
   } 
/*
** Clean Up
*/
 cleanup :
 if( elevDiffP   != NULL ) free(elevDiffP) ;
 if( tileNumberP != NULL ) free(tileNumberP) ;
 if( pointsP     != NULL ) free(pointsP) ;
 if( tempDtmP    != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
 /*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Decimate Group Spots Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tile Decimate Group Spots Error") ;
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
int bcdtmMultiResolution_checkSortOrderDtmObject
(
 BC_DTM_OBJ  *dtmP,
 long        firstPoint,
 long        lastPoint,
 long        axis,
 long        *isSortedP
)
{
 int  ret=DTM_SUCCESS,dbg=0;
 long pnt1,pnt2 ; 
 DTM_TIN_POINT *pnt1P,*pnt2P ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Checking Object Is Sorted") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"firstPoint    = %8ld",firstPoint) ;
    bcdtmWrite_message(0,0,0,"lastPoint     = %8ld",lastPoint) ;
    bcdtmWrite_message(0,0,0,"axis          = %8ld",axis) ;
    bcdtmWrite_message(0,0,0,"isSortedP     = %8ld",*isSortedP) ;
   }
/*
** Initialise
*/
 *isSortedP = 1 ;  
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
**  Check Sort Order
*/
 if( axis == DTM_X_AXIS )
   {
    pnt1  = firstPoint ;
    pnt1P = pointAddrP(dtmP,firstPoint) ;
    for( pnt2 = firstPoint + 1 ; pnt2 <= lastPoint && *isSortedP == 1  ; ++pnt2 )
      {
       pnt2P = pointAddrP(dtmP,pnt2) ;
       if( pnt2P->x < pnt1P->x || ( pnt2P->x == pnt1P->x && pnt2P->y < pnt1P->y ))
         {
          bcdtmWrite_message(1,0,0,"Dtm Point X Axis Sort Order Error") ;
          bcdtmWrite_message(1,0,0,"Point1[%8ld] = %12.5lf %12.5lf %10.4lf",pnt1,pnt1P->x,pnt1P->y,pnt1P->z ) ;
          bcdtmWrite_message(1,0,0,"Point2[%8ld] = %12.5lf %12.5lf %10.4lf",pnt2,pnt2P->x,pnt2P->y,pnt2P->z ) ;
          *isSortedP = 0 ;  
         }
       ++pnt1 ;
       pnt1P = pnt2P ;
      }
   }  
 if( axis == DTM_Y_AXIS )
   {
    pnt1  = firstPoint ;
    pnt1P = pointAddrP(dtmP,firstPoint) ;
    for( pnt2 = firstPoint + 1 ; pnt2 <= lastPoint && *isSortedP == 1 ; ++pnt2 )
      {
       pnt2P = pointAddrP(dtmP,pnt2) ;
       if( pnt2P->y < pnt1P->y || ( pnt2P->y == pnt1P->y && pnt2P->x < pnt1P->x ))
         {
          bcdtmWrite_message(1,0,0,"Dtm Point Y Axis Sort Order Error") ;
          bcdtmWrite_message(1,0,0,"Point1[%8ld] = %12.5lf %12.5lf %10.4lf",pnt1,pnt1P->y,pnt1P->x,pnt1P->z ) ;
          bcdtmWrite_message(1,0,0,"Point2[%8ld] = %12.5lf %12.5lf %10.4lf",pnt2,pnt2P->y,pnt2P->x,pnt2P->z ) ;
          *isSortedP = 0 ;  
         }
       ++pnt1 ;
       pnt1P = pnt2P ;
      }
   }  
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Object Is Sorted Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Object Is Sorted Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmMultiResolution_checkForDuplicatesDtmObject
(
 BC_DTM_OBJ  *dtmP,
 long        firstPoint,
 long        lastPoint,
 long        axis,
 long        *isDuplicatesP
)
{
 int  ret=DTM_SUCCESS,dbg=0;
 long pnt1,pnt2 ; 
 DTM_TIN_POINT *pnt1P,*pnt2P ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Checking Object For Duplicates") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"firstPoint    = %8ld",firstPoint) ;
    bcdtmWrite_message(0,0,0,"lastPoint     = %8ld",lastPoint) ;
    bcdtmWrite_message(0,0,0,"axis          = %8ld",axis) ;
    bcdtmWrite_message(0,0,0,"isDuplicatesP = %8ld",*isDuplicatesP) ;
   }
/*
** Initialise
*/
 *isDuplicatesP = 0 ;  
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
**  Check Sort Order
*/
 if( axis == DTM_X_AXIS )
   {
    pnt1  = firstPoint ;
    pnt1P = pointAddrP(dtmP,firstPoint) ;
    for( pnt2 = firstPoint + 1 ; pnt2 <= lastPoint && *isDuplicatesP == 0  ; ++pnt2 )
      {
       pnt2P = pointAddrP(dtmP,pnt2) ;
       if( pnt2P->x == pnt1P->x && pnt2P->y == pnt1P->y )
         {
          bcdtmWrite_message(1,0,0,"Dtm Point X Axis Duplicate Error") ;
          bcdtmWrite_message(1,0,0,"Point1[%8ld] = %12.5lf %12.5lf %10.4lf",pnt1,pnt1P->x,pnt1P->y,pnt1P->z ) ;
          bcdtmWrite_message(1,0,0,"Point2[%8ld] = %12.5lf %12.5lf %10.4lf",pnt2,pnt2P->x,pnt2P->y,pnt2P->z ) ;
          *isDuplicatesP = 1 ;  
         }
       ++pnt1 ;
       pnt1P = pnt2P ;
      }
   }  
 if( axis == DTM_Y_AXIS )
   {
    pnt1  = firstPoint ;
    pnt1P = pointAddrP(dtmP,firstPoint) ;
    for( pnt2 = firstPoint + 1 ; pnt2 <= lastPoint && *isDuplicatesP == 0 ; ++pnt2 )
      {
       pnt2P = pointAddrP(dtmP,pnt2) ;
       if( pnt2P->x == pnt1P->x && pnt2P->y == pnt1P->y )
         {
          bcdtmWrite_message(1,0,0,"Dtm Point Y Axis Duplicate Error") ;
          bcdtmWrite_message(1,0,0,"Point1[%8ld] = %12.5lf %12.5lf %10.4lf",pnt1,pnt1P->y,pnt1P->x,pnt1P->z ) ;
          bcdtmWrite_message(1,0,0,"Point2[%8ld] = %12.5lf %12.5lf %10.4lf",pnt2,pnt2P->y,pnt2P->x,pnt2P->z ) ;
          *isDuplicatesP = 1 ;  
         }
       ++pnt1 ;
       pnt1P = pnt2P ;
      }
   }  
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Object For Duplicates Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Object For Duplicates Error") ;
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
int bcdtmMultiResolution_sortBinaryXYZFile
(
 char *xyzFileNameP,                      // ==> Input Unsorted XYZ File
 char *outFileNameP,                      // ==> Output Sorted XYZ File
 long sortSize,                           // ==> Partition Size For Sorting Points
 long *numSortedPointsP                   // <== Number Of Sorted Points In OutPut File
)
{
 int  ret=DTM_SUCCESS,dbg=0,tdbg=0,cdbg=0 ;
 long n,m,point,merge,isSorted,isDuplicates,startTime=0,firstPoint,lastPoint,saveLastPoint=0 ;
 __int64 filePosition=0,endFilePosition=0,numRecords=0,numFilePartitions=0 ;  
 __int64 fileWritePosition=0,sortPartitionSize,numXyzPoints=0 ;
 __int64 isDtmPoint[1000],partitionFileOffset[1000],partitionFileSize[1000] ;
 double xMin,yMin,zMin,xMax,yMax,zMax ;
 long   numCachePoints=0,numPartitionPoints=0,partitionCacheOffset[1000],dtmPointsOffset[1000] ;
 DPoint3d    *pointCacheP=NULL,dtmPoint,dtmPoint1 ;
 BC_DTM_OBJ *dtmP=NULL ;
 DTM_TIN_POINT *pointP ;
 FILE *xyzFP=NULL ;
 FILE *outFP=NULL ;
 WChar *wFileNameP=NULL ;

/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Sort Binary XYZ File") ;
    bcdtmWrite_message(0,0,0,"xyzFileNameP      = %s",xyzFileNameP) ;
    bcdtmWrite_message(0,0,0,"outFileNameP      = %s",outFileNameP) ;
    bcdtmWrite_message(0,0,0,"sortSize          = %8ld",sortSize) ;
   }
/*
** Initialise
*/
 startTime = bcdtmClock() ;
 *numSortedPointsP = 0 ;
 sortPartitionSize = sortSize ;
 if( sortPartitionSize > 50000000 ) sortPartitionSize = 50000000 ;
/*
** Open File And Determine Size
*/
 bcdtmUtility_convertMbsToWcs(xyzFileNameP,&wFileNameP) ;
 xyzFP = bcdtmFile_open(wFileNameP,L"rb+") ;
 if( wFileNameP != NULL ) free(wFileNameP) ;
 if( xyzFP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Cannot Open XYZ File %s",xyzFileNameP) ;
    goto errexit ;
   }
/*
** Count Number Of Records In File And get Coordinate Limits
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Counting Records In File") ;
    numRecords = 0 ;
    xMin = yMin = zMin =  DTM_INFINITE ;
    xMax = yMax = zMax = -DTM_INFINITE ;
    while(bcdtmFread(&dtmPoint,sizeof(DPoint3d),1,xyzFP) == 1 )
      {
       ++numRecords ;
       if( dtmPoint.x < xMin ) xMin = dtmPoint.x ;
       if( dtmPoint.y < yMin ) yMin = dtmPoint.y ;
       if( dtmPoint.z < zMin ) zMin = dtmPoint.z ;
       if( dtmPoint.x > xMax ) xMax = dtmPoint.x ;
       if( dtmPoint.y > yMax ) yMax = dtmPoint.y ;
       if( dtmPoint.z > zMax ) zMax = dtmPoint.z ;
      } 
    bcdtmWrite_message(0,0,0,"Number Of Coordinate Records = %10I64d",numRecords) ;
    bcdtmWrite_message(0,0,0,"X ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",xMin,xMax,xMax-xMin) ;
    bcdtmWrite_message(0,0,0,"Y ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",yMin,yMax,yMax-yMin) ;
    bcdtmWrite_message(0,0,0,"Z ** Min = %13.4lf Max = %13.4lf Range = %13.4lf",zMin,zMax,zMax-zMin) ;
   } 
/*
** Determine File Size
*/
 _fseeki64(xyzFP,0,SEEK_END) ;
 endFilePosition = _ftelli64(xyzFP) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"endFilePosition       = %10I64d",endFilePosition) ;
 numXyzPoints = numRecords = endFilePosition / 24 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of XYZ Records = %10ld",numRecords) ;
 if( numRecords >= 1 ) 
   {
    if( numRecords < sortPartitionSize ) sortPartitionSize = numRecords ;
/*
**  Calculate Number Of File Partitions
*/
    numFilePartitions = numRecords / sortPartitionSize ;
    if( numFilePartitions * sortPartitionSize != numRecords ) ++numFilePartitions ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of File Partitions = %8ld",numFilePartitions) ;
/*
**  Calculate File Partition Offsets And Size
*/
    filePosition = 0 ;
    for( n = 0 ; n < numFilePartitions ; ++n )
      {
       partitionFileSize[n]   = sortPartitionSize ;  
       partitionFileOffset[n] = filePosition ;
       filePosition = filePosition + sortPartitionSize * 24 ;
      }
/*
**  Reset Size Of Last Partition
*/
    if( partitionFileOffset[numFilePartitions-1] + partitionFileSize[numFilePartitions-1] * 24 > endFilePosition )
      {
       partitionFileSize[numFilePartitions-1] = ( endFilePosition - partitionFileOffset[numFilePartitions-1] ) / 24 ;
      }
/*
**  Write out File Partitions Offsets And Sizes 
*/
    if( dbg )
      {
       numRecords = 0 ;
       for( n = 0 ; n < numFilePartitions ; ++n )
         {
          numRecords = numRecords + partitionFileSize[n] ;
          bcdtmWrite_message(0,0,0,"Offset = %10I64d Size = %8I64d numRecords = %8I64d",partitionFileOffset[n],partitionFileSize[n],numRecords) ;  
         }
      }
/*
**  Create Dtm Object
*/
    if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,(long)sortPartitionSize,1000) ;
/*
**  Read Sort And Write File Point Partitions
*/
    fileWritePosition = 0 ;
    for( n = 0 ; n < numFilePartitions ; ++n )
      {
/*
**     Read Points
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading %10I64d Points From Partition %2ld",partitionFileSize[n],n) ; 
       _fseeki64(xyzFP,partitionFileOffset[n],SEEK_SET) ;
       for( point = 0 ; point < partitionFileSize[n] ; ++point )
         {
          if( bcdtmFread(&dtmPoint,sizeof(DPoint3d),1,xyzFP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Reading XYZ File %s",xyzFileNameP) ;
             goto errexit ;
            } 
/*
**       Store Point In DTM 
*/
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&dtmPoint,1)) goto errexit ; 
         } 
/*
**     Sort Points On X Axis
*/
       firstPoint = 0 ;
       lastPoint = dtmP->numPoints - 1 ; 
       if( dbg ) bcdtmWrite_message(0,0,0,"X Axis Sorting Points") ;
       if( bcdtmFilter_sortPointRangeDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS)) goto errexit ;
/*
**     Remove Duplicate Points
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Points") ;
       saveLastPoint = lastPoint ;
       if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,firstPoint,&lastPoint)) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
/*
**     Check DTM Object Is Sorted And Duplicates Removed
*/
       if( cdbg )
         {
          if( bcdtmMultiResolution_checkSortOrderDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS,&isSorted)) goto errexit ;
          bcdtmWrite_message(0,0,0,"isSorted = %8ld",isSorted) ; 
          if( isSorted == 0 )
            {
             bcdtmWrite_message(2,0,0,"Partition Points Not Sorted") ;
             goto errexit ;
            } 
          if( bcdtmMultiResolution_checkForDuplicatesDtmObject(dtmP,firstPoint,lastPoint,DTM_X_AXIS,&isDuplicates)) goto errexit ;
          bcdtmWrite_message(0,0,0,"isDuplicates = %8ld",isDuplicates) ; 
          if( isDuplicates == 1 )
            {
             bcdtmWrite_message(2,0,0,"Duplicate Partition Points") ;
             goto errexit ;
            } 
         }   
/*
**     Reset Number Of Points In Partition
*/
       dtmP->numPoints = lastPoint + 1 ;
       partitionFileSize[n] = lastPoint + 1 ;
       if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld partitionFileSize[%4ld] = %8ld",dtmP->numPoints,n,partitionFileSize[n]) ;
/*
**     Write Points To File
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Writing %10ld Points To Partition %2ld",dtmP->numPoints,n) ; 
       _fseeki64(xyzFP,fileWritePosition,SEEK_SET) ;
       for( point = 0 ; point < ( __int64 ) dtmP->numPoints ; ++point )
         {
          pointP = pointAddrP(dtmP,point) ;
          if( bcdtmFwrite(pointP,sizeof(DPoint3d),1,xyzFP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Writing XYZ File %s",xyzFileNameP) ;
             goto errexit ;
            } 
         } 
       fileWritePosition = _ftelli64(xyzFP) ;
/*
**     Reset Number of Points In DTM Object
*/
       dtmP->numPoints = 0 ;
      }
/*
**  Rewind File To Reposition End Of File
*/
    if( dbg ) 
    rewind(xyzFP) ;
/*
**  Check All The Partition Points Are Sorted
*/
     if( cdbg )
       {
        bcdtmWrite_message(1,0,0,"Checking Partition Points Are Sorted") ;
        for( n = 0 ; n < numFilePartitions ; ++n )
          {
/*
**         Read First Point
*/
           if( dbg ) bcdtmWrite_message(0,0,0,"Checking Partition %2ld",n) ; 
           _fseeki64(xyzFP,partitionFileOffset[n],SEEK_SET) ;
           if( bcdtmFread(&dtmPoint,sizeof(DPoint3d),1,xyzFP) != 1 )
             {
              bcdtmWrite_message(1,0,0,"Error Reading XYZ File %s",xyzFileNameP) ;
              goto errexit ;
             } 
           for( point = 1 ; point < partitionFileSize[n] ; ++point )
             {
              if( bcdtmFread(&dtmPoint1,sizeof(DPoint3d),1,xyzFP) != 1 )
                {
                 bcdtmWrite_message(1,0,0,"Error Reading XYZ File %s",xyzFileNameP) ;
                 goto errexit ;
                } 
              if( dtmPoint1.x < dtmPoint.x  || ( dtmPoint1.x == dtmPoint.x && dtmPoint1.y < dtmPoint.y ))
                {
                 bcdtmWrite_message(0,0,0,"File Sort Error At Offset %10ld",ftell(xyzFP)) ;
                 bcdtmWrite_message(0,0,0,"dtmPoint  = %12.5lf %12.5lf %10.4lf",dtmPoint.x,dtmPoint.y,dtmPoint.z) ;
                 bcdtmWrite_message(0,0,0,"dtmPoint1 = %12.5lf %12.5lf %10.4lf",dtmPoint1.x,dtmPoint1.y,dtmPoint1.z) ;
                 bcdtmWrite_message(0,0,0,"point = %10ld of %10ld",point+1,partitionFileSize[n]) ;
                 goto errexit ;
                }
              dtmPoint = dtmPoint1 ;
             }
          } 
       }
/*
**     Open Sorted Binary File
*/
    bcdtmUtility_convertMbsToWcs(outFileNameP,&wFileNameP) ;
    outFP = bcdtmFile_open(wFileNameP,L"wb+") ;
    if( wFileNameP != NULL ) free(wFileNameP) ;
    if( xyzFP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Cannot Open XYZ File %s",outFileNameP) ;
       goto errexit ;
      }
/*
**     Merge The Sorted Points
*/
    if( numFilePartitions == 1 )
      { 
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading And Writing Sorted Points") ;
       _fseeki64(xyzFP,partitionFileOffset[0],SEEK_SET) ;
       lastPoint = 0 ;
       dtmP->numPoints = 0 ; 
       while( bcdtmFread(&dtmPoint,sizeof(DPoint3d),1,xyzFP) == 1 && lastPoint < partitionFileSize[0] )
         {
          if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&dtmPoint,1)) goto errexit ; 
          ++lastPoint ;
         }
       for( point = 0 ; point < dtmP->numPoints ; ++point )
         {
          if( bcdtmFwrite(pointAddrP(dtmP,point),sizeof(DPoint3d),1,outFP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Writing Sorted Mr DTM File") ;
             goto errexit ; 
            }
         }
       *numSortedPointsP = (long) partitionFileSize[0] ;
      }
    else
      { 
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Merging And Writing Sorted Points") ;
/*
**     Allocate Cache Memory For Points
*/
       numPartitionPoints = 100000  ;
       numCachePoints = (long) numFilePartitions * numPartitionPoints ;
       pointCacheP = ( DPoint3d * ) malloc(numCachePoints*sizeof(DPoint3d)) ;
       if( pointCacheP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         }
/*
**     Set Offsts To Point Partitions In Cache
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Setting Partition Offsets") ;
       partitionCacheOffset[0] = 0 ;
       for( n = 1 ; n < numFilePartitions ; ++n )
         {
          partitionCacheOffset[n] = partitionCacheOffset[n-1] + numPartitionPoints ; 
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"partitionCacheOffset[%4ld] = %8ld",n,partitionCacheOffset[n]) ;
         }
/*
**     Read The Initial set Of Points From each Partition
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Reading Initial Set of Points") ;
       for( n = 0 ; n < numFilePartitions ; ++n )
         {
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Reading File Partition %4ld",n) ;
          isDtmPoint[n] = 0 ;
          dtmPointsOffset[n] = 0 ;
          for( m = 0 ; m < numPartitionPoints ; ++m )
            { 
             if( partitionFileSize[n] > 0 )  
               { 
                _fseeki64(xyzFP,partitionFileOffset[n],SEEK_SET) ;
                if( fread(pointCacheP+partitionCacheOffset[n]+isDtmPoint[n],sizeof(DPoint3d),1,xyzFP) != 1 )
                  {
                   bcdtmWrite_message(1,0,0,"Error Reading XYZ File %s",xyzFileNameP) ;
                   goto errexit ;
                  } 
                partitionFileOffset[n] = partitionFileOffset[n] + 24 ;
                --partitionFileSize[n] ;
                ++isDtmPoint[n] ;
               }  
            }
        }  
/*
**     Merge and Write The Points
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Merging And Writing Points") ;
       merge = TRUE ;
       while ( merge )
         {
          merge = FALSE ;
/*
**       Get Smallest Valued Point
*/
          for( n = 0 ; n < numFilePartitions ; ++n )
            {
             if( isDtmPoint[n] )
               {
                dtmPoint1 = *(pointCacheP+partitionCacheOffset[n]+dtmPointsOffset[n]) ;
                if( ( merge == FALSE ) || ( dtmPoint.x > dtmPoint1.x || ( dtmPoint.x == dtmPoint1.x && dtmPoint.y > dtmPoint1.y )))
                  {
                   merge = n + 1 ; 
                   dtmPoint = dtmPoint1 ; 
                  } 
               } 
            }
/*
**        Write Smallest Valued Point
*/
          if( merge )
            {
             if( bcdtmFwrite(&dtmPoint,sizeof(DPoint3d),1,outFP) != 1 )
               {
                bcdtmWrite_message(1,0,0,"Error Writing Out File %s",outFileNameP) ;
                goto errexit ;
               } 
             --merge ;
             --isDtmPoint[merge] ;
             ++dtmPointsOffset[merge];
/*
**           Reload Cache For Point Partition
*/
             if( isDtmPoint[merge] == 0 && partitionFileSize[merge] > 0 )
               {
                dtmPointsOffset[merge] = 0 ;
                _fseeki64(xyzFP,partitionFileOffset[merge],SEEK_SET) ;
                for( m = 0 ; m < numPartitionPoints ; ++m )
                  { 
                   if( partitionFileSize[merge] > 0 )  
                     { 
                      if( bcdtmFread(pointCacheP+partitionCacheOffset[merge]+isDtmPoint[merge],sizeof(DPoint3d),1,xyzFP) != 1 )
                        {
                         bcdtmWrite_message(1,0,0,"Error Reading XYZ File %s",xyzFileNameP) ;
                         goto errexit ;
                        } 
                      partitionFileOffset[merge] = partitionFileOffset[merge] + 24 ;
                      --partitionFileSize[merge] ;
                      ++isDtmPoint[merge] ;
                     }  
                  }
               }
             merge = TRUE ;
            }  
         } 
/*
**     Set Number Of Points In File
*/
       endFilePosition   = _ftelli64(xyzFP) ;
       numXyzPoints      = endFilePosition / 24 ;
       *numSortedPointsP = (long)numXyzPoints ;
      } 
   }
/*
** Check File Is Sorted 
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking File Sort Order") ;
    numRecords = 1 ;
    fseek(outFP,0L,SEEK_SET) ;
    if( bcdtmFread(&dtmPoint,sizeof(DPoint3d),1,outFP) != 1 ) goto errexit ;
    while( bcdtmFread(&dtmPoint1,sizeof(DPoint3d),1,outFP) == 1 )
      {
       ++numRecords ;
       if( dtmPoint1.x < dtmPoint.x  || ( dtmPoint1.x == dtmPoint.x && dtmPoint1.y < dtmPoint.y ))
         {
          bcdtmWrite_message(0,0,0,"File Sort Error At Offset %10ld",ftell(outFP)) ;
          bcdtmWrite_message(0,0,0,"dtmPoint  = %12.5lf %12.5lf %10.4lf",dtmPoint.x,dtmPoint.y,dtmPoint.z) ;
          bcdtmWrite_message(0,0,0,"dtmPoint1 = %12.5lf %12.5lf %10.4lf",dtmPoint1.x,dtmPoint1.y,dtmPoint1.z) ;
         }
       dtmPoint = dtmPoint1 ;
      } 
    bcdtmWrite_message(0,0,0,"Number Of Records Read = %10ld",numRecords) ;
   }
/*
** Write Number Of Points 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points In Sorted File = %10ld",fileWritePosition/24) ;
/*
** Clean Up
*/
 cleanup :
 if( xyzFP != NULL ) { bcdtmFile_close(xyzFP) ; xyzFP = NULL ; }
 if( outFP != NULL ) { bcdtmFile_close(outFP) ; outFP = NULL ; }
 if( dtmP  != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Sort %10I64d Points In XYZ File = %8.3lf seconds",numXyzPoints,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Sort XYZ File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Sort XYZ File Error") ;
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
int bcdtmMultiResolution_tileXYZFile
(
 char                      *xyzFileNameP,
 __int64                   tilePartitionSize,
 BC_MRES_FILE_TILE_OFFSETS **tileFileOffsetsPP,
 long                      *numTilesP
)
{
 int     ret=DTM_SUCCESS,dbg=0,tdbg=0,cdbg=0 ;
 long    n,pnt,tile,firstPoint,lastPoint,startTime=0,tileTime=0 ;
 long    numTiles,memTiles=0,memTileInc=10000,minTilePts=20000,tileMaxPts,tileMinPts ;
 __int64 point,numRecords,filePosition,endFilePosition,fileWritePosition,numFilePartitions ;
 __int64 partitionFileOffset[1000],partitionFileSize[1000] ;
 double  xMin,yMin,zMin,xMax,yMax,zMax ;
 DPoint3d     *pntP,dtmPoint ;
 BC_DTM_OBJ *dtmP=NULL ;
 DTM_POINT_TILE *tileP,*pointTileP=NULL ;
 FILE *xyzFP=NULL ;
 FILE *tileFP=NULL ;
 long saveLastPoint ;
 WChar *wFileNameP=NULL ;

/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Tiling XYZ File") ;
    bcdtmWrite_message(0,0,0,"xyzFileNameP       = %s",xyzFileNameP) ;
    bcdtmWrite_message(0,0,0,"tilePartitionSize  = %8ld",tilePartitionSize) ;
    bcdtmWrite_message(0,0,0,"*tileFileOffsetsPP = %p",*tileFileOffsetsPP) ;
    bcdtmWrite_message(0,0,0,"numTilesP          = %8ld",*numTilesP) ;
   }
/*
** Initialise
*/
 *numTilesP = 0 ;
 tileTime = bcdtmClock() ; 
/*
** Open XYZ File
*/
 bcdtmUtility_convertMbsToWcs(xyzFileNameP,&wFileNameP) ;
 xyzFP = bcdtmFile_open(wFileNameP,L"rb+") ;
 if( wFileNameP != NULL ) free(wFileNameP) ;
 if( xyzFP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Cannot Open XYZ File %s",xyzFileNameP) ;
    goto errexit ;
   }
/*
** Determine File Size
*/
 _fseeki64(xyzFP,0,SEEK_END) ;
 endFilePosition = _ftelli64(xyzFP) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"endFilePosition       = %10I64d",endFilePosition) ;
 numRecords = endFilePosition / 24 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of XYZ Records = %10ld",numRecords) ;
/*
** Calculate Number Of File Partitions
*/
 numFilePartitions = numRecords / tilePartitionSize ;
 if( numFilePartitions * tilePartitionSize != numRecords ) ++numFilePartitions ;
 tilePartitionSize = numRecords / numFilePartitions ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of File Partitions = %8ld",numFilePartitions) ;
/*
** Set Minimum Tile Points
*/
// minTilePts=1000 ; 
  minTilePts=100000 ; 
//   minTilePts=1000000 ;
//   minTilePts=50000 ; 
 
/*
**  Calculate File Partition Offsets And Size
*/
 filePosition = 0 ;
 for( n = 0 ; n < numFilePartitions ; ++n )
   {
    partitionFileSize[n]   = tilePartitionSize ;  
    partitionFileOffset[n] = filePosition ;
    filePosition = filePosition + tilePartitionSize * 24 ;
   }
/*
**  Reset Size Of Last Partition
*/
 if( partitionFileOffset[numFilePartitions-1] + partitionFileSize[numFilePartitions-1] * 24 != endFilePosition )
   {
    partitionFileSize[numFilePartitions-1] = ( endFilePosition - partitionFileOffset[numFilePartitions-1] ) / 24 ;
   }
/*
**  Write out File Partitions Offsets And Sizes 
*/
 if( dbg )
   {
    numRecords = 0 ;
    for( n = 0 ; n < numFilePartitions ; ++n )
      {
       numRecords = numRecords + partitionFileSize[n] ;
       bcdtmWrite_message(0,0,0,"Partition[%4ld] ** Offset = %12I64d Size = %8I64d numRecords = %12I64d",n,partitionFileOffset[n],partitionFileSize[n],numRecords) ;  
      }
    numRecords = 0 ;
   }
/*
**  Create Dtm Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,(long)tilePartitionSize,1000) ;
/*
** Open Output File
*/
/*
 tileFP = fopen("mrTiled.xyz","wb") ;
 if( tileFP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Eror Opening mrTiled.xyz") ;
    goto errexit ;
   }
*/
/*
**  Read Sort And Write File Point Partitions
*/
 fileWritePosition = 0 ;
 for( n = 0 ; n < numFilePartitions ; ++n )
   {
/*
**  Read Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Reading %10I64d Points From Partition %2ld",partitionFileSize[n],n) ; 
    _fseeki64(xyzFP,partitionFileOffset[n],SEEK_SET) ;
    for( point = 0 ; point < partitionFileSize[n] ; ++point )
      {
       if( bcdtmFread(&dtmPoint,sizeof(DPoint3d),1,xyzFP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading XYZ File %s",xyzFileNameP) ;
          goto errexit ;
         } 
/*
**     Store Point In DTM 
*/
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&dtmPoint,1)) goto errexit ; 
      }
/*
**  Check For Duplicate Points
*/
    if( cdbg == 2 )
      {
       lastPoint = dtmP->numPoints - 1 ;
       if( bcdtmFilter_sortPointRangeDtmObject(dtmP,0,lastPoint,DTM_X_AXIS)) goto errexit ;
       saveLastPoint = lastPoint ;
       if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,0,&lastPoint)) goto errexit ;
       bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
       if( saveLastPoint != lastPoint ) goto errexit ;
      } 
/*
**  Tile DTM
*/
    startTime = bcdtmClock() ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"Tiling XYZ Points Into Tiles With A Maximum Of %8ld Points Per Tile",minTilePts) ;
//    if( bcdtmTile_pointsDtmObject(dtmP,NULL,0,dtmP->numPoints,minTilePts,&pointTileP,&numTiles) ) goto errexit ; 
    if( bcdtmMedianTile_pointsDtmObject(dtmP,NULL,0,dtmP->numPoints,minTilePts,&pointTileP,&numTiles) ) goto errexit ; 
    if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Tile %8ld Points Into %8ld Tiles = %8.3lf Secs",dtmP->numPoints,numTiles,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ; 
/*
**  Check Tiling
*/
    if( cdbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Partition Tiles = %8ld",numTiles) ; 
       tileMinPts = tileMaxPts = pointTileP->numTilePts ;
       for( tileP = pointTileP ; tileP < pointTileP + numTiles ; ++tileP )
         {
          if( tileP->numTilePts < tileMinPts ) tileMinPts = tileP->numTilePts ;
          if( tileP->numTilePts > tileMaxPts ) tileMaxPts = tileP->numTilePts ;
         }
       bcdtmWrite_message(0,0,0,"minTilePts = %8ld maxTilePts = %8ld",tileMinPts,tileMaxPts) ; 
       if( tileMaxPts - tileMinPts > 1 )
         {
          bcdtmWrite_message(0,0,0,"Unbalanced Point Tiles") ;
          goto errexit ; 
         }
      } 
/*
**  Write Tiles To Disk
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Tiles To Disk") ;
    _fseeki64(xyzFP,fileWritePosition,SEEK_SET) ;
//    _fseeki64(tileFP,fileWritePosition,SEEK_SET) ;
    for( tile = 0 ; tile < numTiles ; ++tile )
      {
/*
**     Get Bounding Cube For Tile
*/
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Getting Bounding Cube For Tile %8ld of %8ld",tile+1,numTiles) ;
       pntP = ( DPoint3d *)pointAddrP(dtmP,(pointTileP+tile)->tileOffset) ;
       xMin = xMax = pntP->x ;
       yMin = yMax = pntP->y ;
       zMin = zMax = pntP->z ;
       for( pnt = (pointTileP+tile)->tileOffset ; pnt < (pointTileP+tile)->tileOffset + (pointTileP+tile)->numTilePts ; ++pnt )
         {
          pntP = ( DPoint3d *)pointAddrP(dtmP,pnt) ;
          if( pntP->x < xMin ) xMin = pntP->x ;
          if( pntP->x > xMax ) xMax = pntP->x ;
          if( pntP->y < yMin ) yMin = pntP->y ;
          if( pntP->y > yMax ) yMax = pntP->y ;
          if( pntP->z < zMin ) zMin = pntP->z ;
          if( pntP->z > zMax ) zMax = pntP->z ;
         }
/*
**     Check Tile Memory
*/
       if( *numTilesP >= memTiles )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Memory For Tiles") ;
          memTiles = memTiles + memTileInc ;
          if( *tileFileOffsetsPP == NULL ) *tileFileOffsetsPP = ( BC_MRES_FILE_TILE_OFFSETS * ) malloc ( memTiles * sizeof(BC_MRES_FILE_TILE_OFFSETS)) ;
          else                             *tileFileOffsetsPP = ( BC_MRES_FILE_TILE_OFFSETS * ) realloc (*tileFileOffsetsPP,memTiles * sizeof(BC_MRES_FILE_TILE_OFFSETS)) ;
          if( *tileFileOffsetsPP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }  
         }
/*
**     Add Tile To Tile Offsets Array
*/
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Adding Tile To Offsets Array") ;
       (*tileFileOffsetsPP+*numTilesP)->tileNumber      = *numTilesP ;
       (*tileFileOffsetsPP+*numTilesP)->resolutionLevel = 0 ;
       (*tileFileOffsetsPP+*numTilesP)->numPoints       = (pointTileP+tile)->numTilePts ;
       (*tileFileOffsetsPP+*numTilesP)->fileOffset      = fileWritePosition ;
       (*tileFileOffsetsPP+*numTilesP)->xMin = xMin ;
       (*tileFileOffsetsPP+*numTilesP)->xMax = xMax ;
       (*tileFileOffsetsPP+*numTilesP)->yMin = yMin ;
       (*tileFileOffsetsPP+*numTilesP)->yMax = yMax ;
       (*tileFileOffsetsPP+*numTilesP)->zMin = zMin ;
       (*tileFileOffsetsPP+*numTilesP)->zMax = zMax ;
        ++*numTilesP ;
/*
**     Write Tile To Disk
*/
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Writing Tile To Disk") ;
       firstPoint = (pointTileP+tile)->tileOffset ;
       lastPoint  = firstPoint + (pointTileP+tile)->numTilePts - 1 ;
       for( pnt = firstPoint ; pnt <= lastPoint ; ++pnt )
         {
          pntP = ( DPoint3d *) pointAddrP(dtmP,pnt) ;
          if( bcdtmFwrite(pntP,sizeof(DPoint3d),1,xyzFP) != 1 )
            {
             bcdtmWrite_message(1,0,0,"Error Writing File %s",xyzFileNameP) ;
             goto errexit ;
            }
         }
/*
**     Set File Write Position
*/
       fileWritePosition = _ftelli64(xyzFP) ;
//       fileWritePosition = _ftelli64(tileFP) ;
      } 
/*
**  Reset Number Of DTM Points
*/
    dtmP->numPoints = 0 ;    
   }
/*
** Clean Up
*/
 cleanup :
 if( dtmP  != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( xyzFP != NULL ) { bcdtmFile_close(xyzFP) ; xyzFP = NULL ; }
 if( tileFP != NULL ) { bcdtmFile_close(tileFP) ; tileFP = NULL ; }
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Total Tile Time = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),tileTime)) ; 
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tiling XYZ File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Tiling XYZ File Error") ;
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
int bcdtmMultiResolution_createSpatialIndexTinForTiles
(
 BC_DTM_OBJ                **dtmPP,
 BC_MRES_FILE_TILE_OFFSETS *tileFileOffsetsP,
 long                      numTiles
)
{
 int ret=DTM_SUCCESS,dbg=1;
 long   tile,edgeOption,startTime=0 ;
 double xMin,yMin,zMin,xMax,yMax,zMax,maxSide=0.0 ;
 DPoint3d tileHullPts[5] ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Spatial Index Tin For Tiles") ;
    bcdtmWrite_message(0,0,0,"*dtmPP             = %p",*dtmPP) ;
    bcdtmWrite_message(0,0,0,"tileFileOffsetsP   = %p",tileFileOffsetsP) ;
    bcdtmWrite_message(0,0,0,"numTiles           = %8ld",numTiles) ;
   }
/*
** Initialise
*/
 startTime = bcdtmClock() ;   
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(dtmPP)) goto errexit ;   
/*
**  Write The Tile Boundaries To The Spatial Index
*/
 for( tile = 0 ; tile  < numTiles ; ++tile )
   {
    xMin = (tileFileOffsetsP + tile)->xMin ;
    yMin = (tileFileOffsetsP + tile)->yMin ;
    zMin = (tileFileOffsetsP + tile)->zMin ;
    xMax = (tileFileOffsetsP + tile)->xMax ;
    yMax = (tileFileOffsetsP + tile)->yMax ;
    zMax = (tileFileOffsetsP + tile)->zMax ;
    tileHullPts[0].x = xMin ; tileHullPts[0].y = yMin ; tileHullPts[0].z = ( zMin+zMax) / 2.0 ;
    tileHullPts[1].x = xMax ; tileHullPts[1].y = yMin ; tileHullPts[1].z = ( zMin+zMax) / 2.0 ;
    tileHullPts[2].x = xMax ; tileHullPts[2].y = yMax ; tileHullPts[2].z = ( zMin+zMax) / 2.0 ;
    tileHullPts[3].x = xMin ; tileHullPts[3].y = yMax ; tileHullPts[3].z = ( zMin+zMax) / 2.0 ;
    tileHullPts[4].x = xMin ; tileHullPts[4].y = yMin ; tileHullPts[4].z = ( zMin+zMax) / 2.0 ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(*dtmPP,DTMFeatureType::Breakline,tile,1,&(*dtmPP)->nullFeatureId,tileHullPts,5)) goto errexit ;
   }
/*
** Triangulate Spatial Index
*/
 bcdtmWrite_message(0,0,0,"Triangulating Spatial Index") ;
 startTime = bcdtmClock() ;
 edgeOption = 2 ; 
 bcdtmObject_setTriangulationParametersDtmObject(*dtmPP,0.0001,0.0001,edgeOption,maxSide) ;
 if( bcdtmObject_triangulateDtmObject(*dtmPP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numPoints = %8ld numTriangles = %8ld numTriangleEdges = %8ld ** Tin Creation Time = %8.3lf Seconds",(*dtmPP)->numPoints,(*dtmPP)->numTriangles,(*dtmPP)->numLines,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Write To File
*/
 if( dbg ) bcdtmWrite_toFileDtmObject(*dtmPP,L"spatialIndexTin.dtm") ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Spatial Index Tin For Tiles Completed") ;
if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Spatial Index Tin For Tiles Error") ;
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
int bcdtmMultiResolution_filterTiledXYZFile
(
 char                      *xyzFileNameP,
 char                      *mrDtmFileNameP,
 long                      numResolutionLevels,
 long                      minResolutionPoints,
 long                      decimationFactor,  
 BC_MRES_FILE_TILE_OFFSETS *tileOffsetsP,
 long                      numTiles
)
{
 int ret=DTM_SUCCESS,dbg=0,tdbg=0,cdbg=0 ;
 long n,point,filterTime,numFilteredPts,totalFilteredPts=0,numFulcrumPts=0,numPointsRemove=0 ;
 long numTilePoints=0,memTilePoints=0,firstPoint,lastPoint,numResTilePoints ;
 long numTileIndex=0,memTileIndex=0,startTime,edgeOption ;
 __int64  fileWritePosition=0 ;
 double maxSide=0.0,xMin,yMin,zMin,xMax,yMax,zMax ;
 char label[32],buffer[1024] ;
 DPoint3d  *p3dP,dtmPoint,*tilePointsP=NULL,tileHullPts[5];
 BC_DTM_OBJ *dtmP=NULL,*fulcrumDtmP=NULL,*filteredDtmP=NULL,*spatialIndexP=NULL ;
 FILE *xyzFP=NULL ;
 FILE *mrDtmFP=NULL ;
 BC_MRES_FILE_TILE_OFFSETS *tofsP ;             // Required For External Tiled XYZ File - Remove Latter
 BC_MRES2_TILE_FILE_OFFSETS *tileIndexP=NULL,*indexP;
 BC_MRES2_DTM  mrDtmHeader ;
 long saveLastPoint ;
 WChar *wFileNameP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Filtering Tiled XYZ File") ;
    bcdtmWrite_message(0,0,0,"xyzFileNameP        = %s",xyzFileNameP) ;
    bcdtmWrite_message(0,0,0,"mrDtmFileNameP      = %s",mrDtmFileNameP) ;
    bcdtmWrite_message(0,0,0,"numResolutionLevels = %8ld",numResolutionLevels) ;
    bcdtmWrite_message(0,0,0,"minResolutionPoints = %8ld",minResolutionPoints) ;
    bcdtmWrite_message(0,0,0,"decimationFactor    = %8ld",decimationFactor) ;
    bcdtmWrite_message(0,0,0,"tileOffsetsP        = %p",tileOffsetsP) ;
    bcdtmWrite_message(0,0,0,"numTiles            = %8ld",numTiles) ;
   }
/*
** Initialise
*/
 filterTime = bcdtmClock() ; 
/*
** Scan Tiles And Get Number Of Points And Bounding Rectangle
*/
 numResTilePoints = 0 ;
 xMin = tileOffsetsP->xMin ;
 yMin = tileOffsetsP->yMin ;
 zMin = tileOffsetsP->zMin ;
 xMax = tileOffsetsP->xMax ;
 yMax = tileOffsetsP->yMax ;
 zMax = tileOffsetsP->zMax ;
 for( tofsP = tileOffsetsP ; tofsP < tileOffsetsP + numTiles ; ++tofsP )
   {
    numResTilePoints = numResTilePoints + tofsP->numPoints ;
    if( tileOffsetsP->xMin < xMin ) xMin = tileOffsetsP->xMin ;
    if( tileOffsetsP->xMax < xMax ) xMax = tileOffsetsP->xMax ;
    if( tileOffsetsP->yMin < yMin ) yMin = tileOffsetsP->yMin ;
    if( tileOffsetsP->yMax < yMax ) yMax = tileOffsetsP->yMax ;
    if( tileOffsetsP->zMin < zMin ) zMin = tileOffsetsP->zMin ;
    if( tileOffsetsP->zMax < zMax ) zMax = tileOffsetsP->zMax ;
   }
/*
** Write Statistics
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Multi Resolution DTM Points = %12ld",numResTilePoints) ;
    bcdtmWrite_message(0,0,0,"**** xMin = %15.5lf xMax = %15.5lf xRange = %10.4lf",xMin,xMax,xMax-xMin) ;
    bcdtmWrite_message(0,0,0,"**** yMin = %15.5lf yMax = %15.5lf yRange = %10.4lf",yMin,yMax,yMax-yMin) ;
    bcdtmWrite_message(0,0,0,"**** zMin = %15.5lf zMax = %15.5lf zRange = %10.4lf",zMin,zMax,zMax-zMin) ;
   }
/*
** Open XYZ File
*/
 bcdtmUtility_convertMbsToWcs(xyzFileNameP,&wFileNameP) ;
 xyzFP = bcdtmFile_open(wFileNameP,L"rb+") ;
 if( wFileNameP != NULL ) free(wFileNameP) ;
 if( xyzFP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Cannot Open XYZ File %s",xyzFileNameP) ;
    goto errexit ;
   }
/*
** Open ScalableMesh File
*/
 bcdtmUtility_convertMbsToWcs(mrDtmFileNameP,&wFileNameP) ;
 mrDtmFP = bcdtmFile_open(wFileNameP,L"wb+") ;
 if( wFileNameP != NULL ) free(wFileNameP) ;
 if( mrDtmFP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Cannot Open ScalableMesh File %s",xyzFileNameP) ;
    goto errexit ;
   }
/*
** Write MR DTM File Header
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing MR DTM Header") ;
 mrDtmHeader.dtmFileType = BC_DTM_MRES_TYPE ;
 mrDtmHeader.dtmVersionNumber = BC_DTM_MRES_VERSION ;
 mrDtmHeader.numResolutionLevels = numResolutionLevels ;
 mrDtmHeader.numPoints = numResTilePoints ;
 mrDtmHeader.numTiles  = numTiles ;
 mrDtmHeader.xMin      = xMin ;
 mrDtmHeader.yMin      = yMin ;
 mrDtmHeader.zMin      = zMin ;
 mrDtmHeader.xMax      = xMax ;
 mrDtmHeader.yMax      = yMax ;
 mrDtmHeader.zMax      = zMax ;
 mrDtmHeader.spatialIndexOffset = 0 ;
 mrDtmHeader.tileIndexOffset = 0 ;
 if( bcdtmFwrite(( void *) &mrDtmHeader,sizeof(BC_MRES2_DTM),1,mrDtmFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Failed To Write MR DTM File Header") ;
    goto errexit ;
   }
 fileWritePosition = _ftelli64(mrDtmFP) ;
/*
**  Allocate Memory For Tile Index
*/
  numTileIndex = memTileIndex = numTiles ;
  tileIndexP = ( BC_MRES2_TILE_FILE_OFFSETS *) malloc(memTileIndex*sizeof(BC_MRES2_TILE_FILE_OFFSETS)) ;
  if( tileIndexP == NULL )
    {
     bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
     goto errexit ;
    }
  if( dbg ) bcdtmWrite_message(0,0,0,"Size Of Tile Index = %8ld",memTileIndex*sizeof(BC_MRES2_TILE_FILE_OFFSETS)) ;
/*
** Create Tin For Spatial Index
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Spatial Index") ;
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
 for( tofsP = tileOffsetsP ; tofsP < tileOffsetsP + numTiles ; ++tofsP )
   {
    tileHullPts[0].x = tofsP->xMin ; tileHullPts[0].y = tofsP->yMin ; tileHullPts[0].z = ( tofsP->zMin+tofsP->zMax) / 2.0 ;
    tileHullPts[1].x = tofsP->xMax ; tileHullPts[1].y = tofsP->yMin ; tileHullPts[1].z = ( tofsP->zMin+tofsP->zMax) / 2.0 ;
    tileHullPts[2].x = tofsP->xMax ; tileHullPts[2].y = tofsP->yMax ; tileHullPts[2].z = ( tofsP->zMin+tofsP->zMax) / 2.0 ;
    tileHullPts[3].x = tofsP->xMin ; tileHullPts[3].y = tofsP->yMax ; tileHullPts[3].z = ( tofsP->zMin+tofsP->zMax) / 2.0 ;
    tileHullPts[4].x = tofsP->xMin ; tileHullPts[4].y = tofsP->yMin ; tileHullPts[4].z = ( tofsP->zMin+tofsP->zMax) / 2.0 ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,(long)(tofsP-tileOffsetsP),1,&dtmP->nullFeatureId,tileHullPts,5)) goto errexit ;
   }
/*
** Triangulate Spatial Index
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Spatial Index") ;
 startTime = bcdtmClock() ;
 edgeOption = 2 ; 
 bcdtmObject_setTriangulationParametersDtmObject(dtmP,0.0001,0.0001,edgeOption,maxSide) ;
 if( bcdtmObject_triangulateDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numPoints = %8ld numTriangles = %8ld numTriangleEdges = %8ld ** Tin Creation Time = %8.3lf Seconds",dtmP->numPoints,dtmP->numTriangles,dtmP->numLines,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Write To File
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Spatial Index To File") ;
 mrDtmHeader.spatialIndexOffset = fileWritePosition ;
 if( bcdtmWrite_atFilePositionDtmObject(dtmP,mrDtmFP,(long)fileWritePosition))
   {
    bcdtmWrite_message(1,0,0,"Failed To Write MR DTM Spatial Index") ;
    goto errexit ;
   }
 fileWritePosition = _ftelli64(mrDtmFP) ;
 if( bcdtmObject_cloneDtmObject(dtmP,&spatialIndexP)) goto errexit ;
 bcdtmObject_destroyDtmObject(&dtmP) ;
 if( dbg == 1 ) bcdtmWrite_toFileDtmObject(spatialIndexP,L"spatialIndex.dtm") ;
/*
**  Create Dtm Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,tileOffsetsP->numPoints,tileOffsetsP->numPoints) ;
 if( bcdtmObject_createDtmObject(&filteredDtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(filteredDtmP,tileOffsetsP->numPoints,tileOffsetsP->numPoints) ;
 if( bcdtmObject_createDtmObject(&fulcrumDtmP)) goto errexit ;
/*
**  Read Filter And Write File Point Partitions
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Tiles Filtering And Writing Tiles To ScalableMesh") ;
 for( tofsP = tileOffsetsP ; tofsP < tileOffsetsP + numTiles ; ++tofsP )
   {
    if( dbg && (long)(tofsP-tileOffsetsP) % 100 == 0 ) bcdtmWrite_message(0,0,0,"Processing Tile %8ld",(long)(tofsP-tileOffsetsP)+1) ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Tile = %8ld fileOffset = %10I64d numPoints = %8ld",tofsP->tileNumber,tofsP->fileOffset,tofsP->numPoints) ;
/*
**  Read Points
*/
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Reading %10ld Points For Tile %8ld",tofsP->numPoints,tofsP->tileNumber) ; 
    _fseeki64(xyzFP,tofsP->fileOffset,SEEK_SET) ;
    for( point = 0 ; point < tofsP->numPoints ; ++point )
      {
       if( bcdtmFread(&dtmPoint,sizeof(DPoint3d),1,xyzFP) != 1 )
         {
          bcdtmWrite_message(1,0,0,"Error Reading XYZ File %s",xyzFileNameP) ;
          goto errexit ;
         } 
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::RandomSpots,dtmP->nullUserTag,1,&dtmP->nullFeatureId,&dtmPoint,1)) goto errexit ; 
      }
/*
**  Check For Duplicate Points In Tiles
*/
    if( cdbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"Checking For Duplicate Points In Tile %8ld",tofsP->tileNumber) ;
       lastPoint = dtmP->numPoints - 1 ;
       if( bcdtmFilter_sortPointRangeDtmObject(dtmP,0,lastPoint,DTM_X_AXIS)) goto errexit ;
       saveLastPoint = lastPoint ;
       if( bcdtmFilter_removeDuplicatePointsFromRangeDtmObject(dtmP,0,&lastPoint)) goto errexit ;
       if( saveLastPoint != lastPoint ) 
         {
          bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",saveLastPoint-lastPoint) ;
          goto errexit ;
         }
       bcdtmWrite_message(0,0,0,"No Duplicate Points In Tile %8ld",tofsP->tileNumber) ;
      }
/*
**  Set Tile Bounding Cube
*/
    (tileIndexP+tofsP->tileNumber)->xMin = tofsP->xMin ;
    (tileIndexP+tofsP->tileNumber)->yMin = tofsP->yMin ;
    (tileIndexP+tofsP->tileNumber)->zMin = tofsP->zMin ;
    (tileIndexP+tofsP->tileNumber)->xMax = tofsP->xMax ;
    (tileIndexP+tofsP->tileNumber)->yMax = tofsP->yMax ;
    (tileIndexP+tofsP->tileNumber)->zMax = tofsP->zMax ;
    (tileIndexP+tofsP->tileNumber)->zMax = tofsP->zMax ;
    (tileIndexP+tofsP->tileNumber)->numPoints  = tofsP->numPoints ;
    (tileIndexP+tofsP->tileNumber)->tileNumber = tofsP->tileNumber ;
/*
**  Set Number Of Tile Points And Allocate memory To Store Tile Points In Resolution Order
*/
    numTilePoints = dtmP->numPoints ;
    lastPoint     = dtmP->numPoints ;
    if( numTilePoints > memTilePoints )
      {
       memTilePoints = numTilePoints + 1000 ;
       if( tilePointsP == NULL ) tilePointsP = (DPoint3d *) malloc( memTilePoints * sizeof(DPoint3d)) ;
       else                      tilePointsP = (DPoint3d *) realloc( tilePointsP ,memTilePoints * sizeof(DPoint3d)) ; 
       if( tilePointsP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         } 
      }  
/*
**  Filter Points And Accumulate Points In Resolution Order
*/
   for( n = 0 ; n < numResolutionLevels ; ++n )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Resolution Level = %4ld ** dtmP->numPoints = %8ld",n+1,dtmP->numPoints) ;
/*
**     Do Not Filter Lowest Resolution Level
*/
       numPointsRemove = dtmP->numPoints - dtmP->numPoints / decimationFactor ;
       if( n < numResolutionLevels - 1 )
         {
//          if( bcdtmMultiResolution_tinDecimateRandomSpotsDtmObject(dtmP,1,2,numPointsRemove,&numFilteredPts,filteredDtmP)) goto errexit ;
          if( bcdtmMultiResolution_tileDecimateRandomSpotsDtmObject(dtmP,numPointsRemove,&numFilteredPts,filteredDtmP)) goto errexit ;
         }
       else
         {
          if( bcdtmObject_appendDtmObject(filteredDtmP,dtmP)) goto errexit ;
         } 
/*
**     Set Number Of Points For Resolution
*/
       (tileIndexP+tofsP->tileNumber)->resolutionPoints[numResolutionLevels-n-1] = filteredDtmP->numPoints ;
/*
**     Accumulate Points In Resolution Order In Tile Points Array
*/
       firstPoint = lastPoint - filteredDtmP->numPoints ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"lastPoint = %8ld firstPoint = %8ld",lastPoint,firstPoint) ;
       for( point = 0 ; point < filteredDtmP->numPoints ; ++point) 
         {
          memcpy(tilePointsP+firstPoint,pointAddrP(filteredDtmP,point),sizeof(DPoint3d)) ;
          ++firstPoint ;
         }
       lastPoint = lastPoint - filteredDtmP->numPoints ;
       filteredDtmP->numPoints = 0 ;    
      }
/*
**  Report Number Of Points For Each Resolution Level
*/
    if( dbg == 1 )
      { 
       indexP = tileIndexP+tofsP->tileNumber ;
       buffer[0] = 0 ;
       for( n = 0 ; n < numResolutionLevels ; ++n )
         {
          sprintf(label,"%ld",indexP->resolutionPoints[n]) ;
          strcat(buffer,label) ;
          strcat(buffer," ") ; 
         }
       bcdtmWrite_message(0,0,0,"Tile[%8ld] ** %s",tofsP->tileNumber,buffer) ;
      } 
/*
**  Set Number Of Tile Points For Resolution Level
*/ 
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Setting Number Of Tile Points") ;
    numResTilePoints = 0 ;
    for( n = 0 ; n <  numResolutionLevels ; ++n )
      {
       numResTilePoints = numResTilePoints + (tileIndexP+tofsP->tileNumber)->resolutionPoints[n] ;
       (tileIndexP+tofsP->tileNumber)->resolutionPoints[n] = numResTilePoints ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"ResolutionLevel[%2ld] numPoints = %8ld totalPoints = %8ld",n,(tileIndexP+tofsP->tileNumber)->resolutionPoints[n],numResTilePoints) ;
      } 
/*
**  Report Number Of Points For Each Resolution Level
*/
    if( dbg == 1 )
      { 
       indexP = tileIndexP+tofsP->tileNumber ;
       buffer[0] = 0 ;
       for( n = 0 ; n < numResolutionLevels ; ++n )
         {
          sprintf(label,"%ld",indexP->resolutionPoints[n]) ;
          strcat(buffer,label) ;
          strcat(buffer," ") ; 
         }
       bcdtmWrite_message(0,0,0,"Tile[%8ld] ** %s",tofsP->tileNumber,buffer) ;
      } 
/*
**  Check Number Of Resolution Tile Points Is Equal To The Number Of Tile Points
*/
    if( cdbg )
      {
       if( numResTilePoints != numTilePoints )
         {
          if( numResTilePoints != numTilePoints )
            {
             bcdtmWrite_message(0,0,0,"numResTilePoints = %8ld numTilePoints = %8ld",numResTilePoints,numTilePoints) ;
             if( dbg == 2 )
               { 
                for( p3dP = tilePointsP ; p3dP < tilePointsP + numTilePoints ; ++p3dP )
                  {
                   bcdtmWrite_message(0,0,0,"TilePoint[%8ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-tilePointsP),p3dP->x,p3dP->y,p3dP->z) ;
                  } 
               } 
             goto errexit ;
            }
         }
      }
/*
**  Write Tile Points To File
*/
    (tileIndexP+tofsP->tileNumber)->fileOffset = fileWritePosition ;
    if( bcdtmFwrite(tilePointsP,sizeof(DPoint3d),(tileIndexP+tofsP->tileNumber)->numPoints,mrDtmFP) != (tileIndexP+tofsP->tileNumber)->numPoints )
      {
       bcdtmWrite_message(1,0,0,"Error Writing ScalableMesh File") ;
       goto errexit ;
      }
    fileWritePosition = _ftelli64(mrDtmFP) ;
/*
**  Sum Fulcrum Points
*/ 
    totalFilteredPts = totalFilteredPts + filteredDtmP->numPoints ; 
    numFulcrumPts  = numFulcrumPts + dtmP->numPoints ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(fulcrumDtmP,DTMFeatureType::GroupSpots,tofsP->tileNumber,1,&fulcrumDtmP->nullFeatureId,tilePointsP,(tileIndexP+tofsP->tileNumber)->resolutionPoints[0])) goto errexit ;
/*
**  Reset Number Of DTM Points
*/
    dtmP->numPoints = 0 ;    
    filteredDtmP->numPoints = 0 ;    
   }
/*
** Write Number Of Points
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"totalFilteredPts = %8ld ** numFulcrumPts = %8ld",totalFilteredPts,numFulcrumPts) ;
   }
/*
** Write Tiles
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Tiles") ;
 mrDtmHeader.tileIndexOffset = fileWritePosition ;
 for( n = 0 ; n < numTiles ; ++n )
   {
    if( bcdtmFwrite(tileIndexP+n,sizeof(BC_MRES2_TILE_FILE_OFFSETS),1,mrDtmFP) != 1 )
      {
       bcdtmWrite_message(1,0,0,"Error Writing ScalableMesh File") ;
       goto errexit ;
      }
   }
/*
** Rewrite Header
*/
 _fseeki64(mrDtmFP,0,SEEK_SET) ;
 if( bcdtmFwrite(( void *) &mrDtmHeader,sizeof(BC_MRES2_DTM),1,mrDtmFP) != 1 )
   {
    bcdtmWrite_message(1,0,0,"Failed To Re - Write MR DTM File Header") ;
    goto errexit ;
   }
/*
** Filter Fulcrum Points Down To The Number Of Lowest Resolution Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Filtering To Lowest Resolution Level") ;
 if( dbg == 1 ) bcdtmWrite_toFileDtmObject(fulcrumDtmP,L"fulcrum.dtm") ;
 if( bcdtmMultiResolution_filterToLowestResolutionLevelDtmObject(fulcrumDtmP,spatialIndexP,mrDtmFP,mrDtmHeader,minResolutionPoints,decimationFactor,tileIndexP,numTiles)) goto errexit ;
/*
** Write Number Of Fulcrum Points
*/
 if( dbg == 2 ) 
   {
    bcdtmWrite_message(0,0,0,"numFulcrumPts = %8ld totalFilteredPoints = %10ld",numFulcrumPts,totalFilteredPts) ;
    bcdtmObject_setTriangulationParametersDtmObject(fulcrumDtmP,0.0001,0.0001,2,0.0) ;
    if( bcdtmObject_triangulateDtmObject(fulcrumDtmP)) goto errexit ;
    bcdtmWrite_toFileDtmObject(fulcrumDtmP,L"fulcrum.dtm") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( dtmP          != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( fulcrumDtmP   != NULL ) bcdtmObject_destroyDtmObject(&fulcrumDtmP) ;
 if( spatialIndexP != NULL ) bcdtmObject_destroyDtmObject(&spatialIndexP) ;
 if( xyzFP         != NULL ) { bcdtmFile_close(xyzFP)   ; xyzFP   = NULL ; }
 if( mrDtmFP       != NULL ) { bcdtmFile_close(mrDtmFP) ; mrDtmFP = NULL ; }
 if( tilePointsP   != NULL ) { free(tilePointsP) ; tilePointsP = NULL ; }
 if( tileIndexP    != NULL ) { free(tileIndexP)  ; tileIndexP  = NULL ; }
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Total Tile Time = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),filterTime)) ; 
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filtering Tiled XYZ File Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filtering Tiled XYZ File Error") ;
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
int bcdtmMultiResolution_filteredTilePointsCompareFunction(const void *p1P,const void *p2P)
/*
** Compare Function For Qsort Of Void Points
*/
{
 struct filteredTilePoints { long tileNumber ; long resolutionLevel ; long numPoints ; DPoint3d *pointsP ; } *tile1P , *tile2P ;
 tile1P = ( struct filteredTilePoints * ) p1P ;
 tile2P = ( struct filteredTilePoints * ) p2P ;
 if     (  tile1P->tileNumber < tile2P->tileNumber ) return(-1) ;
 else if(  tile1P->tileNumber > tile2P->tileNumber ) return( 1) ;
 else if(  tile1P->resolutionLevel > tile2P->resolutionLevel ) return(-1) ;
 else if(  tile1P->resolutionLevel < tile2P->resolutionLevel ) return( 1)  ;
 return(0) ;
}


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmMath_calculateNormalVectorsVariationForTriangleVerticesDtmObject
(
 BC_DTM_OBJ *dtmP,                /* ==> Pointer To Dtm Object      */
 //long   vectorOption,             /* ==> VectorOption <1=Surface Derivatives,2=Averaged Triangle Surface Normals> */
 double **normalVectorVariationsPP        /* <== Pointer To Normal Vectors  */ 
)
{
 int  ret=DTM_SUCCESS,dbg=0 ;
 long p1,p2,p3,clPtr,numVectors ;
 long p1p2Brk,p1p3Brk,p2p3Brk ; 
 DPoint3d  trgVector ; 
 DTM_TIN_POINT *p1P,*p2P,*p3P ;
 double avgAngle ;
 DTM_TIN_NODE *nodeP ;
 DTM_CIR_LIST *clistP ; 
 std::list<double> angles ;
 std::list<double>::iterator angleIter ;
 std::list<double>::iterator angleIterEnd ; 
 DVec3d vec1, vec2, normalVec;
 
 
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Normal Vectors For Triangle Vertices") ;
/*
** Initialise
*/ 
 if( *normalVectorVariationsPP != NULL ) { free(*normalVectorVariationsPP) ; *normalVectorVariationsPP = NULL ; }
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check DTM In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Allocate Memory For Normal Vectors
*/
 *normalVectorVariationsPP = ( double * ) malloc (dtmP->numPoints * sizeof(double)) ;
 if( *normalVectorVariationsPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Calculate Normals Vectors
*/     
/*
**     Scan Tin
*/
  
  DVec3d normalVecInZAxisDir;

  normalVecInZAxisDir.x = 0 ;
  normalVecInZAxisDir.y = 0 ;
  normalVecInZAxisDir.z = 1 ;

  for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
    {
    (*normalVectorVariationsPP)[p1] = 0;

     avgAngle = 0;
     numVectors  = 0 ;
     angles.clear();
     /*
     avgVector.x = 0.0 ;
     avgVector.y = 0.0 ;
     avgVector.z = 0.0 ;
*/

     nodeP = nodeAddrP(dtmP,p1) ;
     if( ( clPtr = nodeP->cPtr ) != dtmP->nullPtr )
       {
        clistP = clistAddrP(dtmP,clPtr) ;
        if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistP->pntNum)) < 0 ) goto errexit ;
        while( clPtr != dtmP->nullPtr )
          {
           p3 = clistP->pntNum ;
           clPtr = clistP->nextPtr ;
           if( nodeP->hPtr != p2 )
             {
/*
**           Check For Break lines
*/
             p1p2Brk = bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2) ;
             p1p3Brk = bcdtmList_testForBreakLineDtmObject(dtmP,p1,p3) ;
             p2p3Brk = bcdtmList_testForBreakLineDtmObject(dtmP,p2,p3) ;
/*
**           Calculate Normal For Triangle
*/
             /*
             ** Calculate point Address
             */
              p1P = pointAddrP(dtmP,p1) ;
              p2P = pointAddrP(dtmP,p2) ;
              p3P = pointAddrP(dtmP,p3) ;
             /*
             ** Calculate Vectors
             */
              
              vec1.x = p2P->x - p1P->x ;
              vec1.y = p2P->y - p1P->y ;
              vec1.z = p2P->z - p1P->z ;
              vec2.x = p3P->x - p1P->x ;
              vec2.y = p3P->y - p1P->y ;
              vec2.z = p3P->z - p1P->z ;
              
              bsiDVec3d_normalize(&vec1, &vec1);
              bsiDVec3d_normalize(&vec2, &vec2);
                                        
              bsiDVec3d_normalizedCrossProduct(&normalVec, &vec1, &vec2);

              trgVector.x = normalVec.x;
              trgVector.y = normalVec.y;
              trgVector.z = normalVec.z;              

              //bcdtmMath_calculateNormalVectorToPlaneDtmObject(dtmP,p1,p3,p2,&trgVector) ;

/*=
**            Normalize the normal 
*/                            
              /*
              double s = 1.0 / sqrt((trgVector.x * trgVector.x) + (trgVector.y * trgVector.y) + (trgVector.z * trgVector.z));
              trgVector.x *= s;
              trgVector.y *= s;
              trgVector.z *= s;
              */
              
              DVec3d triNormalVec;

              triNormalVec.x = trgVector.x;
              triNormalVec.y = trgVector.y;
              triNormalVec.z = trgVector.z;
              
              DVec3d triNormalVecX;
              DVec3d triNormalVecMinusX;

              triNormalVecX.x = 1;
              triNormalVecX.y = 0;
              triNormalVecX.z = 0;

              triNormalVecMinusX.x = -1;
              triNormalVecMinusX.y = 0;
              triNormalVecMinusX.z = -1;
                                                 
              double angleBetweenVector = bsiDVec3d_angleBetweenVectors(&normalVecInZAxisDir,
                                                                        &triNormalVec);

              //Assuming that the data are 2.5D, the angle difference should not be 
              //more than 90 degrees.
              if (angleBetweenVector > PI / 2)
                {
                angleBetweenVector = PI - angleBetweenVector;
                }

              assert(angleBetweenVector < PI + 0.00001);
             
              avgAngle += angleBetweenVector;
              angles.push_back(angleBetweenVector);
                                             
              ++numVectors ;
             } 
           p2 = p3 ;
           if( clPtr != dtmP->nullPtr )
                {
                clistP = clistAddrP(dtmP,clPtr);
                }
          }
        
        avgAngle /= numVectors;       

        angleIter = angles.begin();
        angleIterEnd = angles.end();

        (*normalVectorVariationsPP)[p1] = 0;

        while (angleIter != angleIterEnd)
            {
            (*normalVectorVariationsPP)[p1] += abs(*angleIter - avgAngle);
            
            angleIter++;
            }

        (*normalVectorVariationsPP)[p1] /= angles.size();
       }
     else
        {
/*
**   Store Vector
*/
        (*normalVectorVariationsPP)[p1] = 0 ;         
        }
    }     
   
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Normal Vectors For Triangle Vertices Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Normal Vectors For Triangle Vertices Error") ;
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

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmMultiResolution_filterToLowestResolutionLevelDtmObject
(
 BC_DTM_OBJ                 *dtmP,
 BC_DTM_OBJ                 *indexP,   
 FILE                       *mrDtmFP,
 BC_MRES2_DTM               mrDtmHeader,
 long                       numResolutionPoints,
 long                       decimationFactor,
 BC_MRES2_TILE_FILE_OFFSETS *tileOffsetsP,
 long                       numTiles
)
{
 int ret=DTM_SUCCESS,dbg=0,tdbg=0,cdbg=0 ;
 long n,m,point,numPoints,numResolutionLevels,numFilteredPts,numPointsRemove ;
 long dtmFeature,tileNumber,firstPoint,lastPoint,numTilePoints,filterTime=0 ;
 long numFilteredTiles=0,memFilteredTiles=0,memFilteredTilesInc=0 ;
 long totalPoints,numPointsBeforeFilter,resolutionPoints[100],mrDtmResolutionLevels ;
 __int64 fileOffset=0 ;
 char label[24],buffer[128],dtmFileName[128] ;
 DPoint3d  *p3dP,*tilePtsP=NULL ;
 DTM_TIN_POINT  *pointP ;
 BC_DTM_FEATURE *dtmFeatureP ;
 BC_DTM_OBJ *filterDtmP=NULL,*filteredDtmP=NULL,*tempDtmP=NULL ;
 struct filteredTilePoints { long tileNumber ; long resolutionLevel ; long numPoints ; DPoint3d *pointsP ; } *filtP,*filt1P,*filt2P,*filteredTilePtsP=NULL ;
 WChar *wFileNameP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Filtering To Lowest Resolution Level") ;
    bcdtmWrite_message(0,0,0,"dtmP                 = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"indexP               = %p",indexP) ;
    bcdtmWrite_message(0,0,0,"mrDtmFP              = %p",&mrDtmFP) ;
    bcdtmWrite_message(0,0,0,"mrDtmHeader          = %p",&mrDtmHeader) ;
    bcdtmWrite_message(0,0,0,"numPoints            = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"numResolutionPoints  = %8ld",numResolutionPoints) ;
    bcdtmWrite_message(0,0,0,"decimationFactor     = %8ld",decimationFactor) ;
    bcdtmWrite_message(0,0,0,"tileOffsetsP         = %p",tileOffsetsP) ;
    bcdtmWrite_message(0,0,0,"numTiles             = %8ld",numTiles) ;
   }
/*
** Write Number Of Points Per Tile
*/
 if( dbg == 2 )
   {
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       bcdtmWrite_message(0,0,0,"Tile[%8I64d] ** numPoints = %8ld",dtmFeatureP->dtmUserTag,dtmFeatureP->numDtmFeaturePts) ;
       if( dtmFeatureP->dtmUserTag == 244 )
         {
          bcdtmWrite_message(0,0,0,"Tile[%8I64d] ** Number Of Points = %8ld",dtmFeatureP->dtmUserTag,dtmFeatureP->numDtmFeaturePts) ;
          for( point = dtmFeatureP->dtmFeaturePts.firstPoint ; point < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ; ++point )
            {
             pointP = pointAddrP(dtmP,point) ;
             bcdtmWrite_message(0,0,0,"Point[%4ld] = %12.5lf %12.5lf %12.5",point-dtmFeatureP->dtmFeaturePts.firstPoint,pointP->x,pointP->y,pointP->z) ;
            } 
         }
      }
  } 
/*
** Initialise
*/
 filterTime = bcdtmClock() ; 
 if( numResolutionPoints < 1000 ) numResolutionPoints = 1000 ;
/*
** Set Number Of Resolution Levels
*/
 numPoints = dtmP->numPoints ;
 numResolutionLevels = 0 ;
 while( numPoints > numResolutionPoints )
   {
    ++numResolutionLevels ;
    numPoints = numPoints / decimationFactor ;
   }
 if( numResolutionLevels + mrDtmHeader.numResolutionLevels > 20 ) numResolutionLevels = 20 - mrDtmHeader.numResolutionLevels ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numResolutionLevels = %8ld",numResolutionLevels) ;
/*
** Only Filter If necessary
*/
 if( numResolutionLevels > 0 )
   {
/*
**  Initialise Memory Allocation Parameters
*/
    numFilteredTiles = 0 ; 
    memFilteredTiles = 0 ; 
    memFilteredTilesInc = mrDtmHeader.numTiles * numResolutionLevels ; 
/*
**  Create Dtm Objects For Filter Function
*/
    if( bcdtmObject_cloneDtmObject(dtmP,&filterDtmP)) goto errexit ;
    if( bcdtmObject_createDtmObject(&filteredDtmP)) goto errexit ;
/*
**  Write Triangulated DTM's To File
*/
    if( dbg == 2 )
      { 
       filterDtmP->numFeatures = 0 ; 
       filterDtmP->edgeOption = 3 ;
       filterDtmP->maxSide = 16000.0 ;
       if( bcdtmObject_triangulateDtmObject(filterDtmP)) goto errexit ;
       bcdtmWrite_toFileDtmObject(filterDtmP,L"resolution0.dtm") ;
       bcdtmObject_destroyDtmObject(&filterDtmP) ; 
       if( bcdtmObject_cloneDtmObject(dtmP,&filterDtmP)) goto errexit ;
      }
/*
**  Filter To Lowest Resolution Points
*/
    filterTime = bcdtmClock() ; 
    for( n = 0 ; n < numResolutionLevels ; ++n )
      {
       numPointsBeforeFilter = filterDtmP->numPoints ;
       numPointsRemove = filterDtmP->numPoints - filterDtmP->numPoints / decimationFactor ;
       if( bcdtmMultiResolution_tinDecimateGroupSpotsDtmObject(filterDtmP,1,1,numPointsRemove,&numFilteredPts,filteredDtmP)) goto errexit ;
//       if( bcdtmMultiResolution_tileDecimateGroupSpotsDtmObject(filterDtmP,numPointsRemove,&numFilteredPts,filteredDtmP)) goto errexit ;
       if( dbg == 1 ) 
         {
          bcdtmWrite_message(0,0,0,"Number of Points Before Filter      = %8ld",numPointsBeforeFilter) ;
          bcdtmWrite_message(0,0,0,"**** Number of Points After  Filter = %8ld",filterDtmP->numPoints) ;
          bcdtmWrite_message(0,0,0,"**** Number of Points Filtered      = %8ld",filteredDtmP->numPoints) ;
          bcdtmWrite_message(0,0,0,"**** Number of Total Points         = %8ld",filterDtmP->numPoints+filteredDtmP->numPoints) ;
         }
       if( cdbg )
         { 
          if( numPointsBeforeFilter != filterDtmP->numPoints + filteredDtmP->numPoints )
            {
             bcdtmWrite_message(1,0,0,"numPointsBeforeFiltering = %8ld ** numPointsAfterFiltering = %8ld",numPointsBeforeFilter,filterDtmP->numPoints+filteredDtmP->numPoints) ;
             goto errexit ;
            } 
         } 
/*
**     Write Triangulated Filtered DTM To File
*/
       if( dbg == 1 ) 
         {
          if( bcdtmObject_cloneDtmObject(filterDtmP,&tempDtmP)) goto errexit ;
          tempDtmP->numFeatures = 0 ; 
          tempDtmP->edgeOption = 3 ;
          tempDtmP->maxSide = 16000.0 ;
          if( bcdtmObject_triangulateDtmObject(tempDtmP)) goto errexit ;
          sprintf(buffer,"%ld",n+1) ;
          strcpy(dtmFileName,"resolution") ;
          strcat(dtmFileName,buffer) ;
          strcat(dtmFileName,".dtm") ;
          bcdtmUtility_convertMbsToWcs(dtmFileName,&wFileNameP) ;
          bcdtmWrite_toFileDtmObject(tempDtmP,wFileNameP) ;
          if( wFileNameP != NULL ) free(wFileNameP) ;
          bcdtmObject_destroyDtmObject(&tempDtmP) ;
         }  
/*
**     Copy Filtered Points To Filtered Point Structure
*/
       totalPoints = 0 ;
       for( dtmFeature = 0 ; dtmFeature < filteredDtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(filteredDtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots )
            {
             tileNumber     = (long) dtmFeatureP->dtmUserTag ;
             numTilePoints  = dtmFeatureP->numDtmFeaturePts ; 
             firstPoint     = dtmFeatureP->dtmFeaturePts.firstPoint ;
             lastPoint      = firstPoint + numTilePoints - 1 ;
             totalPoints    = totalPoints + numTilePoints ;
/*
**           Allocate Memory For Tile Points
*/ 
             tilePtsP = ( DPoint3d * ) malloc(numTilePoints*sizeof(DPoint3d)) ;
             if( tilePtsP == NULL )
               {
                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               } 
/*
**           Copy Points
*/
             p3dP = tilePtsP ;
             for( point = firstPoint ; point <= lastPoint ; ++point )
               {
                pointP = pointAddrP(filteredDtmP,point) ;
                p3dP->x = pointP->x ;
                p3dP->y = pointP->y ;
                p3dP->z = pointP->z ;
                ++p3dP ;
               } 
/*
**           Check Filtered Tile Points Memory
*/
             if( numFilteredTiles == memFilteredTiles )
               {
                memFilteredTiles = memFilteredTiles + memFilteredTilesInc ;
                if( filteredTilePtsP == NULL ) filteredTilePtsP = ( struct filteredTilePoints * ) malloc( memFilteredTiles * sizeof( struct filteredTilePoints )) ;
                else                           filteredTilePtsP = ( struct filteredTilePoints * ) realloc( filteredTilePtsP, memFilteredTiles * sizeof( struct filteredTilePoints )) ;  
                if( filteredTilePtsP == NULL )
                  {
                   bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                   goto errexit ;
                  } 
                for( filtP = filteredTilePtsP + numFilteredTiles ; filtP < filteredTilePtsP + memFilteredTiles ; ++filtP )
                  {
                   filtP->tileNumber = 0 ;
                   filtP->resolutionLevel = 0 ;
                   filtP->numPoints = 0 ;
                   filtP->pointsP = NULL ;
                  } 
               } 
/*
**           Store Filtered Tile Points
*/        
             (filteredTilePtsP+numFilteredTiles)->tileNumber = tileNumber ;            
             (filteredTilePtsP+numFilteredTiles)->resolutionLevel = n  ;            
             (filteredTilePtsP+numFilteredTiles)->numPoints = numTilePoints ;            
             (filteredTilePtsP+numFilteredTiles)->pointsP   = tilePtsP ;            
             ++numFilteredTiles ;
             tilePtsP = NULL ; 
            }
         }  
/*
**     Check Total Number Of Filtered Points Adds Up
*/
       if( totalPoints != filteredDtmP->numPoints )
         {
          bcdtmWrite_message(1,0,0,"totalPoints = %8ld numPointsFiltered = %8ld",totalPoints,filteredDtmP->numPoints) ;
          goto errexit ;
         } 
      }
/*
**  Write Unfiltered Point To Filtered Tiled Points
*/
    totalPoints = 0 ;
    for( dtmFeature = 0 ; dtmFeature < filterDtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(filterDtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots )
         {
          tileNumber     = (long) dtmFeatureP->dtmUserTag ;
          numTilePoints  = dtmFeatureP->numDtmFeaturePts ; 
          firstPoint     = dtmFeatureP->dtmFeaturePts.firstPoint ;
          lastPoint      = firstPoint + numTilePoints - 1 ;
          totalPoints    = totalPoints + numTilePoints ;
/*
**        Allocate Memory For Tile Points
*/ 
          tilePtsP = ( DPoint3d * ) malloc(numTilePoints*sizeof(DPoint3d)) ;
          if( tilePtsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            } 
/*
**        Copy Points
*/
          p3dP = tilePtsP ;
          for( point = firstPoint ; point <= lastPoint ; ++point )
            {
             pointP = pointAddrP(filterDtmP,point) ;
             p3dP->x = pointP->x ;
             p3dP->y = pointP->y ;
             p3dP->z = pointP->z ;
             ++p3dP ;
            } 
/*
**        Check Filtered Tile Points Memory
*/
          if( numFilteredTiles == memFilteredTiles )
            {
             memFilteredTiles = memFilteredTiles + memFilteredTilesInc ;
             if( filteredTilePtsP == NULL ) filteredTilePtsP = ( struct filteredTilePoints * ) malloc( memFilteredTiles * sizeof( struct filteredTilePoints )) ;
             else                           filteredTilePtsP = ( struct filteredTilePoints * ) realloc( filteredTilePtsP, memFilteredTiles * sizeof( struct filteredTilePoints )) ;  
             if( filteredTilePtsP == NULL )
               {
                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               } 
             for( filtP = filteredTilePtsP + numFilteredTiles ; filtP < filteredTilePtsP + memFilteredTiles ; ++filtP )
               {
                filtP->tileNumber = 0 ;
                filtP->resolutionLevel = 0 ;
                filtP->numPoints = 0 ;
                filtP->pointsP = NULL ;
               } 
            } 
/*
**        Store Filtered Tile Points
*/        
          (filteredTilePtsP+numFilteredTiles)->tileNumber = tileNumber ;            
          (filteredTilePtsP+numFilteredTiles)->resolutionLevel = numResolutionLevels ;            
          (filteredTilePtsP+numFilteredTiles)->numPoints = numTilePoints ;            
          (filteredTilePtsP+numFilteredTiles)->pointsP   = tilePtsP ;            
          ++numFilteredTiles ;
          tilePtsP = NULL ;  
         }
      }  
    if( tdbg ) bcdtmWrite_message(0,0,0,"Total Filter Time = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),filterTime)) ; 
    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"numFilteredTiles = %8ld ** memFilteredTiles = %8ld",numFilteredTiles,memFilteredTiles) ;
/*
**  Sort Filtered Tile Points On Tile Number And Descending Resolution Level
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Filtered Tile Points") ;
    qsort(filteredTilePtsP,numFilteredTiles,sizeof(struct filteredTilePoints),bcdtmMultiResolution_filteredTilePointsCompareFunction) ;
/*
**  Write NumBer Of Points Per Resolution Level
*/
    if( dbg == 2 )
      {
       numPoints = 0 ;
       tileNumber = filteredTilePtsP->tileNumber ;
       for( filtP = filteredTilePtsP ; filtP < filteredTilePtsP+numFilteredTiles ; ++filtP )
         {
          if( filtP->tileNumber == tileNumber ) numPoints = numPoints + filtP->numPoints ;
          else
            {
             bcdtmWrite_message(0,0,0,"Number Of Points For Tile %8ld = %8ld",tileNumber,numPoints) ;
             tileNumber = filtP->tileNumber ;
             numPoints = filtP->numPoints ;
            }  
          bcdtmWrite_message(0,0,0,"tile = %8ld resolutionLevel = %8ld numPoints = %8ld",filtP->tileNumber,filtP->resolutionLevel,filtP->numPoints) ;
         }
       bcdtmWrite_message(0,0,0,"Number Of Points For Tile %8ld = %8ld",tileNumber,numPoints) ;
      } 
/*
**  Update ScalableMesh File
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Updating ScalableMesh File") ;
    mrDtmResolutionLevels = mrDtmHeader.numResolutionLevels + numResolutionLevels  ;
    numTilePoints = 0 ;
    filt1P = filtP = filteredTilePtsP ;
    tileNumber = filtP->tileNumber ;
    while( filtP < filteredTilePtsP + numFilteredTiles ) 
      {
       while( filtP < filteredTilePtsP + numFilteredTiles && filtP->tileNumber == tileNumber ) ++filtP ;
       --filtP ;
/*
**     Count Number of Filtered Points For Tile
*/
       numPoints = 0 ;
       for( n = 0 ; n <= numResolutionLevels ; ++n ) resolutionPoints[n] = 0 ;
       for( filt2P = filt1P ; filt2P <= filtP ; ++filt2P )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"tileNumber = %8ld resolutionLevel = %2ld numPoints = %8ld",filt2P->tileNumber,filt2P->resolutionLevel,filt2P->numPoints) ;
          numPoints = numPoints + filt2P->numPoints ;  
          resolutionPoints[numResolutionLevels-filt2P->resolutionLevel] = filt2P->numPoints ; 
         }
/*
**     Write Out Number Of Points For Resolution Levels For Tile
*/
       if( dbg == 2 )
         {
          buffer[0] = 0 ;
          for( n = 0 ; n <= numResolutionLevels ; ++n )
            {
             sprintf(label,"%ld",resolutionPoints[n]) ;
             strcat(buffer,label) ;
             strcat(buffer," ") ; 
            }
          bcdtmWrite_message(0,0,0,"Tile[%8ld] ** %s",filt1P->tileNumber,buffer) ;
         }
/*
**     Check Memory
*/
       if( numPoints > numTilePoints )
         {
          numTilePoints = numPoints * 2 ;
/*
**        Allocate Memory For Tile Points
*/ 
          if( tilePtsP == NULL ) tilePtsP = ( DPoint3d * ) malloc(numTilePoints*sizeof(DPoint3d)) ;
          else                   tilePtsP = ( DPoint3d * ) realloc(tilePtsP,numTilePoints*sizeof(DPoint3d)) ;
          if( tilePtsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            } 
         }
/*
**     Copy Tile Points
*/
       p3dP = tilePtsP ;
       totalPoints = 0 ;
       for( filt2P = filt1P ; filt2P <= filtP ; ++filt2P )
         {
          if( filt2P->numPoints > 0 ) memcpy(p3dP,filt2P->pointsP,filt2P->numPoints*sizeof(DPoint3d)) ; 
          resolutionPoints[numResolutionLevels-filt2P->resolutionLevel] = filt2P->numPoints ;
          totalPoints = totalPoints + filt2P->numPoints ; 
          p3dP = p3dP + filt2P->numPoints ;
         }
/*
**     Sum The Resolution Level Points
*/
       for( n = 1 ; n <= numResolutionLevels ; ++n )
         {
          resolutionPoints[n] = resolutionPoints[n] + resolutionPoints[n-1] ;
         } 
/*
**     Write Out Points
*/
       if( dbg == 2 )
         {
          bcdtmWrite_message(0,0,0,"Points In Filtered Order For Tile %8ld",filtP->tileNumber) ;
          for( p3dP = tilePtsP ; p3dP < tilePtsP + totalPoints ; ++p3dP )
            {
             bcdtmWrite_message(0,0,0,"Point[%8ld] ** %12.5lf %12.5lf %10.4lf",(long)(p3dP-tilePtsP),p3dP->x,p3dP->y,p3dP->z) ;
            }
         } 
/*
**     Check For Correct Number of Points
*/
      if( totalPoints != (tileOffsetsP+filt1P->tileNumber)->resolutionPoints[0]) 
        {
         bcdtmWrite_message(0,0,0,"Warning **** Inconsistent Point Count ** tile = %8ld  totalpoints = %8ld resolutionPoints = %8ld",filt1P->tileNumber,totalPoints,(tileOffsetsP+filt1P->tileNumber)->resolutionPoints[0]) ;
        } 
/*
**     Update MRDTM File Offsets resolution points
*/
       m = mrDtmResolutionLevels - 1 ;
       for( n = mrDtmHeader.numResolutionLevels - 1 ; n >= 0 ; --n )
         {
          (tileOffsetsP+filt1P->tileNumber)->resolutionPoints[m] = (tileOffsetsP+filt1P->tileNumber)->resolutionPoints[n] ;
          --m ;
         } 
       for( n = 0 ; n <= numResolutionLevels ; ++n )
         {
          (tileOffsetsP+filt1P->tileNumber)->resolutionPoints[n] = resolutionPoints[n] ;
         } 
/*
**     Write Out Resolution Point Index
*/
       if( dbg == 2 )
         {
          buffer[0] = 0 ;
          for( n = 0 ; n < mrDtmResolutionLevels ; ++n )
            {
             sprintf(label,"%ld",(tileOffsetsP+filt1P->tileNumber)->resolutionPoints[n]) ;
             strcat(buffer,label) ;
             strcat(buffer," ") ; 
            }
          bcdtmWrite_message(0,0,0,"Tile[%8ld] ** %s",filt1P->tileNumber,buffer) ;
         } 
/*
**     Write Updated Resolution Points Index To ScalableMesh File
*/
       fileOffset = mrDtmHeader.tileIndexOffset+filtP->tileNumber*sizeof(BC_MRES2_TILE_FILE_OFFSETS) ;
       _fseeki64(mrDtmFP,fileOffset,SEEK_SET) ;
       if( bcdtmFwrite(tileOffsetsP+filt1P->tileNumber,sizeof(BC_MRES2_TILE_FILE_OFFSETS),1,mrDtmFP) != 1 )  
         {
          bcdtmWrite_message(0,0,0,"Error Writing ScalableMesh File") ;
          goto errexit ;
         }  
/*
**     Write Resolution Points To Mr File
*/
       fileOffset = (tileOffsetsP+filt1P->tileNumber)->fileOffset ;
       _fseeki64(mrDtmFP,fileOffset,SEEK_SET) ;
       if( bcdtmFwrite(tilePtsP,totalPoints*sizeof(DPoint3d),1,mrDtmFP) != 1 )  
         {
          bcdtmWrite_message(0,0,0,"Error Writing ScalableMesh File") ;
          goto errexit ;
         } 
/*
**     Get Next Tile
*/
       ++filtP ;
       if( filtP <  filteredTilePtsP + numFilteredTiles ) 
         {
          filt1P = filtP ;
          tileNumber = filtP->tileNumber ;
         }
      }
/*
**  Rewrite File Header
*/
    mrDtmHeader.numResolutionLevels = mrDtmResolutionLevels ;
    _fseeki64(mrDtmFP,0,SEEK_SET) ;
    if( bcdtmFwrite(&mrDtmHeader,sizeof(BC_MRES2_DTM),1,mrDtmFP) != 1 )  
      {
       bcdtmWrite_message(0,0,0,"Error Writing ScalableMesh File") ;
       goto errexit ;
      } 
   }
/*
** Clean Up
*/
 cleanup :
 if( tilePtsP     != NULL ) free(tilePtsP) ;
 for( n = 0 ; n < numFilteredTiles ; ++n )
   {
    if( (filteredTilePtsP+n)->pointsP != NULL ) free((filteredTilePtsP+n)->pointsP) ;
   }
 if( filterDtmP   != NULL ) bcdtmObject_destroyDtmObject(&filterDtmP) ;
 if( filteredDtmP != NULL ) bcdtmObject_destroyDtmObject(&filteredDtmP) ;
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Total Filter Time = %8.3lf Secs",bcdtmClock_elapsedTime(bcdtmClock(),filterTime)) ; 
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filtering To Lowest Resolution Level Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filtering To Lowest Resolution Level Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
#endif 
