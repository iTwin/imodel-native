/*--------------------------------------------------------------------------------------+
|    $RCSfile: TransformationUtils.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/07 14:26:44 $
|     $Author: Raymond.Gauthier $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#ifdef VANCOUVER_API
#include <DgnGeoCoord/DgnGeoCoord.h>
#else
#include <DgnPlatform/DgnGeoCoord.h>
#endif

using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

inline void ReprojectPt(DPoint3d& ptOut, const DPoint3d& ptIn, BaseGCSPtr& sourceGcs, BaseGCSPtr& destinationGcs, GeoCoordInterpretation geoInterSrc, GeoCoordInterpretation geoInterDst)
    {
    if (destinationGcs.IsValid())
        {
        assert(sourceGcs.IsValid());
        GeoPoint srcLatLong;
        GeoPoint dstLatLong;

        if (geoInterSrc == GeoCoordInterpretation::Cartesian)
            {
            sourceGcs->LatLongFromCartesian(srcLatLong, ptIn);
            }
        else
            {
            sourceGcs->LatLongFromXYZ(srcLatLong, ptIn);
            }

        sourceGcs->LatLongFromLatLong(dstLatLong, srcLatLong, *destinationGcs);

        if (geoInterDst == GeoCoordInterpretation::Cartesian)
            {
            destinationGcs->CartesianFromLatLong(ptOut, dstLatLong);
            }
        else
            {
            destinationGcs->XYZFromLatLong(ptOut, dstLatLong);
            }
        }
    else
        {
        ptOut = ptIn;
        }
    }

inline void Reproject(bvector<DPoint3d>& ptsOut, const bvector<DPoint3d>& ptsIn, BaseGCSPtr& sourceGcs, BaseGCSPtr& destinationGcs, GeoCoordInterpretation geoInterSrc, GeoCoordInterpretation geoInterDst)
    {
    ptsOut.resize(ptsIn.size());

    for (size_t ptInd = 0; ptInd < ptsIn.size(); ptInd++)
        {
        ReprojectPt(ptsOut[ptInd], ptsIn[ptInd], sourceGcs, destinationGcs, geoInterSrc, geoInterDst);
        }
    }

inline void ReprojectRange(DRange3d& rangeOut, const DRange3d& rangeIn, BaseGCSPtr& sourceGcs, BaseGCSPtr& destinationGcs, GeoCoordInterpretation geoInterSrc, GeoCoordInterpretation geoInterDst)
    {
    bvector<DPoint3d> corners(8);
    bvector<DPoint3d> reprojCorners(8);

    rangeIn.Get8Corners(&corners[0]);
    Reproject(reprojCorners, corners, sourceGcs, destinationGcs, geoInterSrc, geoInterDst);

    rangeOut = DRange3d::From(&reprojCorners[0], (int)reprojCorners.size());
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
