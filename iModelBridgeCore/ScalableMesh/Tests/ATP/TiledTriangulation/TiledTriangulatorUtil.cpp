/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ATP/TiledTriangulation/TiledTriangulatorUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

//#include "DcStmCorePCH.h"
//#include "MrDTMUtil.h"
//#include "ScalableMeshATPPch.h"
#include "TiledTriangulatorUtil.h"
#include "MrDTMUtil.h"
#include "MrDTMFace.h"
#include <ImagePP/h/HmrMacro.h>

using namespace std;

#include <ImagePP/all/h/HVE2DSegment.h>
#include <DgnPlatform/VecMath.h>
#include <ImagePP/h/HArrayAutoPtr.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>

//BEGIN_GEODTMAPP_NAMESPACE
#pragma optimize("",off)

bool isEqualPoint2d(const DPoint2d& point1, const DPoint2d& point2)
    {
    return (LegacyMath::DEqual(point1.x, point2.x) && LegacyMath::DEqual(point1.y, point2.y));
    }

bool isEqualPoint3d(const DPoint3d& point1, const DPoint3d& point2)
    {
    return (LegacyMath::DEqual(point1.x, point2.x) && LegacyMath::DEqual(point1.y, point2.y) && LegacyMath::DEqual(point1.z, point2.z));
    }
//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 09/10
//=======================================================================================
/*void dumpDTMInTinFile(Bentley::TerrainModel::BcDTM* dtmP, WString& fileName, const __int64* indP)
    {
    StatusInt status;

    if (indP != 0)
        {
        WChar dtmFileName[1000];

        _snwprintf(dtmFileName,
                   1000,
                   fileName.c_str(),
                   *indP);

        status = dtmP->Save(dtmFileName);
        }
    else
        {
        status = dtmP->Save((WChar *)fileName.c_str());
        }

    assert(status == 0);
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*void printTileContourPolygonToMS(vector<DPoint3d> contour_polyline)
    {
    if (contour_polyline.size() > 0)
        {
        Transform uorToMeter;
        Transform transform;

        GetTransformForPoints(uorToMeter, transform);

        bsiTransform_multiplyDPoint3dArrayInPlace(&transform, &contour_polyline[0], (int)contour_polyline.size());
        // Break polyline since microstation does not allow the display of polylines containing more than 5000 points.
        double intPart = 0.0;
        modf (contour_polyline.size() / 5000 , &intPart);
        int numberBreaks = int(intPart)+1;

        for (int i = 0; i < numberBreaks; i++)
            {
            MSElement lineElement;
            
            mdlLineString_create (&lineElement, NULL, &contour_polyline[i*5000], i+1==numberBreaks? contour_polyline.size() % 5000 : 5000);
            UInt32 color = 3;
            UInt32 weight = 2;
            Int32 style = 0;
            //mdlElement_setSymbology(&lineElement, &color, &weight, &style);
            lineElement.hdr.dhdr.symb.color = color;
            lineElement.hdr.dhdr.symb.weight = weight;
            lineElement.hdr.dhdr.symb.style = style;
            EditElementHandle eeh1 (&lineElement, ScalableTerrainModelLib::GetHost().GetScalableTerrainModelAdmin()._GetActiveModelRef());
            eeh1.AddToModel ();
            }
        }
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Richard.Bois  07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt addLinearsIn (DTMPtr& dtmPtr,
                                             list<HFCPtr<HVEDTMLinearFeature>>& linearList,
                                             unsigned int                       maxNumberOfPoints)
    {
    assert((dtmPtr != 0) && (dtmPtr->GetBcDTM() != 0) && (dtmPtr->GetBcDTM()->GetTinHandle()));
    int status = SUCCESS;

    if (linearList.size() > 0)
        {
        BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());

        //Compute the number of points
        list<HFCPtr<HVEDTMLinearFeature> >::iterator linearIter    = linearList.begin();
        list<HFCPtr<HVEDTMLinearFeature> >::iterator linearIterEnd = linearList.end();
        size_t nbOfLinearPoints = 0;

        while (linearIter != linearIterEnd)
            {
            nbOfLinearPoints += (*linearIter)->GetSize();
            linearIter++;
            }

        if (nbOfLinearPoints > maxNumberOfPoints) return ERROR;

        linearIter    = linearList.begin();
        linearIterEnd = linearList.end();

        UInt32                tileNumber = 0;
        HArrayAutoPtr<DPoint3d> linePts;
        size_t                  linePtsMaxSize = 0;

        int globalLinearPointInd = 0;

        while (linearIter != linearIterEnd)
            {
            if (linePtsMaxSize < (*linearIter)->GetSize())
                {
                linePtsMaxSize = (*linearIter)->GetSize();
                linePts = new DPoint3d[linePtsMaxSize];
                }

            size_t nbLinePts = 0;

            for (size_t indexPoints = 0 ; indexPoints < (*linearIter)->GetSize(); indexPoints++)
                {
                linePts[nbLinePts].x = (*linearIter)->GetPoint(indexPoints).GetX();
                linePts[nbLinePts].y = (*linearIter)->GetPoint(indexPoints).GetY();
                linePts[nbLinePts].z = (*linearIter)->GetPoint(indexPoints).GetZ();
                nbLinePts++;

                globalLinearPointInd++;
                }

            if (nbLinePts > 0)
                {
                if (((*linearIter)->GetSize() == 1) || (nbLinePts > 1))
                    {
                    //Ensure that those features are closed if filtered, otherwise they won't work as expected.
                    if (nbLinePts < (*linearIter)->GetSize())
                        {
                        if (isClosedFeature((DTMFeatureType)(*linearIter)->GetFeatureType()))
                            {
                                linePts[nbLinePts] = linePts[0];
                                nbLinePts++;
                            }
                        }

                    status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, (DTMFeatureType)(*linearIter)->GetFeatureType(), tileNumber, 1, &dtmObjP->nullFeatureId, linePts.get(), (long)nbLinePts);
                    }
                else //Change the linear feature to a random spot.
                    {
                    status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, tileNumber, 1, &dtmObjP->nullFeatureId, linePts.get(), (long)nbLinePts);
                    }
                }

            if (status != SUCCESS)
                {
                break;
                }

            linearIter++;
            tileNumber++;
            }
        }

    return status;
    }
    */
