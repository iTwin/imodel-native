/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshUtilityFunctions.cpp $
|    $RCSfile: ScalableMeshUtilityFunctions.cpp,v $
|   $Revision: 1.18 $
|       $Date: 2013/03/27 15:53:21 $
|     $Author: Jean-Francois.Cote $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <ScalableMesh/IScalableMeshQuery.h>
#include "InternalUtilityFunctions.h"
#include "SMPointIndex.h"

using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;
USING_NAMESPACE_BENTLEY_TERRAINMODEL


/*----------------------------------------------------------------------+
| Include standard library header files                                 |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| Include MicroStation SDK header files
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| Include Imagepp header files                                          |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| Include BCivil general header files                                   |
+----------------------------------------------------------------------*/

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/

/*==================================================================*/
/*                                                                  */
/*          INTERNAL FUNCTIONS                                      */
/*                                                                  */
/*==================================================================*/






/*==================================================================*/
/*                                                                  */
/* BoxBoxIntersectionRange Code                                     */
/*                                                                  */
/* Note : This code was kindly provided by Earlin Lutz.             */
/*==================================================================*/

// BARE C CODE -- to be inserted/included in file with includes etc.

// Initialize a DPlane3d from origin, x point, and y point.
// (Plane normal is unit length)
static void InitPlane (DPlane3d & plane, DPoint3d const & xyz0, DPoint3d const &xyz1, DPoint3d const &xyz2)
    {
    DVec3d normal;
    DVec3d   vector01, vector02;
    bsiDVec3d_subtractDPoint3dDPoint3d (&vector01, &xyz1, &xyz0);
    bsiDVec3d_subtractDPoint3dDPoint3d (&vector02, &xyz2, &xyz0);
    bsiDVec3d_normalizedCrossProduct (&normal, &vector01, &vector02);
    bsiDPlane3d_initFromOriginAndNormal (&plane, &xyz0, &normal);
    }

static int s_faceToBoxPoint[6][4] =
    {
    {1,0,2,3},
    {4,5,7,6},
    {0,4,6,2},
    {1,3,7,5},
    {0,1,5,4},
    {2,6,7,3},
    };

// Construct plane origin&normal for each of the 6 planes of an 8 point box in standard point order.
void InitBoxPlanes (DPoint3d const *pBoxPoints, DPlane3d *pPlanes)
    {
    for (int planeIndex = 0; planeIndex < 6; planeIndex++)
        {
        InitPlane (
                pPlanes[planeIndex], 
                pBoxPoints[s_faceToBoxPoint[planeIndex][0]],
                pBoxPoints[s_faceToBoxPoint[planeIndex][1]],
                pBoxPoints[s_faceToBoxPoint[planeIndex][3]]
                );
        }
    }

// Return the 5 point linestring around a face of a box.
int GetBoxFace (DPoint3d const *pBoxPoints, int faceSelect, DPoint3d *pPoints)
    {
    faceSelect = faceSelect % 6;
    for (int i = 0; i < 4; i++)
        {
        pPoints[i] = pBoxPoints[s_faceToBoxPoint[faceSelect][i]];
        }
    pPoints[4] = pPoints[0];
    return 5;
    }

// Clip a (small) CONVEX polygon to a plane.
// Clipped polygon is updated IN PLACE
// The number of points may decrease (possibly to zero).
// The number of points may increase.  For a convex polygon, the increase can only be one point.
//   (For a non convex polygon, the increase can be larger -- the algorithm is still correct under "parity" logic
//     that handles concave sections as XOR dropouts)
// USES FIXED BUFFER FOR 100 POINTS -- perfectly adequate for range box clips, but not for general case.
static void ClipConvexPolygonToPlane (DPoint3d *polygonPoints, int &n, DPlane3d & plane)
    {
    DPoint3d clippedPoints[100];
    int m = 0;
    double h0=0, h1=0;
    for (int i = 0; i < n; i++)
        {
        h1 = bsiDPlane3d_evaluate (&plane, &polygonPoints[i]);
        if (i == 0)
            {
            if (h1 <= 0.0)
               clippedPoints[m++] = polygonPoints[i];
            }
        else
            {
            if (h0 * h1 < 0.0)
                {
                double s = -h0 / (h1 - h0);
                bsiDPoint3d_interpolate (&clippedPoints[m++],
                    &polygonPoints[i-1], s, &polygonPoints[i]);
                }
            if (h1 <= 0.0)
                clippedPoints[m++] = polygonPoints[i];
            }
        h0 = h1;
        }
    if (m > 0)
        {
        if (!bsiDPoint3d_pointEqual (&clippedPoints[0], &clippedPoints[m-1]))
            clippedPoints[m++] = clippedPoints[0];
        memcpy (polygonPoints, clippedPoints, m * sizeof (DPoint3d));
        }
    n = m;
    }

