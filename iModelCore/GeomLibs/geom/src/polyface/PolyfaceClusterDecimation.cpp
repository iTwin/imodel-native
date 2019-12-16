/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include  <Bentley/bmap.h>

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
PolyfaceHeaderPtr   PolyfaceQuery::ClusteredVertexDecimate (double tolerance, double minCompressionRatio)
    {
    bmap<Point3d, size_t, IPointComparator> pointMap; 
    DPoint3dCP                      points = GetPointCP();
    DVec3dCP                        normals = GetNormalCP();
    DPoint2dCP                      params = GetParamCP();
    PolyfaceAuxDataCPtr             auxData = GetAuxDataCP();
    bool                            doNormals = (nullptr != normals), doParams = (nullptr != params), doAux = auxData.IsValid();

    struct Cluster : RefCountedBase
        {
        bvector<int32_t>    m_pointIndices;
        bvector<int32_t>    m_normalIndices;
        bvector<int32_t>    m_paramIndices;
        bvector<int32_t>    m_auxIndices;
        size_t              m_outputPointIndex;
        size_t              m_outputParamIndex;
        size_t              m_outputNormalIndex;
        size_t              m_outputAuxIndex;
        DVec3d              m_outputNormal;
 
        void Add (int32_t pointIndex, int32_t normalIndex, int32_t paramIndex, int32_t auxIndex)
            {
            m_pointIndices.push_back(pointIndex);
            if (normalIndex >= 0)
                m_normalIndices.push_back(normalIndex);                                                          

            if (paramIndex >= 0)
                m_paramIndices.push_back(paramIndex);

            if (auxIndex >= 0)
                m_auxIndices.push_back(auxIndex);
            }
        };

    bvector<RefCountedPtr<Cluster>> clusters;
    bvector<Cluster*>               inputPointIndexToCluster(GetPointCount(), nullptr);

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this); visitor->AdvanceToNextFace(); )
        {
        if ((doNormals && nullptr == visitor->GetClientNormalIndexCP()) || (doParams && nullptr == visitor->GetClientParamIndexCP()))
            continue;   // degenerate facet...

        for (size_t i=0, count = visitor->NumEdgesThisFace(); i<count; i++)
            {
            int32_t     inputPointIndex  = visitor->GetClientPointIndexCP()[i];
            int32_t     inputNormalIndex = doNormals ? visitor->GetClientNormalIndexCP()[i] : -1;
            int32_t     inputParamIndex  = doParams  ? visitor->GetClientParamIndexCP()[i] : -1; 
            int32_t     inputAuxIndex    = doAux     ? visitor->GetClientAuxIndexCP()[i] : -1;

            auto foundCluster = inputPointIndexToCluster[inputPointIndex];
            if (foundCluster)
                foundCluster->Add(inputPointIndex, inputNormalIndex, inputParamIndex, inputAuxIndex);
            else
                {
                DPoint3d    dPoint = points[inputPointIndex];
                Point3d     iPoint;

                iPoint.x = (int32_t) (dPoint.x / tolerance);
                iPoint.y = (int32_t) (dPoint.y / tolerance);
                iPoint.z = (int32_t) (dPoint.z / tolerance);

                Cluster*    thisCluster = nullptr;
                auto        pointMapInsert = pointMap.Insert(iPoint, clusters.size()); // try to insert new cluster for this point
                if (pointMapInsert.second) // no cluster existed
                    {
                    clusters.push_back(new Cluster()); // add to global list to match index in pointMap
                    thisCluster = clusters.back().get();
                    }
                else
                    thisCluster = clusters[pointMapInsert.first->second].get(); // pointMap already has Cluster

                thisCluster->Add(inputPointIndex, inputNormalIndex, inputParamIndex, inputAuxIndex);
                inputPointIndexToCluster[inputPointIndex] = thisCluster;
                }
            }
        }

    if (clusters.size() > GetPointCount() * minCompressionRatio)
        return nullptr;     // No Clusters found.

    PolyfaceHeaderPtr decimatedPolyface = PolyfaceHeader::CreateVariableSizeIndexed();
    decimatedPolyface->Point().SetActive(true);
    decimatedPolyface->PointIndex().SetActive(true);

    if (doNormals)
        {
        decimatedPolyface->Normal().SetActive(true);
        decimatedPolyface->NormalIndex().SetActive(true);
        }

    if (doParams)
        {
        decimatedPolyface->Param().SetActive(true);
        decimatedPolyface->ParamIndex().SetActive(true);
        }

    LightweightPolyfaceBuilderPtr       coordinateMap = LightweightPolyfaceBuilder::Create(*decimatedPolyface);
    PolyfaceAuxData::ChannelsCP         inputAuxChannels = doAux ? &auxData->GetChannels() : nullptr;
    PolyfaceAuxData::Channels           outputAuxChannels;

    if (doAux)
        outputAuxChannels.Init( GetAuxDataCP()->GetChannels());

    // Build clustered points.
    for (auto& cluster : clusters)
        {
        DPoint3d    point  = DPoint3d::FromZero();
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
            DPoint2d    param  = DPoint2d::FromZero();

            for (auto& paramIndex : cluster->m_paramIndices)
                param.Add(params[paramIndex]);
        
            param.Scale(scale);
            cluster->m_outputParamIndex = coordinateMap->FindOrAddParam(param);
            }
        if (doAux)
            {
            cluster->m_outputAuxIndex = outputAuxChannels.GetValueCount();
            for (size_t iChannel = 0; iChannel < inputAuxChannels->size(); iChannel++)
                {
                PolyfaceAuxChannelCR inputChannel  = *inputAuxChannels->at(iChannel);
                PolyfaceAuxChannelR  outputChannel = *outputAuxChannels.at(iChannel);

                for (size_t iData = 0; iData < inputChannel.GetData().size(); iData++)
                    {
                    for (size_t k = 0, blockSize = inputChannel.GetBlockSize(); k<blockSize; k++)
                        {
                        double           value = 0.0;
                        double const*    inValues = inputChannel.GetData().at(iData)->GetValues().data();

                        for (auto& auxIndex : cluster->m_auxIndices)
                            value += inValues[auxIndex * blockSize + k];
                        
                        value *= scale;
                        outputChannel.GetData().at(iData)->AddValue(value);
                        }
                    }
                }
            }
        }

    bvector<int32_t>            outputAuxIndices;

    bvector<Cluster*>           faceClusters;
    bvector<bool>               faceClusterVisibility;
    bvector<DPoint3d>           faceVertices;
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this); visitor->AdvanceToNextFace(); )
        {
        faceClusters.clear();
        faceClusterVisibility.clear();
        faceVertices.clear();
        for (size_t i=0, count = visitor->NumEdgesThisFace(); i<count; i++)
            {
            int32_t         inputPointIndex = visitor->GetClientPointIndexCP()[i];
            Cluster*        cluster = inputPointIndexToCluster[inputPointIndex];
            if (cluster == nullptr)
                continue; // degenerate facet, ignored above...

            bool isVisible = visitor->GetVisibleCP()[i];
            // Ensure clustered points aren't repeated.
            auto faceClusterIter = std::find(faceClusters.begin(), faceClusters.end(), cluster);
            if (faceClusterIter == faceClusters.end())
                {
                faceClusters.push_back(cluster);
                faceClusterVisibility.push_back(isVisible);
                faceVertices.push_back(decimatedPolyface->GetPointCP()[cluster->m_outputPointIndex]);
                }
            else
                {
                // If this was originally multiple points, turn on visibility if on for any of the points.
                // Note that this visibility glombing is just inside the current face; should not introduce artifacts.
                size_t faceClusterIndex = faceClusterIter - faceClusters.begin();
                if (isVisible && !faceClusterVisibility[faceClusterIndex])
                    faceClusterVisibility[faceClusterIndex] = true;
                }
            }
        if (faceClusters.size() > 2)
            {
            for (size_t faceClusterIndex = 0; faceClusterIndex < faceClusters.size(); ++faceClusterIndex)
                {
                auto& cluster = faceClusters[faceClusterIndex];
                coordinateMap->AddPointIndex(cluster->m_outputPointIndex, faceClusterVisibility[faceClusterIndex]);

                if (doNormals)
                    {
                    DVec3d                  faceNormal;
                    constexpr double        s_minClusterNormalDot = .7;

                    bsiPolygon_polygonNormalAndArea(&faceNormal, nullptr, faceVertices.data(), (int) faceVertices.size());

                    DVec3dCR                normal = cluster->m_outputNormal.DotProduct (faceNormal) > s_minClusterNormalDot ? cluster->m_outputNormal : faceNormal;
                    coordinateMap->AddNormalIndex(coordinateMap->FindOrAddNormal(normal));
                    }
                if (doParams)
                    coordinateMap->AddParamIndex(cluster->m_outputParamIndex);

                if (doAux)
                    outputAuxIndices.push_back((int32_t) cluster->m_outputAuxIndex + 1);
                }
            coordinateMap->AddPointIndexTerminator();
            if (doNormals)
                coordinateMap->AddNormalIndexTerminator();
            if (doParams)
                coordinateMap->AddParamIndexTerminator();
            if (doAux)
                outputAuxIndices.push_back(0);
            }
        }

    if (doAux)
        {
        PolyfaceAuxDataPtr  decimatedAuxData = new PolyfaceAuxData(std::move(outputAuxIndices), std::move(outputAuxChannels));
        decimatedPolyface->SetAuxData(decimatedAuxData);
        }

    return decimatedPolyface;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