bool isClosedFeature(DTMFeatureType featureType)
    {
    bool isClosed;

    switch (featureType)
        {
        case DTMFeatureType::Void :
        case DTMFeatureType::BreakVoid :
        case DTMFeatureType::DrapeVoid :
        case DTMFeatureType::Hole :
        case DTMFeatureType::Island :
        case DTMFeatureType::Hull :
        case DTMFeatureType::DrapeHull :
        case DTMFeatureType::Polygon :
        case DTMFeatureType::Region    :
            isClosed = true;
            break;
        default :
            isClosed = false;
            break;
        }

    return isClosed;
    }

/*---------------------------------------------------------------------------------**//**
* Adds a polygon as a feature in the DTM. Also, triangulation is called if specified.
* @bsimethod                                                    Richard.Bois  4/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int addPolygonAsFeatureInDTM (Bentley::TerrainModel::DTMPtr& dtmPtr,
                              const Bentley::ScalableMesh::IScalableMeshQueryParametersPtr& mrDTMQueryParamsPtr,
                              vector<DPoint3d>& polygon,
                              DTMFeatureType& featureType,
                              const bool triangulateAfter)
    {
    /*int status = SUCCESS;
        BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());

    status = bcdtmObject_storeDtmFeatureInDtmObject (dtmObjP, featureType, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(polygon[0]), (long)polygon.size());

    if (triangulateAfter && status == SUCCESS) status = TriangulateDTM(dtmPtr, mrDTMQueryParamsPtr);*/
    assert(!"Not supported by new SM API");
    return ERROR;
    }


