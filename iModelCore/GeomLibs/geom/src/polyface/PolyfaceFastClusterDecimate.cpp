/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceFastClusterDecimate.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include  <Bentley/bset.h>
#include  <Bentley/bmap.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr   PolyfaceQuery::FastClusteredDecimate (double tolerance)
    {
    PolyfaceZYXMap                  pointMap (DPoint3dZYXTolerancedSortComparison (tolerance, 0.0));
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

        Cluster() { };
        Cluster(int32_t pointIndex, int32_t normalIndex, int32_t paramIndex) : m_pointIndices(1, pointIndex), m_normalIndices(normalIndex, 1), m_paramIndices(paramIndex, 1) { }
                                                                                                 
        void Add (int32_t pointIndex, int32_t normalIndex, int32_t paramIndex)
            {
            m_pointIndices.push_back(pointIndex);
            m_normalIndices.push_back(normalIndex);
            m_paramIndices.push_back(paramIndex);
            }
        };
    
    bmap<size_t, RefCountedPtr<Cluster>> clusters;
       
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this); visitor->AdvanceToNextFace(); )
        {
        for (size_t i=0, count = visitor->NumEdgesThisFace(); i<count; i++)
            {
            int32_t     inputPointIndex  = visitor->GetClientPointIndexCP()[i];
            int32_t     inputNormalIndex = doNormals ? visitor->GetClientNormalIndexCP()[i] : -1;
            int32_t     inputParamIndex  = doParams  ? visitor->GetClientParamIndexCP()[i] : -1; 
            size_t      clusterIndex;
            auto        pointMapInsert = pointMap.Insert(points[inputPointIndex], nextClusterIndex);

            if (pointMapInsert.second)
                clusterIndex = nextClusterIndex++;
            else
                clusterIndex = pointMapInsert.first->second;

            auto        clusterInsert = clusters.Insert(clusterIndex, new Cluster());

            clusterInsert.first->second->Add(inputPointIndex, inputNormalIndex, inputParamIndex);
            }
        }


    if (clusters.size() == GetPointCount())
        return nullptr;     // No Clusters found.

    IFacetOptionsPtr            facetOptions = IFacetOptions::Create();
    IPolyfaceConstructionPtr    builder = IPolyfaceConstruction::Create(*facetOptions, 1.0E-12);

    // Build clustered points.
    bmap <size_t, size_t>           inputToClusterPoint, inputToClusterNormal, inputToClusterParam;

    for (auto& pair : clusters)
        {
        auto const& cluster = pair.second;
        DPoint3d    point  = DPoint3d::FromZero();
        DVec3d      normal = DVec3d::From(0.0, 0.0, 0.0);
        DPoint2d    param  = DPoint2d::FromZero();
        double      scale = 1.0 / (double) cluster->m_pointIndices.size();

        for (auto& pointIndex : cluster->m_pointIndices)
            point.Add(points[pointIndex]);
        
        point.Scale(scale);
        size_t  clusterPointIndex = builder->FindOrAddPoint(point);

        for (auto& pointIndex : cluster->m_pointIndices)
            inputToClusterPoint[pointIndex] = clusterPointIndex;

        if (doNormals)
            {
            for (auto& normalIndex : cluster->m_normalIndices)
                normal.Add(normals[normalIndex]);
        
            normal.Scale(scale);
            size_t  clusterNormalIndex = builder->FindOrAddNormal(normal);

            for (auto& normalIndex : cluster->m_normalIndices)
                inputToClusterNormal[normalIndex] = clusterNormalIndex;
            }
        if (doParams)
            {
            for (auto& paramIndex : cluster->m_paramIndices)
                param.Add(params[paramIndex]);
        
            param.Scale(scale);
            size_t  clusterParamIndex = builder->FindOrAddParam(param);

            for (auto& paramIndex : cluster->m_paramIndices)
                inputToClusterParam[paramIndex] = clusterParamIndex;
            }
        }

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this); visitor->AdvanceToNextFace(); )
        {
        bset<size_t>        unique;
        bvector<int32_t>    undecimatedInputIndices;
        for (size_t i=0, count = visitor->NumEdgesThisFace(); i<count; i++)
            {
            int32_t         inputPointIndex = visitor->GetClientPointIndexCP()[i];
            size_t          clusterIndex = inputToClusterPoint[inputPointIndex];
            auto            insert = unique.insert(clusterIndex);

            if (insert.second)
                undecimatedInputIndices.push_back(inputPointIndex);
            }
        if (undecimatedInputIndices.size() > 2)                                                                                       
            {
            for (auto& inputIndex : undecimatedInputIndices)
                {
                builder->AddPointIndex(inputToClusterPoint[inputIndex], true /* TBD... Visibilty?? */);
                if (nullptr != normals)
                    builder->AddNormalIndex(inputToClusterNormal[inputIndex]);
                if (nullptr != params)
                    builder->AddParamIndex(inputToClusterParam[inputIndex]);
                }
            builder->AddPointIndexTerminator();
            if (nullptr != normals)
                builder->AddNormalIndexTerminator();
            if (nullptr != params)
                builder->AddParamIndexTerminator();
            }
        }
    return builder->GetClientMeshPtr();
    }


END_BENTLEY_GEOMETRY_NAMESPACE