// Use the planes of box2 as clippers for the faces of box1.
// Extend the (evolving) range each clipped face.
void ExtendRangeForBoxIntersect (DPoint3d const *pBoxPoints1, DPoint3d const *pBoxPoints2, DRange3d &range)
    {
    DPoint3d xyz[100];
    DPlane3d planeSet2[6];
    InitBoxPlanes (pBoxPoints2, planeSet2);
    for (int select1 = 0; select1 < 6; select1++)
        {
        int nXYZ = GetBoxFace (pBoxPoints1, select1, xyz);
        for (int select2 = 0; select2 < 6; select2++)
            ClipConvexPolygonToPlane (xyz, nXYZ, planeSet2[select2]);
        bsiDRange3d_extendByDPoint3dArray (&range, xyz, nXYZ);
        }
    }

// Return the range of the intersection of two boxes.
// Each box is described by 8 corner points.
// The corner points correspond to the "x varies fastest, then y, then z" ordering.
// Hence the point order for the unit cube coordinates and view box positions is
//  (0,0,0)  (i.e. lower left rear)
//  (1,0,0)  (i.e. lower right rear)
//  (0,1,0)  (i.e. upper left rear)
//  (1,1,0)  (i.e. upper right rear)
//  (0,0,1)  (i.e. lower left front)
//  (1,0,1)  (i.e. lower right front)
//  (0,1,1)  (i.e. upper left front)
//  (1,1,1)  (i.e. upper right front)
// The box sides are (ASSUMED TO BE) planar -- this is not tested.
// The box sides do  not have to be parallel -- skewed frustum of a perspective view box is ok.
//! @param [in] boxPoints1 8 points of first box.
//! @param [in] boxPoints2 8 points of second box.
//! @param [out] intersectionRange range of the intersection of the two boxes.
void BoxBoxIntersectionRange
(
DPoint3dCP boxPoints1,
DPoint3dCP boxPoints2,
DRange3d &intersectionRange
)
    {
    bsiDRange3d_init (&intersectionRange);
    ExtendRangeForBoxIntersect (boxPoints1, boxPoints2, intersectionRange);
    ExtendRangeForBoxIntersect (boxPoints2, boxPoints1, intersectionRange);
    }

static ::DPoint3d const s_npcViewBox[8] =
    {
    {0,0,0},
    {1,0,0},
    {0,1,0},
    {1,1,0},
    {0,0,1},
    {1,0,1},
    {0,1,1},
    {1,1,1}
    };

/*==================================================================*/
/*                                                                  */
/* End Of The BoxBoxIntersectionRange Code                          */
/*                                                                  */
/*==================================================================*/
 
/*---------------------------------------------------------------------------------**//**
* This function computes the visible area based on the definition of the viewbox
* and the DTM ranges.
* The viewbox coordinate smust be provided in UOR and the correcponding uor to meter
* ratio must be provided.
* @bsimethod                                                    MathieuStPierre  2/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool GetVisibleAreaForView(::DPoint3d** areaPt, int& nbPts, const DPoint3d viewBox[], DRange3d& dtmRange, DRange3d& dtmIntersectionRange)
    {        
    // Work out which bit of the triangulation is displayed on screen.    
    DRange3d dtmViewRange;
    bool isVisible = false;
       
    dtmViewRange = dtmRange;
     
    DPoint3d dtmBox[8];    

    //The BoxBoxIntersectionRange function needs a box to perform correctly, so add a very 
    //small artificial range for any coordinate whose real range equals zero.    
    if (dtmViewRange.low.x == dtmViewRange.high.x)
        {        
        dtmViewRange.low.x -= 0.0000001;        
        }

    if (dtmViewRange.low.y == dtmViewRange.high.y)
        {        
        dtmViewRange.low.y -= 0.0000001;        
        }

    if (dtmViewRange.low.z == dtmViewRange.high.z)
        {        
        dtmViewRange.low.z -= 0.0000001;        
        }

    bsiDRange3d_box2Points(&dtmViewRange, dtmBox);
                
    BoxBoxIntersectionRange(dtmBox, viewBox, dtmIntersectionRange);
     
    if (dtmIntersectionRange.high.x > dtmIntersectionRange.low.x && 
        dtmIntersectionRange.high.y > dtmIntersectionRange.low.y)
        {

        if (areaPt != 0)
            {            
            nbPts = 5;
            *areaPt = new DPoint3d[nbPts];

            DPoint3d lowPt(dtmIntersectionRange.low);
            DPoint3d highPt(dtmIntersectionRange.high);            
                 
            (*areaPt)[0].x = lowPt.x;
            (*areaPt)[0].y = lowPt.y;
            (*areaPt)[0].z = lowPt.z;
            
            (*areaPt)[1].x = (*areaPt)[0].x;
            (*areaPt)[1].y = highPt.y;
            (*areaPt)[1].z = (*areaPt)[0].z;
            
            (*areaPt)[2].x = highPt.x;
            (*areaPt)[2].y = (*areaPt)[1].y ;
            (*areaPt)[2].z = (*areaPt)[0].z;
            
            (*areaPt)[3].x = (*areaPt)[2].x ;
            (*areaPt)[3].y = (*areaPt)[0].y;
            (*areaPt)[3].z = (*areaPt)[0].z;

            (*areaPt)[4].x = (*areaPt)[0].x;
            (*areaPt)[4].y = (*areaPt)[0].y;
            (*areaPt)[4].z = (*areaPt)[0].z;
            }

        isVisible = true;
        }
    
    return isVisible;        
    } 

/*----------------------------------------------------------------------------+
|GetReprojectedBox
| Reprojects the 3D box from source to target GCS.
+----------------------------------------------------------------------------*/ 
void GetReprojectedBox(BaseGCSPtr& sourceGCSPtr, 
                       BaseGCSPtr& targetGCSPtr,  
                       DPoint3d    boxPoints[], 
                       DPoint3d    reprojectedBoxPoints[])
    { 
    GeoPoint  sourceLatLong;
    GeoPoint  targetLatLong;    
    StatusInt stat1;
    StatusInt stat2;
    StatusInt stat3;
    
    for (int boxPtInd = 0; boxPtInd < 8; boxPtInd++)
        {                   
        stat1 = sourceGCSPtr->LatLongFromCartesian(sourceLatLong, boxPoints[boxPtInd]);
        stat2 = sourceGCSPtr->LatLongFromLatLong(targetLatLong, sourceLatLong, *targetGCSPtr);
        stat3 = targetGCSPtr->CartesianFromLatLong(reprojectedBoxPoints[boxPtInd], targetLatLong);            
        }     
    }