vector<DPoint2d> convertDRange2dToVectorDPoint2d (const DRange2d& inputRange)
    {
    vector<DPoint2d> regionPoints (5);
    regionPoints[0].x = inputRange.low.x;
    regionPoints[0].y = inputRange.low.y;
    regionPoints[1].x = inputRange.high.x;
    regionPoints[1].y = inputRange.low.y;
    regionPoints[2].x = inputRange.high.x;
    regionPoints[2].y = inputRange.high.y;
    regionPoints[3].x = inputRange.low.x;
    regionPoints[3].y = inputRange.high.y;
    regionPoints[4].x = regionPoints[0].x;
    regionPoints[4].y = regionPoints[0].y;
    return regionPoints;
    }

vector<DPoint3d> convertDRange2dToVectorDPoint3d (const DRange2d& inputRange)
    {
    vector<DPoint3d> regionPoints (5);
    regionPoints[0].x = inputRange.low.x;
    regionPoints[0].y = inputRange.low.y;
    regionPoints[0].z = 0.0;
    regionPoints[1].x = inputRange.high.x;
    regionPoints[1].y = inputRange.low.y;
    regionPoints[1].z = 0.0;
    regionPoints[2].x = inputRange.high.x;
    regionPoints[2].y = inputRange.high.y;
    regionPoints[2].z = 0.0;
    regionPoints[3].x = inputRange.low.x;
    regionPoints[3].y = inputRange.high.y;
    regionPoints[3].z = 0.0;
    regionPoints[4].x = regionPoints[0].x;
    regionPoints[4].y = regionPoints[0].y;
    regionPoints[4].z = 0.0;
    return regionPoints;
    }

