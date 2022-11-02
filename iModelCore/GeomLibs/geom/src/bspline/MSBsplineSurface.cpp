/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#define MDLERR_NOPOLES ERROR
#define MDLERR_INSFMEMORY ERROR
static void (*sFreeSurface)(MSBsplineSurface *) = 0;
static int (*sAllocateSurface)(MSBsplineSurface *) = 0;

#include "msbsplinemaster.h"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::ExtractTo (MSBsplineSurfaceR dest)
    {
    dest = *this;
    Zero ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef msbspline_constructors
MSBsplineSurface::MSBsplineSurface ()
    {
    memset (this, 0, sizeof (MSBsplineSurface));
    }
#endif

#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetAllocationFunctions (
            int  (*AllocateSurface)(MSBsplineSurface *),
            void (*FreeSurface)(MSBsplineSurface *)
            )
    {
    sAllocateSurface  = AllocateSurface;
    sFreeSurface      = FreeSurface;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::ReleaseMem ()
    {
    MSBsplineSurface * surface = this;

    if (sFreeSurface)
        {
        sFreeSurface (surface);
        return;
        }

    if (NULL != surface->poles)
        BSIBaseGeom::Free (poles);
    if (NULL != surface->uKnots)
        BSIBaseGeom::Free (uKnots);
    if (NULL != surface->vKnots)
        BSIBaseGeom::Free (vKnots);
    if (NULL != surface->weights)
        BSIBaseGeom::Free (weights);
    surface->poles   = NULL;
    surface->uKnots   = NULL;
    surface->vKnots   = NULL;
    surface->weights = NULL;
    bspsurf_freeBoundaries (surface);
    }

void MSBsplineSurface::DeleteBoundaries ()
    {
    bspsurf_freeBoundaries (this);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::Zero ()
    {
    memset (this, 0, sizeof (MSBsplineSurface));
    }


MSBsplineSurfacePtr MSBsplineSurface::CreateFromPolesAndOrder
(
bvector<DPoint3d> const &pointVector,
bvector<double> const *weightVector,
bvector<double> const *uKnotVector,
int uOrder,
int numUPoints,
bool uClosed,
bvector<double> const *vKnotVector,
int vOrder,
int numVPoints,
bool vClosed,
bool inputPolesAlreadyWeighted
)
    {
    // Indent to force destructor of result after failure.
        {
        MSBsplineSurfacePtr result = MSBsplineSurface::CreatePtr ();
        if (SUCCESS == result.get ()->Populate (pointVector, weightVector,
                    uKnotVector, uOrder, numUPoints, uClosed,
                    vKnotVector, vOrder, numVPoints, vClosed, inputPolesAlreadyWeighted))
            return result;
        }
    MSBsplineSurfacePtr nullPtr;
    return nullPtr;

    }
void FixKnots(bvector<double> &knots, int order, int poles, bool closed)
    {
    // knot vectors are popping up with first and last extraneous ...
    if (poles + order + 2 == (int)knots.size()
        && knots.front () < 0.0
        && knots.back () > 1.0
        && !closed
        )
        {
        knots.pop_back();
        knots.erase(knots.begin());
        }
    else if (poles + 2 * order + 1 == (int)knots.size()
        && knots.front() < 0.0
        && knots.back() > 1.0
        && closed
        )
        {
        knots.pop_back();
        knots.erase(knots.begin());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::Populate
(
bvector<DPoint3d> const &pointVector,
bvector<double> const *weightVector,
bvector<double> const *uKnotVectorIn,
int uOrder,
int numUPoints,
bool uClosed,
bvector<double> const *vKnotVectorIn,
int vOrder,
int numVPoints,
bool vClosed,
bool inputPointsHaveWeightsAppliedAlready
)
    {
    size_t numPoints = pointVector.size ();
    size_t numWeights = weightVector != NULL  ? weightVector->size () : 0;
    bvector<double> *uKnotVector = nullptr;
    bvector<double> *vKnotVector = nullptr;
    bvector<double> uKnotsA;
    bvector<double> vKnotsA;
    if (uKnotVectorIn != nullptr)
        {
        uKnotsA = *uKnotVectorIn;
        FixKnots(uKnotsA, uOrder, numUPoints, uClosed);
        uKnotVector = &uKnotsA;
        }
    if (vKnotVectorIn != nullptr)
        {
        vKnotsA = *vKnotVectorIn;
        FixKnots(vKnotsA, vOrder, numVPoints, vClosed);
        vKnotVector = &vKnotsA;
        }

    size_t numUKnots   = uKnotVector != NULL   ? uKnotVector->size () : 0;
    size_t numVKnots   = vKnotVector != NULL   ? vKnotVector->size () : 0;
    StatusInt status = ERROR;
    if (   uOrder > 1
        && vOrder > 1
        && numUPoints >= uOrder
        && numVPoints >= vOrder
        && numUPoints * numVPoints == numPoints
        && (numWeights == 0 || numWeights == numPoints)
        && (numUKnots == 0 || numUKnots == BsplineParam::NumberAllocatedKnots(numUPoints, uOrder, uClosed))
        && (numVKnots == 0 || numVKnots == BsplineParam::NumberAllocatedKnots(numVPoints, vOrder, vClosed))
        )
        {
        Zero ();
        uParams.order = uOrder;
        uParams.numPoles = numUPoints;
        uParams.closed = uClosed;

        vParams.order = vOrder;
        vParams.numPoles  = numVPoints;
        vParams.closed = vClosed;
        rational = numWeights > 0;
        if (SUCCESS == (status = Allocate ()))
            {
            for (size_t i = 0; i < numPoints; i++)
                poles[i] = pointVector[i];
            if (weights != NULL)
                for (size_t i = 0; i < numPoints; i++)
                    {
                    weights[i] = weightVector? weightVector->at(i): 1.0;
                    if (!inputPointsHaveWeightsAppliedAlready)
                        poles[i].Scale (weights[i]);
                    }

            if (numUKnots == 0)
                status = bspknot_computeKnotVector (uKnots, &uParams, NULL);
            else
                {
                uParams.numKnots = BsplineParam::NumberInteriorKnots (numUPoints, uOrder, uClosed);
                for (size_t i = 0; i < numUKnots; i++)
                    uKnots[i] = uKnotVector->at(i);
                }
            
            if (numVKnots == 0)
                status = bspknot_computeKnotVector (vKnots, &vParams, NULL);
            else
                {
                vParams.numKnots = BsplineParam::NumberInteriorKnots (numVPoints, vOrder, vClosed);
                for (size_t i = 0; i < numVKnots; i++)
                    vKnots[i] = vKnotVector->at(i);
                }
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::Allocate ()
    {
    MSBsplineSurface * surface = this;

    if (sAllocateSurface)
        return sAllocateSurface (surface);

    int totalPoles, allocSize;
    surface->display.curveDisplay   = true;
    surface->display.polygonDisplay = false;
    surface->uParams.numRules = 10;
    surface->vParams.numRules = 10;

    /* Initiailize all pointers to zero */
    surface->poles = NULL;
    surface->weights = surface->uKnots = surface->vKnots = NULL;
    surface->boundaries = NULL;

    totalPoles = surface->uParams.numPoles * surface->vParams.numPoles;
    if (totalPoles <= 0)
        return MDLERR_NOPOLES;

    /* Allocate poles memory */
    allocSize = totalPoles * sizeof (DPoint3d);
    if (NULL == (surface->poles = static_cast<DPoint3d *>(BSIBaseGeom::Malloc (allocSize))))
        return MDLERR_INSFMEMORY;

    /* Allocate U Knot Vector */
    allocSize = uParams.NumberAllocatedKnots () * sizeof (double);

    if (NULL == (surface->uKnots = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
        {
        ReleaseMem ();
        return MDLERR_INSFMEMORY;
        }

    /* Allocate V Knot Vector */
    allocSize = vParams.NumberAllocatedKnots () * sizeof (double);

    if (NULL == (surface->vKnots = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
        {
        ReleaseMem ();
        return MDLERR_INSFMEMORY;
        }

    /* Allocate weights (if necessary) */
    if (surface->rational)
        {
        allocSize = totalPoles * sizeof (double);
        if (NULL == (surface->weights = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
            {
            ReleaseMem ();
            return MDLERR_INSFMEMORY;
            }
        }

    if (surface->numBounds)
        {
        if (NULL == (surface->boundaries = static_cast<BsurfBoundary *> (BSIBaseGeom::Calloc (surface->numBounds, sizeof (BsurfBoundary)))))
            {
            ReleaseMem ();
            return MDLERR_INSFMEMORY;
            }
        }

    return MSB_SUCCESS;
    }

MSBsplineStatus MSBsplineSurface::AllocateUKnots ()
    {
    if (NULL != uKnots)
        BSIBaseGeom::Free (uKnots);
    int allocSize = uParams.NumberAllocatedKnots () * sizeof (double);        
    if (NULL == (uKnots = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
        return MDLERR_INSFMEMORY;
    return SUCCESS;
    }

MSBsplineStatus MSBsplineSurface::AllocateVKnots ()
    {
    if (NULL != vKnots)
        BSIBaseGeom::Free (vKnots);
    int allocSize = vParams.NumberAllocatedKnots () * sizeof (double);        
    if (NULL == (vKnots = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
        return MDLERR_INSFMEMORY;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::CopyFrom (MSBsplineSurfaceCR input)
    {
    return bspsurf_copySurface (this, (MSBsplineSurfaceCP)&input);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::GetKnotRange (double &min, double &max, int direction) const
    {
    if (direction == BSSURF_U)
        {
        min = uKnots[uParams.order - 1];
        int numKnots = uParams.NumberAllocatedKnots ();
        max = uKnots[numKnots - uParams.order];
        }
    else
        {
        min = vKnots[vParams.order - 1];
        int numKnots = vParams.NumberAllocatedKnots ();
        max = vKnots[numKnots - vParams.order];
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::GetParameterRegion (double &uMin, double &uMax, double &vMin, double &vMax) const
    {
    GetKnotRange (uMin, uMax, BSSURF_U);
    GetKnotRange (vMin, vMax, BSSURF_V);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::GetPoleRange (DRange3dR range) const
    {
    range = DRange3d::From (poles, weights, (int)(int)GetNumPoles ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::GetPoleRange (DRange3dR range, TransformCR transform) const
    {
    range = DRange3d::From (transform, poles, weights, (int)GetNumPoles ());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::IsPhysicallyClosed (bool& uClosed, bool& vClosed)
    {
    bspsurf_isPhysicallyClosed (&uClosed, &vClosed, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MSBsplineSurface::IsSolid (double tolerance)
    {
    return bspsurf_isSolid (this, tolerance);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::GetUKnots (bvector<double> &data) const
    {
    data.clear ();
    size_t n = GetNumUKnots ();
    data.reserve (n);
    for (size_t i = 0; i < n; i++)
        data.push_back(uKnots[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::GetVKnots (bvector<double> &data) const
    {
    data.clear ();
    size_t n = GetNumVKnots ();
    data.reserve (n);
    for (size_t i = 0; i < n; i++)
        data.push_back(vKnots[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::EvaluatePoint (DPoint3dR xyz, double u, double v) const
    {
    bspsurf_evaluateSurfacePoint (&xyz, NULL, NULL, NULL, u, v, this);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::EvaluatePointAndUnitNormal (DRay3dR ray, double u, double v) const
    {
    DVec3d dXdu, dXdv;
    bspsurf_evaluateSurfacePoint (&ray.origin, NULL, &dXdu, &dXdv, u, v, this);
    double a = ray.direction.NormalizedCrossProduct (dXdu, dXdv);
    return a != 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::EvaluateNormalizedFrame (TransformR frame, double u, double v) const
    {
    DVec3d dXdu, dXdv;
    DPoint3d origin;
    bspsurf_evaluateSurfacePoint (&origin, NULL, &dXdu, &dXdv, u, v, this);
    RotMatrix matrix = RotMatrix::From2Vectors (dXdu, dXdv);
    bool stat = matrix.SquareAndNormalizeColumns (matrix, 0, 1);
    frame.InitFrom (matrix, origin);
    return stat;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::EvaluatePoint (DPoint3dR xyz, DVec3dR dPdU, DVec3dR dPdV, double u, double v) const
    {
    bspsurf_evaluateSurfacePoint (&xyz, NULL, &dPdU, &dPdV, u, v, this);

    double uMax, uMin, vMax, vMin;
    GetKnotRange (uMin, uMax, BSSURF_U);
    GetKnotRange (vMin, vMax, BSSURF_V);
    dPdU.Scale (1/(uMax - uMin));
    dPdV.Scale (1/(vMax - vMin));
    }

    //! Compute a grid of points uniformly spaced in each parameter direction.
    //! @param [in] numUPoint number of points in u direction.
    //! @param [in] numVPoint number of points in v direction
    //! @param [out] uParams u-direction evaluation parameters.
    //! @param [out] vParams v-direction evaluation parameters.
    //! @param [out] gridPoints 
void MSBsplineSurface::EvaluateUniformGrid (size_t numUPoint, size_t numVPoint, bvector<double> &uParamsOut, bvector<double> &vParamsOut, bvector<DPoint3d> &gridPoints) const
    {
    double uMax, uMin, vMax, vMin;
    GetKnotRange (uMin, uMax, BSSURF_U);
    GetKnotRange (vMin, vMax, BSSURF_V);
    uParamsOut.clear ();
    vParamsOut.clear ();
    DoubleOps::AppendInterpolated (uParamsOut, uMin, uMax, numUPoint, true);
    DoubleOps::AppendInterpolated (vParamsOut, vMin, vMax, numVPoint, true);
    bvector<DPoint2d> uvParams;
    for (size_t j = 0; j < numVPoint; j++)
        for (size_t i = 0; i < numUPoint; i++)
            {
            DPoint3d xyz;
            bspsurf_evaluateSurfacePoint (&xyz, NULL, NULL, NULL, uParamsOut[i], vParamsOut[j], this);
            gridPoints.push_back (xyz);
            }
    }

    //! Compute a grid of points uniformly spaced in each parameter direction.
    //! @param [in] numUPoint number of points in u direction.
    //! @param [in] numVPoint number of points in v direction
    //! @param [out] uvParams computed paramters.
    //! @param [out] gridPoints computed points.
void MSBsplineSurface::EvaluateUniformGrid (size_t numUPoint, size_t numVPoint, bvector<DPoint2d> &uvParams, bvector<DPoint3d> &gridPoints) const
    {
    double uMax, uMin, vMax, vMin;
    GetKnotRange (uMin, uMax, BSSURF_U);
    GetKnotRange (vMin, vMax, BSSURF_V);
    bvector<double> uParamsOut;
    bvector<double> vParamsOut;
    uvParams.clear ();
    DoubleOps::AppendInterpolated (uParamsOut, uMin, uMax, numUPoint, true);
    DoubleOps::AppendInterpolated (vParamsOut, vMin, vMax, numVPoint, true);
    for (size_t j = 0; j < numVPoint; j++)
        for (size_t i = 0; i < numUPoint; i++)
            {
            DPoint3d xyz;
            uvParams.push_back (DPoint2d::From (uParamsOut[i], vParamsOut[j]));
            bspsurf_evaluateSurfacePoint (&xyz, NULL, NULL, NULL, uParamsOut[i], vParamsOut[j], this);
            gridPoints.push_back (xyz);
            }
    }


static bool check (int a, int b)
    {
    return a == b;
    }

bool bsiSVD_principalCurvaturesFromParametricDerivatives
(
DVec3dCR dPdu,
DVec3dCR dPdv,
DVec3dCR dPduu,
DVec3dCR dPdvv,
DVec3dCR dPduv,
DVec3dR unitA,
double &curvatureA,
DVec3dR unitB,
double &curvatureB
)
    {            
    // conventional names L, M, N for curvature 2x2 entries ...
    // http://en.wikipedia.org/wiki/Parametric_surface
    // http://www.cs.iastate.edu/~cs577/
    DVec3d cross = DVec3d::FromCrossProduct (dPdu, dPdv);
    double L0 = dPduu.DotProduct (cross);
    double M0 = dPduv.DotProduct (cross);
    double N0 = dPdvv.DotProduct (cross);
    double E = dPdu.DotProduct (dPdu);
    double F = dPdu.DotProduct (dPdv);
    double G = dPdv.DotProduct (dPdv);
    double bb = E * G - F * F;

    if (bb > 0.0)
        {
        double L, M, N;
        double b = sqrt (bb);
        if (  DoubleOps::SafeDivide (L, L0, b, 0.0)
           && DoubleOps::SafeDivide (M, M0, b, 0.0)
           && DoubleOps::SafeDivide (N, N0, b, 0.0)
           )
            {
            RotMatrix F1 = RotMatrix::FromRowValues
                    (
                    E, F, 0,
                    F, G, 0,
                    0, 0, 1);
            RotMatrix F2 = RotMatrix::FromRowValues
                    (
                    L, M ,0,
                    M, N, 0,
                    0, 0, 1
                    );
            RotMatrix F1Inverse;
            if (F1Inverse.InverseOf (F1))
                {
                RotMatrix Q;
                Q.InitProductRotMatrixTransposeRotMatrix (F1Inverse, F2);
                Polynomial::Power::Degree2 quadratic
                    (
                    L * N - M * M,
                    2.0 * M * F -  (L * G + E * N),
                    E * G - F * F
                    );
                double curvature[2];
                double curvatureQ[2];
                DVec2d parametricDirection[2];
                int numRoot = quadratic.RealRoots (curvature);
                int numRoot1 = bsiSVD_realEigenvalues2x2 (Q.form3d[0][0], Q.form3d[0][1], Q.form3d[1][0], Q.form3d[1][1],
                                    curvatureQ, parametricDirection);
                check (numRoot, numRoot1);
                if (numRoot == 2)
                    {
                    int i = 0;
                    int j = 1;
                    if (fabs (curvature[0]) <= fabs (curvature[1]))
                        {
                        i = 1;
                        j = 0;
                        }
                    unitA = DVec3d::FromSumOf (dPdu, parametricDirection[i].x, dPdv, parametricDirection[i].y);
                    unitB = DVec3d::FromSumOf (dPdu, parametricDirection[j].x, dPdv, parametricDirection[j].y);
                    curvatureA = curvature[i];
                    curvatureB = curvature[j];
                    unitA.Normalize ();
                    unitB.Normalize ();
                    return true;
                    }
                }
            }
        }
    
    // fall through in disaster cases?
    // hm.. what if dPdu is zero and dPdv is not?
    RotMatrix R, R1;
    DVec3d normal;
    normal.NormalizedCrossProduct (dPdu, dPdv); // If we got this far, this is probably 0.
    R.InitFromColumnVectors (dPdu, dPdv, normal);
    if (   !R1.SquareAndNormalizeColumns (R, 0, 1)
        && !R1.SquareAndNormalizeColumns (R, 0, 2)
        && !R1.SquareAndNormalizeColumns (R, 1, 2))
        {
        R1 = R; // garbage...
        }
    curvatureA = curvatureB = 0.0;
    DVec3d unitC;
    R1.GetColumns (unitA, unitB, unitC);
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::EvaluatePrincipalCurvature
        (
        DPoint3dR xyz,      //!< [out] point on surface
        DVec3dR unitA,      //!< [out] unit vector tangent to surface and in the direction of the curvature with larger absolute value.
        double &curvatureA,  //!< [out] curvature corresponsing to unitA.
        DVec3dR unitB,      //!< [out] unit vector tangent to surface and in the direction of the curvature with smaller absolute value
        double &curvatureB,  //!< [out] curvature corresponsing to unitB.
        double u,           //!< [in] u parameter on surface
        double v            //!< [in] v parameter on surface
        ) const
    {
    DVec3d dPdu, dPdv, dPduu, dPdvv, dPduv, normal;
    EvaluateAllPartials (xyz, dPdu, dPdv, dPduu, dPdvv, dPduv, normal, u, v);
    return bsiSVD_principalCurvaturesFromParametricDerivatives (
                    dPdu, dPdv, dPduu, dPdvv, dPduv,
                    unitA, curvatureA, unitB, curvatureB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::MakeRational ()
    {
    return bspsurf_makeRationalSurface (this, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::SwapUV ()
    {
    return bspsurf_swapUV (this, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::MakeReversed (int direction)
    {
    return bspsurf_reverseSurface (this, this, direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::TransformSurface (Transform const *transformP)
    {
    return bspsurf_transformSurface (this, this, transformP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::TransformSurface (TransformCR transform)
    {
    return bspsurf_transformSurface (this, this, &transform);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MSBsplineSurface::IsPlane () const
    {
    Transform frame;
    if (!GetPrincipalExtents (frame))
        return false;
    double a = frame.MatrixColumnMagnitude (0)
             + frame.MatrixColumnMagnitude (1);
    double c = frame.MatrixColumnMagnitude (2);
    return DoubleOps::AlmostEqual (a, a + c);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MSBsplineSurface::IsSameStructure (MSBsplineSurfaceCR other) const
    {
    size_t numPoles = GetNumPoles ();
    if (numPoles != other.GetNumPoles ())
        return false;
    if (GetNumUPoles () != other.GetNumUPoles ())
        return false;
    if (GetNumVPoles () != other.GetNumVPoles ())
        return false;

    if (GetNumVKnots () != other.GetNumVKnots ())
        return false;
    if (GetNumVKnots () != other.GetNumVKnots ())
        return false;

    if (GetVOrder () != other.GetVOrder ())
        return false;
    if (GetVOrder () != other.GetVOrder ())
        return false;
    if (GetIsUClosed () != other.GetIsUClosed ())
        return false;
    if (GetIsVClosed () != other.GetIsVClosed ())
        return false;

    if (HasWeights () != other.HasWeights ())
        return false;

    if (GetNumBounds () != other.GetNumBounds ())
        return false;
    if (GetNumBounds () != 0)
        {

        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MSBsplineSurface::IsSameStructureAndGeometry (MSBsplineSurfaceCR other, double tolerance) const
    {
    size_t numPoles = GetNumPoles ();
    if (!IsSameStructure (other))
        return false;
    if (HasWeights ())
        {
        for (size_t i = 0; i < numPoles; i++)
            if (! bsputil_isSameRationalPointTolerance (&this->poles[i], this->weights[i],
                                                &other.poles[i], other.weights[i], tolerance))
                return false;
        }
    else
        {
        for (size_t i = 0; i < numPoles; i++)
            if (! bsputil_isSamePointTolerance (&this->poles[i], &other.poles[i], tolerance))
                return false;
        }
    size_t numUKnots = GetNumUKnots ();
    size_t numVKnots = GetNumVKnots ();
    for (size_t i = 0; i < numUKnots; i++)
        if (!MSBsplineCurve::AreSameKnots (uKnots[i], other.uKnots[i]))
            return false;
    for (size_t i = 0; i < numVKnots; i++)
        if (!MSBsplineCurve::AreSameKnots (vKnots[i], other.vKnots[i]))
            return false;

    if (GetNumBounds () > 0)
        {
        CurveVectorPtr boundaryA = GetUVBoundaryCurves (false, true);
        CurveVectorPtr boundaryB = GetUVBoundaryCurves (false, true);
        if (!boundaryA->IsSameStructureAndGeometry (*boundaryB))
            return false;
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MSBsplineSurface::GetPrincipalExtents (TransformR frame) const
    {
    bvector<DPoint3d> myPoles;
    GetUnWeightedPoles (myPoles);
    return DPoint3dOps::PrincipalExtents (myPoles, frame);
    }


// setup control vars for loop over edges with possible wraparound
//     for (; i1 < i1Limit; i0 = i1, i1++)
//        {  process i0..i1}
//  In open case the i0,i1 pairs are 01,12,etc
//  In closed case, i0 starts at numVertex-1 and i1 at 0 so the closure edge is returned first.
void SetupChordLoop (size_t &i0, size_t &i1, size_t &i1Limit, size_t numVertex, bool closed)
    {
    i1Limit = numVertex;
    if (closed)
        {
        i1 = 0;
        i0 = numVertex - 1;
        }
    else
        {
        i0 = 0;
        i1 = 1;
        }

    }
bool    MSBsplineSurface::IsPlanarBilinear (double angleTol) const
    {
    if (GetUOrder () != 2 || GetVOrder () != 2)
        return false;
    size_t i0A, i1A, i0, i1, i1Limit;
    size_t j0, j1, j1Limit;
    SetupChordLoop (i0A, i1A, i1Limit, GetNumUPoles (), GetIsUClosed ());
    SetupChordLoop (j0, j1, j1Limit, GetNumVPoles (), GetIsVClosed ());
    for (; j1 < j1Limit; j0 = j1++)
        {
        for (i0 = i0A, i1 = i1A;i1 < i1Limit; i0 = i1++)
            {
            DPoint3d xyz00 = GetUnWeightedPole (i0, j0);
            DPoint3d xyz10 = GetUnWeightedPole (i1, j0);
            DPoint3d xyz01 = GetUnWeightedPole (i0, j1);
            DPoint3d xyz11 = GetUnWeightedPole (i1, j1);
            DBilinearPatch3d patch (xyz00, xyz10, xyz01, xyz11);
            if (!patch.IsPlanar (angleTol))
                return false;
            }
        }
    return true;
    }
bool    MSBsplineSurface::IsPlanarBilinear () const
    {
    return IsPlanarBilinear (Angle::SmallAngle ());
    }


static bool IsSmallDelta (DRange3dCR deltaRange, DRange3dCR range, double relativeTolerance)
    {
    double maxAbs = 1.0 + range.LargestCoordinate ();
    double delta = deltaRange.low.Distance (deltaRange.high);
    return delta < relativeTolerance * maxAbs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::IsBidirectionalTranslation (double relativeTolerance) const
    {
    size_t numChains = GetIntNumVPoles ();
    size_t numPerChain = GetIntNumUPoles ();
    DRange3d deltaRange, chainRange;
    DPoint3d poleB;
    DVec3d delta;
    if (relativeTolerance < Angle::SmallAngle ())
        relativeTolerance = Angle::SmallAngle ();
    for (size_t j = 1; j < numChains; j++)
        {
        deltaRange.Init ();
        chainRange.Init ();
        for (size_t i = 0; i < numPerChain; i++)
            {
            poleB = GetPole (j, i);
            delta.DifferenceOf (poleB, GetPole (0, i));
            deltaRange.Extend (delta);
            chainRange.Extend (poleB);
            }
        if (!IsSmallDelta (deltaRange, chainRange, relativeTolerance))
            return false;            
        }
    return true;        
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::NormalizeSurface ()
    {
    bspsurf_normalizeSurface (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineSurface::FractionToKnot (double f, int direction) const
    {
    double knot0, knot1;
    GetKnotRange (knot0, knot1, direction);
    return knot0 + f * (knot1 - knot0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineSurface::KnotToFraction (double knot, int direction) const
    {
    double knot0, knot1;
    GetKnotRange (knot0, knot1, direction); 
    return (knot - knot0) / (knot1 - knot0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::AddKnot (double unnormalizedKnotValue, int newMultiplicity, int direction)
    {
    double min, max;
    GetKnotRange (min, max, direction);
    //double fraction = KnotToFraction (unnormalizedKnotValue, direction);
    return bspknot_addKnotSurface (this, unnormalizedKnotValue, KNOT_TOLERANCE_BASIS, newMultiplicity, false, direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::NormalizeKnots ()
    {
    bspknot_normalizeKnotVector (uKnots, uParams.numPoles, uParams.order, uParams.closed);
    bspknot_normalizeKnotVector (vKnots, vParams.numPoles, vParams.order, vParams.closed);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::MakeClosed (int direction)
    {
    return bsprsurf_closeSurface (this, this, direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::MakeOpen (double uv, int direction)
    {
    double fraction = KnotToFraction (uv, direction);
    return bsprsurf_openSurface (this, this, fraction, direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::MakeBezier (MSBsplineSurfaceP outSurface)
    {
    return bsprsurf_makeBezierSurface (outSurface, this);
    }

void GetBasePole (size_t select, bool closed, double *knots, size_t order, size_t numPoles, size_t &basePole, size_t &maxBasePole)
    {
    basePole = select;
    maxBasePole = numPoles - order;
    if (closed)
        {
        maxBasePole = numPoles - 1;
        if (mdlBspline_knotsShouldBeOpened (knots, (int)(2*order), NULL, NULL, 0, (int)order, (int)closed))
            {
            basePole += numPoles;
            basePole -= order / 2;
            }        
        while (basePole >= numPoles)
            basePole -= numPoles;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::GetIntervalCounts (size_t &numU, size_t &numV) const
    {

    ptrdiff_t numUKnots = GetNumUKnots ();      // these are complete sets -- not closed issues.
    ptrdiff_t numVKnots = GetNumVKnots ();

    ptrdiff_t uDegree = GetUOrder () - 1;
    ptrdiff_t vDegree = GetVOrder () - 1;

    ptrdiff_t u = numUKnots - 2 * uDegree - 1;
    ptrdiff_t v = numVKnots - 2 * vDegree - 1;

    if (u <= 0 || v <= 0)
        {
        numU = numV = 0;
        return false;
        }
    numU = (size_t)u;
    numV = (size_t)v;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::ClearBeziers(bvector<bvector<MSBsplineSurface>>& beziers)
    {
    // Bug #800802: the objects in this 2D array are not refcounted!
    size_t vCount = beziers.size();
    size_t uCount = (vCount > 0) ? beziers.front().size() : 0;
    for (size_t j = 0; j < vCount; ++j)
        {
        for (size_t i = 0; i < uCount; ++i)
            {
            beziers[j][i].ReleaseMem();
            }
        }
    beziers.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::MakeBeziers (
bvector<bvector<MSBsplineSurface>>& beziers,
bvector<bvector<DRange2d>> *knotRanges
) const
    {
    ClearBeziers(beziers);
    if (knotRanges)
        knotRanges->clear ();
    MSBsplineSurface tmpSurf;
    tmpSurf.CopyFrom (*this);
    if (GetIsUClosed ())
        tmpSurf.MakeOpen (GetUKnot (GetIntUOrder () - 1), BSSURF_U);
    if (GetIsVClosed ())
        tmpSurf.MakeOpen (GetVKnot (GetIntVOrder () - 1), BSSURF_V);

    int jPatch = 0, num = vParams.numPoles - vParams.order + 1;
    BSurfPatch patch;
    bvector<DPoint3d> cps;
    bvector<double> cws;
    DPoint3d point;
    memset (&patch, 0, sizeof (patch));

    while (jPatch<num)
        {
        beziers.push_back(bvector<MSBsplineSurface>());
        if (knotRanges)
            knotRanges->push_back(bvector<DRange2d>());
        for (size_t iPatch = 0; tmpSurf.GetPatch (patch, iPatch, jPatch); iPatch++)
            {
            if (!patch.isNullU && !patch.isNullV)
                {
                MSBsplineSurface surf;
                surf.Zero ();
                beziers.back ().push_back(surf);
                cps.clear ();
                cws.clear ();

                if (rational)
                    {
                    for (size_t k = 0; k< patch.xyzw.size (); k++)
                        {
                        point.Init (patch.xyzw[k].x, patch.xyzw[k].y, patch.xyzw[k].z);
                        cps.push_back (point);
                        cws.push_back (patch.xyzw[k].w);
                        }
                    }
                else
                    {
                    for (size_t k = 0; k< patch.xyzw.size (); k++)
                        {
                        point.Init (patch.xyzw[k].x, patch.xyzw[k].y, patch.xyzw[k].z);
                        cps.push_back (point);
                        }
                    }
                beziers.back().back().Populate (cps, rational ? &cws : NULL, NULL, uParams.order, uParams.order, false,
                                                    NULL, vParams.order, vParams.order, false, true);
                beziers.back().back().display = display;
                if (knotRanges)
                    {
                    DRange2d uvRange = DRange2d::From(
                        patch.uMin, patch.vMin,
                        patch.uMax, patch.vMax);
                    knotRanges->back ().push_back (uvRange);
                    }
                }
            }
        if (beziers.back ().size () == 0)
            {
            beziers.pop_back ();
            if (knotRanges)
                knotRanges->pop_back ();
            }
        jPatch++;
        }

    tmpSurf.ReleaseMem ();
    return beziers.size () > 0 ? MSB_SUCCESS : MSB_ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::ElevateDegree (int newDegree, int edge)
    {
    return bsprsurf_elevateDegreeSurface (this, this, newDegree, edge);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::CleanKnots ()
    {
    return mdlBspline_cleanSurfaceKnots (this, this);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void weightSumDPoint4d
(
DPoint4d&       Cw, 
double          alpha, 
DPoint4d        Aw, 
double          beta, 
DPoint4d        Bw 
)
    {
    Cw.x = alpha * Aw.x + beta * Bw.x;
    Cw.y = alpha * Aw.y + beta * Bw.y;
    Cw.z = alpha * Aw.z + beta * Bw.z;
    Cw.w = alpha * Aw.w + beta * Bw.w;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static double distanceDPoint4d
(
DPoint4d    Pw, 
DPoint4d    Qw 
)
    {
    double  distance2; 

    distance2  = (Pw.x - Qw.x)*(Pw.x - Qw.x) + (Pw.y - Qw.y)*(Pw.y - Qw.y);
    distance2 += (Pw.z - Qw.z)*(Pw.z - Qw.z);
    distance2 += (Pw.w - Qw.w)*(Pw.w - Qw.w);

    return sqrt(distance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static double getRemovalBound
(
MSBsplineSurfaceCP  pSurf, 
int                 r,  // index of removal knot
int                 s,  // times want to remove.
int                 dir // direction of knots to remove: 1=u, 2=v, 3=both.
)
    {
    //bool    error = false;

    int   i, j, row, col, ii, jj, first, last, off, n, m;

    int  p, q, num;

    double    *U = pSurf->uKnots, *V = pSurf->vKnots, del, omd, dw;

    DPoint4d A;
    
    int uNum = pSurf->uParams.numPoles; 
    n = uNum - 1;
    int vNum = pSurf->vParams.numPoles;
    m = vNum - 1;
    p = pSurf->uParams.order - 1;
    q = pSurf->vParams.order - 1;
    num = uNum*vNum;
    
    i = std::max (p, q);
    bvector<double> alf (2*i+1), oma (2*i+1), bet (2*i+1), omb (2*i+1);
    bvector<DPoint4d> tmpPoles (2*i+1);
    bvector<DPoint4d> polesWeighted (num);
    
    for (int k=0; k<num; k++)
        {
        if (pSurf->rational)
            polesWeighted[k].InitFrom (*(&pSurf->poles[k]), pSurf->weights[k]);
        else
            polesWeighted[k].InitFrom (*(&pSurf->poles[k]), 1.0);
        }

    /* Remove knot in the requested direction */

    double mr = -1.0;
    
    if( dir == 1)
        {
        /* Remove in u-direction */

        first = r-p;    
        last  = r-s;    
        off   = first-1;

        /* Save some parameters */

        i = first;
        j = last;
        while( (j-i) > 0 )
            {
            alf[i-first] = (U[i+p+1] - U[i])/(U[r] - U[i]);
            oma[i-first] = 1.0 - alf[i-first];
            bet[j-first] = (U[j+p+1] - U[j])/(U[j+p+1] - U[r]);
            omb[j-first] = 1.0 - bet[j-first];
            i++;  j--;  
            }

        del = (U[r] - U[i])/(U[i+p+1] - U[i]);
        omd = 1.0 - del;

        /* Update maximum error for the requested rows */

        for( col=0; col<=m; col++ )
            {
            i  = first;
            j  = last;
            ii = 1;
            jj = last-off;

            tmpPoles[0] = polesWeighted[off + col*uNum];
            tmpPoles[last+1-off] = polesWeighted[(last+1) + col*uNum];

            /* Get new control points for one removal step */

            while( (j-i) > 0 )
                {
                weightSumDPoint4d(tmpPoles[ii], alf[i-first], polesWeighted[i+col*uNum], oma[i-first], tmpPoles[ii-1]);
                weightSumDPoint4d(tmpPoles[jj], bet[j-first], polesWeighted[j+col*uNum], omb[j-first], tmpPoles[jj+1]);
                i ++;  j --;  
                ii++;  jj--;
                }

            /* Compute the error */

            if( (j-i) < 0 )
                {
                dw = distanceDPoint4d(tmpPoles[ii-1],tmpPoles[jj+1]);
                }
            else
                {
                weightSumDPoint4d(A, del, tmpPoles[jj+1], omd, tmpPoles[ii-1]);
                dw = distanceDPoint4d(polesWeighted[i+col*uNum], A);
                }

            if( dw > mr )  mr = dw;
            } 
        }
        
    else
        {
        /* Remove in v-direction */

        first = r-q;    
        last  = r-s;    
        off   = first-1;

        /* Save some parameters */

        i = first;
        j = last;
        while( (j-i) > 0 )
            {
            alf[i-first] = (V[i+q+1]-V[i])/(V[r]-V[i]);
            oma[i-first] = 1.0-alf[i-first];
            bet[j-first] = (V[j+q+1]-V[j])/(V[j+q+1]-V[r]);
            omb[j-first] = 1.0-bet[j-first];
            i++;  j--;  
            }

        del = (V[r]-V[i])/(V[i+q+1]-V[i]);
        omd = 1.0-del;

        /* Update maximum error for the requested rows */

        for( row=0; row<=n; row++ )
            {
            i  = first;
            j  = last;
            ii = 1;
            jj = last-off;

            tmpPoles[0] = polesWeighted[row + off*uNum];
            tmpPoles[last+1-off] = polesWeighted[row + (last+1)*uNum];

            /* Get new control points for one removal step */

            while( (j-i) > 0 )
                {
                weightSumDPoint4d(tmpPoles[ii], alf[i-first], polesWeighted[row + i*uNum], oma[i-first], tmpPoles[ii-1]);
                weightSumDPoint4d(tmpPoles[jj], bet[j-first], polesWeighted[row + j*uNum], omb[j-first], tmpPoles[jj+1]);
                i ++;  j --;  
                ii++;  jj--;
                }

            /* Compute the error */

            if( (j-i) < 0 )
                {
                dw = distanceDPoint4d(tmpPoles[ii-1], tmpPoles[jj+1]);
                }
            else
                {
                weightSumDPoint4d(A, del, tmpPoles[jj+1], omd, tmpPoles[ii-1]);
                dw = distanceDPoint4d(polesWeighted[row+i*uNum], A);
                }

            if( dw > mr )  mr = dw;
            }
        }
         
    return mr;
    }
    
/*---------------------------------------------------------------------------------**//**
* @description Remove knots from surface.
* @remarks Individual oversaturated knots are flattened and reduced to maximum legal multiplicity before global knot removal.
* @param tol           IN      max surface deviation after knot removal
* @param dir           IN      direction of knots to remove: 1=u, 2=v, 3=both. We should use dir = 3 carefully. According to my testing,
*                              using dir = 3 removed less knots than RemoveKnotsBounded (1, tol) and RemoveKnotsBounded (2, tol).
* @return SUCCESS if knot removal succeeded
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus RemoveKnots
(
MSBsplineSurfaceP   pSurf,
int                 dir,
double              tol
)
    {
    const double WMIN = 1e-5;
    const double WMAX = 200.0;
    bool rmf, wfl, rat = false;
    int krm, i, j, k, l, row, col, ii, jj, first, last, off, fout, n, m, r, s,
            ru = 0, su = 0, rv = 0, sv = 0, ns, ms;
    int p, q, noremu = 0, noremv = 0;
    double *UQ = pSurf->uKnots, *VQ = pSurf->vKnots, lam = 0, oml = 0, wmin, wmax, pmax, al, be, bu = 0, bv = 0, wi, wj; 
    
    n = pSurf->uParams.numPoles - 1;
    m = pSurf->vParams.numPoles - 1;
    p = pSurf->uParams.order - 1;
    q = pSurf->vParams.order - 1;
    r = pSurf->uParams.NumberAllocatedKnots () - 1;
    s = pSurf->vParams.NumberAllocatedKnots () - 1;
    ns = n;
    ms = m;
    int num = pSurf->uParams.numPoles * pSurf->vParams.numPoles;
    bvector<DPoint4d> polesWeighted (num);
        
    for (k=0; k<num; k++)
        {
        if (pSurf->rational)
            polesWeighted[k].InitFrom (*(&pSurf->poles[k]), pSurf->weights[k]);
        else
            polesWeighted[k].InitFrom (*(&pSurf->poles[k]), 1.0);
        }
    
    if( pSurf->rational )
        {
        surfaceMinWeightAndMaxMagnitude(wmin, pmax, pSurf);
        tol = (tol * wmin) / (1.0 + pmax);
        rat = true;
        }
    
    i = std::max (p, q);
    j = std::max (n, m);
    
    int uNum = n + 1;
    //int vNum = m + 1;
    //int iNum = 2*i+1;
    int jNum = j + 1;
    int uKnot = r+1;
    int vKnot = s+1;
    bvector<double> alf (2*i+1), oma (2*i+1), bet (2*i+1), omb (2*i+1);
    bvector<DPoint4d> tmpPoles ((j+1)*(2*i+1));
    bvector<double> bru, brv;
    bvector<int> sru, nru, srv, nrv;
    bvector<double> er (uKnot*vKnot), te (uKnot*vKnot);
    
    if( dir == 1 || dir == 3 )
        {
        bru.resize (r+1);
        sru.resize (r+1);
        nru.resize (r+1);
        for( i=0; i<=r; i++ )
            {
            bru[i] = fc_hugeVal;
            sru[i] = 0;
            nru[i] = 0;
            }
            
        ru = p+1;
        while( ru <= n )
            {
            i = ru;
            while( ru <= n && fabs (UQ[ru] - UQ[ru+1]) < RELATIVE_BSPLINE_KNOT_TOLERANCE )  
                ru++;
            sru[ru] = ru-i+1;

            bru[ru] = getRemovalBound (pSurf, ru, sru[ru], 1);
            ru++;
            }
        }

    if( dir == 2 || dir == 3 )
        {
        brv.resize (s+1);
        srv.resize (s+1);
        nrv.resize (s+1);
        for( j=0; j<=s; j++ )
            {
            brv[j] = fc_hugeVal;
            srv[j] = 0;
            nrv[i] = 0;
            }
        
        rv = q+1;
        while( rv <= m )
            {
            i = rv;
            while( rv <= m && fabs (VQ[rv] - VQ[rv+1])< RELATIVE_BSPLINE_KNOT_TOLERANCE )  
                rv++;
            srv[rv] = rv-i+1;

            brv[rv] = getRemovalBound (pSurf, rv, srv[rv], 2);

            rv++;
            }
        }
    
    for( i=0; i<uKnot*vKnot; i++ )
        er[i] = 0.0;
    
    /* Try to remove each knot */


    while( true )
        {
        /* Find knot with smallest error */

        if( dir == 1 || dir == 3 )
            {
            bu = bru[p+1];
            noremu = nru[p+1];
            su = sru[p+1];
            ru = p+1;
            for( i=p+2; i<=r-p-1; i++ )
                {
                if( bru[i] < bu )  
                    {
                    bu = bru[i];  
                    su = sru[i];  
                    ru = i;
                    noremu = nru[i];
                    }
                }
            }
        
        if( dir == 2 || dir == 3 )
            {
            bv = brv[q+1];
            noremv = nrv[q+1]; 
            sv = srv[q+1];
            rv = q+1;
            for( j=q+2; j<=s-q-1; j++ )
                {
                if( brv[j] < bv )  
                    {
                    bv = brv[j];  
                    sv = srv[j];  
                    rv = j;
                    noremv = nrv[j];
                    }
                }
            }
        
        if( dir == 1 )  {  if( noremu == 1 )  break;  }  else
        if( dir == 2 )  {  if( noremv == 1 )  break;  }  else
        if( dir == 3 )  
            {  
            if( noremu == 1 && noremv == 1)  
                break;
            }

        if( dir == 3 )  
            {
            if( bu < bv && noremu != 1)  
                krm = 1;  
            else  
                krm = 2;
            }
        else
            {
            krm = dir;
            }
            
        if (krm == 1)
            {
            /* Remove in the u-direction */

            if( (p+su)%2 )
                {
                k   = (p+su+1)/2;
                l   = ru-k+p+1;
                al  = (UQ[ru]-UQ[ru-k  ])/(UQ[ru-k+p+1]-UQ[ru-k  ]);
                be  = (UQ[ru]-UQ[ru-k+1])/(UQ[ru-k+p+2]-UQ[ru-k+1]);
                lam = al/(al+be);
                oml = 1.0-lam; 
                } 
            else
                {
                k = (p+su)/2;
                l = ru-k+p;
                }

            /* Check the error */

            rmf = true;
            for( i=ru-k; i<=l; i++ )
                {
                if( fabs (UQ[i] - UQ[i+1] ) > RELATIVE_BSPLINE_KNOT_TOLERANCE )
                    {
                    for( j=q; j<=s-q-1; j++ )
                        {
                        if( fabs (VQ[j] - VQ[j+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE )  
                            { 
                            te[i+j*uKnot] = er[i+j*uKnot]+bu;
                            if( te[i+j*uKnot] > tol )  
                                {  
                                rmf = false;  
                                break;  
                                }
                            }
                        }
                    }    
                if( rmf == false )  
                    break;
                }

            /* If error test passed -> update error vector and remove knot */

            if( rmf == true )
                {
                for( i=ru-k; i<=l; i++ )  
                    {
                    for( j=q; j<=s-q-1; j++ )
                        {
                        if( fabs (UQ[i] - UQ[i+1] ) > RELATIVE_BSPLINE_KNOT_TOLERANCE
                              &&  fabs (VQ[j] - VQ[j+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE ) 
                            {
                            er[i+j*uKnot] = te[i+j*uKnot];
                            }
                        }
                    }

                fout  = (2*ru-su-p)/2; 
                first = ru-p;              
                last  = ru-su;
                off   = first-1;

                /* Save some parameters */

                i = first;
                j = last;
                while( (j-i) > 0 )
                    {
                    alf[i-first] = (UQ[i+p+1]-UQ[i])/(UQ[ru]-UQ[i]);
                    oma[i-first] = 1.0-alf[i-first];
                    bet[j-first] = (UQ[j+p+1]-UQ[j])/(UQ[j+p+1]-UQ[ru]);
                    omb[j-first] = 1.0-bet[j-first];
                    i++;  j--;  
                    }

                /* Remove the knot for each row */

                wfl = true;
                for( col=0; col<=m; col++ )
                    {
                    i  = first;
                    j  = last;
                    ii = 1;
                    jj = last-off;

                    tmpPoles[col] = polesWeighted[off + col*uNum];
                    tmpPoles[col + (last+1-off)*jNum] = polesWeighted[(last+1) + col*uNum];

                    /* Get new control points for one removal step */

                    while( (j-i) > 0 )
                        {
                        weightSumDPoint4d(tmpPoles[col + ii*jNum], alf[i-first], polesWeighted[i + col*uNum], oma[i-first], tmpPoles[col + (ii-1)*jNum]);
                        weightSumDPoint4d(tmpPoles[col + jj*jNum], bet[j-first], polesWeighted[j + col*uNum], omb[j-first], tmpPoles[col + (jj+1)*jNum]);
                        i ++;  j --;  
                        ii++;  jj--;
                        }

                    /* Check for disallowed weights */

                    if( rat == 1 )
                        {
                        i    = first;
                        j    = last;
                        wmin = fc_hugeVal;
                        wmax = fc_1em15;
                        while( (j-i) > 0 )
                            {
                            wi = tmpPoles[col + (i-off)*jNum].w;
                            wj = tmpPoles[col + (j-off)*jNum].w;

                            if( wi < wmin )  wmin = wi;
                            if( wj < wmin )  wmin = wj;
                            if( wi > wmax )  wmax = wi;
                            if( wj > wmax )  wmax = wj;
                            i++;  j--;
                            }
                        if( wmin < WMIN || wmax > WMAX )  
                            {  
                            wfl = false;  
                            break;  
                            }
                        }

                if( (p+su)%2 )
                    {
                    weightSumDPoint4d(tmpPoles[col + (jj+1)*jNum], lam, tmpPoles[col + (jj+1)*jNum], oml, tmpPoles[col + (ii-1)*jNum]);
                    }            

                } /* End for each row */

                /* See if weights are in the allowable range  */

                if( wfl == false )  
                    {  
                    nru[ru] = 1;
                    continue;  
                    }
                else
                    {
                    /* Save control points */

                    for( col=0; col<=m; col++ )
                        {
                        i = first;
                        j = last;
                        while( (j-i) > 0 )
                            {
                            polesWeighted[i + col*uNum] = tmpPoles[col + (i-off)*jNum];
                            polesWeighted[j + col*uNum] = tmpPoles[col + (j-off)*jNum];
                            i++;  j--;
                            }
                        }
                    }

                    /* Successful removal -> shift down some entinties */

                if( su == 1 )  
                    {
                    for( j=q; j<=s-q-1; j++ )
                        {
                        if( fabs (VQ[j] - VQ[j+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE ) 
                            {
                            er[(ru-1)+j*uKnot] = std::max (er[(ru-1)+j*uKnot], er[ru+j*uKnot]);
                            }
                        }
                    }

                if( su > 1 )  sru[ru-1] = sru[ru]-1;

                for( i=ru+1; i<=r; i++ )
                    {
                    bru[i-1] = bru[i];
                    sru[i-1] = sru[i];
                    UQ [i-1] = UQ [i];
                    for( j=q; j<=m; j++ )  
                        er[(i-1) + j*uKnot] = er[i+j*uKnot];
                    }

                for( col=0; col<=m; col++ )
                    {
                    for( i=fout+1; i<=n; i++ )
                        {
                        polesWeighted[(i-1) + col*uNum] = polesWeighted[i + col*uNum];
                        }
                    }

                n--;  r--;
                pSurf->uParams.numPoles = n+1;
                for (int cpi = 0; cpi < pSurf->uParams.numPoles; cpi++)
                    for (int cpj = 0; cpj < pSurf->vParams.numPoles; cpj++)
                    {
                    pSurf->poles[cpi+cpj*pSurf->uParams.numPoles].x = polesWeighted[cpi+cpj*(ns+1)].x;
                    pSurf->poles[cpi+cpj*pSurf->uParams.numPoles].y = polesWeighted[cpi+cpj*(ns+1)].y;
                    pSurf->poles[cpi+cpj*pSurf->uParams.numPoles].z = polesWeighted[cpi+cpj*(ns+1)].z;
                    if (rat)
                        pSurf->weights[cpi+cpj*pSurf->uParams.numPoles] = polesWeighted[cpi+cpj*(ns+1)].w;
                    }

                /* If no more internal knots -> finished */

                if( dir == 1 )  {  if( n == p              )  break;  }  else
                if( dir == 3 )  {  if( n == p && m == q )  break;  }

                /* Update error bounds */

                k = std::max (ru-p, p+1);
                l = std::min (n, ru+p-su);
                for( i=k; i<=l; i++ )
                    {
                    if( fabs (UQ[i] - UQ[i+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE && nru[i] != 1 ) 
                        { 
                        bru[i] = getRemovalBound (pSurf, i, sru[i], 1);
                        }
                    }

                if( dir == 3 )
                    {
                    for( j=q+1; j<=s-q-1; j++ )
                        {
                        if( fabs (VQ[j] - VQ[j+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE && nrv[j] != 1 ) 
                            { 
                            brv[j] = getRemovalBound (pSurf, j, srv[j], 2);
                            }
                        }
                    }
                }
            else
                {
                /* Knot is not removable */

                nru[ru] = 1;
                }
            }
        if (krm == 2)
            {
            if( (q+sv)%2 )
                {
                k   = (q+sv+1)/2;
                l   = rv-k+q+1;
                al  = (VQ[rv]-VQ[rv-k  ])/(VQ[rv-k+q+1]-VQ[rv-k  ]);
                be  = (VQ[rv]-VQ[rv-k+1])/(VQ[rv-k+q+2]-VQ[rv-k+1]);
                lam = al/(al+be);
                oml = 1.0-lam; 
                }
            else
                {
                k = (q+sv)/2;
                l = rv-k+q;
                }

            /* Check the error */

            rmf = true;
            for( j=rv-k; j<=l; j++ )
                {
                if( fabs (VQ[j] - VQ[j+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE )
                    {
                    for( i=p; i<=r-p-1; i++ )
                        {
                        if( fabs (UQ[i] - UQ[i+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE )  
                            { 
                            te[i + j*uKnot] = er[i + j*uKnot]+bv;
                            if( te[i + j*uKnot] > tol )  
                                {  
                                rmf = false;  
                                break;  
                                }
                            }
                        }
                    }  
                if( rmf == false )  
                    break;
                }

            /* If error test passed -> update error vector and remove knot */

            if( rmf == true )
                {
                for( j=rv-k; j<=l; j++ )  
                    {
                    for( i=p; i<=r-p-1; i++ )
                        {
                        if( fabs (VQ[j] - VQ[j+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE
                              && fabs (UQ[i] - UQ[i+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE ) 
                            {
                            er[i + j*uKnot] = te[i + j*uKnot];
                            }
                        }
                    }

                fout  = (2*rv-sv-q)/2; 
                first = rv-q;              
                last  = rv-sv;
                off   = first-1;

                /* Save some parameters */

                i = first;
                j = last;
                while( (j-i) > 0 )
                    {
                    alf[i-first] = (VQ[i+q+1]-VQ[i])/(VQ[rv]-VQ[i]);
                    oma[i-first] = 1.0-alf[i-first];
                    bet[j-first] = (VQ[j+q+1]-VQ[j])/(VQ[j+q+1]-VQ[rv]);
                    omb[j-first] = 1.0-bet[j-first];
                    i++;  j--;  
                    }

                /* Remove knot for each column */

                wfl = true;
                for( row=0; row<=n; row++ )
                    {
                    i  = first;
                    j  = last;
                    ii = 1;
                    jj = last-off;

                    tmpPoles[row] = polesWeighted[row + off*uNum];
                    tmpPoles[row + (last+1-off)*jNum] = polesWeighted[row + (last+1)*uNum];

                    /* Get new control points for one removal step */

                    while( (j-i) > 0 )
                        {
                        weightSumDPoint4d(tmpPoles[row + ii*jNum], alf[i-first], polesWeighted[row + i*uNum], oma[i-first], tmpPoles[row + (ii-1)*jNum]);
                        weightSumDPoint4d(tmpPoles[row + jj*jNum], bet[j-first], polesWeighted[row + j*uNum], omb[j-first], tmpPoles[row + (jj+1)*jNum]);
                        i ++;  j --;  
                        ii++;  jj--;
                        }

                    /* Check for disallowed weights */

                    if( rat == 1 )
                        {
                        i    = first;
                        j    = last;
                        wmin = fc_hugeVal;
                        wmax = fc_1em15;
                        while( (j-i) > 0 )
                            {
                            wi = tmpPoles[row + (i-off)*jNum].w;
                            wj = tmpPoles[row + (j-off)*jNum].w;

                            if( wi < wmin )  wmin = wi;
                            if( wj < wmin )  wmin = wj;
                            if( wi > wmax )  wmax = wi;
                            if( wj > wmax )  wmax = wj;
                            i++;  j--;
                            }
                        if( wmin < WMIN || wmax > WMAX )  
                            {  
                            wfl = false;  
                            break;  
                            }
                        }

                    /* Save new control points */

                    if( (q+sv)%2 )
                        {
                        weightSumDPoint4d(tmpPoles[row + (jj+1)*jNum], lam, tmpPoles[row + (jj+1)*jNum], oml, tmpPoles[row + (ii-1)*jNum]);
                        }            

                    } /* End for each column */

                /* See if weights are in the allowable range  */

                if( wfl == false )  
                    {  
                    nrv[rv] = 1;
                    continue;  
                    }
                else
                    {
                    /* Save control points */ 

                    for( row=0; row<=n; row++ )
                        {
                        i = first;
                        j = last;
                        while( (j-i) > 0 )
                            {
                            polesWeighted[row + i*uNum] = tmpPoles[row + (i-off)*jNum];
                            polesWeighted[row + j*uNum] = tmpPoles[row + (j-off)*jNum];
                            i++;  j--;
                            }
                        }
                    }

                /* Successful removal -> shift down some entinties */

                if( sv == 1 )  
                    {
                    for( i=p; i<=r-p-1; i++ )
                        {
                        if( fabs (UQ[i] - UQ[i+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE ) 
                            {
                            er[i + (rv-1)*uKnot] = std::max (er[i + (rv-1)*uKnot], er[i + rv*uKnot]);
                            }
                        }
                    }

                if( sv > 1 )  srv[rv-1] = srv[rv]-1;

                for( j=rv+1; j<=s; j++ )
                    {
                    brv[j-1] = brv[j];
                    srv[j-1] = srv[j];
                    VQ [j-1] = VQ [j];
                    for( i=p; i<=n; i++ )  
                        er[i + (j-1)*uKnot] = er[i + j*uKnot];
                    }

                for( row=0; row<=n; row++ )
                    {
                    for( j=fout+1; j<=m; j++ )
                        {
                        polesWeighted[row + (j-1)*uNum] = polesWeighted[row + j*uNum];
                        }
                    }

                m--;  s--;
                pSurf->vParams.numPoles = m+1;
                for (int cpi = 0; cpi < pSurf->uParams.numPoles; cpi++)
                    for (int cpj = 0; cpj < pSurf->vParams.numPoles; cpj++)
                    {
                    pSurf->poles[cpi+cpj*pSurf->uParams.numPoles].x = polesWeighted[cpi+cpj*pSurf->uParams.numPoles].x;
                    pSurf->poles[cpi+cpj*pSurf->uParams.numPoles].y = polesWeighted[cpi+cpj*pSurf->uParams.numPoles].y;
                    pSurf->poles[cpi+cpj*pSurf->uParams.numPoles].z = polesWeighted[cpi+cpj*pSurf->uParams.numPoles].z;
                    if (rat)
                        pSurf->weights[cpi+cpj*pSurf->uParams.numPoles] = polesWeighted[cpi+cpj*pSurf->uParams.numPoles].w;
                    }
                    
                /* If no more internal knots -> finished */

                if( dir == 1  )  {  if( m == q          )  break;  }  else
                if( dir == 3 )  {  if( n == p && m == q )  break;  }

                /* Update error bounds */

                k = std::max (rv-q, q+1);
                l = std::min (m, rv+q-sv);
                for( j=k; j<=l; j++ )
                    {
                    if( fabs (VQ[j] - VQ[j+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE && nrv[j] != 1 ) 
                        { 
                        brv[j] = getRemovalBound (pSurf, j, srv[j], 2);
                        }
                    }

                if( dir == 3 )
                    {
                    for( i=p+1; i<=r-p-1; i++ )
                        {
                        if( fabs (UQ[i] - UQ[i+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE && nru[i] != 1 ) 
                            { 
                            bru[i] = getRemovalBound(pSurf, i, sru[i], 1);
                            }
                        }
                    }
                }
            else
                {
                /* Knot is not removable */

                nrv[rv] = 1;
                }
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::RemoveKnotsBounded (int dir, double tol)
    {
    StatusInt   status = ERROR;
    int         uDegree, vDegree;
    int         iKnot, iMult, mult;
    double      knot;
    bool        bOverSaturatedUKnot = false, bOverSaturatedVKnot = false;

    uDegree = uParams.order - 1;
    vDegree = vParams.order - 1;
    int r = uParams.NumberAllocatedKnots () - 1;
    int s = vParams.NumberAllocatedKnots () - 1;
    
    if (dir != 2)
        {
        // flatten first u-knot if oversaturated
        if (uKnots[uDegree + 1] - uKnots[uDegree] < RELATIVE_BSPLINE_KNOT_TOLERANCE)
            {
            bOverSaturatedUKnot = true;
            knot = uKnots[0];
            for (iKnot = 1; iKnot <= r && uKnots[iKnot] - knot < RELATIVE_BSPLINE_KNOT_TOLERANCE; iKnot++)
                uKnots[iKnot] = knot;
            }

        // flatten last u-knot if oversaturated
        if (uKnots[r - uDegree] - uKnots[r - uDegree - 1] < RELATIVE_BSPLINE_KNOT_TOLERANCE)
            {
            bOverSaturatedUKnot = true;
            knot = uKnots[r];
            for (iKnot = r - 1; iKnot >= 0 && knot - uKnots[iKnot] < RELATIVE_BSPLINE_KNOT_TOLERANCE; iKnot--)
                uKnots[iKnot] = knot;
            }

        // flatten interior u-knot if oversaturated
        for (iKnot = uDegree + 1; iKnot < r - uDegree; iKnot += mult)
            {
            knot = uKnots[iKnot];
            for (mult = 1; iKnot + mult <= r && uKnots[iKnot + mult] - knot < RELATIVE_BSPLINE_KNOT_TOLERANCE; mult++);
            if (mult > uDegree)
                {
                for (iMult = 1; iMult < mult; iMult++)
                    uKnots[iKnot + iMult] = knot;
                bOverSaturatedUKnot = true;
                }
            }
        }

    if (dir != 1)
        {
        // flatten first v-knot if oversaturated
        if (vKnots[vDegree + 1] - vKnots[vDegree] < RELATIVE_BSPLINE_KNOT_TOLERANCE)
            {
            bOverSaturatedVKnot = true;
            knot = vKnots[0];
            for (iKnot = 1; iKnot <= s && vKnots[iKnot] - knot < RELATIVE_BSPLINE_KNOT_TOLERANCE; iKnot++)
                vKnots[iKnot] = knot;
            }

        // flatten last v-knot if oversaturated
        if (vKnots[s - vDegree] - vKnots[s - vDegree - 1] < RELATIVE_BSPLINE_KNOT_TOLERANCE)
            {
            bOverSaturatedVKnot = true;
            knot = vKnots[s];
            for (iKnot = s - 1; iKnot >= 0 && knot - vKnots[iKnot] < RELATIVE_BSPLINE_KNOT_TOLERANCE; iKnot--)
                vKnots[iKnot] = knot;
            }

        // flatten interior v-knot if oversaturated
        for (iKnot = vDegree + 1; iKnot < s - vDegree; iKnot += mult)
            {
            knot = vKnots[iKnot];
            for (mult = 1; iKnot + mult <= s && vKnots[iKnot + mult] - knot < RELATIVE_BSPLINE_KNOT_TOLERANCE; mult++);
            if (mult > vDegree)
                {
                for (iMult = 1; iMult < mult; iMult++)
                    vKnots[iKnot + iMult] = knot;
                bOverSaturatedVKnot = true;
                }
            }
        }
    
    if (SUCCESS == (status = CleanKnots ()))
        status = RemoveKnots (this, dir, tol);
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::InitFromPointsAndOrder (int uOrder, int vOrder, int uNumPoles, int vNumPoles, DPoint3dP pPoints)
    {
    MSBsplineStatus status = MSB_ERROR;

    if (pPoints != NULL && uNumPoles > 1 && vNumPoles > 1)
        {
        Zero ();
        uParams.order = uOrder;
        vParams.order = vOrder;
        uParams.numPoles = uNumPoles;
        vParams.numPoles = vNumPoles;
        if (SUCCESS == (status = Allocate ()))
            {
            memcpy (poles, pPoints, (uNumPoles*vNumPoles)*sizeof(DPoint3d));
            status = bspknot_computeKnotVector (uKnots, &uParams, NULL);
            status = bspknot_computeKnotVector (vKnots, &vParams, NULL);
            }
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::CopyClosed (MSBsplineSurfaceCR source, int edge)
    {
    return bsprsurf_closeSurface (this, (MSBsplineSurfaceCP)&source, edge);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::CopyOpen (MSBsplineSurfaceCR source, double unnormalizedKnot, int edge)
    {
    return bsprsurf_openSurface (this, (MSBsplineSurfaceCP)&source, source.KnotToFraction (unnormalizedKnot, edge), edge);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::CopyReversed (MSBsplineSurfaceCR source, int edge)
    {
    return bspsurf_reverseSurface (this, (MSBsplineSurfaceCP)&source, edge);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MSBsplineSurface::IsDegenerateEdge (int edgeCode, double tolerance)
    {
    return bspsurf_isDegenerateEdge (edgeCode, this, tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::EvaluateAllPartials (DPoint3dR xyz, DVec3dR dPdU, DVec3dR dPdV,
                                DVec3dR dPdUU, DVec3dR dPdVV, DVec3dR dPdUV,
                                    DVec3dR norm, double u, double v) const
    {
    bspsurf_computePartials (&xyz, NULL, &dPdU, &dPdV, &dPdUU, &dPdVV, &dPdUV, &norm, u, v, const_cast <MSBsplineSurfaceP> (this));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::UnWeightPoles()
    {
    if (rational && weights != NULL)
        bsputil_unWeightPoles (poles, poles, weights, uParams.numPoles * vParams.numPoles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::WeightPoles()
    {
    if (rational && weights != NULL)
        bsputil_weightPoles (poles, poles, weights, uParams.numPoles * vParams.numPoles);
    }


static bool CheckConditionalPointer (bool expectPointer, void *pointer)
    {
    if (expectPointer)
        return pointer != NULL;
    return pointer == NULL;
    }
static bool ValidOrder (int order)
    {
    return order >= 2 && order <= MAX_ORDER; //TFS#22859
    }    
bool MSBsplineSurface::HasValidPoleAllocation () const { return poles != NULL;}
bool MSBsplineSurface::HasValidWeightAllocation () const { return CheckConditionalPointer (rational != 0, weights);}
bool MSBsplineSurface::HasValidBoundaryAllocation () const
    {
    if (!CheckConditionalPointer (numBounds != 0, boundaries))
        return false;
    for (size_t i = 0; i < (size_t)numBounds; i++)
        {
        // every boundary must have points ...
        if (boundaries[i].numPoints <= 0)
            return false;
        if (boundaries[i].points == NULL)
            return false;
        }
    return true;
    }
    
bool MSBsplineSurface::HasValidKnotAllocation () const {return uKnots != NULL && vKnots != NULL;}
bool MSBsplineSurface::HasValidOrder () const {return ValidOrder (uParams.order) && ValidOrder (vParams.order);}
bool MSBsplineSurface::HasValidPoleCounts () const {return uParams.numPoles >= uParams.order && vParams.numPoles >= vParams.order;}

bool MSBsplineSurface::HasValidCountsAndAllocations () const
    {
    return
           HasValidPoleAllocation ()
        && HasValidKnotAllocation ()
        && HasValidWeightAllocation ()
        && HasValidBoundaryAllocation ()
        && HasValidOrder ()
        && HasValidPoleCounts ()
        ;
    }


bool MSBsplineCurve::HasValidPoleAllocation () const { return poles != NULL;}
bool MSBsplineCurve::HasValidWeightAllocation () const { return CheckConditionalPointer (rational != 0, weights);}
bool MSBsplineCurve::HasValidKnotAllocation () const {return knots != NULL;}
bool MSBsplineCurve::HasValidOrder () const {return ValidOrder (params.order);}
bool MSBsplineCurve::HasValidPoleCounts () const {return params.numPoles >= params.order;}

bool MSBsplineCurve::HasValidCountsAndAllocations () const
    {
    return
           HasValidPoleAllocation ()
        && HasValidKnotAllocation ()
        && HasValidWeightAllocation ()
        && HasValidOrder ()
        && HasValidPoleCounts ()
        ;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDPoint2d MSBsplineSurface::ControlPolygonFractionToKnot (size_t i, size_t j, double u, double v) const
    {
    size_t numI = GetNumUPoles ();
    size_t numJ = GetNumVPoles ();
    size_t orderU = GetUOrder ();
    size_t orderV = GetVOrder();
    bool stat = true;
    if (i + 1 >= numI)
        {
        i = numI - 1;
        stat = false;
        }
    if (j + 1 >= numJ)
        {
        j = numJ - 1;
        stat = false;
        }
    return ValidatedDPoint2d
        (
        DPoint2d::From
            (
            DoubleOps::Interpolate (uKnots[orderU + i - 1], u, uKnots[orderU + i]),
            DoubleOps::Interpolate (vKnots[orderV + j - 1], v, vKnots[orderV + j])
            ),
        stat
        ); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDPoint4d MSBsplineSurface::ControlPolygonFractionToControlPolygonDPoint4d (size_t i, size_t j, double u, double v) const
    {
    size_t numI = GetNumUPoles ();
    size_t numJ = GetNumVPoles ();
    bool stat = true;
    if (i + 1 >= numI)
        {
        i = numI - 1;
        stat = false;
        }
    if (j + 1 >= numJ)
        {
        j = numJ - 1;
        stat = false;
        }
    return ValidatedDPoint4d
        (
        DPoint4d::FromInterpolate
            (
            DPoint4d::FromInterpolate (GetPoleDPoint4d (i,j), u, GetPoleDPoint4d (i+1,j)),
            v,
            DPoint4d::FromInterpolate (GetPoleDPoint4d (i,j+1), u, GetPoleDPoint4d (i+1,j+1))
            ),
        stat
        ); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDPoint3d MSBsplineSurface::ControlPolygonFractionToControlPolygonDPoint3d (size_t i, size_t j, double u, double v) const
    {
    auto xyzw = ControlPolygonFractionToControlPolygonDPoint4d (i,j,u,v);
    DPoint3d xyz;
    bool stat1 = xyzw.Value ().GetProjectedXYZ (xyz);
    return ValidatedDPoint3d (xyz, xyzw.IsValid () && stat1);
    }


/*----------------------------------------------------------------------+
Return larger of
 (1) abstol
 (2) larger of relTol or RELATIVE_RESOLUTION times
          larger of dataSize or SMALLEST_ALLOWED_REFERENCE_SIZE
+----------------------------------------------------------------------*/
static double  bsputil_sizeToTol
(
double dataSize,
double absTol,
double relTol
)
    {
    double tol;
    dataSize = fabs (dataSize);
    if (dataSize < SMALLEST_ALLOWED_REFERENCE_SIZE)
        dataSize = SMALLEST_ALLOWED_REFERENCE_SIZE;
    if (relTol < RELATIVE_RESOLUTION)
        relTol = RELATIVE_RESOLUTION;
    tol = dataSize * relTol;
    if (tol < absTol)
        tol = absTol;
    return tol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static double  expandMaxAbsDoubleArray
(
double *pData,
int     count,
double  dMax
)
    {
    int  i;
    double dCurr;
    for (i = 0; i < count; i++)
        {
        dCurr = fabs (pData[i]);
        if (dCurr > dMax)
            dMax = dCurr;
        }
    return dMax;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineSurface::Resolution (double abstol, double reltol) const
    {
    return bsputil_sizeToTol (
                expandMaxAbsDoubleArray
                    (
                    (double*)poles,
                    3 * uParams.numPoles * vParams.numPoles,
                    SMALLEST_ALLOWED_REFERENCE_SIZE
                    ),
                abstol,
                reltol);
    }
Public GEOMDLLIMPEXP double  MSBsplineCurve::Resolution (double abstol, double reltol) const
    {
    return bsputil_sizeToTol (
                expandMaxAbsDoubleArray
                    (
                    (double *)poles,
                    3 * params.numPoles,
                    0.0
                    ),
                abstol,
                reltol);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::Resolution () const
    {
    return bsputil_sizeToTol (
                expandMaxAbsDoubleArray
                    (
                    (double *)poles,
                    3 * params.numPoles,
                    SMALLEST_ALLOWED_REFERENCE_SIZE
                    ),
                0.0,
                0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineSurface::Resolution () const
    {
    return bsputil_sizeToTol (
                expandMaxAbsDoubleArray
                    (
                    (double*)poles,
                    3 * uParams.numPoles * vParams.numPoles,
                    SMALLEST_ALLOWED_REFERENCE_SIZE
                    ),
                0.0,
                0.0);
    }
bool MinimalValidate(BsplineParam const &params)
    {
    if (params.order < 2
        || params.order > MAX_BSORDER)
        return false;
    if (params.numPoles < params.order)
        return false;
    return true;
    }

bool MSBsplineSurface::IsValidGeometry(GeometryValidatorPtr &validator) const
    {
    if (!validator.IsValid())
        return true;
    if (!MinimalValidate (uParams))
        return false;
    if (!MinimalValidate(vParams))
        return false;
    int numUPoles = GetIntNumUPoles ();
    int numVPoles = GetIntNumVPoles ();
    int numUKnots = GetIntNumUKnots ();
    int numVKnots = GetIntNumVKnots();

    // int uOrder = GetIntUOrder ();
    // int vOrder = GetIntVOrder();

    int numPoles = numUPoles * numVPoles;
    if (poles == nullptr)
        return false;
    if (uKnots == nullptr)
        return false;
    if (vKnots == nullptr)
        return false;

    if (!validator->IsValidGeometry(poles, (uint32_t)numPoles))
        return false;
    if (!validator->IsValidGeometry(uKnots, (uint32_t)numUKnots))
        return false;
    if (!validator->IsValidGeometry(vKnots, (uint32_t)numVKnots))
        return false;

    if ((weights == nullptr) != (rational == 0))
        return false;
    static double s_minWeight = 1.0e-5;
    static double s_maxWeight = 1.0e5;
    if (weights != nullptr && !validator->IsValidGeometry(weights, numPoles, s_minWeight, s_maxWeight))
        return false;

    // enforce non-decreasing knots:
    for (int k0 = 0; k0 + 1 < numUKnots; k0++)
        if (uKnots[k0] > uKnots[k0 + 1])
            return false;

    for (int k0 = 0; k0 + 1 < numVKnots; k0++)
        if (vKnots[k0] > vKnots[k0 + 1])
            return false;

    return true;
    }
