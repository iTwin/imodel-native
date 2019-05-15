/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
#include <Geom/cluster.h>
#include "pf_halfEdgeArray.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void PolyfaceHeader::BuildNormalsFast (double creaseTolerance, double sizeTolerance)
    {
    if (0 != GetNormalCount())
        return;    

    // So we can skip expensive visitors.
    ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();


    bvector<DPoint3d>               facePoints;
    DPoint3dCP                      points = GetPointCP();
    LightweightPolyfaceBuilderPtr   builder = LightweightPolyfaceBuilder::Create(*this);    
    constexpr   double              s_minArea = 1.0E-14;
    static      DVec3d              s_defaultNormal = DVec3d::From(0.0, 0.0, 1.0);


    struct      Facet : RefCountedBase
        {

        DVec3d      m_normal;
        double      m_area;

        Facet () { }
        void Init(bvector<DPoint3d> const& points)
            {
            if ((m_area = bsiPolygon_polygonNormalAndArea(&m_normal, nullptr, points.data(), (int) points.size())) < s_minArea)
               m_normal = s_defaultNormal;
            }
        };

    typedef RefCountedPtr<Facet> FacetPtr;

    struct VertexFacets
        {
        bvector<FacetPtr>   m_facets;

        DVec3d ComputeSharedNormal(Facet const* thisFacet, double minDot)
            {
            bool    shared = false;
            DVec3d  normal = thisFacet->m_normal;

            for (auto&  facet : m_facets)
                {
                if (facet.get() != thisFacet &&
                    facet->m_normal.DotProduct(thisFacet->m_normal) > minDot)
                    {
                    shared = true;
                    normal.Add(facet->m_normal);
                    }
                }
            if (shared)
                normal.Normalize();

            return normal;
            }
        };


    bmap<int32_t, VertexFacets> pointIndexToFacets;
    bmap<size_t, FacetPtr>      indexIndexToFacet;
    FacetPtr                    facet = new Facet();


    for (size_t i=0, indexCount = GetPointIndexCount(); i < indexCount; i++)
        {
        int32_t const pointIndex = abs(GetPointIndexCP()[i]);

        if (0 == pointIndex && !facePoints.empty())
            {
            
            facet->Init(facePoints);
            facet = new Facet();

            facePoints.clear();
            }
        else
            {
            int32_t             zeroBasedIndex = pointIndex - 1;
            VertexFacets        facets;

            facePoints.push_back(points[zeroBasedIndex]);
            pointIndexToFacets.Insert(pointIndex, facets).first->second.m_facets.push_back(facet);
            indexIndexToFacet.Insert(i, facet);
            }
        }

    double  minDot = cos(creaseTolerance);
    double  minArea = sizeTolerance * sizeTolerance;
                                            

    for (size_t i=0, indexCount = GetPointIndexCount(); i < indexCount; i++)
        {
        int32_t const   pointIndex = abs(GetPointIndexCP()[i]);
        
        if (0 == pointIndex)
            {
            builder->AddNormalIndexTerminator();
            }
        else
            {
            auto        thisFacet = indexIndexToFacet.find(i)->second.get();
            DVec3d      normal = (thisFacet->m_area < minArea) ? thisFacet->m_normal : pointIndexToFacets.find(pointIndex)->second.ComputeSharedNormal(thisFacet, minDot);

            builder->AddNormalIndex(builder->FindOrAddNormal(normal));
            }
        }

    Normal().SetActive(true);
    NormalIndex().SetActive(true);
    };

