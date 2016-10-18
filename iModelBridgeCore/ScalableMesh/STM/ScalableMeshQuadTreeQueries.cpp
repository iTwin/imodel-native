#include <ScalableMeshPCH.h>

#include "ImagePPHeaders.h"
#include "ScalableMesh.h"

#include <Geom/DPoint3dOps.h>
#include <Geom/Polyface.h>
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshQuadTreeQueries.h"
#include "ScalableMeshQuadTreeQueries.hpp"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

StatusInt   AppendClippedToMesh::_ProcessUnclippedPolyface(PolyfaceQueryCR polyfaceQuery)
    {
    bvector<int> indices(polyfaceQuery.GetPointIndexCount());
    if (!indices.empty())
        {
        memcpy(indices.data(), polyfaceQuery.GetPointIndexCP(), indices.size()*sizeof(int));
        for (auto& i : indices) i += (int)m_targetMesh->GetNbPoints();
        m_targetMesh->AppendMesh(polyfaceQuery.GetPointCount(), const_cast<DPoint3d*>(polyfaceQuery.GetPointCP()), polyfaceQuery.GetPointIndexCount(), polyfaceQuery.GetPointIndexCP(), 0, 0, 0, 0, 0, 0);
        }
    return true;
    }

StatusInt   AppendClippedToMesh::_ProcessClippedPolyface(PolyfaceHeaderR polyfaceHeader)
    {
    bvector<DPoint3d> pts;
    bvector<int32_t> indices;
    pts.insert(pts.end(), polyfaceHeader.GetPointCP(), polyfaceHeader.GetPointCP() + polyfaceHeader.GetPointCount());
    PolyfaceVisitorPtr vis = PolyfaceVisitor::Attach(polyfaceHeader);
    bvector<int> &pointIndex = vis->ClientPointIndex();
    for (vis->Reset(); vis->AdvanceToNextFace();)
        {
        DPoint3d tri[3] = { pts[pointIndex[0]], pts[pointIndex[1]], pts[pointIndex[2]] };
        DPoint3d centroid;
        DVec3d normal;
        double area;
        PolygonOps::CentroidNormalAndArea(tri, 3, centroid, normal, area);
        if (m_clipVec->PointInside(centroid, 1e-8)) continue;
        indices.push_back(pointIndex[0] + 1 + (int)m_targetMesh->GetNbPoints());
        indices.push_back(pointIndex[1] + 1 + (int)m_targetMesh->GetNbPoints());
        indices.push_back(pointIndex[2] + 1 + (int)m_targetMesh->GetNbPoints());
        }
    if(indices.size() > 0) m_targetMesh->AppendMesh(pts.size(), &pts[0], indices.size(), &indices[0], 0, 0, 0, 0, 0, 0);
    return true;
    }

template class ScalableMeshQuadTreeViewDependentPointQuery<DPoint3d, DRange3d>;

template class ScalableMeshQuadTreeLevelPointIndexQuery<DPoint3d, DRange3d>;

template class ScalableMeshQuadTreeLevelMeshIndexQuery<DPoint3d, DRange3d>;

template class ScalableMeshQuadTreeLevelIntersectIndexQuery<DPoint3d, DRange3d>;

template class ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery<DPoint3d, DRange3d>;

template class ScalableMeshQuadTreeViewDependentMeshQuery<DPoint3d, DRange3d>;

template class ScalableMeshQuadTreeContextMeshQuery<DPoint3d, DRange3d>;