/**----------------------------------------------------------------------------
 Move an intersection point on a vanishing line a litte from the vanishing line.
-----------------------------------------------------------------------------*/
void MoveIntersectionPointFromVanishingLine(const HGF2DLocation& intersectionPoint,  
                                            const HGF2DLocation& pointOnIntersectedSegment, 
                                            HGF2DLocation&       movedIntersectionPoint)
    {
    if (pointOnIntersectedSegment.GetX() < intersectionPoint.GetX())
        {
        movedIntersectionPoint.SetX(intersectionPoint.GetX() - (intersectionPoint.GetX() - pointOnIntersectedSegment.GetX()) / 10);
        }
    else
    if (pointOnIntersectedSegment.GetX() > intersectionPoint.GetX())
        {
        movedIntersectionPoint.SetX(intersectionPoint.GetX() + (pointOnIntersectedSegment.GetX() - intersectionPoint.GetX()) / 10);                           
        }
    else
        {
        movedIntersectionPoint.SetX(intersectionPoint.GetX());                            
        }

    if (pointOnIntersectedSegment.GetY() < intersectionPoint.GetY())
        {
        movedIntersectionPoint.SetY(intersectionPoint.GetY() - (intersectionPoint.GetY() - pointOnIntersectedSegment.GetY()) / 10);
        }
    else
    if (pointOnIntersectedSegment.GetY() > intersectionPoint.GetY())
        {
        movedIntersectionPoint.SetY(intersectionPoint.GetY() + (pointOnIntersectedSegment.GetY() - intersectionPoint.GetY()) / 10);                           
        }
    else
        {
        movedIntersectionPoint.SetY(intersectionPoint.GetY());                            
        }    
    }

