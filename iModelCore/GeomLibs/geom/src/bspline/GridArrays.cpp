/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../DeprecatedFunctions.h"

// MINIMAL INCLUDES -- intended to be directly included in bsppolyface until bubbled to public api..

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


static void SplitWeights (bvector<DPoint4d> const &xyzw, bvector<DPoint3d> &xyz, bvector<double> &w)
    {
    size_t n = xyzw.size ();
    xyz.resize (n);
    w.resize (n);
    for (size_t i = 0; i < n; i++)
        {
        xyz[i].x = xyzw[i].x;
        xyz[i].y = xyzw[i].y;
        xyz[i].z = xyzw[i].z;
        w[i]     = xyzw[i].w;
        }
    }



struct GridArrays
{
bvector<double> uValue;
bvector<double> vValue;
bvector<double> blendU;
bvector<double> dBlenddU;
bvector<double> blendV;
bvector<double> dBlenddV;

bvector<DPoint2d>  uv;
bvector<DPoint2d>  uv01;
bvector<DPoint3d> xyz;
bvector<double>   xyzWeight;
bvector<DPoint3d> dXdu;
bvector<double>   dWdu;
bvector<DPoint3d> dXdv;
bvector<double>   dWdv;
bvector<DVec3d>   normal;

size_t numU;
size_t numV;

bool TryGetXYZWUV01 (size_t i, size_t j, DPoint3d &xyzOut, double &wOut, DPoint2d &uvOut, bool deweightXYZ = false)
    {
    if (i < numU && j < numV)
        {
        size_t index = i + j * numU;
        xyzOut = xyz[index];
        wOut   = xyzWeight[index];
        if (deweightXYZ)
            xyzOut.SafeDivide (xyzOut, wOut);
        uvOut   = uv[index];
        return true;
        }
    return false;
    }

bool TryGetXYZWUV01 (size_t i, size_t j, DPoint4d &xyzOut, DPoint2d &uvOut)
    {
    if (i < numU && j < numV)
        {
        size_t index = i + j * numU;
        xyzOut.InitFrom (xyz[index], xyzWeight[index]);
        uvOut   = uv[index];
        return true;
        }
    return false;
    }



// return false if indices do not identify a grid quad
bool TryRayPierce (size_t i, size_t j, DRay3dCR ray, DPoint2d uv01Out [2], double tOut[2], DPoint3d xyzOut[2], size_t &numOut, double tolerance01 = 1.0e-10)
    {
    numOut = 0;
    double onePlus = 1.0 + tolerance01;
    double zeroMinus = - tolerance01;
    DPoint3d xyz00, xyz10, xyz01, xyz11;
    DPoint2d uv00a, uv10a, uv01a, uv11a;
    double   w00, w10, w01, w11;
    if (   TryGetXYZWUV01 (i, j, xyz00, w00, uv00a)
        && TryGetXYZWUV01 (i+1, j, xyz10, w10, uv10a)
        && TryGetXYZWUV01 (i, j+1, xyz01, w01, uv01a)
        && TryGetXYZWUV01 (i+1, j+1, xyz11, w11, uv11a)
        )
        {
        DPoint2d st[2]; // local parameters within the smallest grid quad
        double rayParam[2];    // ray parameters
        DPoint3d xyzLocal[2];
        int numIntersect = ray.IntersectHyperbolicParaboloid (xyzLocal, rayParam, st, xyz00, w00, xyz10, w10, xyz01, w01, xyz11, w11);
        for (int k = 0; k < numIntersect; k++)
            {
            if (st[k].x >= zeroMinus && st[k].y >= zeroMinus && st[k].x <= onePlus && st[k].y <= onePlus)
                {
                uv01Out[numOut] = DPoint2d::FromInterpolateBilinear (uv00a, uv10a, uv01a, uv11a, st[k].x, st[k].y);
                tOut[numOut]    = rayParam[k];
                xyzOut[numOut]  = xyzLocal[k];
                numOut++;
                }
            }
        return true;
        }
    return false;
    }

// return false if indices do not identify a grid quad
bool AppendCurvePierce (size_t i, size_t j,
ICurvePrimitiveCR curve,
DRange3dCR curveRange,
bvector<CurveAndSolidLocationDetail> &intersections
)
    {
    DPoint3d xyz00, xyz10, xyz01, xyz11;
    DPoint2d uv00a, uv10a, uv01a, uv11a;
    double   w00, w10, w01, w11;
    // For range and triangle pierce, deweight the quad points ...
    // this will give slightly altered coordinates. ugh.
    if (   TryGetXYZWUV01 (i, j, xyz00, w00, uv00a, true)
        && TryGetXYZWUV01 (i+1, j, xyz10, w10, uv10a, true)
        && TryGetXYZWUV01 (i, j+1, xyz01, w01, uv01a, true)
        && TryGetXYZWUV01 (i+1, j+1, xyz11, w11, uv11a, true)
        )
        {
        DRange3d quadRange = DRange3d::From (xyz00);
        quadRange.Extend (xyz10);
        quadRange.Extend (xyz11);
        quadRange.Extend (xyz01);
        if (quadRange.IntersectsWith (curveRange))
            {
            DBilinearPatch3d xyzPatch (xyz00, xyz10, xyz01, xyz11);
            size_t num0 = intersections.size ();
            curve.AppendCurveBilinearPatchIntersections (xyzPatch, intersections);
            // Promote uv coordinates from bilinear patch to 
            for (size_t k = num0; k < intersections.size (); k++)
                {
                DPoint2d uvQuad = intersections[k].m_solidDetail.GetUV ();
                DPoint2d uvPatch = DPoint2d::FromInterpolateBilinear (
                    uv00a, uv10a, uv01a, uv11a, uvQuad.x, uvQuad.y);
                intersections[k].m_solidDetail.SetUV (uvPatch);
                }
            }
        return true;
        }
    return false;
    }




// return false if indices do not identify a grid quad
// i,j specified the lower left of the quad.
// The triangle (within the quad!!!) is formed thusly:
//   Number the quad points 0123 in a CCW order:  00,10,11,01
//   baseVertexSelect is the "origin", successor is "x axis", predecessor is "y axis target"
bool TryClosestPointByTriangle (size_t iGrid, size_t jGrid, size_t baseVertexSelect, DPoint3dCR spacePoint, DPoint2dR uv01Out, DPoint3dR xyzOut, DPoint3dR triangleUVBounded, DPoint3dR triangleUVUnbounded)
    {
    DPoint3d myXYZ[4];
    DPoint2d myUV[4];
    double   w[4];
    if (   TryGetXYZWUV01 (iGrid, jGrid, myXYZ[0], w[0], myUV[0], true)
        && TryGetXYZWUV01 (iGrid+1, jGrid, myXYZ[1], w[1], myUV[1], true)
        && TryGetXYZWUV01 (iGrid+1, jGrid+1, myXYZ[2], w[2], myUV[2], true)
        && TryGetXYZWUV01 (iGrid, jGrid+1, myXYZ[3], w[3], myUV[3], true)
        )
        {
        size_t i0 = baseVertexSelect % 4;
        size_t i1 = (baseVertexSelect + 1) % 4;
        size_t i2 = (baseVertexSelect + 3) % 4;
        DPoint3d triangleXYZ[3];
        DPoint2d triangleUV[3];
        triangleXYZ[0] = myXYZ[i0];
        triangleXYZ[1] = myXYZ[i1];
        triangleXYZ[2] = myXYZ[i2];

        triangleUV[0] = myUV[i0];
        triangleUV[1] = myUV[i1];
        triangleUV[2] = myUV[i2];
        bsiDPoint3d_minDistToTriangle (&spacePoint, &triangleXYZ[0], &triangleXYZ[1], &triangleXYZ[2], &xyzOut, &triangleUVBounded, &triangleUVUnbounded);
        uv01Out = DPoint2d::FromSumOf (triangleUV[0], triangleUVBounded.x, triangleUV[1], triangleUVBounded.y, triangleUV[2], triangleUVBounded.z);
        return true;
        }
    return false;
    }


bvector<DPoint4d> xyzw_work;        // Can be reused within any method.

// Evaluate raw tensor products and derivatives -- do not deweight or take cross products for normals.
bool EvaluateBezier (double u0, double u1, int numUin, double v0, double v1, int numVin, MSBsplineSurfaceCP patch, bool reverseU, bool derivatives)
    {
    uv.clear ();
    xyz.clear ();
    xyzWeight.clear ();
    dXdu.clear ();
    dXdv.clear ();
    dWdu.clear ();
    dWdv.clear ();
    normal.clear ();
    if (numUin <= 0 || numVin <= 0)
        return false;
    numU = (size_t) numUin;
    numV = (size_t) numVin;
    size_t orderU = (size_t)patch->uParams.order;
    size_t orderV = (size_t)patch->vParams.order;
    bool rational = patch->rational && (NULL != patch->weights);
    if (!reverseU)
        TensorProducts::EvaluateBezierBasisFunctionGrid1D (uValue, blendU, orderU, u0, u1, numU);
    else
        TensorProducts::EvaluateBezierBasisFunctionGrid1D (uValue, blendU, orderU, u1, u0, numU);
    TensorProducts::EvaluateBezierBasisFunctionGrid1D (vValue, blendV, orderV, v0, v1, numV);
    TensorProducts::AssembleGrid2D (uv, uValue, vValue);
    if (derivatives)
        {
        TensorProducts::EvaluateBezierDerivativeBasisFunctions (dBlenddU, uValue, orderU);
        TensorProducts::EvaluateBezierDerivativeBasisFunctions (dBlenddV, vValue, orderV);
        }
    TensorProducts::EvaluateGrid2D (xyz, blendU, blendV, patch->poles, orderU, orderV);
    if (rational)
        TensorProducts::EvaluateGrid2D (xyzWeight, blendU, blendV, patch->weights, orderU, orderV);
        
    if (derivatives)
        {
        TensorProducts::EvaluateGrid2D (dXdu, dBlenddU, blendV, patch->poles, orderU, orderV);
        TensorProducts::EvaluateGrid2D (dXdv, blendU, dBlenddV, patch->poles, orderU, orderV);
        
        if (rational)
            {
            TensorProducts::EvaluateGrid2D (dWdu, dBlenddU, blendV, patch->weights, orderU, orderV);
            TensorProducts::EvaluateGrid2D (dWdv, blendU, dBlenddV, patch->weights, orderU, orderV);
            }
        }
    return true;
    }
 
// Evaluate raw tensor products and derivatives -- do not deweight or take cross products for normals.
bool EvaluateBezier (double u0, double u1, size_t numUIn, double v0, double v1, size_t numVIn, BSurfPatchCR patch, bool reverseU, size_t numDerivatives)
    {
    uv.clear ();
    xyz.clear ();
    xyzWeight.clear ();
    dXdu.clear ();
    dXdv.clear ();
    dWdu.clear ();
    dWdv.clear ();
    normal.clear ();
    if (numUIn <= 0 || numVIn <= 0)
        return false;
    numU = numUIn;
    numV = numVIn;
    size_t orderU = patch.uOrder;
    size_t orderV = patch.vOrder;

    if (!reverseU)
        TensorProducts::EvaluateBezierBasisFunctionGrid1D (uValue, blendU, orderU, u0, u1, numU);
    else
        TensorProducts::EvaluateBezierBasisFunctionGrid1D (uValue, blendU, orderU, u1, u0, numU);
    TensorProducts::EvaluateBezierBasisFunctionGrid1D (vValue, blendV, orderV, v0, v1, numV);
    TensorProducts::AssembleGrid2D (uv, uValue, vValue);
    if (numDerivatives > 0)
        {
        TensorProducts::EvaluateBezierDerivativeBasisFunctions (dBlenddU, uValue, orderU);
        TensorProducts::EvaluateBezierDerivativeBasisFunctions (dBlenddV, vValue, orderV);
        }
    TensorProducts::EvaluateGrid2D (xyzw_work, blendU, blendV, &patch.xyzw[0], orderU, orderV);
    SplitWeights (xyzw_work, xyz, xyzWeight);
    if (numDerivatives > 0)
        {
        TensorProducts::EvaluateGrid2D (xyzw_work, dBlenddU, blendV, &patch.xyzw[0], orderU, orderV);
        SplitWeights (xyzw_work, dXdu, dWdu);
        TensorProducts::EvaluateGrid2D (xyzw_work, blendU, dBlenddV, &patch.xyzw[0], orderU, orderV);
        SplitWeights (xyzw_work, dXdv, dWdv);
        }
    return true;
    }

// Evaluate raw tensor products and derivatives -- do not deweight or take cross products for normals.
bool EvaluateBezier (bvector<DPoint2d> uvIn,
    double u0, double u1,   // demap from this interval to 0..1
    double v0, double v1,
    MSBsplineSurfaceCP patch, bool derivatives)
    {
    uv = uvIn;
    xyz.clear ();
    xyzWeight.clear ();
    dXdu.clear ();
    dXdv.clear ();
    dWdu.clear ();
    dWdv.clear ();
    normal.clear ();
    size_t numIn = uvIn.size ();
    if (numIn <= 0)
        return false;
        
        
    double du = u1 - u0;
    double dv = v1 - v0;
    double au = 1.0 / du;
    double av = 1.0 / dv;
    uv01.resize (numIn);
    for (size_t i = 0; i < numIn; i++)
        {
        uv01[i].x = (uv[i].x - u0) * au;
        uv01[i].y = (uv[i].y - v0) * av;
        }
    size_t orderU = (size_t)patch->uParams.order;
    size_t orderV = (size_t)patch->vParams.order;
    bool rational = patch->rational && (NULL != patch->weights);

    if (!derivatives)
        {
        TensorProducts::Evaluate (xyz, uv01, patch->poles, orderU, orderV);
        if (rational)
            TensorProducts::Evaluate (xyzWeight, uv01, patch->weights, orderU, orderV);
        }
    else
        {
        TensorProducts::Evaluate (xyz, dXdu, dXdv, uv01, patch->poles, orderU, orderV);
        if (rational)
            {
            TensorProducts::Evaluate (dWdu, dWdu, dWdv, uv01, patch->weights, orderU, orderV);
            TensorProducts::Evaluate (xyzWeight, uv01, patch->weights, orderU, orderV);
            }
        }
    return true;
    }



bool DeWeightPoint (size_t i)
    {
    if (i < xyz.size ())
        {
        double w = xyzWeight[i];
        double a;
        bool stat = DoubleOps::SafeDivide (a, 1.0, w, 1.0);
        xyz[i].Scale (a);
        return stat;
        }
    return false;
    }

bool DeWeightPointAndDerivatives (size_t i)
    {
    if (i < xyz.size ())
        {
        double w = xyzWeight[i];
        double a;
        bool stat = DoubleOps::SafeDivide (a, 1.0, w, 1.0);
        xyz[i].Scale (a);
        dXdu[i].SumOf (dXdu[i], a, xyz[i], - dWdu[i] * a);
        dXdv[i].SumOf (dXdv[i], a, xyz[i], - dWdv[i] * a);
        return stat;
        }
    return false;
    }
double NearbyCoordinate (double a, double delta)
    {
    return (a < delta) ? a + delta : a - delta;
    }
// ASSUME point, derivatives are already cartesian
bool ComputeNormalFromDerivatives (size_t i, MSBsplineSurfaceCP surface, bool reverse)
    {
    if (dXdu[i].Magnitude () < fc_epsilon)
        bspsurf_evaluateSurfacePoint (NULL, NULL, &dXdu[i], NULL,
                            uv[i].x, NearbyCoordinate (uv[i].y, fc_epsilon), surface);

    if (dXdv[i].Magnitude () < fc_epsilon)
        bspsurf_evaluateSurfacePoint (NULL, NULL, NULL, &dXdv[i],
                            NearbyCoordinate (uv[i].x, fc_epsilon), uv[i].y, surface);
    
    normal[i].CrossProduct (*(DVec3d*)&dXdu[i], *(DVec3d*)&dXdv[i]);
    normal[i].Normalize ();
    if (reverse)
        normal[i].Negate ();
    return true;
    }

bool ResolveCoordinatesAndNormals (MSBsplineSurfaceCP surface, bool reverse)
    {
    size_t n = xyz.size ();
    bool weighted = xyzWeight.size () == n;
    bool hasTangents = dXdu.size () == n && dXdv.size () == n;
    if (weighted)
        {
        if (hasTangents)
            for (size_t i = 0; i < n; i++)
                DeWeightPointAndDerivatives (i);
        else
            for (size_t i = 0; i < n; i++)
                DeWeightPoint (i);
        }

    if (hasTangents)
        {
        normal.resize (n);
        for (size_t i = 0; i < n; i++)
            ComputeNormalFromDerivatives (i, surface, reverse);
        }
    return true;
    }
};




END_BENTLEY_GEOMETRY_NAMESPACE