int insertTrianglesAsFeatureInDTM (Bentley::TerrainModel::DTMPtr& dtmPtr,
                                   HPMMemoryManagedVector<DPoint3d>& triangles,
                                   //vector<DPoint3d>& triangles,
                                   DTMFeatureType featureType)
    {
    struct ToBcPtConverter
        {
        DPoint3d operator () (const DPoint3d& inputPt) const
            {
            DPoint3d outPt = {inputPt.x, inputPt.y, inputPt.z};
            return outPt;
            }
        } ConvertToBcPt;

    int status = SUCCESS;
    BC_DTM_OBJ* dtmObjP (dtmPtr->GetBcDTM()->GetTinHandle() );

//    const int numTriangles = (int)triangles.size()/3;
    DPoint3d triangle[4];
    for (unsigned int t = 0; t < triangles.size() && status == SUCCESS; t+=3)
        {
        for (int i = 0; i < 3; i++) triangle[i] = ConvertToBcPt(triangles[t+i]);
        triangle[3] = triangle[0];

        status = bcdtmObject_storeDtmFeatureInDtmObject (dtmObjP, featureType, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &triangle[0], 4);
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* Creates a tinned dtm using a list of triangles.
*
* NOTE: Currently might not work as expected. See note below.
*
* @bsimethod                                                    Richard.Bois  4/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int createDTMFromTriangles (Bentley::TerrainModel::DTMPtr& dtmPtr,
                            const Bentley::ScalableMesh::IScalableMeshQueryParametersPtr& mrDTMQueryParamsPtr,
                            HPMMemoryManagedVector<DPoint3d>& triangles,
                            //vector<DPoint3d>& triangles,
                            DTMFeatureType featureType,
                            const bool triangulate)
    {
    assert(!"Not supported by new SM API");
    return ERROR;
    }

/*----------------------------------------------------------------------------+
| GetAllMeshFacesAndComputeBoundingCircles
  Utility function that fills a vector with faces and their respective properties.
+----------------------------------------------------------------------------*/
void getAllMeshFacesAndComputeBoundingCircles (Bentley::TerrainModel::BcDTMMeshPtr mesh, vector<FaceWithProperties>& VectorFaces)
{

    const int numFaces = mesh->GetFaceCount();
    string faceCt = "" + std::to_string(numFaces);
    faceCt += "";
        VectorFaces.resize(numFaces);

        for (int i = 0; i<numFaces; i++)
    {
        //VectorFaces[i].SetFace(*mesh->GetFace(i));
        VectorFaces[i].SetFace(mesh->GetFace(i));
        VectorFaces[i].ComputeBoundingCircle();
    }
}

void getRangeFromPoints(DPoint3dCP linePoints, const int nbPoints, DRange2d& range)
    {
    range.low.x = linePoints[0].x;
    range.low.y = linePoints[0].y;
    range.high.x = linePoints[0].x;
    range.high.y = linePoints[0].y;

    for (int i = 1; i < nbPoints; i++)
        {
        range.low.x  = min(range.low.x,linePoints[i].x);
        range.low.y  = min(range.low.y,linePoints[i].y);
        range.high.x = max(range.high.x,linePoints[i].x);
        range.high.y = max(range.high.y,linePoints[i].y);
        }
    }

bool getLineIntersection(double p0_x, double p0_y, double p1_x, double p1_y,
                         double p2_x, double p2_y, double p3_x, double p3_y/*, double *i_x, double *i_y*/)
    {
    double s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom/*, t*/;
    s10_x = p1_x - p0_x;
    s10_y = p1_y - p0_y;
    s32_x = p3_x - p2_x;
    s32_y = p3_y - p2_y;

    denom = s10_x * s32_y - s32_x * s10_y;
    if (denom == 0)
        return false; // Collinear
    bool denomPositive = denom > 0;

    s02_x = p0_x - p2_x;
    s02_y = p0_y - p2_y;
    s_numer = s10_x * s02_y - s10_y * s02_x;
    if ((s_numer < -1.e-3) == denomPositive)
        return false; // No collision

    t_numer = s32_x * s02_y - s32_y * s02_x;
    if ((t_numer < -1.e-3) == denomPositive)
        return false; // No collision

    if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
        return false; // No collision
    // Collision detected... We do not need the collision point!
    //t = t_numer / denom;
    //if (i_x != NULL)
    //    *i_x = p0_x + (t * s10_x);
    //if (i_y != NULL)
    //    *i_y = p0_y + (t * s10_y);

    return true;
    }

bool getLineIntersection(double p0_x, double p0_y, double p1_x, double p1_y,
                         double p2_x, double p2_y, double p3_x, double p3_y,
                         double *i_x, double *i_y)
    {
    double s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom/*, t*/;
    s10_x = p1_x - p0_x;
    s10_y = p1_y - p0_y;
    s32_x = p3_x - p2_x;
    s32_y = p3_y - p2_y;

    denom = s10_x * s32_y - s32_x * s10_y;
    if (denom == 0)
        return false; // Collinear
    bool denomPositive = denom > 0;

    s02_x = p0_x - p2_x;
    s02_y = p0_y - p2_y;
    s_numer = s10_x * s02_y - s10_y * s02_x;
    if ((s_numer < -1.e-3) == denomPositive)
        return false; // No collision

    t_numer = s32_x * s02_y - s32_y * s02_x;
    if ((t_numer < -1.e-3) == denomPositive)
        return false; // No collision

    if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
        return false; // No collision
    // Collision detected...
    double t = t_numer / denom;
    if (i_x != NULL)
        *i_x = p0_x + (t * s10_x);
    if (i_y != NULL)
        *i_y = p0_y + (t * s10_y);

    return true;
    }

bool isLineIntersectingRange(DPoint3d* linePtsP, DRange2d& range)
    {
    DRange2d localLineRange;
    getRangeFromPoints((DPoint3dP)&linePtsP[0], 2, localLineRange);

//    const double distance = sqrt(pow(range.low.x - range.high.x,2)+pow(range.low.y-range.high.y,2));
    //const double scale = /*factor*/5.0/100.0;
    //const double scale = 0.0;
    DRange2d newRange = range;
    //newRange.low.x  -= distance*scale;
    //newRange.low.y  -= distance*scale;
    //newRange.high.x += distance*scale;
    //newRange.high.y += distance*scale;

    if (/*true*/bsiDRange2d_checkOverlap(&newRange, &localLineRange))
        {
        DPoint3d p1, p2, p3, p4 ;
        p1.x = newRange.low.x;
        p1.y = newRange.low.y;
        p1.z = 0.;
        p2.x = newRange.low.x;
        p2.y = newRange.high.y;
        p2.z = 0.;
        p3.x = newRange.high.x;
        p3.y = newRange.high.y;
        p3.z = 0.;
        p4.x = newRange.high.x;
        p4.y = newRange.low.y;
        p4.z = 0.;

        const bool lineIntersectsRange = getLineIntersection(linePtsP[0].x, linePtsP[0].y, linePtsP[1].x, linePtsP[1].y, p1.x, p1.y, p2.x, p2.y) ||
                                         getLineIntersection(linePtsP[0].x, linePtsP[0].y, linePtsP[1].x, linePtsP[1].y, p2.x, p2.y, p3.x, p3.y) ||
                                         getLineIntersection(linePtsP[0].x, linePtsP[0].y, linePtsP[1].x, linePtsP[1].y, p3.x, p3.y, p4.x, p4.y) ||
                                         getLineIntersection(linePtsP[0].x, linePtsP[0].y, linePtsP[1].x, linePtsP[1].y, p4.x, p4.y, p1.x, p1.y) ||
                                         getLineIntersection(linePtsP[1].x, linePtsP[1].y, linePtsP[0].x, linePtsP[0].y, p1.x, p1.y, p2.x, p2.y) ||
                                         getLineIntersection(linePtsP[1].x, linePtsP[1].y, linePtsP[0].x, linePtsP[0].y, p2.x, p2.y, p3.x, p3.y) ||
                                         getLineIntersection(linePtsP[1].x, linePtsP[1].y, linePtsP[0].x, linePtsP[0].y, p3.x, p3.y, p4.x, p4.y) ||
                                         getLineIntersection(linePtsP[1].x, linePtsP[1].y, linePtsP[0].x, linePtsP[0].y, p4.x, p4.y, p1.x, p1.y);
        const bool lineInsideRange = lineIntersectsRange ? false
            : newRange.low.x  <= localLineRange.low.x  &&
            newRange.low.y  <= localLineRange.low.y  &&
            newRange.high.x >= localLineRange.high.x &&
            newRange.high.y >= localLineRange.high.y;
        if ( lineIntersectsRange || lineInsideRange )
            {
            return true;
            }
        }
    return false;
    }

bool isLineIntersectingCircle(const double& x1, const double& y1,
                              const double& x2, const double& y2,
                              const double& r2,
                              const HFCPtr<HGF2DCoordSys>& coordSys)
    { //assumes center of circle at (0,0)
      // Line points : (x1,y1), (x2,y2)
      // Circle radius squared : r2
    bool isIntersect = false;
    const double dx  = x2-x1;
    const double dy  = y2-y1;
    const double dr2 = dx*dx+dy*dy;
    const double D = x1*y2-x2*y1;

    const double discriminant = r2*dr2-D*D;
    if (discriminant > 0.0)
        { // possible intersection
        double u1 =  (D*dy+((0<dy)-(dy<0))*dx*sqrt(discriminant))/dr2;
        double v1 = (-D*dx+fabs(dy)*sqrt(discriminant))/dr2;
        // check if cross points lies on the segment
        HVE2DSegment segment (HGF2DPosition(x1,y1), HGF2DPosition(x2,y2), coordSys);
        bool point1OnSegment = segment.IsPointOn(HGF2DLocation(u1,v1,coordSys));
        if (!point1OnSegment)
            {
            // check the other cross point
            double u2 =  (D*dy-((0<dy)-(dy<0))*dx*sqrt(discriminant))/dr2;
            double v2 = (-D*dx-fabs(dy)*sqrt(discriminant))/dr2;
            bool point2OnSegment = segment.IsPointOn(HGF2DLocation(u2,v2,coordSys));
            isIntersect = point2OnSegment;
            }
        else isIntersect = true;
        }
    else if (discriminant == 0.0) isIntersect = true;

    return isIntersect;
    }
#pragma optimize("",on)
//END_GEODTMAPP_NAMESPACE