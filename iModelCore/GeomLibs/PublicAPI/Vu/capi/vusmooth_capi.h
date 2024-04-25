/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct VuScalarFunction
{
GEOMAPI_VIRTUAL double Evaluate (VuSetP graph, VuP node) = 0;
};

// Virtual function declarations for a an object that tests if edges need to be subdivided.
struct VuEdgeSubdivisionTestFunction
{
public:
// return the number edges required between two uv positions.
// The return is a double -- i.e. asking for 2.5 edges is ok.  This allows comparison of split requests
// so splitting can start with high priority edges, and use as a continuous weighting function in
// iterative adjustment.
// Returns less than 1 are valid -- they indicte that this edge can be stretched.
// Default implementation returns 1.0;
GEOMAPI_VIRTUAL double ComputeNumEdgesRequired(DPoint2dCR uvA, DPoint2dCR uvB);

};

Public GEOMDLLIMPEXP void vu_graphRange
(
VuSetP pGraph,
DRange3d *pRange
);

Public GEOMDLLIMPEXP double vu_largestRangeEdge
(
VuSetP pGraph
);

/**
@description Apply laplacian smoothing to interior vertex coordinates.
@param [in] pGraph
@param [in] relTol stopping tolerance, as fraction of graph range.
@param [in] maxSweep maximum number of sweeps.
@param [in] flipInterval number of sweeps be calls to flip triangles
@param [out] pShiftFraction final shift as fraction of tolerance
@param [out] pNumSweep total number of sweeps.
*/
Public GEOMDLLIMPEXP bool    vu_smoothInteriorVertices
(
VuSetP pGraph,
double relTol,
int    maxSweep,
int    flipInterval,
double *pShiftFraction,
int *pNumSweep
);

/**
@description Apply laplacian smoothing to interior vertex coordinates.
@param [in] pGraph
@param [in] pWeightFunction function returning a face weight when called with
                a node of the graph.  This is (internally, afterwards)
                multiplied by the parametric triangle area.
                (Hence the result should be positive!!!)
@param [in] pWeightFunction function returning a xyz coordinates for given uv.
@param [in] relTol stopping tolerance, as fraction of graph range.
@param [in] maxSweep maximum number of sweeps.
@param [in] flipInterval number of sweeps be calls to flip triangles
@param [out] pShiftFraction final shift as fraction of tolerance
@param [out] pNumSweep total number of sweeps.
*/
Public GEOMDLLIMPEXP bool    vu_smoothInteriorVertices
(
VuSetP pGraph,
VuScalarFunction *pFaceWeightFunction,
VuCoordinateMappingFunction *pCoordinateMappingFunction,
double relTol,
int    maxSweep,
int    flipInterval,
int    maxFlipPerEdgePerSweep,
double *pShiftFraction,
int *pNumSweep
);

/*---------------------------------------------------------------------------------**//**
* Perform one step of surface refinement.
* @param IN graph graph to be modified.
* @param IN testFunc object to test if edges need to be subdivided.
*                testFunc.ComputeNumEdgesRequired (uvA, uvB) is called to ask if an edge between specified uv coordinates needs to be subdivided.
* @return number of edge splits.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t    vu_refineSurface
(
VuSetP pGraph,
VuEdgeSubdivisionTestFunction &testFunc
);
END_BENTLEY_GEOMETRY_NAMESPACE