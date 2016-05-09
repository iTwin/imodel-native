/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/InternalUtilityFunctions.cpp $
|    $RCSfile: InternalUtilityFunctions.cpp,v $
|   $Revision: 1.19 $
|       $Date: 2012/08/20 16:31:58 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
#include "SMPointIndex.h"
#include "InternalUtilityFunctions.h"
#include <ScalableMesh/IScalableMeshClipContainer.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <ScalableMesh/IScalableMeshQuery.h>

//#define GPU
#undef static_assert
#include <ppl.h>
#ifdef GPU
#include <amp.h>
#endif

USING_NAMESPACE_IMAGEPP;



static int s_faceToBoxPoint[6][4] =
    {
    {1,0,2,3},
    {4,5,7,6},
    {0,4,6,2},
    {1,3,7,5},
    {0,1,5,4},
    {2,6,7,3},
    };

DPoint3d PtToPtConverter::operator () (const DPoint3d& inputPt) const
{
    DPoint3d outPt(inputPt);
    // DPoint3d outPt = {inputPt.x, inputPt.y, 0};
    return outPt;
}

DPoint3d PtToPtConverter::operator () (const HGF3DCoord<double>& inputPt) const
    {   
    DPoint3d outPt = {inputPt.GetX(), inputPt.GetY(), inputPt.GetZ()};
    //  DPoint3d outPt = {inputPt.GetX(), inputPt.GetY(), 0};
    return outPt;
    }

void PtToPtConverter::Transform(DPoint3d* ptsOut, const DPoint3d* ptsIn, size_t nbPts)
    {
    memcpy(ptsOut, ptsIn, nbPts * sizeof(DPoint3d));
    }
    
/*---------------------------------------------------------------------------------**//**
* Calculate normals for points of a mesh
+---------------+---------------+---------------+---------------+---------------+------*/
double checkNormal (DVec3dCR viewNormal, DVec3dCR normal) restrict (amp,cpu)
    {
    return (viewNormal.x*normal.x + viewNormal.y*normal.y + viewNormal.z*normal.z);
    }