/*----------------------------------------------------------------------------+
|GetShapeInFrontOfProjectivePlane
| This function returns the tile in front of the projective plane and the 
| percentage of the tile in front of the projective plane.
+----------------------------------------------------------------------------*/ 
int GetShapeInFrontOfProjectivePlane(vector<DPoint3d>&       shapeInFrontOfProjectivePlane, 
                                     double&                 ratioShapeInFrontToTile,
                                     const vector<DPoint3d>& tileborderPoints, 
                                     const double            rootToViewMatrix[][4])
    {  
    int status = SUCCESS;

    assert(rootToViewMatrix[3][3] != 0.0);    

    ratioShapeInFrontToTile = 1.0;

    double   shapeWithVanishingLine[8];
    DRange3d tileExtent;
        
    bsiDRange3d_initFromArray(&tileExtent, &*tileborderPoints.begin(), (int)tileborderPoints.size());
    
    //Line perpendicular to X
    if (rootToViewMatrix[3][0] == 0)
        {
        shapeWithVanishingLine[0] = tileExtent.low.x;
        shapeWithVanishingLine[0] -= fabs(shapeWithVanishingLine[0] / 100); // Add some margin.
        shapeWithVanishingLine[1] = -rootToViewMatrix[3][3] / rootToViewMatrix[3][1];                                    

        shapeWithVanishingLine[2] = tileExtent.high.x;
        shapeWithVanishingLine[2] += fabs(shapeWithVanishingLine[2] / 100); // Add some margin.
        shapeWithVanishingLine[3] = shapeWithVanishingLine[1];
        }
    else //Line perpendicular to Y
    if (rootToViewMatrix[3][1] == 0)
        {                
        shapeWithVanishingLine[0] = -rootToViewMatrix[3][3] / rootToViewMatrix[3][0];                                    
        shapeWithVanishingLine[1] = tileExtent.low.y;
        shapeWithVanishingLine[1] -= fabs(shapeWithVanishingLine[1] / 100); // Add some margin.

        shapeWithVanishingLine[2] = shapeWithVanishingLine[0];
        shapeWithVanishingLine[3] = tileExtent.high.y;
        shapeWithVanishingLine[3] += fabs(shapeWithVanishingLine[3] / 100); // Add some margin.                
        }
    else
        {
        // Find point of the line that limit the domain                                         
        shapeWithVanishingLine[0] = tileExtent.low.x;
        shapeWithVanishingLine[0] -= fabs(shapeWithVanishingLine[0] / 100); // Add some margin.
        shapeWithVanishingLine[1] =(-rootToViewMatrix[3][3] - rootToViewMatrix[3][0] * shapeWithVanishingLine[0]) / rootToViewMatrix[3][1];            
        
        shapeWithVanishingLine[2] = tileExtent.high.x;
        shapeWithVanishingLine[2] += fabs(shapeWithVanishingLine[2] / 100); // Add some margin.
        shapeWithVanishingLine[3] =(-rootToViewMatrix[3][3] - rootToViewMatrix[3][0] * shapeWithVanishingLine[2]) / rootToViewMatrix[3][1];
        //shapeWithVanishingLine[3].z = 0.0;
        }

    HFCPtr<HGF2DCoordSys> pDummyCoordSys(new HGF2DCoordSys);

    HGF2DLocation firstVanishingLinePt(shapeWithVanishingLine[0], shapeWithVanishingLine[1], pDummyCoordSys);
    HGF2DLocation secondVanishingLinePt(shapeWithVanishingLine[2], shapeWithVanishingLine[3], pDummyCoordSys);                   

    HVE2DSegment  vanishingSegment(firstVanishingLinePt, secondVanishingLinePt);
    
    int nbPoints = (int)tileborderPoints.size();
    
    HArrayAutoPtr<double>            tileCornerCoordinateBuffer2D(new double[nbPoints * 2]);
    vector<DPoint3d>::const_iterator pointIter(tileborderPoints.begin());    

    for (int ptInd = 0; ptInd < nbPoints; ptInd++)
        {              
        tileCornerCoordinateBuffer2D[ptInd * 2] = pointIter->x;
        tileCornerCoordinateBuffer2D[ptInd * 2 + 1] = pointIter->y;
        pointIter++;
        }  

    assert(pointIter == tileborderPoints.end());
                                                
    HGF2DLocationCollection shapeA;
    HGF2DLocationCollection shapeB;
    HGF2DLocation           pointInShapeA(pDummyCoordSys);
    HGF2DLocation           pointInShapeB(pDummyCoordSys);
    HGF2DLocation           intersectionPoint;
    HGF2DLocation           borderSegmentFirstPt(pDummyCoordSys);
    HGF2DLocation           borderSegmentsecondPt(pDummyCoordSys);    
    bool                    isInShapeB = false;
                
    //Vanishing line intersecting with the tile border.
    for (int segmentInd = 0; segmentInd < 4; segmentInd++)
        {
        HGF2DLocation firstPt(tileCornerCoordinateBuffer2D[segmentInd * 2], tileCornerCoordinateBuffer2D[segmentInd * 2 + 1], pDummyCoordSys);

        borderSegmentFirstPt.SetX(tileCornerCoordinateBuffer2D[segmentInd * 2]);
        borderSegmentFirstPt.SetY(tileCornerCoordinateBuffer2D[segmentInd * 2 + 1]);
        
        if (segmentInd  == 3)
            {
            borderSegmentsecondPt.SetX(tileCornerCoordinateBuffer2D[0]);
            borderSegmentsecondPt.SetY(tileCornerCoordinateBuffer2D[1]);
            }
        else
            {
            borderSegmentsecondPt.SetX(tileCornerCoordinateBuffer2D[(segmentInd + 1) * 2]);
            borderSegmentsecondPt.SetY(tileCornerCoordinateBuffer2D[(segmentInd + 1) * 2 + 1]);
            }

        HVE2DSegment borderSegment(borderSegmentFirstPt, borderSegmentsecondPt);
                                                
        if (vanishingSegment.IntersectSegmentSCS(borderSegment, &intersectionPoint) == HVE2DSegment::CROSS_FOUND)
            {
            if (isInShapeB == false)
                {
                //Create an intersection point not directly on the vanishing line.
                HGF2DLocation intersectionPointToShapeA(pDummyCoordSys);
                HGF2DLocation intersectionPointToShapeB(pDummyCoordSys);
                
                MoveIntersectionPointFromVanishingLine(intersectionPoint, 
                                                       borderSegmentFirstPt, 
                                                       intersectionPointToShapeA);

                MoveIntersectionPointFromVanishingLine(intersectionPoint, 
                                                       borderSegmentsecondPt, 
                                                       intersectionPointToShapeB);
               
                if ((shapeA.empty() == true) || (shapeA.back() != borderSegmentFirstPt))
                    {
                    shapeA.push_back(borderSegmentFirstPt);
                    }
                
                shapeA.push_back(intersectionPointToShapeA);

                if ((shapeB.empty() == true) || (shapeB.back() != intersectionPointToShapeB))
                    {
                    shapeB.push_back(intersectionPointToShapeB);
                    }
                
                shapeB.push_back(borderSegmentsecondPt);      
                pointInShapeA = borderSegmentFirstPt;
                pointInShapeB = borderSegmentsecondPt;
                isInShapeB = true;
                }
            else  
                {
                HGF2DLocation intersectionPointToShapeA(pDummyCoordSys);
                HGF2DLocation intersectionPointToShapeB(pDummyCoordSys);
                
                MoveIntersectionPointFromVanishingLine(intersectionPoint, 
                                                       borderSegmentFirstPt, 
                                                       intersectionPointToShapeB);

                MoveIntersectionPointFromVanishingLine(intersectionPoint, 
                                                       borderSegmentsecondPt, 
                                                       intersectionPointToShapeA);
               
                if ((shapeB.empty() == true) || (shapeB.back() != borderSegmentFirstPt))
                    {
                    shapeB.push_back(borderSegmentFirstPt);
                    }

                shapeB.push_back(intersectionPointToShapeB);                           
        
                if ((shapeA.empty() == true) || (shapeA.back() != intersectionPointToShapeA))
                    {
                    shapeA.push_back(intersectionPointToShapeA);
                    }
                
                shapeA.push_back(borderSegmentsecondPt);                    
                isInShapeB = false; 
                }
            } 
        else
            {
            if (isInShapeB == false)
                {
                if ((shapeA.empty() == true) || (shapeA.back() != borderSegmentFirstPt))
                    {                    
                    shapeA.push_back(borderSegmentFirstPt);                    
                    }
                
                shapeA.push_back(borderSegmentsecondPt);                    
                }
            else
                {
                if ((shapeB.empty() == true) || (shapeB.back() != borderSegmentFirstPt))
                    {                    
                    shapeB.push_back(borderSegmentFirstPt);                    
                    }
                
                shapeB.push_back(borderSegmentsecondPt);                    
                }
            }                                                
        }  

    if (shapeB.size() > 0)
        {                                    
        double divA = rootToViewMatrix[3][0] * pointInShapeA.GetX() + rootToViewMatrix[3][1] * pointInShapeA.GetY() + rootToViewMatrix[3][3];            
        double divB = rootToViewMatrix[3][0] * pointInShapeB.GetX() + rootToViewMatrix[3][1] * pointInShapeB.GetY() + rootToViewMatrix[3][3];            
                        
        HGF2DLocationCollection* pLocationCollection = 0;
        
        if (divA > 0)
            {
            assert(divB < 0);
            pLocationCollection = &shapeA;               
            }    
        else
            {                    
            pLocationCollection = &shapeB;               
            }
                        
        nbPoints = (int)pLocationCollection->size();                

        HArrayAutoPtr<double> tileCutByVanishingLineCornerBuffer2D(new double[nbPoints * 2]);

        HGF2DLocationCollection::iterator locationIter(pLocationCollection->begin());
        HGF2DLocationCollection::iterator locationIterEnd(pLocationCollection->end());

        int      borderPtInd = 0;
        DPoint3d point3D;

        point3D.z = 0;

        while (locationIter != locationIterEnd)
            {                        
            point3D.x = locationIter->GetX();
            point3D.y = locationIter->GetY();
            
            shapeInFrontOfProjectivePlane.push_back(point3D);
                        
            tileCutByVanishingLineCornerBuffer2D[borderPtInd * 2] = locationIter->GetX();
            tileCutByVanishingLineCornerBuffer2D[borderPtInd * 2 + 1] = locationIter->GetY();

            borderPtInd++;
            locationIter++;
            }     

        HVE2DPolygonOfSegments tileOriShape(8, tileCornerCoordinateBuffer2D, pDummyCoordSys);
        HVE2DPolygonOfSegments tileCutByVanishingLineShape(nbPoints * 2, tileCutByVanishingLineCornerBuffer2D, pDummyCoordSys);
        
        ratioShapeInFrontToTile = tileCutByVanishingLineShape.CalculateArea() / tileOriShape.CalculateArea();

     //   assert(ratioShapeInFrontToTile <= 1.0);   
        }

        return status;
    }

