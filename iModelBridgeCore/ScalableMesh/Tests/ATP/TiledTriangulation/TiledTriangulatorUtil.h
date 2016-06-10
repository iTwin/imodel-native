/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ATP/TiledTriangulation/TiledTriangulatorUtil.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "MrDTMFace.h"
#include <wtypes.h>
#include <ImagePP/h/ImageppAPI.h>
/*#include <Bentley/BeCriticalSection.h>
#include <ImagePP/h/HTypes.h>
#include <ImagePP/h/ExportMacros.h>
#include <ImagePP/h/HmrMacro.h>
#include <ImagePP/h/ImagePPClassId.h>
#include <ImagePP/all/h/HPMClassKey.h>
#include <ImagePP/all/h/HPMPersistentObject.h>

#include <ImagePP/all/h/HFCPtr.h>*/
//#include <ImagePP/all/h/HVEDTMLinearFeature.h>
//#include <ImagePP/all/h/HPMPooledVector.h>
#include <ImagePP/all/h/HGF2DCoordSys.h>
#include <ImagePP/all/h/HGF2DLocation.h>
#include <ScalableMesh/IScalableMesh.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMClass.h>

//BEGIN_GEODTMAPP_NAMESPACE
//void dumpDTMInTinFile(Bentley::TerrainModel::BcDTM* dtmP, wstring& fileName, const __int64* indP);

/*StatusInt addLinearsIn (DTMPtr& dtmPtr,
                        list<HFCPtr<HVEDTMLinearFeature>>& linearList,
                        unsigned int                       maxNumberOfPoints);*/
USING_NAMESPACE_IMAGEPP

bool isClosedFeature(DTMFeatureType featureType);

int addPolygonAsFeatureInDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr,
                              const BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshQueryParametersPtr& mrDTMQueryParamsPtr,
                              vector<DPoint3d>& polygon,
                              const int& featureType,
                              const bool triangulateAfter = true);

vector<DPoint2d> convertDRange2dToVectorDPoint2d (const DRange2d& inputRange);

vector<DPoint3d> convertDRange2dToVectorDPoint3d (const DRange2d& inputRange);

int insertTrianglesAsFeatureInDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr,
                                   HPMMemoryManagedVector<DPoint3d>& triangleList,
                                   //vector<DPoint3d>& triangles,
                                   const int featureType);

int createDTMFromTriangles (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr,
                            const BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshQueryParametersPtr& mrDTMQueryParamsPtr,
                            HPMMemoryManagedVector<DPoint3d>& triangleList,
                            //vector<DPoint3d>& triangles,
                            const int featureType,
                            const bool triangulate = true);

void getAllMeshFacesAndComputeBoundingCircles (BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMMeshPtr mesh, vector<FaceWithProperties>& VectorFaces);

void getRangeFromPoints(DPoint3dCP linePoints, const int nbPoints, DRange2d& range);

bool getLineIntersection(double p0_x, double p0_y, double p1_x, double p1_y,
                         double p2_x, double p2_y, double p3_x, double p3_y);

bool getLineIntersection(double p0_x, double p0_y, double p1_x, double p1_y,
                         double p2_x, double p2_y, double p3_x, double p3_y
                         , double *i_x, double *i_y);

bool isLineIntersectingRange(DPoint3d* linePtsP, DRange2d& range);

bool isLineIntersectingCircle(const double& x1, const double& y1,
                              const double& x2, const double& y2,
                              const double& r,
                              const HFCPtr<HGF2DCoordSys>& coordSys);

bool isEqualPoint2d(const DPoint2d& point1, const DPoint2d& point2);

bool isEqualPoint3d(const DPoint3d& point1, const DPoint3d& point2);

template <class PointType>
struct IsEqualPoint : public std::unary_function <PointType, bool>
    {
    PointType m_Point;
    IsEqualPoint (PointType& point) { m_Point = point; }
    };

template<>
struct IsEqualPoint<DPoint2d>
    {
    DPoint2d m_Point;
    IsEqualPoint (DPoint2d& point) { m_Point = point; }
    bool operator () (const DPoint2d& point) const
        {
        return isEqualPoint2d(point, m_Point);
        }
    };

template<>
struct IsEqualPoint<DPoint3d>
    {
    DPoint3d m_Point;
    IsEqualPoint (DPoint3d& point) { m_Point = point; }
    bool operator () (const DPoint3d& point) const
        {
        return isEqualPoint3d(point, m_Point);
        }
    };

template<>
struct IsEqualPoint<HGF2DLocation>
    {
    HGF2DLocation m_Point;
    IsEqualPoint (HGF2DLocation& point) { m_Point = point; }
    bool operator () (const HGF2DLocation& point) const
        {
        return point.IsEqualTo(m_Point);
        }
    };

struct DPoint3dOnSegmentComparer
    {
    private:
        DPoint3d m_start;
        DPoint3d m_end;
    public:
        DPoint3dOnSegmentComparer () {}
        DPoint3dOnSegmentComparer (const DPoint3d& start, const DPoint3d& end) { SetStart(start); SetEnd(end); }
        void SetStart(const DPoint3d& start) { m_start = start; }
        void SetEnd  (const DPoint3d& end)   { m_end   = end;   }
        bool operator() (DPoint3d& X1, DPoint3d& X2)
            {
            DPoint3d direction;
            direction.x = m_end.x - m_start.x;
            direction.y = m_end.y - m_start.y;
            direction.z = m_end.z - m_start.z;
            // First point
            DPoint3d X1direction;
            X1direction.x = X1.x - m_start.x;
            X1direction.y = X1.y - m_start.y;
            X1direction.z = X1.z - m_start.z;
            const double X1RelativePosition = X1direction.x*direction.x + X1direction.y*direction.y + X1direction.y*direction.y;
            // Second point
            DPoint3d X2direction;
            X2direction.x = X2.x - m_start.x;
            X2direction.y = X2.y - m_start.y;
            X2direction.z = X2.z - m_start.z;
            const double X2RelativePosition = X2direction.x*direction.x + X2direction.y*direction.y + X2direction.y*direction.y;
            return X1RelativePosition < X2RelativePosition;
            }
    };

//void printTileContourPolygonToMS(vector<DPoint3d> contour_polyline);

//END_GEODTMAPP_NAMESPACE