void CalcNormals (DVec3d**      calculatedNormals,                  
                  const DVec3d& viewNormalParam, 
                  size_t        nbPoints, 
                  DPoint3d*     pPoints, 
                  size_t        nbFaceIndexes, 
                  int32_t*        pFaceIndexes)
    {
    assert(calculatedNormals == 0);

    int triangleCount = (int)(nbFaceIndexes / 3);
    bvector<DVec3d> sTriangleNormals;
    sTriangleNormals.resize (triangleCount);
    DVec3d viewNormal = viewNormalParam;
#ifdef GPU
    concurrency::array_view <DVec3d, 1> triangleNormals (concurrency::extent<1> (triangleCount), sTriangleNormals.data());
    concurrency::array_view <DPoint3d, 1> points ((int)nbPoints, pPoints);
    concurrency::array_view <int, 1> faceIndx((int)nbFaceIndexes, pFaceIndexes);
    concurrency::parallel_for_each (concurrency::extent<1> (triangleCount), [=](concurrency::index<1> idx) restrict (amp,cpu)
#else
    auto triangleNormals = sTriangleNormals.data ();
    auto points = pPoints;
    auto faceIndx = pFaceIndexes;
    concurrency::combinable<size_t> _numberSwapped;
    concurrency::parallel_for (0, triangleCount, 1, [&](int aidx) restrict (cpu)
#endif
        {
#ifdef GPU
        int aidx = idx[0];
#endif
        int faceInx = aidx * 3;
        int p1 = faceIndx[faceInx] - 1;
        int p2 = faceIndx[faceInx + 1] - 1;
        int p3 = faceIndx[faceInx + 2] - 1;

        DVec3d normal;
        DPoint3d origin = points[p1];
        DPoint3d target1 = points[p2];
        DPoint3d target2 = points[p3];

        double x1 = target1.x - origin.x;
        double y1 = target1.y - origin.y;
        double z1 = target1.z - origin.z;

        double x2 = target2.x - origin.x;
        double y2 = target2.y - origin.y;
        double z2 = target2.z - origin.z;
        normal.x = y1 * z2 - z1 * y2;
        normal.y = z1 * x2 - x1 * z2;
        normal.z = x1 * y2 - y1 * x2;

        if (checkNormal (viewNormal, normal) >= 0)
            {
            _numberSwapped.local ()++;
            normal.x = -normal.x;
            normal.y = -normal.y;
            normal.z = -normal.z;
            faceIndx[faceInx] = p3 + 1;
            faceIndx[faceInx + 2] = p1 + 1;
            }
        triangleNormals[aidx] = normal;
        });
    size_t numberswapped = 0;

    _numberSwapped.combine_each ([&](size_t v)
        {
        numberswapped += v;
        });

#ifdef GPU
    triangleNormals.synchronize ();
    faceIndx.synchronize ();
#endif

    *calculatedNormals = new DVec3d[nbPoints];

    for (size_t i = 0; i < nbPoints; i++)
        {
        DVec3d* norm = &(*calculatedNormals)[i];
        norm->x = 0;
        norm->y = 0;
        norm->z = 0;
        }
    for (size_t i = 0, face = 0; i < nbFaceIndexes; face++, i += 3)
        {
        DVec3d* faceNorm = &triangleNormals[face];
        for (int j = 0; j < 3; j++)
            {
            long ptNum = pFaceIndexes[i + j] - 1;
            //if (ptNum >= 0 && ptNum < m_nbPoints)
                {
                DVec3d* norm = &(*calculatedNormals)[ptNum];
                norm->x += faceNorm->x;
                norm->y += faceNorm->y;
                norm->z += faceNorm->z;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Creates a shape from a list of DPoint3d. The coordinate system is provided.
* This list of point must contain the closing point. If the first point is not
* identical to the last point then the shape cannot be created. 
* If the list of points does not form a valid shape then NULL will be returned. 
* To be valid the list of points must close, not autocross and not be auto-contiguous.
* @bsimethod                                                    AlainRobert  2/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HVE2DShape> CreateShapeFromPoints(const DPoint3d* points, size_t numberOfPoints, HFCPtr<HGF2DCoordSys> coordSys)
    {
    HFCPtr<HVE2DShape> pResult;

    // We copy the list of points to an array more suitable to convert to a shape.
    HArrayAutoPtr<double> tempBuffer(new double[numberOfPoints * 2]);
    
    for (size_t idxPoint = 0 ; idxPoint < numberOfPoints; idxPoint++)
        {
        tempBuffer[idxPoint * 2]     = points[idxPoint].x;
        tempBuffer[idxPoint * 2 + 1] = points[idxPoint].y;    
        }                            

    // We validate the shape ... it must be a valid shape in order to be processed
    // The shape must not autocross neither be autocontiguous to be valid
    HVE2DPolySegment shapeContour(numberOfPoints * 2, tempBuffer, coordSys);

    if (!(shapeContour.AutoCrosses() && shapeContour.IsAutoContiguous()))
        {
        pResult = new HVE2DPolygonOfSegments(numberOfPoints * 2, tempBuffer, coordSys);
        }

    return pResult;

    }

/*---------------------------------------------------------------------------------**//**
* Creates a shape from a list of DPoint3d
* @bsimethod                                                    AlainRobert  2/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FillBBoxFromIppExtent(DPoint3d    boxPoints[],
                                HGF2DExtent theExtent, 
                                double      zMin, 
                                double      zMax)
    {
    // We recreate a bbox
    boxPoints[0].x = theExtent.GetXMin();
    boxPoints[0].y = theExtent.GetYMin();
    boxPoints[0].z = zMin;

    boxPoints[1].x = theExtent.GetXMax();
    boxPoints[1].y = theExtent.GetYMin();
    boxPoints[1].z = zMin;

    boxPoints[2].x = theExtent.GetXMin();
    boxPoints[2].y = theExtent.GetYMax();
    boxPoints[2].z = zMin;

    boxPoints[3].x = theExtent.GetXMax();
    boxPoints[3].y = theExtent.GetYMax();
    boxPoints[3].z = zMin;

    boxPoints[4].x = theExtent.GetXMin();
    boxPoints[4].y = theExtent.GetYMin();
    boxPoints[4].z = zMax;

    boxPoints[5].x = theExtent.GetXMax();
    boxPoints[5].y = theExtent.GetYMin();
    boxPoints[5].z = zMax;

    boxPoints[6].x = theExtent.GetXMin();
    boxPoints[6].y = theExtent.GetYMax();
    boxPoints[6].z = zMax;

    boxPoints[7].x = theExtent.GetXMax();
    boxPoints[7].y = theExtent.GetYMax();
    boxPoints[7].z = zMax;

    return SUCCESS;
    }
#if 0
int CutLinears(list<HFCPtr<HVEDTMLinearFeature>>& linearList, list<HFCPtr<HVEDTMLinearFeature>>& cutLinearList, HFCPtr<HVE2DPolygonOfSegments> queryPolyLine)
    {   
    int status = SUCCESS;

    if (linearList.size() > 0)
        {        
        list<HFCPtr<HVEDTMLinearFeature> >::iterator linearIter    = linearList.begin();
        list<HFCPtr<HVEDTMLinearFeature> >::iterator linearIterEnd = linearList.end();

        while (linearIter != linearIterEnd) 
            {                               
            HGF3DPoint firstPt = (*linearIter)->GetPoint(0);
            HGF3DPoint secondPt;
            bool       addAllPts = false;
            unsigned __int64 lastIndexPtAdd = UINT64_MAX;

            HFCPtr<HVEDTMLinearFeature> cutFeatureP = new HVEDTMLinearFeature((*linearIter)->GetFeatureType(), 0);
            IDTMFile::FeatureType type = (*linearIter)->GetFeatureType();
            
            switch (type)
                {
                case DTMFeatureType::Void:
                case DTMFeatureType::BreakVoid:
                case DTMFeatureType::DrapeVoid:
                case DTMFeatureType::Hole:
                case DTMFeatureType::Island:
                case DTMFeatureType::Hull:
                case DTMFeatureType::DrapeHull:
                case DTMFeatureType::Polygon:
                case DTMFeatureType::Region:                
                    addAllPts = true;
                    break;
                default :
                    addAllPts = false;
                    break;
                }

            //TR 353473 - Once in the queryPolyline don't cut any segment even if there are outside to ensure 
            //            that there is no connecting segment inserted between two segments that are not continuous 
            //            that is overlapping the query polyline.
            bool canSkipLine = true;

            for (size_t indexPoints = 1 ; indexPoints < (*linearIter)->GetSize(); indexPoints++)
                {                
                secondPt = (*linearIter)->GetPoint(indexPoints);

                HVE2DSegment line(HGF2DLocation(firstPt.GetX(), firstPt.GetY(), queryPolyLine->GetCoordSys()), HGF2DLocation(secondPt.GetX(), secondPt.GetY(), queryPolyLine->GetCoordSys()));

                HGF2DLocationCollection CrossPoints;

                // If the segment is not NULL and if the segment intersects the query polyline or if the segment is inside the query polyline, add the extremity points               
                if (!line.IsNull() && 
                    (queryPolyLine->Intersect(line, &CrossPoints) > 0 || queryPolyLine->IsPointIn(HGF2DLocation(firstPt.GetX(), firstPt.GetY(), queryPolyLine->GetCoordSys())) || (canSkipLine == false)))
                    {
                    if (addAllPts)
                        {
                        for (size_t indexPoints2 = 0 ; indexPoints2 < (*linearIter)->GetSize(); indexPoints2++)
                            {  
                            cutFeatureP->AppendPoint((*linearIter)->GetPoint(indexPoints2));
                            }
                        break;
                        }

                    if (lastIndexPtAdd != (indexPoints-1))
                        {  
                        cutFeatureP->AppendPoint((*linearIter)->GetPoint(indexPoints-1));   
                        }      

                    cutFeatureP->AppendPoint((*linearIter)->GetPoint(indexPoints));   
                    lastIndexPtAdd = indexPoints;
                    canSkipLine = false;
                    }

                firstPt = secondPt;
                }

            if (cutFeatureP->GetSize() > 0)
                cutLinearList.push_back(cutFeatureP);

            linearIter++;
            }           
        }

    return status;
    }
#endif




/*---------------------------------------------------------------------------------**//**
* Reprojects a point from source to target GCS. If returns value is SUCCESS then 
* the coordinate was successfully transformed. If the return value is 1 then the reprojection was
* correctly performed yet the coordinate was outside the user-defined domain of validity
* of either geographic coordinate system. If any other value then an error occured and
* the reprojection of the point did not complete. Note that a value of 2 usually indicates
* the coordiante was outside the mathematical domain of either geographic
* coordinate system. Negative values denote other errors.
* @bsimethod                                                    AlainRobert  2/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ReprojectPoint(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                         BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,
                         const DPoint3d& inPoint,
                         DPoint3d& outPoint)
    {
    GeoPoint  sourceLatLong;
    GeoPoint  targetLatLong;
    StatusInt stat1;
    StatusInt stat2;
    StatusInt stat3;
    StatusInt status = SUCCESS;

    stat1 = sourceGCSPtr->LatLongFromCartesian(sourceLatLong, inPoint);
    stat2 = sourceGCSPtr->LatLongFromLatLong(targetLatLong, sourceLatLong, *targetGCSPtr);
    stat3 = targetGCSPtr->CartesianFromLatLong(outPoint, targetLatLong);
    if (SUCCESS == status)
        {
        // Status returns hardest error found in the three error statuses
        // The hardest error is the first one encountered that is not a warning (value 1 [cs_CNVRT_USFL])
        if (SUCCESS != stat1)
            status = stat1;
        if ((SUCCESS != stat2) && ((SUCCESS == status) || (1 == status))) // If stat2 has error and status not already hard error
            {
            if (0 > stat2) // If stat2 is negative ... this is the one ...
                status = stat2;
            else  // Both are positive (status may be SUCCESS) we use the highest value which is either warning or error
                status = (stat2 > status ? stat2 : status);
            }
        if ((SUCCESS != stat3) && ((SUCCESS == status) || (1 == status))) // If stat3 has error and status not already hard error
            {
            if (0 > stat3) // If stat3 is negative ... this is the one ...
                status = stat3;
            else  // Both are positive (status may be SUCCESS) we use the highest value
                status = (stat3 > status ? stat3 : status);
            }
        }
    return status;
    }


/*---------------------------------------------------------------------------------**//**
*   Everything in this nameless scope should remain in sync with its counterpart
*   in "msj\mstn\library\rasterlib\rastercore\HIEMstnGCS.cpp"
+---------------+---------------+---------------+---------------+---------------+------*/
BEGIN_UNNAMED_NAMESPACE

typedef  HGF2DPositionCollection     GeoDomainShape;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeAboutPrimeMeridianAndEquator (GeoDomainShape&     shape, 
                                                double              allowedDeltaAboutPrimeMeridian,
                                                double              allowedDeltaAboutEquator)
    {
    shape.push_back(HGF2DCoord<double>(-allowedDeltaAboutPrimeMeridian, -allowedDeltaAboutEquator));
    shape.push_back(HGF2DCoord<double>(-allowedDeltaAboutPrimeMeridian, allowedDeltaAboutEquator));
    shape.push_back(HGF2DCoord<double>(allowedDeltaAboutPrimeMeridian, allowedDeltaAboutEquator));
    shape.push_back(HGF2DCoord<double>(allowedDeltaAboutPrimeMeridian, -allowedDeltaAboutEquator));
    shape.push_back(HGF2DCoord<double>(-allowedDeltaAboutPrimeMeridian, -allowedDeltaAboutEquator));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeAboutMeridianAndEquator  (GeoDomainShape&     shape, 
                                            double              specifiedMeridian,
                                            double              allowedDeltaAboutMeridian,
                                            double              allowedDeltaAboutEquator)
    {
    const double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    const double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;
    const double minLatitude = -allowedDeltaAboutEquator;
    const double maxLatitude = allowedDeltaAboutEquator; 

    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeAboutMeridianAndParallel (GeoDomainShape&     shape, 
                                            double              specifiedMeridian,
                                            double              allowedDeltaAboutMeridian,
                                            double              specifiedParallel,
                                            double              allowedDeltaAboutParallel)
    {
    const double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    const double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;
    const double minLatitude = specifiedParallel - allowedDeltaAboutParallel;
    const double maxLatitude = specifiedParallel + allowedDeltaAboutParallel;   

    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeAboutMeridianAndBoundParallel    (GeoDomainShape&     shape, 
                                                    double              specifiedMeridian,
                                                    double              allowedDeltaAboutMeridian,
                                                    double              specifiedParallel,
                                                    double              allowedDeltaAboutParallel,
                                                    double              southMostAllowedParallel,
                                                    double              northMostAllowedParallel)
    {
    const double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    const double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;

    double minLatitude = specifiedParallel - allowedDeltaAboutParallel;
    if (minLatitude < -southMostAllowedParallel)
        minLatitude = -southMostAllowedParallel;

    double maxLatitude = specifiedParallel + allowedDeltaAboutParallel;
    if (maxLatitude > northMostAllowedParallel)
        maxLatitude = northMostAllowedParallel;

    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeAboutMeridianAndTwoStandardBoundParallel (GeoDomainShape&     shape, 
                                                            double              specifiedMeridian,
                                                            double              allowedDeltaAboutMeridian,
                                                            double              standardParallel1,
                                                            double              standardParallel2,
                                                            double              allowedDeltaAboutParallels,
                                                            double              southMostAllowedParallel,
                                                            double              northMostAllowedParallel)
    {
    const double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    const double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;

    double minLatitude;
    double maxLatitude;
    if (standardParallel1 < standardParallel2)
        {
        minLatitude = standardParallel1 - allowedDeltaAboutParallels;
        if (minLatitude < southMostAllowedParallel)
            minLatitude = southMostAllowedParallel;
        maxLatitude = standardParallel2 + allowedDeltaAboutParallels;
        if (maxLatitude > northMostAllowedParallel)
            maxLatitude = northMostAllowedParallel;
        }
    else
        {
        minLatitude = standardParallel2 - allowedDeltaAboutParallels;
        if (minLatitude < southMostAllowedParallel)
            minLatitude = southMostAllowedParallel;
        maxLatitude = standardParallel1 + allowedDeltaAboutParallels;
        if (maxLatitude > northMostAllowedParallel)
            maxLatitude = northMostAllowedParallel;
        }



    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));

    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetRangeAboutBoundMeridianAndBoundParallel   (GeoDomainShape&     shape, 
                                                        double              specifiedMeridian,
                                                        double              allowedDeltaAboutMeridian,
                                                        double              westMostAllowedMeridian,
                                                        double              eastMostAllowedMeridian,
                                                        double              specifiedParallel,
                                                        double              allowedDeltaAboutParallel,
                                                        double              southMostAllowedParallel,
                                                        double              northMostAllowedParallel)
    {
    double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    if (minLongitude < westMostAllowedMeridian)
        minLongitude = westMostAllowedMeridian;
    double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;
    if (maxLongitude > eastMostAllowedMeridian)
        maxLongitude = eastMostAllowedMeridian;
    double minLatitude = specifiedParallel - allowedDeltaAboutParallel;
    if (minLatitude < southMostAllowedParallel) 
        minLatitude = southMostAllowedParallel;   
    double maxLatitude = specifiedParallel + allowedDeltaAboutParallel; 
    if (maxLatitude > northMostAllowedParallel)
        maxLatitude = northMostAllowedParallel;   

    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, maxLatitude));
    shape.push_back(HGF2DCoord<double>(maxLongitude, minLatitude));
    shape.push_back(HGF2DCoord<double>(minLongitude, minLatitude));
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline double GetUTMZoneCenterMeridian(int zoneNumber)
    {
    return (zoneNumber - 30) * 6;
    }


/*---------------------------------------------------------------------------------**//**
* Returns the domain of application for GCS. This domain is the math domain intersected
* with the logical domain if one is set.
* @bsimethod                                                    AlainRobert  2/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetGeoDomainShape
(
const BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS& gcs,
GeoDomainShape&                         shape
) 
    {
    using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;

    // Some explanation about the values specified below and their intent.
    // First it must be inderstood that the current implementation is in progress.
    // The present implementation fixes some reported issues related to the
    // display and management of rasters when reprojection is invloved.
    // The principle attempts to define the geo domain of a specific projection using
    // extent defined as min and max longitude and latitude. Such definition is adequate
    // for many projections but not all. For example Lamber Comformal Conic domain is
    // domain is correctle defined using such definition. For transverse mercator and derivatives
    // the domain can likewise be defined using this method. Others like Oblique Mercator
    // or stereo graphic projections cannot as their area definition is not alligned
    // to latitude and longitudes. We assume that an smaller area can be defined using
    // plain geo extent but we are not sure. When the North and South pole are included we
    // have not yet defined a way to indicate this representation other than by specifying
    // exact min or max to either North or Sout pole latitude but the actual
    // case never occured so the implementation has currently been postponed
    // till more adequate research can be done.
    //
    // Concerning the definition of Transverse Mercators and derivative the mathematical domain
    // is usually defined from North to South pole on a longitude with of some
    // specific value ... We provide a very large area in this case. In practice we have had
    // cases where the datum shift during the reprojection process shifted the North and South pole
    // sufficiently that a longitude located on one side of the Earth became in the other datum
    // on the other size of the pole (17E Longitude became 163W Longitude)
    // For this reason we have decided to limit the upper and lower latitudes for all
    // projections to 89.9 degrees (any greater values resulted in the problem in our case)
    // This means that the zone will remain about 12 kilometers from the poles. For cartography
    // made in the pole areas, other projection methods will have to be used.
    const BaseGCS::ProjectionCodeValue projectionCode = gcs.GetProjectionCode();
    switch (projectionCode)
        {
        case BaseGCS::pcvCassini : // Not so sure about this one ... check http://www.radicalcartography.net/?projectionref
        case BaseGCS::pcvEckertIV :
        case BaseGCS::pcvEckertVI :
        case BaseGCS::pcvMillerCylindrical :
        case BaseGCS::pcvUnity :
        case BaseGCS::pcvGoodeHomolosine :
        case BaseGCS::pcvModifiedStereographic :
        case BaseGCS::pcvEqualAreaAuthalicNormal :
        case BaseGCS::pcvEqualAreaAuthalicTransverse :
        case BaseGCS::pcvSinusoidal :
        case BaseGCS::pcvVanderGrinten :
        case BaseGCS::pcvRobinsonCylindrical :
        case BaseGCS::pcvWinkelTripel :
        case BaseGCS::pcvEquidistantCylindrical :
        case BaseGCS::pcvEquidistantCylindricalEllipsoid :
        case BaseGCS::pcvPlateCarree :
            // good around the globe          
            return GetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);

        case BaseGCS::pcvMercatorScaleReduction :
        case BaseGCS::pcvMercator :
        case BaseGCS::pcvPopularVisualizationPseudoMercator :
            // good pretty close 90 degrees east and west of central meridian
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   gcs.GetCentralMeridian (), 179.999999, 
                                                   80.0);
        
        case BaseGCS::pcvLambertEquidistantAzimuthal :
        case BaseGCS::pcvAzimuthalEquidistantElevatedEllipsoid :
        case BaseGCS::pcvLambertEqualAreaAzimuthal :
        case BaseGCS::pcvOrthographic :
        case BaseGCS::pcvObliqueStereographic :
        case BaseGCS::pcvSnyderObliqueStereographic :
        case BaseGCS::pcvPolarStereographic :
        case BaseGCS::pcvPolarStereographicStandardLatitude :
        case BaseGCS::pcvGnomonic :
        case BaseGCS::pcvBipolarObliqueConformalConic :
            // Even though it cannot be computed, the domain must be set as the caller may not check the return status.
            GetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);
            return BSIERROR; // return not implemented;

#if NOT_YET
            // This one is a bit complicated by the fact the hemisphere can be centered anywhere on earth. 
            // If centered at a pole, the domain extends from 0 to -90 in latitude and around the globe in longitude
            // If centered somewhere on the equator, then it is valid from North to South pole but 90 degrees east and west of center
            // If centered elsewhere, the area is not easily representable in the form of min max of lat long...
            return GetRangeAboutBoundMeridianAndBoundParallel(shape,
                                                              gcs.GetOriginLongitude(), 90.0,
                                                              -180.0, 180.0,
                                                              gcs.GetOriginLatitude(), 90.0,
                                                              -90.0, 90.0);
#endif   

        case BaseGCS::pcvTransverseMercator :
        case BaseGCS::pcvGaussKrugerTranverseMercator :
        case BaseGCS::pcvSouthOrientedTransverseMercator :
        case BaseGCS::pcvTransverseMercatorAffinePostProcess :
        case BaseGCS::pcvTransverseMercatorMinnesota :
        case BaseGCS::pcvTransverseMercatorWisconsin:
        case BaseGCS::pcvTransverseMercatorKruger :
            // Transverse Mercator will work relatively well from North to South pole and XX degrees either way of longitude of origin
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   gcs.GetCentralMeridian(), 15.0, 
                                                   89.9);

#if defined (TOTAL_SPECIAL)
        case BaseGCS::pcvTotalTransverseMercatorBF :
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   gcs.GetCentralMeridian(), 30.0, 
                                                   89.9);
#endif

        // The following are close enough to TM but require latitude origin
        case BaseGCS::pcvObliqueCylindricalHungary :
        case BaseGCS::pcvTransverseMercatorOstn97 :
        case BaseGCS::pcvTransverseMercatorOstn02 :
            // Transverse Mercator will work relatively well from North to South pole and XX degrees either way of longitude of origin
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   gcs.GetOriginLongitude(), 30.0, 
                                                   89.9);

        case BaseGCS::pcvCzechKrovak :
        case BaseGCS::pcvCzechKrovakObsolete :
        case BaseGCS::pcvCzechKrovak95 :
        case BaseGCS::pcvCzechKrovak95Obsolete :
            // Hard-coded domain as origin longitude give 17.39W which couldn't be used as a central meridian for this
            // area. According to Alain Robert, these projections are oblique/conical and this strange origin longitude
            // could have been used as a mean of correction for non-standard prime meridian (not greenwich) used.
            return GetRangeAboutMeridianAndParallel(shape, 
                                                    17.5, 7.5, 
                                                    49.5, 2.5);
        case BaseGCS::pcvTransverseMercatorDenmarkSys34 :
        case BaseGCS::pcvTransverseMercatorDenmarkSys3499 :
        case BaseGCS::pcvTransverseMercatorDenmarkSys3401 :
            {
            int region = gcs.GetDanishSys34Region();

            // 1  ==> jylland
            // 2  ==> sjælland
            // 3  ==> bornholm

            if (1 == region)
                {
                shape.push_back(HGF2DCoord<double>(8.2930, 54.7757));
                shape.push_back(HGF2DCoord<double>(7.9743, 55.0112));
                shape.push_back(HGF2DCoord<double>(7.5544, 56.4801));
                shape.push_back(HGF2DCoord<double>(8.0280, 57.1564));
                shape.push_back(HGF2DCoord<double>(10.4167, 58.0417));
                shape.push_back(HGF2DCoord<double>(10.9897, 57.7786));
                shape.push_back(HGF2DCoord<double>(11.5395, 57.1551));
                shape.push_back(HGF2DCoord<double>(12.0059, 56.5088));
                shape.push_back(HGF2DCoord<double>(11.7200, 54.9853));
                shape.push_back(HGF2DCoord<double>(10.5938, 54.5951));
                shape.push_back(HGF2DCoord<double>(8.2930, 54.7757)); 
                }
            else if (2 == region)
                {
                shape.push_back(HGF2DCoord<double>(11.5108, 54.4367));
                shape.push_back(HGF2DCoord<double>(10.2526, 54.6795));
                shape.push_back(HGF2DCoord<double>(9.6333, 55.0286));
                shape.push_back(HGF2DCoord<double>(9.6157, 55.3831));
                shape.push_back(HGF2DCoord<double>(10.0748, 56.0823));
                shape.push_back(HGF2DCoord<double>(11.5664, 56.9520));
                shape.push_back(HGF2DCoord<double>(13.2099, 565104));
                shape.push_back(HGF2DCoord<double>(13.2097, 54.8276));
                shape.push_back(HGF2DCoord<double>(12.8531, 54.6593));
                shape.push_back(HGF2DCoord<double>(12.1009, 54.5007));
                shape.push_back(HGF2DCoord<double>(11.5108, 54.4367));
                }
            else 
                {
                assert (3 == region);
                shape.push_back(HGF2DCoord<double>(14.510, 54.942));
                shape.push_back(HGF2DCoord<double>(14.510, 55.431));
                shape.push_back(HGF2DCoord<double>(15.300, 55.431));
                shape.push_back(HGF2DCoord<double>(15.300, 54.942));
                shape.push_back(HGF2DCoord<double>(14.510, 54.942));
                }


            }  
            return BSISUCCESS;

        // The conic
        case BaseGCS::pcvAmericanPolyconic :
        case BaseGCS::pcvModifiedPolyconic :
            // For conics we can extent 90 degrees east and west amd from xx degrees up or down from lowest/upper standard parallels
            return GetRangeAboutMeridianAndBoundParallel(shape,
                                                         gcs.GetCentralMeridian(), 89.999999,
                                                         gcs.GetOriginLatitude(), 30.0,
                                                         -89.9, 89.9);
                                                                  
        case BaseGCS::pcvLambertTangential :
        case BaseGCS::pcvLambertConformalConicOneParallel :
        case BaseGCS::pcvSnyderTransverseMercator :
            // For conics we can extent 90 degrees east and west amd from xx degrees up or down from lowest/upper standard parallels
            return GetRangeAboutMeridianAndBoundParallel(shape, 
                                                         gcs.GetOriginLongitude(), 89.999999,
                                                         gcs.GetOriginLatitude(), 30.0,
                                                         -89.9, 89.9);

        case BaseGCS::pcvBonne :
            return GetRangeAboutMeridianAndBoundParallel(shape, 
                                                         gcs.GetOriginLongitude(), 170.999999,
                                                         gcs.GetOriginLatitude(), 60.0,
                                                         -89.9, 89.9);

        case BaseGCS::pcvEquidistantConic :
        case BaseGCS::pcvAlbersEqualArea :
        case BaseGCS::pcvLambertConformalConicTwoParallel :
        case BaseGCS::pcvLambertConformalConicWisconsin :
        case BaseGCS::pcvLambertConformalConicBelgian :
        case BaseGCS::pcvLambertConformalConicMinnesota:
        case BaseGCS::pcvLambertConformalConicAffinePostProcess :
            // For conics we can extent 90 degrees east and west amd from xx degrees up or down from lowest/upper standard parallels
            return GetRangeAboutMeridianAndTwoStandardBoundParallel(shape, 
                                                                    gcs.GetOriginLongitude(), 89.9999,
                                                                    gcs.GetStandardParallel1(), gcs.GetStandardParallel2(), 30.0,
                                                                    -80.0, 80.0);

        case BaseGCS::pcvObliqueCylindricalSwiss :
            // This projection is usually only used in Switzerland but can also be used in Hungary
            // we cannot hard code the extent based on the Switzerland extent but must instead compute the
            // extent based on the latitude and longitude of origin.
            return GetRangeAboutMeridianAndParallel(shape, 
                                                    gcs.GetOriginLongitude(), 6.0, 
                                                    gcs.GetOriginLatitude(), 6.0);


        // Other local projections
        case BaseGCS::pcvHotineObliqueMercator :
        case BaseGCS::pcvNewZealandNationalGrid :
        case BaseGCS::pcvMollweide :
        case BaseGCS::pcvRectifiedSkewOrthomorphic :
        case BaseGCS::pcvRectifiedSkewOrthomorphicCentered :
        case BaseGCS::pcvRectifiedSkewOrthomorphicOrigin :
        case BaseGCS::pcvHotineObliqueMercator1UV :
        case BaseGCS::pcvHotineObliqueMercator1XY :
        case BaseGCS::pcvHotineObliqueMercator2UV :
        case BaseGCS::pcvHotineObliqueMercator2XY :
        case BaseGCS::pcvObliqueMercatorMinnesota :
            // Even though it cannot be computed, the domain must be set as the caller may not check the return status.
            GetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);
            return BSIERROR; // return not implemented;

#if NOT_YET
            // This one is a bit complicated by the fact the hemisphere can be centered anywhere on earth. 
            // If centered at a pole, the domain extends from 0 to -90 in latitude and around the globe in longitude
            // If centered somewhere on the equator, then it is valid from North to South pole but 90 degrees east and west of center
            // If centered elsewhere, the area is not easily representable in the form of min max of lat long...
            return GetRangeAboutBoundMeridianAndBoundParallel(shape,
                                                              gcs.GetOriginLongitude(), 30.0,
                                                              -180.0, 180.0,
                                                              gcs.GetOriginLatitude(), 30.0,
                                                              -89.9, 89.9);
#endif

        // Other
        case BaseGCS::pcvNonEarth :
        case BaseGCS::pcvNonEarthScaleRotation :
        case BaseGCS::pcvObliqueConformalConic :
            GetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);
            return BSIERROR; // return not implemented;

        case BaseGCS::pcvUniversalTransverseMercator :
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   GetUTMZoneCenterMeridian(gcs.GetUTMZone()), 15.0, 
                                                   89.9);

#if defined (TOTAL_SPECIAL)
        case BaseGCS::pcvTotalUniversalTransverseMercatorBF :
            return GetRangeAboutMeridianAndEquator(shape, 
                                                   GetUTMZoneCenterMeridian(gcs.GetUTMZone()), 30.0, 
                                                   89.9);

#endif //TOTAL_SPECIAL
        default:
            break;
        }

    HASSERT(!"Not implemented ... please do so");
    return BSIERROR; 
    }

END_UNNAMED_NAMESPACE


BEGIN_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Returns the domain of application for a give geographic coordinate system. 
* This domain is the mathematical domain intersected
* with the logical domain if one is set.
* @bsimethod                                                    AlainRobert  2/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetGeoDomain
(
 BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& GCSPtr,
 double* minLongitude,
 double* maxLongitude,
 double* minLatitude,
 double* maxLatitude
) 
    {

    vector<HGF2DCoord<double>> myPoints;
    StatusInt status = GetGeoDomainShape(*GCSPtr, myPoints);

    if (myPoints.size() > 0)
        {
        *minLongitude = myPoints[0].GetX();
        *maxLongitude = myPoints[0].GetX();
        *minLatitude = myPoints[0].GetY();
        *maxLatitude = myPoints[0].GetY();
        for (size_t indexPoint = 1 ; indexPoint < myPoints.size() ; indexPoint++)
            {
            *minLongitude = min(myPoints[indexPoint].GetX(), *minLongitude);
            *maxLongitude = max(myPoints[indexPoint].GetX(), *maxLongitude);
            *minLatitude = min(myPoints[indexPoint].GetY(), *minLatitude);
            *maxLatitude = max(myPoints[indexPoint].GetY(), *maxLatitude);
             
            }
        }
    return status;
    
    }

/*---------------------------------------------------------------------------------**//**
* Returns the domain of application for GCS. 
* The domain returned is expressed as latitude/longitude in degrees. The coordinates
* are to be interpreted for processing purposes as Plate Carree where multiple
* coordinates may be defined for either North or South pole to represent the whole 
* range of effective values. The values for the longitude are effectively limited to the
* -180 to 180 degrees range. This may result in strange behavior at the split point
* 
* @bsimethod                                                    AlainRobert  2/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetGeoMathematicalDomain(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& GCSPtr, GeoDomainShape& shape) 
    {
    StatusInt status = SUCCESS;
    double longMin;
    double longMax;
    double latMin;
    double latMax;
    HGF2DCoord<double> previousPoint;
    HGF2DCoord<double> firstPoint;
    HGF2DCoord<double> thePoint;

    status = GetGeoDomain(GCSPtr, &longMin, &longMax, &latMin, &latMax);

    shape.push_back(HGF2DCoord<double>(longMin, latMin));
    shape.push_back(HGF2DCoord<double>(longMin, latMax));
    shape.push_back(HGF2DCoord<double>(longMax, latMax));
    shape.push_back(HGF2DCoord<double>(longMax, latMin));
    shape.push_back(HGF2DCoord<double>(longMin, latMin));

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* Returns the domain of application for GCS. 
* @bsimethod                                                    AlainRobert  2/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetMathematicalDomain(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& GCSPtr, GeoDomainShape& shape)
    {
    //if(HRFGeoCoordinateProvider::GetServices() == NULL)
    //    return ERROR;

    StatusInt status = SUCCESS;

    GeoDomainShape geoPoints;
    
    status = GetGeoMathematicalDomain(GCSPtr, geoPoints);
    BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr pLL84 = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS(L"LL84");
   //RasterBaseGcsPtr pTarget = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromKeyName(L"LL84");
   // IRasterBaseGcsPtr pSource = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromBaseGcs(GCSPtr.get());

    // We then create the Image++ compatible geographic transformation between these
    // Geographic coordinate systems...
    HFCPtr<HCPGCoordModel> pTransfo = new HCPGCoordModel(*pLL84, *GCSPtr);

    HFCPtr<HGF2DCoordSys> pSourceCS = new HGF2DCoordSys();
    HFCPtr<HGF2DCoordSys> pTargetCS = new HGF2DCoordSys(*pTransfo, pSourceCS);


    HFCPtr<HVE2DShape> geoDomainShape = new HVE2DPolygonOfSegments(HGF2DPolygonOfSegments(geoPoints), pSourceCS);
    HFCPtr<HVE2DShape> cartesianDomain = static_cast<HVE2DShape*>(geoDomainShape->AllocateCopyInCoordSys (pTargetCS));

    HGF2DLocationCollection listOfPoints;
    cartesianDomain->Drop (&listOfPoints, 1);
    unsigned long ptIndex;
    for (ptIndex = 0 ; ptIndex < listOfPoints.size() ; ptIndex++)
        {
        shape.push_back(listOfPoints[ptIndex].GetPosition());
        }

    return status;
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Returns the shape defining the intersection of the two give GCS domains. It is quite likely
* the intersection will be empty. The given coordinate systems is the coordinate system
* that is used for the latitude/longitude underlying geographic coordinate system of either
* given GCS 
AlainRobert  2/2009
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HVE2DShape> GetGCSDomainsIntersection(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& firstGCSPtr, 
                                             BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& secondGCSPtr, 
                                             HFCPtr<HGF2DCoordSys> latitudeLongitudeCoordSys)
    {
    GeoDomainShape sourceGeoDomain;
    GeoDomainShape destinationGeoDomain;

    GetGeoMathematicalDomain(firstGCSPtr, sourceGeoDomain);
    GetGeoMathematicalDomain(secondGCSPtr, destinationGeoDomain);

    // Create shapes from the two geo domains
    HFCPtr<HVE2DShape> sourceDomainShape = new HVE2DPolygonOfSegments(HGF2DPolygonOfSegments(sourceGeoDomain), latitudeLongitudeCoordSys);
    HFCPtr<HVE2DShape> destinationDomainShape = new HVE2DPolygonOfSegments(HGF2DPolygonOfSegments(destinationGeoDomain), latitudeLongitudeCoordSys);
    
    // We obtain the intersection of the two geo domains ... this is in geographic lat/long coordinates the domain of transformation from
    // target to/from source GCS ... Note that if the two GCS all ill chosen, the result may be empty
    HFCPtr<HVE2DShape> resultDomainShape = sourceDomainShape->IntersectShape (*destinationDomainShape);

    return resultDomainShape;

    }

void MergeExtentsInPlace (DRange2d &destRange, const DRange2d& sourceRange)
    {
    if (sourceRange.low.x < destRange.low.x) destRange.low.x = sourceRange.low.x;
    if (sourceRange.high.x > destRange.high.x) destRange.high.x = sourceRange.high.x;
    if (sourceRange.low.y < destRange.low.y) destRange.low.y = sourceRange.low.y;
    if (sourceRange.high.y > destRange.high.y) destRange.high.y = sourceRange.high.y;
    }

/*---------------------------------------------------------------------------------**//**
* This function reprojects a range from one coordinate system to another. It applies
* the domain of validity of both coordinate system so the result is the extent of the
* intersection of source range and domain of validity of both given geographic
* coordinate systems. ERROR will be returned if the result is empty. 
* Note that the result range may extend slightly outside the domain of validity of 
* either given geographic coordinate systems and thus precautions must be taken when using 
* this result.
* @bsimethod                                                    AlainRobert  2/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ReprojectRangeDomainLimited(DRange3d& reprojectedRange, 
                                      const DRange3d& initialRange, 
                                      BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCS,
                                      BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS)
    {
   // if(HRFGeoCoordinateProvider::GetServices() == NULL)
   //     return ERROR;

    // In order to compute the range in given geographic coordinate system, we only have the content extent
    // of the index. Hopefully, eventually, we will have the properly computed hull.
    // Create the three coordinate systems required for transformation
    HFCPtr<HGF2DCoordSys> latLongCoordinateSystem = new HGF2DCoordSys();

    HFCPtr<HVE2DShape> resultDomainShape = GetGCSDomainsIntersection(sourceGCS, targetGCS, latLongCoordinateSystem);

    // We now create Image++ compatible Geographic Coordinate System objects ...
    BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr pLL84 = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS(L"LL84");
    //IRasterBaseGcsPtr pLL84 = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromKeyName(L"LL84");
   // IRasterBaseGcsPtr pSource = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromBaseGcs(sourceGCS.get());
   // IRasterBaseGcsPtr pTarget = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromBaseGcs(targetGCS.get());
    
    // We then create the Image++ compatible geographic transformation between these
    // Geographic coordinate systems...
    HFCPtr<HCPGCoordModel> pSourceToLL84 = new HCPGCoordModel(*sourceGCS, *pLL84);
    HFCPtr<HCPGCoordModel> pTransfo = new HCPGCoordModel(*targetGCS, *sourceGCS);

    // We create two dummies coordinate systems linked using this geographic transformation
    HFCPtr<HGF2DCoordSys> pSourceCS = new HGF2DCoordSys(*pSourceToLL84, latLongCoordinateSystem);
    HFCPtr<HGF2DCoordSys> pTargetCS = new HGF2DCoordSys(*pTransfo, pSourceCS);

    HVE2DRectangle boundingShape(initialRange.low.x, initialRange.low.y,  initialRange.high.x, initialRange.high.y, pSourceCS);
    boundingShape.SetTolerance(min (initialRange.high.x - initialRange.low.x, initialRange.high.y - initialRange.low.y) / 400.0);

    HFCPtr<HVE2DShape> extentDomained = boundingShape.IntersectShape(*resultDomainShape);


    // And now we perform the reprojection proper. The purpose of using this Image++ architecture compliant
    // transformation allows automatic densification, needle removal and processing of berserking
    // transformations. What it does not provide however is error processing in case te transformation
    // requested is meaningless.
    HFCPtr<HVE2DShape> extentDomainedInTarget = static_cast<HVE2DShape*>(extentDomained->AllocateCopyInCoordSys(pTargetCS));

    // The result may be empty
    if (extentDomainedInTarget->IsEmpty())
        return ERROR;

    // We finaly extract the extent of the result shape
    HGF2DExtent extent = extentDomainedInTarget->GetExtent();
    reprojectedRange.low.x = extent.GetXMin();
    reprojectedRange.low.y = extent.GetYMin();
    reprojectedRange.low.z = initialRange.low.z;

    reprojectedRange.high.x = extent.GetXMax();
    reprojectedRange.high.y = extent.GetYMax();
    reprojectedRange.high.z = initialRange.high.z;

    return SUCCESS;
    }


/*----------------------------------------------------------------------------+
|GetReprojectedBoxDomainLimited
| Reprojects the 3D box from source to target GCS yet performs a limitations 
| of the box to both source and target geographic coordinate systems mathematical domains.
| The source and target arrays can be the same array.
| If the result box cannot be correctly represented when reprojected because then
| a non SUCCESS value is returned.
| The queryShape may be NULL if none is provided.
| The additionalSourceExtent must be expressed in the coordinates of the source. If
| this extent is not defined (empty) then it will be disregarded.
+----------------------------------------------------------------------------*/ 
StatusInt GetReprojectedBoxDomainLimited(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr, 
                                         BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,  
                                         DPoint3d                             boxPoints[], 
                                         DPoint3d                             reprojectedBoxPoints[], 
                                         DRange3d                             additionalSourceExtent,
                                         HFCPtr<HVE2DShape>                   queryShape)
    { 
    //if(HRFGeoCoordinateProvider::GetServices() == NULL)
    //    return ERROR;

    // Here we have the viewbox expressed in the cartesian target GCS yet this viewbox may exceed the
    // Target GCS mathematical domain. It is also likely that it will exceeed in addition the source GCS
    // mathematical domain. For this reason, we will perform the clipping of the viewbox to the domains
    // It is unsure what the result viewbox shape will be. 
    // Since the wiewbox is a 3D object but the operations for clipping to the mathematical domains is 2D
    // we will preserve the elevations separately.
    StatusInt status = SUCCESS;

    // If we get here, something went wrong ... The most likely possibility is that the
    // coordinates exceeded one of the mathematical domains of either GCS. We will thus perform extensive operations to
    // limit the box extent prior to reprojection.
    // We first obtain the geographic domain from both GCS

    // If the two coordinate systems are equivalent then we do not reproject,
    // even if the viewbox extends out of the domain as since no reprojection occurs no domain
    // limitations are effectively applied.
    if (sourceGCSPtr->IsEquivalent(*targetGCSPtr))
        {
        memcpy (reprojectedBoxPoints, boxPoints, sizeof(DPoint3d) * 8);
        return SUCCESS;
        }

    // Create the three coordinate systems required for transformation
    HFCPtr<HGF2DCoordSys> latLongCoordinateSystem = new HGF2DCoordSys();


    HFCPtr<HVE2DShape> resultDomainShape = GetGCSDomainsIntersection(sourceGCSPtr, targetGCSPtr, latLongCoordinateSystem);

    // We create a LL84 baseGCS in which are effectively expressed the geo domain
    BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr pLL84 = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS(L"LL84");
    //IRasterBaseGcsPtr pLL84 = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromKeyName(L"LL84");
    //IRasterBaseGcsPtr pSource = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromBaseGcs(sourceGCSPtr.get());
    //IRasterBaseGcsPtr pTarget = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromBaseGcs(targetGCSPtr.get());

    //Now we limit this domain to the query extent which is given in source coordinates
    HFCPtr<HCPGCoordModel> pSourceToLL84 = new HCPGCoordModel(*sourceGCSPtr, *pLL84);
    HFCPtr<HGF2DCoordSys> pSourceCS = new HGF2DCoordSys(*pSourceToLL84, latLongCoordinateSystem);

    // Intersect with additional extent if one is defined
    HFCPtr<HVE2DShape> limitedDomainShapeA = resultDomainShape;
    if ((additionalSourceExtent.high.x > additionalSourceExtent.low.x) && (additionalSourceExtent.high.y > additionalSourceExtent.low.y))
        {
        HVE2DRectangle additionalRectangle(additionalSourceExtent.low.x, additionalSourceExtent.low.y, additionalSourceExtent.high.x, additionalSourceExtent.high.y, pSourceCS);
        limitedDomainShapeA = resultDomainShape->IntersectShape (additionalRectangle);
        }

    // Intersect with querySHape if one is provided.
    HFCPtr<HVE2DShape> limitedDomainShape = limitedDomainShapeA;
    if (queryShape != NULL)
        {
        HFCPtr<HVE2DShape> queryShapeCopy = static_cast<HVE2DShape*>(queryShape->Clone());
        queryShapeCopy->SetCoordSys(pSourceCS);

        limitedDomainShape = limitedDomainShapeA->IntersectShape (*queryShapeCopy);
        }


    if (limitedDomainShape->IsEmpty())
        {
        return ERROR;
        }
    // We now convert this geographic lat/long domain into the target GCS coordinates
    HFCPtr<HCPGCoordModel> pTransfo = new HCPGCoordModel(*targetGCSPtr, *pLL84);
    HFCPtr<HGF2DCoordSys> pTargetCS = new HGF2DCoordSys(*pTransfo, latLongCoordinateSystem);
    HFCPtr<HVE2DShape> cartesianDomain = static_cast<HVE2DShape*>(limitedDomainShape->AllocateCopyInCoordSys (pTargetCS));

    // At this point we have the domain in the raget GCS ... What we will do is create 8 shapes representing the flatten
    // copies of each of the 8 viewbox faces to the x-y plane. We will interesct the union of these 8 shapes witht the domain
    // The net result will be a shape in target GCS of the limit. We will then convert into source GCS then obtain the
    // extent of the result shape (since we will not be reprojecting at this stage we can use coordinates outside the
    // domains of application of individual GCS). Then we will recreate a viewbox based on this extent and the elevation values of the
    // initial bounding box.
    
    HFCPtr<HVE2DShape> pFacesUnion = new HVE2DVoidShape(pTargetCS);

    for (size_t idxFaces = 0 ; idxFaces < 6 ; idxFaces++)
        {
        // Create a polysegment
        HVE2DPolySegment myFacePolySegment (pTargetCS);
        for (size_t idxPt = 0 ; idxPt < 4 ; idxPt++)
            {
            HGF2DPosition currentPoint(boxPoints[s_faceToBoxPoint[idxFaces][idxPt]].x, 
                                       boxPoints[s_faceToBoxPoint[idxFaces][idxPt]].y);
            myFacePolySegment.AppendPosition(currentPoint);
            }
        // We close the polysegment
        HGF2DPosition endPoint(boxPoints[s_faceToBoxPoint[idxFaces][0]].x, 
                               boxPoints[s_faceToBoxPoint[idxFaces][0]].y);
        myFacePolySegment.AppendPosition(endPoint);

        assert(!myFacePolySegment.AutoCrosses());
        
        // Note that the polysegment may be autocontiguous ... Usually this would resolve into a void shape
        if (!myFacePolySegment.IsAutoContiguous())
            {
            HVE2DPolygonOfSegments myFace(myFacePolySegment);
            pFacesUnion = pFacesUnion->UnifyShape(myFace);
            }
        }
    // We now have the union of all xy projected faces ...

    HFCPtr<HVE2DShape> resultDomainShapeInTarget = static_cast<HVE2DShape*>(resultDomainShape->AllocateCopyInCoordSys (pTargetCS));

    // The viewbox may extend out of our domain ... it must be limited.
    // We intersect with our limited domain
    HFCPtr<HVE2DShape> limitedDomainFlattenViewBoxIntersection = pFacesUnion->IntersectShape(*cartesianDomain);

    if (limitedDomainFlattenViewBoxIntersection->IsEmpty())
        {
        return ERROR;
        }

    // We set the tolerance such that no more than 1/100 of shape size is set
    HGF2DExtent shapeExtent = limitedDomainFlattenViewBoxIntersection->GetExtent();
    double minExtentSize = min(shapeExtent.GetWidth(), shapeExtent.GetHeight());
    double tolerance = minExtentSize/100.0;
    limitedDomainFlattenViewBoxIntersection->SetTolerance (tolerance);

    // Finaly we transform the result in the source GCS
    HFCPtr<HVE2DShape> finalSourceViewBoxPrint = static_cast<HVE2DShape*>(limitedDomainFlattenViewBoxIntersection->AllocateCopyInCoordSys (pSourceCS));

    shapeExtent.ChangeCoordSys(pSourceCS);
    if (shapeExtent.IsDefined())
        {
    

        // Obtain the zMin and zMax of viewbox
        double zMin = boxPoints[0].z;
        double zMax = boxPoints[0].z;
        for (size_t idxPoint = 1 ; idxPoint < 8 ; idxPoint++)
            {
            zMin = min(zMin, boxPoints[idxPoint].z);
            zMax = max(zMax, boxPoints[idxPoint].z);
            }

        FillBBoxFromIppExtent(reprojectedBoxPoints, shapeExtent, zMin, zMax);
        }
     
    return status;
    }


/*----------------------------------------------------------------------------+
|GetReprojectedBox
| Reprojects the 3D box from source to target GCS.
+----------------------------------------------------------------------------*/ 
HFCPtr<HVE2DShape> ReprojectShapeDomainLimited(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr, 
                                      BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr,  
                                      const DPoint3d*   pi_pSourcePt,
                                      size_t  pi_SourcePtQty)
            
    {
   // if(HRFGeoCoordinateProvider::GetServices() == NULL)
   //     return NULL;

    // Something went wrong ... we need to limit the source shape to the domains
    // Create the three coordinate systems required for transformation
    HFCPtr<HGF2DCoordSys> latLongCoordinateSystem = new HGF2DCoordSys();

    HFCPtr<HVE2DShape> resultDomainShape = GetGCSDomainsIntersection(sourceGCSPtr, targetGCSPtr, latLongCoordinateSystem);

    if (NULL == resultDomainShape || resultDomainShape->IsEmpty())
        return NULL;

    // We create a LL84 baseGCS in which are effectively expressed the geo domain
    //IRasterBaseGcsPtr pLL84 = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromKeyName(L"LL84");
    BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr pLL84 = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS(L"LL84");
   // IRasterBaseGcsPtr pSource = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromBaseGcs(sourceGCSPtr.get());
   // IRasterBaseGcsPtr pTarget = HRFGeoCoordinateProvider::GetServices()->_CreateRasterBaseGcsFromBaseGcs(targetGCSPtr.get());

    //Now we limit this domain to the query extent which is given in source coordinates
    HFCPtr<HCPGCoordModel> pSourceToLL84 = new HCPGCoordModel(*sourceGCSPtr, *pLL84);
    HFCPtr<HGF2DCoordSys> pSourceCS = new HGF2DCoordSys(*pSourceToLL84, latLongCoordinateSystem);


    // We now convert this geographic lat/long domain into the target GCS coordinates
    HFCPtr<HCPGCoordModel> pTransfo = new HCPGCoordModel(*targetGCSPtr, *pLL84);
    HFCPtr<HGF2DCoordSys> pTargetCS = new HGF2DCoordSys(*pTransfo, latLongCoordinateSystem);

    HFCPtr<HVE2DShape> pShapeToReproject = CreateShapeFromPoints(pi_pSourcePt, pi_SourcePtQty, pTargetCS);

    // Intersect shape to reproject with domain
    HFCPtr<HVE2DShape> pDomainedShape = pShapeToReproject->IntersectShape(*resultDomainShape);

    if (NULL == pDomainedShape)
        return NULL;

    // Allocate a reprojected copy
    HFCPtr<HVE2DShape> pReprojectedShape = static_cast<HVE2DShape*>(pDomainedShape->AllocateCopyInCoordSys(pSourceCS));

    return pReprojectedShape;
    }
/*----------------------------------------------------------------------------+
| AddIslandToDTM
| Protected... Utility method used to set the shape of a DTM object.
| The AddIsland is usually called by the utility method AddShapeToDTM() to add a 
| single simple shape. If a void shape (HVE2DVoidShape) is provided it will work but
| of course add nothing.
+----------------------------------------------------------------------------*/ 
int AddIslandToDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&              dtmPtr, 
                   const HVE2DShape&    island)
    {
    assert((dtmPtr != 0) && (dtmPtr->GetBcDTM() != 0) && (dtmPtr->GetBcDTM()->GetTinHandle()));
        
    int status = SUCCESS;
    
    HGF2DLocationCollection                 pointCollection;
    HGF2DLocationCollection::const_iterator pointIter;
    HGF2DLocationCollection::const_iterator pointIterEnd;
    HArrayAutoPtr<DPoint3d>                      featurePtsPtr;

    BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());

    pointCollection.clear();

    island.Drop(&pointCollection, island.GetTolerance());
    
    // It may occur that the given shape is a void shape ... so we validate
    // the number of points to make sure it is greater than 0    
    if (pointCollection.size() > 0)
        {
        featurePtsPtr = new DPoint3d[pointCollection.size()];
        
        pointIter    = pointCollection.begin();
        pointIterEnd = pointCollection.end();
        int pointInd = 0;        

        while (pointIter != pointIterEnd)
            {                                      
            featurePtsPtr[pointInd].x = pointIter->GetX();    
            featurePtsPtr[pointInd].y = pointIter->GetY();
            featurePtsPtr[pointInd].z = 0;              
            pointInd++;
            pointIter++;
            }                       

        // Check if given DTM is already triangulated  ...

        long numDrapePoints = (long)pointCollection.size();  // Default value to be used if not tinned

        if (dtmObjP->dtmState == DTMState::Tin)
            {
            // Since the tin is already triangulated we are going to drape the island upon tin then use this as island
            // This insures that the elevation of the island is coherent with original DTM
            
            // The following code used to be in use but this resulted in clips with a lot more points
            // while it appears that the draping is automatically performed on clipping if the outter void is drape
            // So it was desactivated and the code is only preserved as an example
            // DTM_DRAPE_POINT *drapePointsP = NULL;
            // status = bcdtmDrape_stringDtmObject(dtmObjP, featurePtsPtr, pointCollection.size(), FALSE, &drapePointsP, &numDrapePoints);
            // if (0 == status) 
            //     {
            //    featurePtsPtr = new DPoint3d[numDrapePoints];
            //    for (long idxDum = 0 ; idxDum < numDrapePoints ; idxDum++)
            //        {
            //        featurePtsPtr[idxDum].x = drapePointsP[idxDum].drapeX;
            //        featurePtsPtr[idxDum].y = drapePointsP[idxDum].drapeY;
            //        featurePtsPtr[idxDum].z = drapePointsP[idxDum].drapeZ;
            //        }
            //    }
            //
            // Now only the original clip points are draped on the TIN and it appears to work the same

            long drapeFlag = 0;
            for (unsigned long idxDum = 0 ; idxDum < pointCollection.size() ; idxDum++)
                {
                status = bcdtmDrape_pointDtmObject(dtmObjP, featurePtsPtr[idxDum].x, featurePtsPtr[idxDum].y, &(featurePtsPtr[idxDum].z), &drapeFlag );
                }

            }
        

        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Island, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, featurePtsPtr.get(), numDrapePoints);
    }

    assert(status == 0);

    return status;
    }


/*----------------------------------------------------------------------------+
| AddHolesToDTM
| Protected... Utility method used to set the shape of a DTM object.
| The AddHoles is usually called by the utility method AddShapeToDTM() to add a 
| list of holes. 
+----------------------------------------------------------------------------*/ 
int AddHolesToDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&                     dtmPtr, 
                  const HVE2DShape::HoleList& holeList)
    {
    assert((dtmPtr != 0) && (dtmPtr->GetBcDTM() != 0) && (dtmPtr->GetBcDTM()->GetTinHandle()));
        
    int status = 0;
    HVE2DShape::HoleList::const_iterator holeIter(holeList.begin());
    HVE2DShape::HoleList::const_iterator holeIterEnd(holeList.end());

    HGF2DLocationCollection                 pointCollection;
    HGF2DLocationCollection::const_iterator pointIter;
    HGF2DLocationCollection::const_iterator pointIterEnd;
    HArrayAutoPtr<DPoint3d>                      featurePtsPtr;

    BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());

    while (holeIter != holeIterEnd)
        {        
        (*holeIter)->Drop(&pointCollection, (*holeIter)->GetTolerance());
        
        featurePtsPtr = new DPoint3d[pointCollection.size()];
        
        pointIter = pointCollection.begin();
        pointIterEnd = pointCollection.end();
        int pointInd = 0;        

        while (pointIter != pointIterEnd)
            {                                      
            featurePtsPtr[pointInd].x = pointIter->GetX();    
            featurePtsPtr[pointInd].y = pointIter->GetY();
            featurePtsPtr[pointInd].z = 0;              
            pointInd++;
            pointIter++;
            }      

        long numDrapePoints = (long)pointCollection.size();  // Default value to be used if not tinned

        if (dtmObjP->dtmState == DTMState::Tin)
            {
            // Since the tin is already triangulated we are going to drape the island upon tin then use this as island
            // This insures that the elevation of the island is coherent with original DTM
            
            // The following code used to be in use but this resulted in clips with a lot more points
            // while it appears that the draping is automatically performed on clipping if the outter void is drape
            // So it was desactivated and the code is only preserved as an example
            // DTM_DRAPE_POINT *drapePointsP = NULL;
            // status = bcdtmDrape_stringDtmObject(dtmObjP, featurePtsPtr, pointCollection.size(), FALSE, &drapePointsP, &numDrapePoints);
            // if (0 == status) 
            //     {
            //    featurePtsPtr = new DPoint3d[numDrapePoints];
            //     for (long idxDum = 0 ; idxDum < numDrapePoints ; idxDum++)
            //         {
            //         featurePtsPtr[idxDum].x = drapePointsP[idxDum].drapeX;
            //         featurePtsPtr[idxDum].y = drapePointsP[idxDum].drapeY;
            //         featurePtsPtr[idxDum].z = drapePointsP[idxDum].drapeZ;
            //         }
            //     }
            //
            // Now only the original clip points are draped on the TIN and it appears to work the same
            long drapeFlag = 0;
            for (unsigned long idxDum = 0 ; idxDum < pointCollection.size() ; idxDum++)
                {
                status = bcdtmDrape_pointDtmObject(dtmObjP, featurePtsPtr[idxDum].x, featurePtsPtr[idxDum].y, &(featurePtsPtr[idxDum].z), &drapeFlag );
                }
            }

        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Hole, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, featurePtsPtr.get(), numDrapePoints);

        pointCollection.clear();

        assert(status == 0);
       
        holeIter++;
        }

    return status;
    }


/*----------------------------------------------------------------------------+
| AddClipToDTM
| Protected... Utility method used to set the shape of a DTM object.
| Usually called internaly after the ClipShape has been properly computed and
| DTM been generated with the result of a query.]
| The AddShapeToDTM is usually called by the utility method SetShapeToDTM()
| that will first add an all encompassing void shape to the DTM then call the present
| method. Individual components will then be added as islands or holes depending
| if the internal basic shape is included or not. Note that the present
| function will operate correctly any kind of shape except the Universe shape
| which cannot be supported (and needs not be supported as it represents no
| shape clipping at all).
+----------------------------------------------------------------------------*/ 
int AddClipToDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&           dtmPtr,
                  const HVE2DShape& shape)
    {
    StatusInt status = SUCCESS;
    if (shape.IsComplex())
        {
        HVE2DShape::ShapeList::const_iterator shapeIterator;
        shapeIterator = shape.GetShapeList().begin();
        for (; shapeIterator != shape.GetShapeList().end() ; ++shapeIterator)
            {
            // Recurse into self method ... the complex shape component can be a complex shape itself
            AddClipToDTM (dtmPtr, **shapeIterator);
            }
        }
    else if (shape.IsSimple())
        {
        status = AddIslandToDTM(dtmPtr, 
                           shape);
                           

        assert(status == 0);
        }
    else
        {
        // Holed shape
        const HVE2DHoledShape& holedShape = static_cast<const HVE2DHoledShape&>(shape);

        status = AddIslandToDTM(dtmPtr, 
                           holedShape.GetBaseShape());
                           

        assert(status == 0);

        if (holedShape.HasHoles())
            {                    
            status = AddHolesToDTM(dtmPtr, holedShape.GetHoleList());                                

            assert(status == 0);
            }
        }
    return status;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshPointQuery::SetClipToDTM
| Protected... Utility method used to set the shape of a DTM object.
| Usually called internaly after the ClipShape has been properly computed and
| DTM been generated with the result of a query.
+----------------------------------------------------------------------------*/ 
#ifndef NDEBUG
    static bool s_intersectWithConvexHull = true;
#endif

int SetClipToDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&              dtmPtr,
                 const DRange3d&      spatialIndexRange,
                 const HVE2DShape&    shape)
    {

    StatusInt status = SUCCESS;
    HFCPtr<HVE2DShape> newShape;
    BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());

    HFCPtr<HVE2DPolygonOfSegments> convexHull;
    DPoint3d   *tVerticesP = NULL;
    long nVertices = 0;

    // If the tin is already triangulated then we use the convex hull as outter void shape
    if (dtmObjP->dtmState == DTMState::Tin)
        {


        status  = bcdtmList_extractHullDtmObject(dtmObjP, &tVerticesP, &nVertices);

        
        if (nVertices > 0)
            {
            HArrayAutoPtr<double> tempBuffer(new double[(nVertices + 1) * 2]);

            for (long ptIndex = 0 ; ptIndex < nVertices ; ptIndex++)
                {
                tempBuffer[ptIndex * 2]     = tVerticesP[ptIndex].x;
                tempBuffer[ptIndex * 2 + 1] = tVerticesP[ptIndex].y;    
                }

            HVE2DPolySegment shapeContour(nVertices * 2, tempBuffer, shape.GetCoordSys());
            if (!(shapeContour.AutoCrosses() && shapeContour.IsAutoContiguous()))
                {
                convexHull = new HVE2DPolygonOfSegments(nVertices * 2, tempBuffer, shape.GetCoordSys());
                }
            }        
        }

