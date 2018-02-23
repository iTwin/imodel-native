/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceClusterDecimation.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include  <Bentley/bmap.h>
#include  <Bentley/bset.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


struct IPointComparator
    {
    bool operator()(Point3dCR lhs, Point3dCR rhs) const
            {
            if (lhs.x < rhs.x)
                return true;
            else if (lhs.x > rhs.x)
                return false;

            if (lhs.y < rhs.y)
                return true;
            else if (lhs.y > rhs.y)
                return false;

            return lhs.z < rhs.z;
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr   PolyfaceQuery::ClusteredVertexDecimate (double tolerance)
    {
    bmap<Point3d, size_t, IPointComparator> pointMap; 
    DPoint3dCP                      points = GetPointCP();
    DVec3dCP                        normals = GetNormalCP();
    DPoint2dCP                      params = GetParamCP();
    size_t                          nextClusterIndex = 0;
    bool                            doNormals = (nullptr != normals), doParams = (nullptr != params);

    struct Cluster : RefCountedBase
        {
        bvector<int32_t>    m_pointIndices;
        bvector<int32_t>    m_normalIndices;
        bvector<int32_t>    m_paramIndices;
        size_t              m_outputPointIndex;
        size_t              m_outputParamIndex;
        DVec3d              m_outputNormal;

        void Add (int32_t pointIndex, int32_t normalIndex, int32_t paramIndex)
            {
            m_pointIndices.push_back(pointIndex);
            m_normalIndices.push_back(normalIndex);                                                          }
        };
    
    bmap<size_t, RefCountedPtr<Cluster>> clusters;
    bmap <size_t, Cluster*>     inputPointIndexToCluster;
       
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this); visitor->AdvanceToNextFace(); )
        {
        for (size_t i=0, count = visitor->NumEdgesThisFace(); i<count; i++)
            {
            if ((doNormals && nullptr == visitor->GetClientNormalIndexCP()) || (doParams && nullptr == visitor->GetClientParamIndexCP()))
                continue;   // degenerate facet...

            int32_t     inputPointIndex  = visitor->GetClientPointIndexCP()[i];
            int32_t     inputNormalIndex = doNormals ? visitor->GetClientNormalIndexCP()[i] : -1;
            int32_t     inputParamIndex  = doParams  ? visitor->GetClientParamIndexCP()[i] : -1; 
            size_t      clusterIndex;

            auto        foundCluster = inputPointIndexToCluster.find(inputPointIndex);
            if (foundCluster == inputPointIndexToCluster.end())
                {
                DPoint3d    dPoint = points[inputPointIndex];
                Point3d     iPoint;

                iPoint.x = (int32_t) (dPoint.x / tolerance);
                iPoint.y = (int32_t) (dPoint.y / tolerance);
                iPoint.z = (int32_t) (dPoint.z / tolerance);

                auto        pointMapInsert = pointMap.Insert(iPoint, nextClusterIndex);

                if (pointMapInsert.second)
                    clusterIndex = nextClusterIndex++;
                else
                    clusterIndex = pointMapInsert.first->second;
                
                auto        clusterInsert = clusters.Insert(clusterIndex, new Cluster());

                clusterInsert.first->second->Add(inputPointIndex, inputNormalIndex, inputParamIndex);
                inputPointIndexToCluster.Insert(inputPointIndex, clusterInsert.first->second.get());
                }
            else
                {
                foundCluster->second->Add(inputPointIndex, inputNormalIndex, inputParamIndex);
                }
            }
        }


    if (clusters.size() == GetPointCount())
        return nullptr;     // No Clusters found.

    PolyfaceHeaderPtr                   decimatedPolyface = PolyfaceHeader::CreateVariableSizeIndexed();
    LightweightPolyfaceBuilderPtr       coordinateMap = LightweightPolyfaceBuilder::Create(*decimatedPolyface);

    // Build clustered points.
    for (auto& pair : clusters)
        {
        auto const& cluster = pair.second;
        DPoint3d    point  = DPoint3d::FromZero();
        DPoint2d    param  = DPoint2d::FromZero();
        double      scale = 1.0 / (double) cluster->m_pointIndices.size();

        for (auto& pointIndex : cluster->m_pointIndices)
            point.Add(points[pointIndex]);
        
        point.Scale(scale);
        cluster->m_outputPointIndex = coordinateMap->FindOrAddPoint(point);

        if (doNormals)
            {
            cluster->m_outputNormal = DVec3d::FromZero();

            for (auto& normalIndex : cluster->m_normalIndices)
                cluster->m_outputNormal.Add(normals[normalIndex]);
        
            cluster->m_outputNormal.Normalize();
            }
        if (doParams)
            {
            for (auto& paramIndex : cluster->m_paramIndices)
                param.Add(params[paramIndex]);
        
            param.Scale(scale);
            cluster->m_outputParamIndex = coordinateMap->FindOrAddParam(param);
            }
        }

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this); visitor->AdvanceToNextFace(); )
        {
        bset<Cluster*>          unique;
        bvector<Cluster*>       faceClusters;
        bvector<DPoint3d>       faceVertices;

        for (size_t i=0, count = visitor->NumEdgesThisFace(); i<count; i++)
            {
            int32_t         inputPointIndex = visitor->GetClientPointIndexCP()[i];
            auto foundCluster = inputPointIndexToCluster.find(inputPointIndex);
            if (inputPointIndexToCluster.end() == foundCluster)
                continue; // degenerate facet, ignored above...

            Cluster*        cluster = foundCluster->second;
            auto            insert = unique.insert(cluster);

            if (insert.second)
                {
                faceClusters.push_back(cluster);
                faceVertices.push_back(decimatedPolyface->GetPointCP()[cluster->m_outputPointIndex]);
                }
            }
        if (faceClusters.size() > 2)                                                                                       
            {
            for (auto& cluster : faceClusters)
                {
                coordinateMap->AddPointIndex(cluster->m_outputPointIndex, true /* Visiblity,,, TBD if necessary */ );
                if (doParams)
                    coordinateMap->AddParamIndex(cluster->m_outputParamIndex);

                if (doNormals)
                    {
                    DVec3d                  faceNormal;
                    constexpr double        s_minClusterNormalDot = .7;

                    bsiPolygon_polygonNormalAndArea(&faceNormal, nullptr, faceVertices.data(), (int) faceVertices.size());

                    DVec3dCR                normal = cluster->m_outputNormal.DotProduct (faceNormal) > s_minClusterNormalDot ? cluster->m_outputNormal : faceNormal;
                    coordinateMap->AddNormalIndex(coordinateMap->FindOrAddNormal(normal));
                    }
                }
            coordinateMap->AddPointIndexTerminator();
            if (doNormals)
                coordinateMap->AddNormalIndexTerminator();
            if (doParams)
                coordinateMap->AddParamIndexTerminator();
            }
        }
    return decimatedPolyface;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
