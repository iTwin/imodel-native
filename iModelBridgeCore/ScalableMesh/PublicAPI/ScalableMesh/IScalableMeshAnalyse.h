/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshAnalyse.h $
|     $Date: 2016/08/23 10:24:32 $
|     $Author:Stephane.Nullans $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/ScalableMeshDefs.h>
#include <Geom\Polyface.h>
#include <Bentley\RefCounted.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/ClipVector.h>
#include <ScalableMesh/IScalableMeshQuery.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
/*__PUBLISH_SECTION_END__*/
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SMVolumeSegment {
    std::vector<double> VolumeRanges; // vector of abcissa couples (min-max) 
    double volume; // the integrated volume (sum of volume ranges)
    };

class ISMGridVolume
    {
    public:
        BENTLEY_SM_EXPORT ISMGridVolume() { 
            m_direction = DVec3d::From(0, 0, 1); // fixed to Z for now
            m_resolution = 0.1; // 10 cm
            m_gridSizeLimit = 1000;
            m_totalVolume = m_cutVolume = m_fillVolume = 0;
            m_VolSegments = NULL;
            m_bInitialised = false;
            };
        
        BENTLEY_SM_EXPORT virtual ~ISMGridVolume() {
            delete[] m_VolSegments;
            m_VolSegments = NULL;
            };

        virtual bool InitFrom(double _resolution, const DRange3d& _rangeMesh, const DRange3d& _rangeRegion);

        BENTLEY_SM_EXPORT bool GetGridSize(int &_xSize, int &_ySize)
            {
            if (!m_bInitialised)
                return false;
            _xSize = m_xSize;
            _ySize = m_ySize;
            return true;
            }

        DVec3d m_direction;     // the projection direction
        double m_resolution;    // the grid resolution wanted
        DPoint3d m_center;      // the 3SM center
        int m_gridSizeLimit;    // used to clamp grid size

        // output data ------------------------------------------------
        double m_totalVolume;
        double m_cutVolume;
        double m_fillVolume;
        SMVolumeSegment* m_VolSegments; // table of segments ; one per grid element
        DRange3d m_range;               // range of the 3D grid volume

    protected:
        int m_xSize;        // Size of the Grid in X
        int m_ySize;        // Size of the Grid in Y

        double m_xStep;     // resolution on X
        double m_yStep;     // resolution on Y
        bool m_bInitialised;
    };

class IScalableMeshAnalyse abstract : public RefCountedBase
    {
    protected:
        virtual DTMStatusInt _ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid) = 0;
        virtual DTMStatusInt _ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, IScalableMeshNodePtr anotherMesh, double resolution, ISMGridVolume& grid) = 0;

    public:
        // Compute Volume between the 3SM and a given polygon, returns different values
        BENTLEY_SM_EXPORT DTMStatusInt ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, double resolution, ISMGridVolume& grid) {
            return _ComputeDiscreteVolume(polygon, resolution, grid); }
        BENTLEY_SM_EXPORT DTMStatusInt ComputeDiscreteVolume(const bvector<DPoint3d>& polygon, IScalableMeshNodePtr anotherMesh, double resolution, ISMGridVolume& grid) {
            return _ComputeDiscreteVolume(polygon, anotherMesh, resolution, grid);
            }
    };

typedef RefCountedPtr<IScalableMeshAnalyse>                          IScalableMeshAnalysePtr;

END_BENTLEY_SCALABLEMESH_NAMESPACE