#ifdef NDEBUG
    if (NULL != convexHull)
#else
    //In Debug shape can be so long that it might be sounds to remove the intersection with the convex 
    //hull, even more when the shape completely reside inside the convex hull.
    if (NULL != convexHull && s_intersectWithConvexHull)
#endif
        {
        newShape = convexHull->IntersectShape(shape);

        bool shapeFullyIncluded = false;

        shapeFullyIncluded = (HVE2DShape::S_IN == convexHull->CalculateSpatialPositionOf (*newShape)) && (!convexHull->AreContiguous(*newShape));

        if (shapeFullyIncluded)
            {
            // The use of DRAPE VOID insures the convex hull is draped upon the TIN regardless of the Z of the void
            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::DrapeVoid, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, tVerticesP, nVertices);
            }
        else
            {

            double width = spatialIndexRange.high.x - spatialIndexRange.low.x;

            HFCPtr<HVE2DPolygonOfSegments> hullCopy;
            if (convexHull->CalculateRotationDirection() == HVE2DSimpleShape::CCW)
                hullCopy = convexHull->AllocateParallelCopy(width / 200.0, HVE2DVector::ALPHA);
            else
                hullCopy = convexHull->AllocateParallelCopy(width / 200.0, HVE2DVector::BETA);

            HGF2DLocationCollection thePoints;
            hullCopy->Drop (&thePoints, hullCopy->GetTolerance());



            
            HArrayAutoPtr<DPoint3d>                      featurePtsPtr;

            featurePtsPtr = new DPoint3d[thePoints.size()];

            for (size_t idx = 0 ; idx < thePoints.size() ; idx++)
                {
                featurePtsPtr[idx].x = thePoints[idx].GetX();
                featurePtsPtr[idx].y = thePoints[idx].GetY();
                featurePtsPtr[idx].z = 0; // As mentionned below the Z is disregarded
                }
            // The use of DRAPE VOID insures the convex hull is draped upon the TIN regardless of the Z of the void
            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::DrapeVoid, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, featurePtsPtr, (long)thePoints.size());

            }


        }
    else
        {
        bool shapeFullyIncluded = false;
        HVE2DRectangle indexRangeShape(spatialIndexRange.low.x, 
                                       spatialIndexRange.low.y,
                                       spatialIndexRange.high.x, 
                                       spatialIndexRange.high.y, 
                                       shape.GetCoordSys());

        shapeFullyIncluded = (HVE2DShape::S_IN == indexRangeShape.CalculateSpatialPositionOf (shape)) && (!indexRangeShape.AreContiguous(shape));
        HArrayAutoPtr<DPoint3d>                      featurePtsPtr;

        featurePtsPtr = new DPoint3d[5];

        double width = spatialIndexRange.high.x - spatialIndexRange.low.x;
        double height = spatialIndexRange.high.y - spatialIndexRange.low.y;

        if (!shapeFullyIncluded)
            { 
            featurePtsPtr[0].x = spatialIndexRange.low.x - (width/200.0);    
            featurePtsPtr[0].y = spatialIndexRange.low.y - (height/200.0);
            featurePtsPtr[0].z = 0; 
            featurePtsPtr[1].x = spatialIndexRange.low.x - (width/200.0);    
            featurePtsPtr[1].y = spatialIndexRange.high.y + (height/200.0);
            featurePtsPtr[1].z = 0; 
            featurePtsPtr[2].x = spatialIndexRange.high.x + (width/200.0);    
            featurePtsPtr[2].y = spatialIndexRange.high.y + (height/200.0);
            featurePtsPtr[2].z = 0; 
            featurePtsPtr[3].x = spatialIndexRange.high.x + (width/200.0);    
            featurePtsPtr[3].y = spatialIndexRange.low.y - (height/200.0);
            featurePtsPtr[3].z = 0; 
            featurePtsPtr[4].x = spatialIndexRange.low.x - (width/200.0);    
            featurePtsPtr[4].y = spatialIndexRange.low.y - (height/200.0);
            featurePtsPtr[4].z = 0; 
            }
        else
            {
            featurePtsPtr[0].x = spatialIndexRange.low.x;    
            featurePtsPtr[0].y = spatialIndexRange.low.y;
            featurePtsPtr[0].z = 0; 
            featurePtsPtr[1].x = spatialIndexRange.low.x;    
            featurePtsPtr[1].y = spatialIndexRange.high.y;
            featurePtsPtr[1].z = 0; 
            featurePtsPtr[2].x = spatialIndexRange.high.x;    
            featurePtsPtr[2].y = spatialIndexRange.high.y;
            featurePtsPtr[2].z = 0; 
            featurePtsPtr[3].x = spatialIndexRange.high.x;    
            featurePtsPtr[3].y = spatialIndexRange.low.y;
            featurePtsPtr[3].z = 0; 
            featurePtsPtr[4].x = spatialIndexRange.low.x;    
            featurePtsPtr[4].y = spatialIndexRange.low.y;
            featurePtsPtr[4].z = 0; 
            }

        // The use of DRAPE VOID insures the convex hull is draped upon the TIN regardless of the Z of the void
        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::DrapeVoid, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, featurePtsPtr.get(), 5);


        HFCPtr<HVE2DShape> extentShape(new HVE2DRectangle(spatialIndexRange.low.x, 
                                                      spatialIndexRange.low.y,
                                                      spatialIndexRange.high.x, 
                                                      spatialIndexRange.high.y, 
                                                                shape.GetCoordSys()));


        newShape = extentShape->IntersectShape(shape);
    
    }

           
    if (tVerticesP != NULL)
        free(tVerticesP);


    // Then we will add the individual islands and holes to this outter shape ...
    return AddClipToDTM(dtmPtr, *newShape);
    }





