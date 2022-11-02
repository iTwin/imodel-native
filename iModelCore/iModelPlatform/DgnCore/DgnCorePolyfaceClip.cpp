/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

enum    ClipStatus
    {
    ClipStatus_ClipRequired,
    ClipStatus_TrivialReject,
    ClipStatus_TrivialAccept
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ClipVector::ClipPolyface(PolyfaceQueryCR polyface, PolyfaceQuery::IClipToPlaneSetOutput& output, bool triangulateOutput) const
    {
    T_ClipPlaneSets         clipPlaneSets;

    FOR_EACH (ClipPrimitivePtr const& primitive, *this)
        if (NULL != primitive->GetClipPlanes())
            clipPlaneSets.push_back(*primitive->GetClipPlanes());
    
    return polyface.ClipToPlaneSetIntersection(clipPlaneSets, output, triangulateOutput);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FrustumPolyfaceClipper : PolyfaceQuery::IClipToPlaneSetOutput
{
    bool                        m_unclipped;
    IPolyfaceConstructionR      m_builder;

    FrustumPolyfaceClipper(IPolyfaceConstructionR builder) : m_builder(builder), m_unclipped(false) { }
    StatusInt _ProcessUnclippedPolyface(PolyfaceQueryCR polyfaceQuery) override { m_unclipped = true; return SUCCESS; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt _ProcessClippedPolyface(PolyfaceHeaderR polyfaceHeader) override
    {
    m_builder.AddPolyface(polyfaceHeader); 

    return SUCCESS;
    }
};  // FrustumPolyfaceClipper

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void addFrustumToPolyface(IPolyfaceConstructionR builder, FrustumCR frustum)
    {
    bvector<size_t>             indices;
    static int s_boxQuadIndices[6][4]
        =   {
            {0,1,5,4},
            {1,3,7,5},
            {3,2,6,7},
            {0,4,6,2},
            {1,0,2,3},  // bottom
            {7,6,4,5}   // top
        };
    
    builder.FindOrAddPoints(frustum.m_pts, 8, indices);
    for (size_t i=0; i<6; i++)
        {
        for (size_t k=0; k<4; k++)
            builder.AddPointIndex(indices[s_boxQuadIndices[i][k]], true);
        
        builder.AddPointIndexTerminator();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipStatus clipPolyfaceByFrustum(IPolyfaceConstructionR output, PolyfaceHeaderR polyface, FrustumCR clipFrustum)
    {
    ConvexClipPlaneSet          convexSet(6);

    ClipUtil::RangePlanesFromFrustum(&convexSet.front(), clipFrustum, true, true, 0.0);

    ClipPlaneSet                clipPlaneSet(convexSet);
    T_ClipPlaneSets             planeSets;

    planeSets.push_back(ClipPlaneSet(convexSet));
    FrustumPolyfaceClipper      clipOutput(output);
    
    polyface.ClipToPlaneSetIntersection(planeSets, clipOutput, false);

    if (clipOutput.m_unclipped)
        return ClipStatus_TrivialAccept;
        
    return 0 == output.GetClientMeshR().GetNumFacet() ? ClipStatus_TrivialReject : ClipStatus_ClipRequired;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double ClipUtil::ComputeFrustumOverlap(FrustumCR thisFrustum, FrustumCR testFrustumRootPoints) 
    {
    Frustum thisFrustumRootPoints = thisFrustum;

    IFacetOptionsPtr            facetOptions       = IFacetOptions::New();
    IPolyfaceConstructionPtr    thisFrustumBuilder = IPolyfaceConstruction::New(*facetOptions), 
                                intersectBuilder   = IPolyfaceConstruction::New(*facetOptions);

    addFrustumToPolyface(*thisFrustumBuilder, thisFrustumRootPoints);

    DPoint3d                    origin = DPoint3d::From(0.0, 0.0, 0.0);
    double                      thisVolume;
    PolyfaceHeaderR             thisFrustumPolyface = thisFrustumBuilder->GetClientMeshR();

    if (!thisFrustumPolyface.IsClosedByEdgePairing() || (thisVolume = thisFrustumPolyface.SumTetrahedralVolumes(origin)) < 0.0)
        {
        BeAssert(false);
        return 0.0;
        }

    switch (clipPolyfaceByFrustum(*intersectBuilder, thisFrustumPolyface, testFrustumRootPoints))
        {
        case ClipStatus_TrivialReject:
            return 0.0;

        case ClipStatus_TrivialAccept:
            return 1.0;
        }
        
    IPolyfaceConstructionPtr    testFrustumBuilder =  IPolyfaceConstruction::New(*facetOptions);

    addFrustumToPolyface(*testFrustumBuilder, testFrustumRootPoints);
    clipPolyfaceByFrustum(*intersectBuilder, testFrustumBuilder->GetClientMeshR(), thisFrustumRootPoints);

    PolyfaceHeaderR     intersectPolyface = intersectBuilder->GetClientMeshR();

    if (!intersectPolyface.IsClosedByEdgePairing())
        {
       // BeAssert (false);
        return 0.0;
        }
    return intersectPolyface.SumTetrahedralVolumes(origin) / thisVolume; 
    }

static double  s_tinyValue = 1.0E-8;
typedef bvector <ClipPlaneCP> T_ClipPlaneVector;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool computeRayPlanesIntersection(double& tNear, double& tFar, DVec3dCR rayDirection, DPoint3dCR rayOrigin, T_ClipPlaneVector& planes)
    {
    static double   s_hugeValue = 1.0E25;

    tNear = -s_hugeValue, tFar  = s_hugeValue;
    for (T_ClipPlaneVector::iterator curr = planes.begin(), end = planes.end(); curr != end; curr++)
        {
        double          vD = (*curr)->DotProduct(rayDirection), vN = (*curr)->EvaluatePoint(rayOrigin);

        if (fabs(vD) < s_tinyValue)
            {
            if (vN < -s_tinyValue)
                return false;
            }
        else
            {
            double      rayDistance = -vN / vD;
            if (vD < 0.0)
                {
                if (rayDistance < tFar)
                   tFar = rayDistance;
                }
            else
                {
                if (rayDistance > tNear)
                    tNear = rayDistance;
                }
            }
        }
    return tNear <= tFar + s_tinyValue;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool intersectPlaneEdgesWithPlanes(DRange3dR intersectRange, T_ClipPlaneVector& planes)
    {
    bool                            intersectFound = false;

    for (T_ClipPlaneVector::iterator curr = planes.begin(), end = planes.end(), other; curr != end; ++curr)
        {
        for (other = curr, ++other; other != end; other++)
            {
            DPoint3d        origin, otherOrigin, intersectionOrigin;
            DVec3d          intersectionNormal;

            origin.Scale((*curr)->GetNormal(), (*curr)->GetDistance());
            otherOrigin.Scale((*other)->GetNormal(), (*other)->GetDistance());

            if (bsiGeom_planePlaneIntersection(&intersectionOrigin, &intersectionNormal, &origin, &(*curr)->GetNormal(), &otherOrigin, &(*other)->GetNormal()))
                {
                double      tNear, tFar;

                intersectFound = true;
                if (computeRayPlanesIntersection(tNear, tFar, intersectionNormal, intersectionOrigin, planes) && tNear <= tFar)
                    {
                    DPoint3d        points[2];

                    points[0].SumOf(intersectionOrigin,intersectionNormal, tNear);
                    points[1].SumOf(intersectionOrigin,intersectionNormal, tFar);
                    intersectRange.Extend(points, 2);
                    }
                }
            }
        }
    if (!intersectFound)
        {
        // We have no intersections. - So this must be a set of parallel planes.
        for (T_ClipPlaneVector::iterator curr = planes.begin(), end = planes.end(), other; curr != end; ++curr)
            {
            for (other = curr, ++other; other != end; other++)
                {
                if ((*curr)->GetNormal().DotProduct(*(&(*other)->GetNormal())) < 0.0 &&
                    (*curr)->GetDistance() > -(*other)->GetDistance())
                    return false;
                }
            }                                     
        return true;
        }

    return !intersectRange.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipUtil::IntersectClipPlaneSets(DRange3dP intersectRange, ClipPlaneCP planeSet1, size_t nPlanes1, ClipPlaneCP planeSet2, size_t nPlanes2)
    {
    DRange3d                    tmpRange;
    T_ClipPlaneVector           combinedPlanes;

    if (NULL == intersectRange)
        intersectRange = &tmpRange;

    intersectRange->Init();

    for (size_t i=0; i<nPlanes1; i++)
        combinedPlanes.push_back(planeSet1 + i);
 

    for (size_t i=0; i<nPlanes2; i++)
        combinedPlanes.push_back(planeSet2 + i);


    // Take each intersecting range-pair from each of the two sets and clip it by the other planes in 
    // both sets.  This produces correct intersection for all cases (even when one set is contained within the other.
    return intersectPlaneEdgesWithPlanes(*intersectRange, combinedPlanes);
    }


