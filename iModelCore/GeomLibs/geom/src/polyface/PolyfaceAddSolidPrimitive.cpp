/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|GEOMDLLIMPEXP void AddSolidPrimitive (ISolidPrimitiveR primitive);
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../SolidPrimitive/BoxFaces.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     04/12
+-------------+---------------+---------------+---------------+---------------+------*/
bool IPolyfaceConstruction::Add (DgnBoxDetailCR box)
    {
    double maxEdgeLength = GetFacetOptionsR ().GetMaxEdgeLength ();
    bool needEdgeChains = GetFacetOptionsR().GetEdgeChainsRequired();
    size_t edgeCount[3];
    BoxFaces cornerData;
    cornerData.Load (box);
    // box load does not try to make orientation positive -- test it and let the face-by-face constructions get reversed.
    bool reversed = cornerData.IsLeftHanded ();
    if (reversed)
        ToggleIndexOrderAndNormalReversal ();
    bvector<size_t> pointIndexA, pointIndexB;
    bvector<size_t> paramIndexA, paramIndexB;
    bvector<size_t> normalIndexA, normalIndexB;
    bvector<DPoint3d>pointB;
    bvector<DPoint2d>paramB;
    bvector<DVec3d> normalB;
    if (maxEdgeLength > 0.0)
        {
        DVec3d maxLengths = cornerData.MaxEdgeLengths ();
        edgeCount[0] = GetFacetOptionsR ().DistanceStrokeCount (maxLengths.x);
        edgeCount[1] = GetFacetOptionsR ().DistanceStrokeCount (maxLengths.y);
        edgeCount[2] = GetFacetOptionsR ().DistanceStrokeCount (maxLengths.z);
        }
    else
        {
        edgeCount[0] = edgeCount[1] = edgeCount[2] = 1;
        }
    bool needParams = GetFacetOptionsR ().GetParamsRequired ();        
    bool needNormals = GetFacetOptionsR ().GetNormalsRequired ();        
    bool capped = box.m_capped;
    for (int faceIndex = 0; faceIndex < 6; faceIndex++)
        {
        if (!capped && BoxFaces::IsCapFace (faceIndex))
            continue;
        BoxFaces::FaceDirections directions = cornerData.GetFaceDirections (faceIndex);
        DBilinearPatch3d patch = cornerData.GetFace (faceIndex);
        size_t numX = edgeCount[directions.axis[0]];
        size_t numY = edgeCount[directions.axis[1]];
        double dX = 1.0 / (double)numX;
        double dY = 1.0 / (double)numY;
        DSegment1d xMap (0,1), yMap (0, 1);
        bool endFace;
        DRange2d faceDistanceRange;
        cornerData.GetSideWrappedFaceParameterMap (faceIndex, xMap,  yMap, faceDistanceRange, endFace);
        for (size_t iY = 0; iY <= numY; iY++)
            {
            paramB.clear ();
            pointB.clear ();
            normalB.clear ();
            DPoint2d uv;
            DPoint3d xyz;
            DVec3d normal;
            uv.y = iY < numY ? iY * dY : 1.0;
            for (size_t iX = 0; iX <= numX; iX++)
                {
                uv.x = iX < numX ? iX * dX : 1.0;
                paramB.push_back (DPoint2d::From
                        (                        
                        xMap.FractionToPoint (uv.x),
                        yMap.FractionToPoint (uv.y)
                        ));
                patch.EvaluateNormal (uv.x, uv.y, xyz, normal);
                pointB.push_back (xyz);
                normalB.push_back (normal);
                }
            pointIndexB.clear ();
            FindOrAddPoints (pointB, numX + 1, 0, pointIndexB);
            if (needNormals)
                {
                normalIndexB.clear ();
                FindOrAddNormals (normalB, numX + 1, 0, normalIndexB);
                }
            if (needParams)
                {
                paramIndexB.clear ();
                FindOrAddParams (paramB, numX + 1, 0, paramIndexB);
                }
            if(iY > 0)
                {
                for (size_t iX = 0; iX < numX; iX++)
                    {
                    AddPointIndexQuad (
                        pointIndexA[iX], iY == 1,
                        pointIndexA[iX+1], iX + 1 == numX,
                        pointIndexB[iX+1], iY  == numY,
                        pointIndexB[iX], iX == 0
                        );
                    if (needParams)
                        {
                        AddParamIndexQuad (
                            paramIndexA[iX],
                            paramIndexA[iX+1],
                            paramIndexB[iX+1],
                            paramIndexB[iX]
                            );
                        }
                    if (needNormals)
                        {
                        AddNormalIndexQuad (
                            normalIndexA[iX],
                            normalIndexB[iX+1],
                            normalIndexB[iX+1],
                            normalIndexA[iX]
                            );
                        }
                    }
                }
            pointIndexA.swap (pointIndexB);
            if (needParams)
                paramIndexA.swap (paramIndexB);
            if (needNormals)
                normalIndexA.swap (normalIndexB);
            }
        if (endFace)
            {
            SetCurrentFaceParamDistanceRange (faceDistanceRange);
            EndFace ();
            }
        }
    if (needParams)
        GetClientMeshR().BuildPerFaceFaceData ();

    if (needEdgeChains)
        {
        bvector<PolyfaceEdgeChain> &chains = GetClientMeshR().EdgeChain ();
#define OutputAlignedLoops
#ifdef OutputAlignedLoops
        // This should match SS3.  But the reversal logic seems backwards.
        bvector<DPoint3d> profilePoint[2];   // Around caps . . 
        bvector<size_t> profilePointIndex[2];
        cornerData.GetAlignedCapLoops (profilePoint[0], profilePoint[1]);
        FindOrAddPoints (profilePoint[0], profilePoint[0].size (), 0, profilePointIndex[0]);
            profilePointIndex[0].push_back (profilePointIndex[0].front ());
        FindOrAddPoints (profilePoint[1], profilePoint[1].size (), 0, profilePointIndex[1]);
            profilePointIndex[1].push_back (profilePointIndex[1].front ());
        bvector<size_t> lateralPointIndices;
        BeAssert (profilePoint[0].size () == profilePoint[1].size ());

        int selectA = reversed ? 0 : 1;
        int selectB = 1 - selectA;

        chains.push_back (PolyfaceEdgeChain (
            CurveTopologyId (CurveTopologyId::Type::SweepProfile, (uint32_t)0)));
        chains.back ().AddZeroBasedIndices (profilePointIndex[selectA]);
        chains.push_back (PolyfaceEdgeChain (
            CurveTopologyId (CurveTopologyId::Type::SweepProfile, (uint32_t)1)));
        chains.back ().AddZeroBasedIndices (profilePointIndex[selectB]);
        for (size_t i = 0; i < profilePoint[0].size (); i++)
            {
            lateralPointIndices.clear ();
            lateralPointIndices.push_back (profilePointIndex[selectA][i]);
            lateralPointIndices.push_back (profilePointIndex[selectB][i]);
            chains.push_back (PolyfaceEdgeChain (
                CurveTopologyId (CurveTopologyId::Type::SweepLateral, (uint32_t)i)));
            chains.back ().AddZeroBasedIndices (lateralPointIndices);
            }
#endif
#ifdef OutputBySingleSegments
        bvector<DSegment3d> segment;
        bvector<size_t> axisId;
        DPoint3d xyz;
        cornerData.GetEdgeSegments (segment, axisId);
        for (size_t i = 0; i < segment.size (); i++)
            {
            size_t numEdge = edgeCount[axisId[i]];
            pointB.clear ();
            pointB.push_back (segment[i].point[0]);
            for (size_t k = 1; k < numEdge; k++)
                {
                double f = k / (double)(numEdge + 1);
                segment[i].FractionParameterToPoint (xyz, f);
                pointB.push_back (xyz);
                }
            pointB.push_back (segment[i].point[1]);
            pointIndexB.clear ();
            FindOrAddPoints (pointB, pointB.size (), 0, pointIndexB);
            chains.push_back (PolyfaceEdgeChain ());
            chains.back ().AddZeroBasedIndices (pointIndexB);
            }
#endif
        }
    if (reversed)
        ToggleIndexOrderAndNormalReversal ();
    if (box.IsClosedVolume ())
        GetClientMeshR().SetTwoSided (false);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool IPolyfaceConstruction::Add (DgnSphereDetailCR sphere)
    {
    Transform localToWorld, worldToLocal;
    if (sphere.GetNonUniformTransforms (localToWorld, worldToLocal))
        {
        bool reversed = sphere.ParameterizationSign () < 0.0;
        static bool s_applyReverse = false;
        if (!s_applyReverse)
            reversed = false;
        bool closed = sphere.IsClosedVolume ();
        PushState (false);
        ApplyLocalToWorld (localToWorld);
        if (reversed)
            ToggleIndexOrderAndNormalReversal ();
        GetClientMeshR().SetTwoSided (!closed);
        AddEllipsoidPatch (DPoint3d::From (0,0,0),
                1.0, 1.0, 1.0,      // radii
                0,0,                // edge counts
                0.0, msGeomConst_2pi,
                sphere.m_startLatitude, sphere.m_latitudeSweep, sphere.m_capped);
        if (reversed)
            ToggleIndexOrderAndNormalReversal ();
        if (closed)
            GetClientMeshR().SetTwoSided (false);
        PopState ();
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool IPolyfaceConstruction::Add (DgnConeDetailCR cone)
    {
    DEllipse3d ellipse0, ellipse1;
    static double s_startDegrees = -90.0;
    bool ok0 = cone.FractionToSection (0.0, ellipse0);
    bool ok1 = cone.FractionToSection (1.0, ellipse1);
    // The sections fail if zero radius.  (But the center of such ellipses are fine)
    if (ok0 || ok1)
        {
        // hmph... QV wants parameters rotated.
        DEllipse3d ellipse0A = ellipse0;
        DEllipse3d ellipse1A = ellipse1;
        double theta0 = Angle::DegreesToRadians (s_startDegrees);
        double cc = cos (theta0);
        double ss = sin (theta0);
        ellipse0A.vector0.SumOf (ellipse0.vector0, cc, ellipse0.vector90, ss);
        ellipse0A.vector90.SumOf (ellipse0.vector0, -ss, ellipse0.vector90, cc);
        ellipse1A.vector0.SumOf (ellipse1.vector0, cc, ellipse1.vector90, ss);
        ellipse1A.vector90.SumOf (ellipse1.vector0, -ss, ellipse1.vector90, cc);
        DVec3d axis = DVec3d::FromStartEnd (ellipse0.center, ellipse1.center);

        // Use larger end for orientation test
        DVec3d cross0 = ellipse0.CrossProductOfBasisVectors ();
        DVec3d cross1 = ellipse1.CrossProductOfBasisVectors ();
        bool reverse = false;
        if (cross0.MagnitudeSquared () > cross1.MagnitudeSquared ())
            reverse = axis.DotProduct (cross0) < 0.0;
        else
            reverse = axis.DotProduct (cross1) < 0.0;
            
        if (reverse)
            ToggleIndexOrderAndNormalReversal ();
        AddRuled (ellipse0A, ellipse1A, cone.m_capped);
        if (reverse)
            ToggleIndexOrderAndNormalReversal ();
        if (cone.m_capped)
            GetClientMeshR().SetTwoSided (false);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool IPolyfaceConstruction::Add (DgnTorusPipeDetailCR detail)
    {
    DEllipse3d pipeEllipse = detail.VFractionToUSectionDEllipse3d (0.0);
    DRay3d axis;
    double majorSweepRadians;
    size_t minorCount = GetFacetOptionsR ().EllipseStrokeCount (pipeEllipse);
    double arcLength = pipeEllipse.ArcLength ();
    if (detail.TryGetRotationAxis (axis.origin, axis.direction, majorSweepRadians))
        {
        bvector<DPoint3d> points;
        bvector<DVec3d>   pipeTangents;
        DPoint3d xyz;
        DVec3d tangent, deflection;
        for (size_t i = 0; i < minorCount; i++)
            {
            double fraction = i / (double)minorCount;
            pipeEllipse.FractionParameterToDerivatives (xyz, tangent, deflection, fraction);
            points.push_back (xyz);
            pipeTangents.push_back (tangent);
            }
        xyz = points[0];
        points.push_back (xyz);
        tangent = pipeTangents[0];
        pipeTangents.push_back (tangent);
        bvector<DPoint3d> startPoints, endPoints;
        bool reverse = majorSweepRadians * DgnTorusPipeDetail::GetVector90Sign () < 0.0;   // ellipse orientation is controlled.
        AddRotationalSweepLoop (points, pipeTangents, axis.origin, axis.direction, majorSweepRadians,
            reverse, arcLength, &startPoints, &endPoints, nullptr);
        
        AddTriangulationPair (startPoints, reverse, endPoints, !reverse,
                detail.HasRealCaps (),
                !Angle::IsFullCircle (majorSweepRadians), CurveTopologyId::Type::SweepProfile);
        if (detail.IsClosedVolume ())
            GetClientMeshR().SetTwoSided (false);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool IPolyfaceConstruction::Add (DgnExtrusionDetailCR detail)
    {
    bvector<CurveVectorPtr> caps;
    caps.push_back (detail.m_baseCurve);
    caps.push_back (detail.FractionToProfile (1.0));
    return AddRuledBetweenCorrespondingCurves (caps, detail.m_capped);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool IPolyfaceConstruction::Add (DgnRuledSweepDetailCR detail)
    {
    return AddRuledBetweenCorrespondingCurves (detail.m_sectionCurves, detail.m_capped);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool IPolyfaceConstruction::Add (DgnRotationalSweepDetailCR detail)
    {
    AddRotationalSweep (detail.m_baseCurve, detail.m_axisOfRotation.origin, detail.m_axisOfRotation.direction, detail.m_sweepAngle, detail.HasRealCaps ());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool IPolyfaceConstruction::AddSolidPrimitive (ISolidPrimitiveCR primitive)
    {
    DgnConeDetail             cone;
    DgnSphereDetail           sphere;
    DgnBoxDetail              box;
    DgnTorusPipeDetail        torus;
    DgnExtrusionDetail        extrusion;
    DgnRuledSweepDetail       ruledSweep;
    DgnRotationalSweepDetail  rotationalSweep;

    if (primitive.TryGetDgnConeDetail (cone))
        return Add (cone);
    else if (primitive.TryGetDgnSphereDetail (sphere))
        return Add (sphere);
    else if (primitive.TryGetDgnBoxDetail (box))
        return Add (box);
    else if (primitive.TryGetDgnTorusPipeDetail (torus))
        return Add (torus);
    else if (primitive.TryGetDgnExtrusionDetail (extrusion))
        return Add (extrusion);
    else if (primitive.TryGetDgnRuledSweepDetail (ruledSweep))
        return Add (ruledSweep);
    else if (primitive.TryGetDgnRotationalSweepDetail (rotationalSweep))
        return Add (rotationalSweep);

    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