/*----------------------------------------------------------------------------+
| CreateShapeFromClips
| Returns the shape that is the result
| of companding all clips set in the clips 
| The order of the content of the clips is important and is processed in the order
| they were set. 
| The given range is used for two purposes. First it indicates the limit of the 
| DTM and the clip shapes are clipped upon this extent. Secondly, it serves as
| the base shape in the event only mask type clips are defined. 
+----------------------------------------------------------------------------*/ 
HFCPtr<HVEShape> CreateShapeFromClips(const DRange3d&               spatialIndexRange,
                                      const IScalableMeshClipContainerPtr& clips)
    {   
    HFCPtr<HGF2DCoordSys>   coordSysPtr(new HGF2DCoordSys());    
    HFCPtr<HVEShape> indexRangeShape = new HVEShape(spatialIndexRange.low.x,
                                                    spatialIndexRange.low.y,
                                                    spatialIndexRange.high.x,
                                                    spatialIndexRange.high.y,
                                                    coordSysPtr);
    
    return CreateShapeFromClips(indexRangeShape, clips);        
    }    

/*----------------------------------------------------------------------------+
| CreateShapeFromClips
| Returns the shape that is the result
| of companding all clips set in the clips 
| The order of the content of the clips is important and is processed in the order
| they were set. 
| The given areaShape is used for two purposes. First it indicates the limit of the 
| DTM and the clip shapes are clipped upon this areaShape. Secondly, it serves as
| the base shape in the event only mask type clips are defined. 
+----------------------------------------------------------------------------*/ 
HFCPtr<HVEShape> CreateShapeFromClips(const HFCPtr<HVEShape>        areaShape,
                                      const IScalableMeshClipContainerPtr& clips)
    {
    HFCPtr<HVEShape> clipShapePtr;
    
    HFCPtr<HVEShape>        subShapePtr;

    HFCPtr<HVEShape>        clipBoundaryShapePtr(new HVEShape(areaShape->GetCoordSys()));
    HFCPtr<HVEShape>        maskShapePtr(new HVEShape(areaShape->GetCoordSys()));

    clipShapePtr = new HVEShape(areaShape->GetCoordSys());
    
    if (clips != 0)
        {
        for (size_t clipInd = 0; clipInd < clips->GetNbClips(); clipInd++)
            {            
            IScalableMeshClipInfoPtr clipInfoP;

            clips->GetClip(clipInfoP, clipInd);        
        
            HArrayAutoPtr<double> tempBuffer(new double[clipInfoP->GetNbClipPoints() * 2]);

            int bufferInd = 0; 
        
            for (size_t pointInd = 0; pointInd < clipInfoP->GetNbClipPoints(); pointInd++)
                {
                tempBuffer[bufferInd * 2]     = clipInfoP->GetClipPoints()[pointInd].x;
                tempBuffer[bufferInd * 2 + 1] = clipInfoP->GetClipPoints()[pointInd].y;    
                bufferInd++;                
                }                            

            // We validate the clip shape ... it must be a valid clip in order to be processed
            // In theory this will not occur, but jsut in case validations were not performed
            // prior to clip addition, this operation serves as a safegard.
            HVE2DPolySegment shapeContour(clipInfoP->GetNbClipPoints() * 2, tempBuffer, areaShape->GetCoordSys());
            if (!(shapeContour.AutoCrosses() && shapeContour.IsAutoContiguous()))
                {
                HVE2DPolygonOfSegments polygon(clipInfoP->GetNbClipPoints() * 2, tempBuffer, areaShape->GetCoordSys());

                subShapePtr = new HVEShape(polygon);

                if (clipInfoP->IsClipMask() == true)
                    {                  
                    maskShapePtr->Unify(*subShapePtr);
                    }
                else
                    {
                    clipBoundaryShapePtr->Unify(*subShapePtr);
                    }
                }        
            }   
        }

    if (clipBoundaryShapePtr->IsEmpty() == false)
        {

        clipBoundaryShapePtr->Intersect(*areaShape);
        clipShapePtr = clipBoundaryShapePtr;
        }

    if (maskShapePtr->IsEmpty() == false)
        {
        if (clipShapePtr->IsEmpty())
            {
            clipShapePtr = areaShape;
            }        

        clipShapePtr->Differentiate(*maskShapePtr);
        }

    return clipShapePtr;
    }    

