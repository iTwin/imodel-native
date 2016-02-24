/*#pragma  once
#include <Bentley/Bentley.h>
#include <DgnPlatform/DgnCoreAPI.h>

using namespace std;
USING_NAMESPACE_BENTLEY_DGNPLATFORM


#include <TerrainModel/TerrainModel.h>
//#include <DgnPlatform/ElementHandle.h>
#include <TerrainModel\ElementHandler\DTMElementHandlerManager.h>
#include <TerrainModel\ElementHandler\IMrDTMHostAdmin.h>
#include <TerrainModel\ElementHandler\TerrainModelElementHandler.h>
USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT
#include <TerrainModel/ElementHandler/DTMDataRef.h>
//#include <DgnPlatform/ElementAgenda.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL

StatusInt DoBatchDrape(ElementAgenda* vectorAgenda, DTMPtr& dtmPtr, bvector<bvector<DPoint3d>>& drapeLines);
StatusInt DrapeOnScalableMesh(DTMPtr& smPtr, std::vector<std::vector<DPoint3d>>& drapedPoints, std::vector<DPoint3d> origPoints);

struct STMDrapeHelper
    {
    static void GetExtentFromPoints(DPoint3dCP linePoints, const int nbPoints, DRange2d& extent);
    static StatusInt DrapeOnScalableMesh(DTMPtr& smPtr, std::vector<std::vector<DPoint3d>>& drapedPoints, std::vector<DPoint3d> origPoints);
    static void DrapeSegmentOnDTM(DTMPtr dtmPtr, DPoint3d* linePtsP, std::vector<DPoint3d>& drapedPoints, Transform meterToUor);
    static StatusInt DrapeLineString(std::vector<DPoint3d>& origPoints, RefCountedPtr<DTMDataRef>& DTMDataRef, std::vector<DPoint3d>& pointDrape);
    struct DPoint3dComparer
        {
        private:
            DPoint3d m_start;
            DPoint3d m_end;
        public:
            DPoint3dComparer()
                {
                }
            DPoint3dComparer(DPoint3d& start, DPoint3d& end)
                {
                SetStart(start); SetEnd(end);
                }
            void SetStart(DPoint3d& start)
                {
                m_start = start;
                }
            void SetEnd(DPoint3d& end)
                {
                m_end = end;
                }
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

    };

//bool DPoint3dEqualityTest(const DPoint3d& point1, const DPoint3d& point2);*/