/*----------------------------------------------------------------------------+
| CreateBcDTM
  Utility function that creates a DTM.
+----------------------------------------------------------------------------*/ 
int CreateBcDTM(DTMPtr& dtmPtr)
    {                  
    BC_DTM_OBJ* bcDtmP = 0;
    int status = bcdtmObject_createDtmObject(&bcDtmP);                                            

    if (status == 0)
        {
        BcDTMPtr bcDtmObjPtr;
#ifdef VANCOUVER_API
        bcDtmObjPtr = BcDTM::CreateFromDtmHandle(*bcDtmP);
#else
        bcDtmObjPtr = BcDTM::CreateFromDtmHandle(bcDtmP);
#endif   

        dtmPtr = bcDtmObjPtr.get();
        }    

    return status;
    }

/*----------------------------------------------------------------------------+
| SetClipsToDTM
  Utility function that adds to the given DTM the clips.
+----------------------------------------------------------------------------*/ 
int SetClipsToDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&  dtmPtr,
                   const DRange3d&                 dtmRange,
                   const IScalableMeshClipContainerPtr&   clips)
    {
    HFCPtr<HVEShape> clipShape = CreateShapeFromClips(dtmRange, clips);

    if ((clipShape != NULL) && !clipShape->IsEmpty())
        {
        return SetClipToDTM(dtmPtr, dtmRange, *clipShape->GetShapePtr());
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------------+
| SetClipsToDTM
  Utility function that adds to the given DTM the clips.
+----------------------------------------------------------------------------*/ 
int SetClipsToDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&  dtmPtr,
                   const DRange3d&                 dtmRange,
                   const vector<DPoint3d>&         regionPoints,
                   const IScalableMeshClipContainerPtr&   clips)
    {
    struct Pt3dToPt2d
        {
        DPoint2d operator() (const DPoint3d& rhs)
            {
            const DPoint2d value = {rhs.x, rhs.y};
            return value;
            }
        };

    // Convert to DPoint2d
    std::vector<DPoint2d> regionPts2d;
    std::transform(regionPoints.begin(), regionPoints.end(), back_inserter(regionPts2d), Pt3dToPt2d());

    const double* regionPtsBuffer = &regionPts2d[0].x;
    HFCPtr<HGF2DCoordSys> pCoordSys(new HGF2DCoordSys());   
    HFCPtr<HVE2DPolygonOfSegments> pPolygon(new HVE2DPolygonOfSegments(regionPts2d.size() * 2, const_cast<double*>(regionPtsBuffer), pCoordSys));
    HFCPtr<HVEShape> areaShape = new HVEShape(*pPolygon);

    HFCPtr<HVEShape> clipShape = CreateShapeFromClips(areaShape, clips);

    if (clipShape->IsEmpty())
        {
        clipShape = areaShape;
        }

    return SetClipToDTM(dtmPtr, dtmRange, *clipShape->GetShapePtr());
    }

/*----------------------------------------------------------------------------+
| AddExtentAndTestMatch
| Utility function that triggers the triangulation of a DTM.
| Some triangulation parameters are obtained from ????
+----------------------------------------------------------------------------*/ 
bool AddExtentAndTestMatch(bool&           shouldAddMatchedCachedTile,
                           MaskList&       maskList,
                           bool            reset, 
                           const DRange2d& extentToCover,
                           const DRange2d& matchedCachedTileExtent)
    { 
    //Sort the by their resolutions in increasing order.
    static HFCPtr<HVEShape> unifiedMatchedTilesShape;
    static HFCPtr<HGF2DCoordSys> defaultCoordSysPtr(new HGF2DCoordSys());        

    if (reset == true)
        {
        unifiedMatchedTilesShape = new HVEShape(defaultCoordSysPtr);
        } 

    HFCPtr<HVEShape>      extentToCoverShape(new HVEShape(extentToCover.low.x, 
                                                              extentToCover.low.y,
                                                              extentToCover.high.x, 
                                                              extentToCover.high.y, 
                                                              defaultCoordSysPtr));     
    
/*
        bestMatchingCachedTiles.sort(MyCachedTileSortPredicate<CachedTile::SharedPtr>);

        tileIter = bestMatchingCachedTiles.begin();
        tileIterEnd = bestMatchingCachedTiles.end();

        HFCPtr<HVEShape> matchedTileShapePtr;

        while (tileIter != tileIterEnd)
            {                           
            */
            HFCPtr<HVEShape> matchedTileShapePtr(new HVEShape(matchedCachedTileExtent.low.x,
                                                              matchedCachedTileExtent.low.y,
                                                              matchedCachedTileExtent.high.x,
                                                              matchedCachedTileExtent.high.y,
                                                              defaultCoordSysPtr));   

            matchedTileShapePtr->Intersect(*extentToCoverShape);

            HVE2DShape::SpatialPosition spatialPosition(unifiedMatchedTilesShape->GetShapePtr()->CalculateSpatialPositionOf(*(matchedTileShapePtr->GetShapePtr()))); 
                                    
            //if ((spatialPosition != HVE2DShape::S_IN) && (spatialPosition != HVE2DShape::S_ON))
                {                                                                      
                if (spatialPosition == HVE2DShape::S_PARTIALY_IN)
                    {
                    HFCPtr<HVEShape> matchedTileShapeTempPtr(new HVEShape(*matchedTileShapePtr));
                    
                    matchedTileShapeTempPtr->Intersect(*unifiedMatchedTilesShape);
                    
                    if (matchedTileShapeTempPtr->GetShapePtr()->IsComplex())
                        {
                        HVE2DShape::ShapeList::const_iterator shapeIterator(matchedTileShapeTempPtr->GetShapePtr()->GetShapeList().begin());
                        HVE2DShape::ShapeList::const_iterator shapeIteratorEnd(matchedTileShapeTempPtr->GetShapePtr()->GetShapeList().end());

                        MaskPoints              maskPoints;
                        HGF2DLocationCollection pointCollection;
                        DPoint3d                pt;

                        pt.z = 0;

                        while (shapeIterator != shapeIteratorEnd)
                            {
                            maskPoints.clear();
                            
                            (*shapeIterator)->Drop(&pointCollection, 0.0);

                            HGF2DLocationCollection::const_iterator pointIter(pointCollection.begin());
                            HGF2DLocationCollection::const_iterator pointIterEnd(pointCollection.end());

                            while (pointIter != pointIterEnd)
                                {                            
                                pt.x = pointIter->GetX();
                                pt.y = pointIter->GetY();                            
                                maskPoints.push_back(pt);
                                pointIter++;
                                }                                                
                                   
                            maskList.push_back(maskPoints);

                            shapeIterator++;
                            }                                
                        }                        
                    else
                        {
                        MaskPoints              maskPoints;
                        HGF2DLocationCollection pointCollection;
                        DPoint3d                pt;

                        pt.z = 0;

                        matchedTileShapeTempPtr->GetShapePtr()->Drop(&pointCollection, 0.0);

                        HGF2DLocationCollection::const_iterator pointIter(pointCollection.begin());
                        HGF2DLocationCollection::const_iterator pointIterEnd(pointCollection.end());

                        while (pointIter != pointIterEnd)
                            {                            
                            pt.x = pointIter->GetX();
                            pt.y = pointIter->GetY();                            
                            maskPoints.push_back(pt);
                            pointIter++;
                            }                                                
                                   
                        maskList.push_back(maskPoints);                        
                        }
                    }

                unifiedMatchedTilesShape->Unify(*matchedTileShapePtr);                
                shouldAddMatchedCachedTile = true;
                }
                /*
            else
                {
                shouldAddMatchedCachedTile = false;
                }
            */
            //The whole area is covered, stop
            /*
            if (unifiedMatchedTilesShape->Matches(*extentToCoverShape))
                {
                    
                tileIter++;
                if (tileIter != tileIterEnd)
                    {
                    bestMatchingCachedTiles.erase(tileIter, tileIterEnd);
                    }
                break;  
                }

*/

    return unifiedMatchedTilesShape->Matches(*extentToCoverShape);
    }



#if 0
/*----------------------------------------------------------------------------+
| AddExtentAndTestMatch
| Utility function that triggers the triangulation of a DTM.
| Some triangulation parameters are obtained from ????
+----------------------------------------------------------------------------*/     
bool AddExtentAndTestMatch(bool&           shouldAddMatchedCachedTile,
                           MaskList&       maskList,
                           bool            reset, 
                           const DRange2d& extentToCover,
                           const DRange2d& matchedCachedTileExtent)
    { 
    //Sort the by their resolutions in increasing order.
    static HFCPtr<HVEShape> unifiedMatchedTilesShape;
    static HFCPtr<HGF2DCoordSys> defaultCoordSysPtr(new HGF2DCoordSys());        

    if (reset == true)
        {
        unifiedMatchedTilesShape = new HVEShape(defaultCoordSysPtr);
        } 

    HFCPtr<HVEShape> extentToCoverShape(new HVEShape(extentToCover.low.x, 
                                                              extentToCover.low.y,
                                                              extentToCover.high.x, 
                                                              extentToCover.high.y, 
                                                              defaultCoordSysPtr));     
    
/*
        bestMatchingCachedTiles.sort(MyCachedTileSortPredicate<CachedTile::SharedPtr>);

        tileIter = bestMatchingCachedTiles.begin();
        tileIterEnd = bestMatchingCachedTiles.end();

        HFCPtr<HVEShape> matchedTileShapePtr;

        while (tileIter != tileIterEnd)
            {                           
            */
            HFCPtr<HVEShape> matchedTileShapePtr(new HVEShape(matchedCachedTileExtent.low.x,
                                                              matchedCachedTileExtent.low.y,
                                                              matchedCachedTileExtent.high.x,
                                                              matchedCachedTileExtent.high.y,
                                                              defaultCoordSysPtr));   

            matchedTileShapePtr->Intersect(*extentToCoverShape);

            HVE2DShape::SpatialPosition spatialPosition(unifiedMatchedTilesShape->GetShapePtr()->CalculateSpatialPositionOf(*(matchedTileShapePtr->GetShapePtr()))); 
                                    
            //if ((spatialPosition != HVE2DShape::S_IN) && (spatialPosition != HVE2DShape::S_ON))
                {                                                                      
                if (spatialPosition == HVE2DShape::S_PARTIALY_IN)
                    {
                    HFCPtr<HVEShape> matchedTileShapeTempPtr(new HVEShape(*matchedTileShapePtr));
                    
                    matchedTileShapeTempPtr->Intersect(*unifiedMatchedTilesShape);
                    
                    if (matchedTileShapeTempPtr->GetShapePtr()->IsComplex())
                        {
                        HVE2DShape::ShapeList::const_iterator shapeIterator(matchedTileShapeTempPtr->GetShapePtr()->GetShapeList().begin());
                        HVE2DShape::ShapeList::const_iterator shapeIteratorEnd(matchedTileShapeTempPtr->GetShapePtr()->GetShapeList().end());

                        MaskPoints              maskPoints;
                        HGF2DLocationCollection pointCollection;
                        DPoint3d                pt;

                        pt.z = 0;

                        while (shapeIterator != shapeIteratorEnd)
                            {
                            maskPoints.clear();
                            
                            (*shapeIterator)->Drop(&pointCollection, HGFDistance());

                            HGF2DLocationCollection::const_iterator pointIter(pointCollection.begin());
                            HGF2DLocationCollection::const_iterator pointIterEnd(pointCollection.end());

                            while (pointIter != pointIterEnd)
                                {                            
                                pt.x = pointIter->GetX();
                                pt.y = pointIter->GetY();                            
                                maskPoints.push_back(pt);
                                pointIter++;
                                }                                                
                                   
                            maskList.push_back(maskPoints);

                            shapeIterator++;
                            }                                
                        }                        
                    else
                        {
                        MaskPoints              maskPoints;
                        HGF2DLocationCollection pointCollection;
                        DPoint3d                pt;

                        pt.z = 0;

                        matchedTileShapeTempPtr->GetShapePtr()->Drop(&pointCollection, HGFDistance());

                        HGF2DLocationCollection::const_iterator pointIter(pointCollection.begin());
                        HGF2DLocationCollection::const_iterator pointIterEnd(pointCollection.end());

                        while (pointIter != pointIterEnd)
                            {                            
                            pt.x = pointIter->GetX();
                            pt.y = pointIter->GetY();                            
                            maskPoints.push_back(pt);
                            pointIter++;
                            }                                                
                                   
                        maskList.push_back(maskPoints);                        
                        }
                    }

                unifiedMatchedTilesShape->Unify(*matchedTileShapePtr);                
                shouldAddMatchedCachedTile = true;
                }
                /*
            else
                {
                shouldAddMatchedCachedTile = false;
                }
            */
            //The whole area is covered, stop
            /*
            if (unifiedMatchedTilesShape->Matches(*extentToCoverShape))
                {
                    
                tileIter++;
                if (tileIter != tileIterEnd)
                    {
                    bestMatchingCachedTiles.erase(tileIter, tileIterEnd);
                    }
                break;  
                }

*/

    return unifiedMatchedTilesShape->Matches(*extentToCoverShape);
    }
#endif
        
extern addLinearsForPresentationModeFP g_addLinearsForPresentationModeFP = 0;

addLinearsForPresentationModeFP GetLinearsForPresentationModeCallback()
    {
    return g_addLinearsForPresentationModeFP;
    }

bool SetLinearsForPresentationModeCallback(addLinearsForPresentationModeFP callbackFP)
    {
    g_addLinearsForPresentationModeFP = callbackFP;
    return true;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AlainRobert  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool GCSMathematicalDomainsOverlap(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                                   BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr)
    {

    HFCPtr<HGF2DCoordSys> pDummyCS = new HGF2DCoordSys();

    HFCPtr<HVE2DShape> pDomainIntersection = GetGCSDomainsIntersection(sourceGCSPtr,
                                                                       targetGCSPtr, 
                                                                       pDummyCS);

    return !(pDomainIntersection->IsEmpty());
    }


#ifdef SCALABLE_MESH_ATP
static double s_getGroundDetectionDuration = 0; 

double GetGroundDetectionDuration()
    {
    return s_getGroundDetectionDuration;
    }

void SetGroundDetectionDuration(double t)
    {
    s_getGroundDetectionDuration = t;
    }

void AddGroundDetectionDuration(double t)
    {
    s_getGroundDetectionDuration += t;
    }

#endif