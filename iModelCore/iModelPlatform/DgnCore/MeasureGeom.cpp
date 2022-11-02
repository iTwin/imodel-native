/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MeasureGeomCollector::GetOutputTransform (TransformR transform, SimplifyGraphic const& graphic)
    {
    transform.InitProduct (m_invCurrTransform, graphic.GetLocalToWorldTransform()); // Account for supplied mdlCurrTrans...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::GetPreFlattenTransform (TransformR transform, SimplifyGraphic const& graphic)
    {
    if (m_inFlatten || m_preFlattenTransform.IsIdentity ())
        return false; // No flatten transform or recursive call where flatten already applied to geometry...

    Transform   worldToLocal;

    worldToLocal.InverseOf (graphic.GetLocalToWorldTransform()); // Account for current transform...
    transform = Transform::FromProduct (worldToLocal, m_preFlattenTransform, graphic.GetLocalToWorldTransform());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsP MeasureGeomCollector::_GetFacetOptionsP ()
    {
    return (m_facetOptions.IsValid () ? m_facetOptions.get () : NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MeasureGeomCollector::AccumulateVolumeSums
(
double          volumeB,
double          areaB,
double          closureErrorB,
DPoint3dCR      centroidB,
DPoint3dCR      momentB2,
double          iXYB,
double          iXZB,
double          iYZB
)
    {
    DPoint3d    momentB1;

    m_amountSum += volumeB;
    m_volumeSum += volumeB;
    m_areaSum += areaB;
    m_closureError += fabs (closureErrorB);

    // Body B global first moments...
    momentB1.Scale ((DVec3dCR) centroidB, volumeB); // Body B first moments around global origin.

    m_moment1.x += momentB1.x;
    m_moment1.y += momentB1.y;
    m_moment1.z += momentB1.z;

    m_moment2.x += momentB2.x + (centroidB.y * momentB1.y + centroidB.z * momentB1.z);
    m_moment2.y += momentB2.y + (centroidB.x * momentB1.x + centroidB.z * momentB1.z);
    m_moment2.z += momentB2.z + (centroidB.x * momentB1.x + centroidB.y * momentB1.y);

    m_iXY += iXYB + centroidB.x * momentB1.y;
    m_iXZ += iXZB + centroidB.x * momentB1.z;
    m_iYZ += iYZB + centroidB.y * momentB1.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MeasureGeomCollector::AccumulateAreaSums
(
double          areaB,
double          perimeterB,
DPoint3dCR      centroidB,
DPoint3dCR      momentB2,
double          iXYB,
double          iXZB,
double          iYZB
)
    {
    DPoint3d    momentB1;

    m_amountSum += areaB;
    m_areaSum += areaB;
    m_perimeterSum += perimeterB;

    // Body B global first moments...
    momentB1.Scale ((DVec3dCR) centroidB, areaB); // Body B first moments around global origin.

    m_moment1.x += momentB1.x;
    m_moment1.y += momentB1.y;
    m_moment1.z += momentB1.z;

    m_moment2.x += momentB2.x + (centroidB.y * momentB1.y + centroidB.z * momentB1.z);
    m_moment2.y += momentB2.y + (centroidB.x * momentB1.x + centroidB.z * momentB1.z);
    m_moment2.z += momentB2.z + (centroidB.x * momentB1.x + centroidB.y * momentB1.y);

    m_iXY += iXYB + centroidB.x * momentB1.y;
    m_iXZ += iXZB + centroidB.x * momentB1.z;
    m_iYZ += iYZB + centroidB.y * momentB1.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void reorientPrincipalMoments (DVec3dCR principalMoments, RotMatrixCR localToGlobal,
        double &Iyyzz, double &Ixxzz, double &Ixxyy,
        double &Ixy, double &Iyz, double &Ixz
        )
    {
    RotMatrix A = RotMatrix::FromRowValues
        (
        principalMoments.x, 0, 0,
        0, principalMoments.y, 0,
        0, 0, principalMoments.z
        );
    RotMatrix QA, QAQT;
    QA.InitProduct (localToGlobal, A);
    QAQT.InitProductRotMatrixRotMatrixTranspose (QA, localToGlobal);
    Iyyzz = QAQT.form3d[0][0];
    Ixxzz = QAQT.form3d[1][1];
    Ixxyy = QAQT.form3d[2][2];

    Ixy = -QAQT.form3d[0][1];
    Iyz = -QAQT.form3d[0][2];
    Ixz = -QAQT.form3d[1][2];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MeasureGeomCollector::AccumulateLengthSums
(
double          lengthB,
DPoint3dCR      centroidB,
DPoint3dCR      momentB2,
double          iXYB,
double          iXZB,
double          iYZB
)
    {
    DPoint3d    momentB1;

    m_amountSum += lengthB;
    m_lengthSum += lengthB;

    // Body B global first moments...
    momentB1.Scale ((DVec3dCR) centroidB, lengthB); // Body B first moments around global origin.

    m_moment1.x += momentB1.x;
    m_moment1.y += momentB1.y;
    m_moment1.z += momentB1.z;

    m_moment2.x += momentB2.x + (centroidB.y * momentB1.y + centroidB.z * momentB1.z);
    m_moment2.y += momentB2.y + (centroidB.x * momentB1.x + centroidB.z * momentB1.z);
    m_moment2.z += momentB2.z + (centroidB.x * momentB1.x + centroidB.y * momentB1.y);

    m_iXY += iXYB + centroidB.x * momentB1.y;
    m_iXZ += iXZB + centroidB.x * momentB1.z;
    m_iYZ += iYZB + centroidB.y * momentB1.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MeasureGeomCollector::AccumulateLengthSums (DMatrix4dCR products)
    {
    double      length;
    DVec3d      centroid, momentA;
    RotMatrix   axes;
    Transform   localToWorld = Transform::FromIdentity ();

    if (products.ConvertInertiaProductsToPrincipalWireMoments (localToWorld, length, centroid, axes, momentA))
        {
        double  iXY, iXZ, iYZ;
        DVec3d  momentB;

        reorientPrincipalMoments (momentA, axes, momentB.x, momentB.y, momentB.z, iXY, iXZ, iYZ);

        AccumulateLengthSums (length, centroid, momentB, iXY, iXZ, iYZ);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::DoAccumulateLengths (CurveVectorCR curves, SimplifyGraphic& graphic)
    {
    Transform   flattenTransform;

    if (GetPreFlattenTransform (flattenTransform, graphic))
        {
        AutoRestore<bool>   saveInFlatten (&m_inFlatten, true);
        CurveVectorPtr      tmpCurves = curves.Clone ();

        tmpCurves->TransformInPlace (flattenTransform);

        return DoAccumulateLengths (*tmpCurves, graphic);
        }

    Transform   outputTransform;

    GetOutputTransform (outputTransform, graphic);

    CurveVectorPtr  tmpCurves = curves.Clone ();

    if (!outputTransform.IsIdentity ())
        tmpCurves->TransformInPlace (outputTransform);

    DMatrix4d   products;

    if (tmpCurves->ComputeSecondMomentWireProducts (products))
        AccumulateLengthSums (products);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::DoAccumulateAreas (CurveVectorCR curves, SimplifyGraphic& graphic)
    {
    if (!curves.IsAnyRegionType ())
        return true; // Not valid type for operation...

    Transform   flattenTransform;

    if (GetPreFlattenTransform (flattenTransform, graphic))
        {
        AutoRestore<bool>   saveInFlatten (&m_inFlatten, true);
        CurveVectorPtr      tmpCurves = curves.Clone ();

        tmpCurves->TransformInPlace (flattenTransform);

        return DoAccumulateAreas (*tmpCurves, graphic);
        }

    double      area, scale;
    DVec3d      centroid, momentA, momentB;
    RotMatrix   axes;
    DMatrix4d   products;
    Transform   outputTransform;

    GetOutputTransform (outputTransform, graphic);

    if (curves.ComputeSecondMomentAreaProducts (products) &&
        products.ConvertInertiaProductsToPrincipalAreaMoments (outputTransform, area, centroid, axes, momentA) &&
        outputTransform.IsRigidScale (scale))
        {
        double  iXY, iXZ, iYZ;

        reorientPrincipalMoments (momentA, axes, momentB.x, momentB.y, momentB.z, iXY, iXZ, iYZ);

        double  length = scale * curves.Length ();

		AccumulateAreaSums (area, length, centroid, momentB, iXY, iXZ, iYZ);

        return true;
        }

    return false; // Try facets...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::_ProcessCurveVector (CurveVectorCR curves, bool isFilled, SimplifyGraphic& graphic)
    {
    switch (m_opType)
        {
        case AccumulateLengths:
            return DoAccumulateLengths (curves, graphic);

        case AccumulateVolumes:
            return true; // Not valid type for operation...

        default:
            return DoAccumulateAreas (curves, graphic);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::DoAccumulateAreas (ISolidPrimitiveCR primitive, SimplifyGraphic& graphic)
    {
    // What does it mean to flatten a solid primitive? Call it an error -- maybe it will get meshed and the mesh will be flattened?
    Transform flattenTransform;

    if (GetPreFlattenTransform (flattenTransform, graphic))
        return false;

    // Compute area moments directly from ISolidPrimitive instead of always converting to BRep or facets...
    double area;
    DVec3d areaCentroid;
    RotMatrix areaAxes;
    DVec3d areaMoments;

    Transform   outputTransform;
    GetOutputTransform (outputTransform, graphic);
    if (outputTransform.IsIdentity ())
        {
        if (!primitive.ComputePrincipalAreaMoments (area, areaCentroid, areaAxes, areaMoments))
            return false;
        }
    else
        {
        ISolidPrimitivePtr localPrimitive = primitive.Clone ();
        localPrimitive->TransformInPlace (outputTransform);
        if (!localPrimitive->ComputePrincipalAreaMoments (area, areaCentroid, areaAxes, areaMoments))
            return false;
        }

    double      iXY, iXZ, iYZ;

    reorientPrincipalMoments (areaMoments, areaAxes, areaMoments.x, areaMoments.y, areaMoments.z, iXY, iXZ, iYZ);

    AccumulateAreaSums (area, 0.0, areaCentroid, areaMoments, iXY, iXZ, iYZ);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::DoAccumulateVolumes (ISolidPrimitiveCR primitive, SimplifyGraphic& graphic)
    {
    if (!primitive.GetCapped ())
        return true; // Not valid type for operation...

    double      amount = 0.0, area = 0.0;
    DVec3d      centroid, moments, momentB, areaMoments, areaCentroid;
    RotMatrix   axes, areaAxes;
    Transform   outputTransform;

    GetOutputTransform (outputTransform, graphic);

    ISolidPrimitivePtr  localPrimitive = primitive.Clone ();

    localPrimitive->TransformInPlace (outputTransform);

    bool status = localPrimitive->ComputePrincipalMoments(amount, centroid, axes, moments) && localPrimitive->ComputePrincipalAreaMoments(area, areaCentroid, areaAxes, areaMoments);
    if (!status)
        return false; // try facets...

    double  iXY, iXZ, iYZ;

    reorientPrincipalMoments (moments, axes, momentB.x, momentB.y, momentB.z, iXY, iXZ, iYZ);

    AccumulateVolumeSums (amount, area, 0.0, centroid, momentB, iXY, iXZ, iYZ);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::_ProcessSolidPrimitive (ISolidPrimitiveCR primitive, SimplifyGraphic& graphic)
    {
    switch (m_opType)
        {
        case AccumulateLengths:
            return true; // Not valid type for operation...

        case AccumulateVolumes:
            return DoAccumulateVolumes (primitive, graphic);

        default:
            return DoAccumulateAreas (primitive, graphic);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::DoAccumulateAreas (MSBsplineSurfaceCR surface, SimplifyGraphic& graphic)
    {
    Transform   flattenTransform;

    if (GetPreFlattenTransform (flattenTransform, graphic))
        {
        AutoRestore<bool>   saveInFlatten (&m_inFlatten, true);
        MSBsplineSurfacePtr tmpSurface = surface.CreateCopyTransformed (flattenTransform);

        return DoAccumulateAreas (*tmpSurface, graphic);
        }

#ifdef CheckConvergence
    DMatrix4d   testProducts [15];
    double      tolerances[] = {0.1, 0.01, 0.001};

    int         k = 0;
    int         numEval = 0;

    for (int i = 1; i < 6; i++)
        {
        for (int j = 0; j< 3; j++, k++)
            {
            if (surface.ComputeSecondMomentAreaProducts (testProducts[k], tolerances[j], i, numEval))
                printf ("Product %g %d %.15lg", tolerances[j], i, products[k].coff[3][3]);
            }
        }

    k = 0;

    for (int i = 1; i < 6; i++)
        {
        for (int j = 0; j< 3;j++, k++)
            printf ("Gauss Rule %d (numeval %d, tol %g): %.15lg (error %7.1e)\n", i, numEval, tolerances[j], testProducts[k].coff[3][3], abs(testProducts[k].coff[3][3] - testProducts[14].coff[3][3]));

        printf ("\n");
        }
#endif

    double      area, scale;
    DVec3d      centroid, momentA, momentB;
    RotMatrix   axes;
    DMatrix4d   products;
    Transform   outputTransform;

    GetOutputTransform (outputTransform, graphic);

    if (surface.ComputeSecondMomentAreaProducts (products) &&
        products.ConvertInertiaProductsToPrincipalAreaMoments (outputTransform, area, centroid, axes, momentA) &&
        outputTransform.IsRigidScale(scale))
        {
        double  iXY, iXZ, iYZ;

        reorientPrincipalMoments(momentA, axes, momentB.x, momentB.y, momentB.z, iXY, iXZ, iYZ);

        CurveVectorPtr bounds = surface.GetUnstructuredBoundaryCurves(0.0, true, true);

        double  length = (bounds.IsValid() ? (scale * bounds->Length()) : 0.0);

		AccumulateAreaSums(area, length, centroid, momentB, iXY, iXZ, iYZ);

        return true;
        }

    return false; // Try facets...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::_ProcessSurface (MSBsplineSurfaceCR surface, SimplifyGraphic& graphic)
    {
    switch (m_opType)
        {
        case AccumulateLengths:
        case AccumulateVolumes:
            return true; // Not valid type for operation...

        default:
            return DoAccumulateAreas (surface, graphic);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::DoAccumulateAreas (PolyfaceQueryCR meshQuery, SimplifyGraphic& graphic)
    {
    Transform   flattenTransform;

    if (GetPreFlattenTransform (flattenTransform, graphic))
        {
        AutoRestore<bool>   saveInFlatten (&m_inFlatten, true);
        PolyfaceHeaderPtr   tmpMeshQuery = PolyfaceHeader::New ();

        tmpMeshQuery->CopyFrom (meshQuery);
        tmpMeshQuery->Transform (flattenTransform);

        return DoAccumulateAreas (*tmpMeshQuery, graphic);
        }

    Transform   outputTransform;

    GetOutputTransform (outputTransform, graphic);

    PolyfaceHeaderPtr  meshData = PolyfaceHeader::New ();

    meshData->CopyFrom (meshQuery);

    if (!outputTransform.IsIdentity ())
        meshData->Transform (outputTransform);

    double      area;
    DVec3d      centroid, moments, momentB;
    RotMatrix   axes;

    if (!meshData->ComputePrincipalAreaMoments (area, centroid, axes, moments))
        return true; // Don't output edges

    double      iXY, iXZ, iYZ;

    reorientPrincipalMoments (moments, axes, momentB.x, momentB.y, momentB.z, iXY, iXZ, iYZ);

    // Special case? Boundary length (if visible edge length == length of visible edge with single adjacent face...i.e. looks like CurveVector?!?).
    AccumulateAreaSums (area, 0.0, centroid, moments, iXY, iXZ, iYZ);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::DoAccumulateVolumes (PolyfaceQueryCR meshQuery, SimplifyGraphic& graphic)
    {
    bool        useRaggedMeshLogic = false;
    Transform   outputTransform;

    GetOutputTransform (outputTransform, graphic);

    PolyfaceHeaderPtr  meshData = PolyfaceHeader::New ();

    meshData->CopyFrom (meshQuery);

    double      amount;
    DVec3d      centroid, moments, momentB;
    RotMatrix   axes;

    if (!meshData->IsClosedByEdgePairing ())
        {
        if (meshData->ComputePrincipalMomentsAllowMissingSideFacets (amount, centroid, axes, moments, true))
            {
            useRaggedMeshLogic = true;
            }
        else
            {
            meshData->Compress ();
            if (!meshData->IsClosedByEdgePairing ())
                return true; // Not valid type for operation...
            }
        }

    if (!outputTransform.IsIdentity ())
        meshData->Transform (outputTransform);

    if (useRaggedMeshLogic)
        {
        // It was already calculated, but maybe transformed since ...
        if (!meshData->ComputePrincipalMomentsAllowMissingSideFacets (amount, centroid, axes, moments, true))
            return true;
        }
    else
        {
        if (!meshData->ComputePrincipalMoments (amount, centroid, axes, moments, true))
            return true; // Don't output edges
        }

    double      iXY, iXZ, iYZ;

    reorientPrincipalMoments (moments, axes, momentB.x, momentB.y, momentB.z, iXY, iXZ, iYZ);

    double      periphery = meshData->SumFacetAreas ();

    AccumulateVolumeSums (amount, periphery, 0.0, centroid, momentB, iXY, iXZ, iYZ);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::_ProcessPolyface (PolyfaceQueryCR meshQuery, bool isFilled, SimplifyGraphic& graphic)
    {
    switch (m_opType)
        {
        case AccumulateLengths:
            return true; // Not valid type for operation...

        case AccumulateVolumes:
            return DoAccumulateVolumes (meshQuery, graphic);

        default:
            return DoAccumulateAreas (meshQuery, graphic);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static double getBRepTolerance (IBRepEntityCR entity, IFacetOptionsPtr& facetOptions)
    {
    double defaultTolerance = 1.0; // 0.99 accuracy. Avoids making the property dialog too slow when bspline surfaces are involved...
    double requestedTolerance = 0.0;

    if (facetOptions.IsValid())
        {
        requestedTolerance = facetOptions->GetChordTolerance();

        if (requestedTolerance > 0.0)
            {
            Transform   invTrans;

            invTrans.InverseOf(entity.GetEntityTransform());
            invTrans.ScaleDoubleArrayByXColumnMagnitude(&requestedTolerance, 1);
            }
        }

    return (requestedTolerance > 0.0 && requestedTolerance < defaultTolerance) ? requestedTolerance : defaultTolerance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::DoAccumulateLengths (IBRepEntityCR entity, SimplifyGraphic& graphic)
    {
    if (IBRepEntity::EntityType::Solid == entity.GetEntityType())
        return true; // Not valid type for operation...

    Transform   flattenTransform;

    if (GetPreFlattenTransform (flattenTransform, graphic))
        {
        CurveVectorPtr curve = T_HOST.GetBRepGeometryAdmin()._BodyToCurveVector(entity);

        if (!curve.IsValid())
            return true;

        return DoAccumulateLengths (*curve, graphic);
        }

    IBRepEntityPtr entityPtr = entity.CreateInstance();

    if (!entityPtr.IsValid())
        return true;

    Transform   outputTransform;

    GetOutputTransform(outputTransform, graphic);
    entityPtr->PreMultiplyEntityTransformInPlace(outputTransform);

    double      tolerance = getBRepTolerance(*entityPtr, m_facetOptions);
    double      length, iXY, iXZ, iYZ;
    DPoint3d    centroid, moments;

    if (SUCCESS == T_HOST.GetBRepGeometryAdmin()._GetLengthProperties(*entityPtr, length, centroid, moments, iXY, iXZ, iYZ, tolerance))
        AccumulateLengthSums(length, centroid, moments, iXY, iXZ, iYZ);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::DoAccumulateAreas (IBRepEntityCR entity, SimplifyGraphic& graphic)
    {
    if (IBRepEntity::EntityType::Wire == entity.GetEntityType ())
        return true; // Not valid type for operation...

    Transform   flattenTransform;

    if (GetPreFlattenTransform(flattenTransform, graphic))
        {
        CurveVectorPtr curve = T_HOST.GetBRepGeometryAdmin()._BodyToCurveVector(entity);

        if (!curve.IsValid())
            return false; // Facet and flatten...

        return DoAccumulateAreas (*curve, graphic);
        }

    IBRepEntityPtr entityPtr = entity.CreateInstance();

    if (!entityPtr.IsValid())
        return true;

    Transform   outputTransform;

    GetOutputTransform(outputTransform, graphic);
    entityPtr->PreMultiplyEntityTransformInPlace(outputTransform);

    double      tolerance = getBRepTolerance(*entityPtr, m_facetOptions);
    double      area, periphery, iXY, iXZ, iYZ;
    DPoint3d    centroid, moments;

    if (SUCCESS == T_HOST.GetBRepGeometryAdmin()._GetAreaProperties(*entityPtr, area, periphery, centroid, moments, iXY, iXZ, iYZ, tolerance))
        AccumulateAreaSums(area, periphery, centroid, moments, iXY, iXZ, iYZ);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::DoAccumulateVolumes (IBRepEntityCR entity, SimplifyGraphic& graphic)
    {
    if (IBRepEntity::EntityType::Solid != entity.GetEntityType())
        return true; // Not valid type for operation...

    IBRepEntityPtr entityPtr = entity.CreateInstance();

    if (!entityPtr.IsValid())
        return true;

    Transform   outputTransform;

    GetOutputTransform(outputTransform, graphic);
    entityPtr->PreMultiplyEntityTransformInPlace(outputTransform);

    double      tolerance = getBRepTolerance(*entityPtr, m_facetOptions);
    double      volume, area, iXY, iXZ, iYZ;
    DPoint3d    centroid, moments;

    if (SUCCESS == T_HOST.GetBRepGeometryAdmin()._GetVolumeProperties(*entityPtr, volume, area, centroid, moments, iXY, iXZ, iYZ, tolerance))
        AccumulateVolumeSums(volume, area, 0.0, centroid, moments, iXY, iXZ, iYZ);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeasureGeomCollector::_ProcessBody (IBRepEntityCR entity, SimplifyGraphic& graphic)
    {
    switch (m_opType)
        {
        case AccumulateLengths:
            return DoAccumulateLengths (entity, graphic);

        case AccumulateVolumes:
            return DoAccumulateVolumes (entity, graphic);

        default:
            return DoAccumulateAreas (entity, graphic);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MeasureGeomCollector::_OutputGraphics (ViewContextR context)
    {
    if (!m_geomPrimitive.IsValid())
        return;

    auto builder = context.CreateSceneGraphic(m_geomTransform);

    switch (m_geomPrimitive->GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr geom = m_geomPrimitive->GetAsICurvePrimitive();
            builder->AddCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, geom), false);
            break;
            }

        case GeometricPrimitive::GeometryType::CurveVector:
            {
            CurveVectorPtr geom = m_geomPrimitive->GetAsCurveVector();
            builder->AddCurveVector(*geom, false);
            break;
            }

        case GeometricPrimitive::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr geom = m_geomPrimitive->GetAsISolidPrimitive();
            builder->AddSolidPrimitive(*geom);
            break;
            }

        case GeometricPrimitive::GeometryType::Polyface:
            {
            PolyfaceHeaderPtr geom = m_geomPrimitive->GetAsPolyfaceHeader();
            builder->AddPolyface(*geom);
            break;
            }

        case GeometricPrimitive::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr geom = m_geomPrimitive->GetAsMSBsplineSurface();
            builder->AddBSplineSurface(*geom);
            break;
            }

        case GeometricPrimitive::GeometryType::BRepEntity:
            {
            IBRepEntityPtr geom = m_geomPrimitive->GetAsIBRepEntity();
            builder->AddBody(*geom);
            break;
            }

        case GeometricPrimitive::GeometryType::TextString:
            {
            TextStringPtr geom = m_geomPrimitive->GetAsTextString();
            builder->AddTextString(*geom);
            break;
            }

        case GeometricPrimitive::GeometryType::Image:
            {
            builder->AddImage(*m_geomPrimitive->GetAsImage());
            break;
            }

        default:
            {
            BeAssert(false);
            break;
            }
        }

    builder->Finish();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MeasureGeomCollector::MeasureGeomCollector (OperationType opType)
    {
    m_opType = opType;

    m_invCurrTransform.InitIdentity();
    m_preFlattenTransform.InitIdentity();
    m_inFlatten = false;

    m_amountSum = 0.0;
    m_volumeSum = 0.0;
    m_areaSum = 0.0;
    m_perimeterSum = 0.0;
    m_lengthSum = 0.0;
    m_iXY = m_iXZ = m_iYZ = 0.0;

    m_moment1.Zero();
    m_moment2.Zero();
    m_spinMoments.Zero();
    m_centroidSum.Zero();

    m_closureError = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MeasureGeomCollector::SetResultOptions (IFacetOptionsP facetOptions, TransformCP invCurrTrans)
    {
    if (NULL != invCurrTrans)
        m_invCurrTransform = *invCurrTrans;

    if (NULL != facetOptions)
        {
        m_facetOptions = facetOptions;

        // If chord tolerance was specified it's relative to curr trans...and we want it to start out in "local"...
        if (NULL != invCurrTrans && 0.0 != m_facetOptions->GetChordTolerance ())
            {
            double      tolerance = m_facetOptions->GetChordTolerance ();
            Transform   fwdCurrTrans;

            fwdCurrTrans.InverseOf (m_invCurrTransform);
            fwdCurrTrans.ScaleDoubleArrayByXColumnMagnitude (&tolerance, 1); // mdlCurrTrans_scaleDoubleArray...

            m_facetOptions->SetChordTolerance (tolerance);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MeasureGeomCollector::SetPreFlattenTransform (TransformCR preFlattenTrans)
    {
    m_preFlattenTransform = preFlattenTrans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MeasureGeomCollector::GetVolume () const {return m_volumeSum;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MeasureGeomCollector::GetArea () const {return m_areaSum;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MeasureGeomCollector::GetPerimeter () const {return m_perimeterSum;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MeasureGeomCollector::GetLength () const {return m_lengthSum;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MeasureGeomCollector::GetClosureError () const {return m_closureError;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCR MeasureGeomCollector::GetCentroid () const
    {
    m_centroidSum.SafeDivide (m_moment1, m_amountSum);

    return m_centroidSum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MeasureGeomCollector::GetIXY () const
    {
    DPoint3d    centroid = GetCentroid ();

    return (m_iXY - m_amountSum * centroid.x * centroid.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MeasureGeomCollector::GetIXZ () const
    {
    DPoint3d    centroid = GetCentroid ();

    return (m_iXZ - m_amountSum * centroid.x * centroid.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double MeasureGeomCollector::GetIYZ () const
    {
    DPoint3d    centroid = GetCentroid ();

    return (m_iYZ - m_amountSum * centroid.y * centroid.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCR MeasureGeomCollector::GetMoments () const
    {
    DPoint3d    centroid = GetCentroid ();

    // Have moments about global origin. Shift to centroid. In spin moments, x entry is yy + zz etc.
    m_spinMoments.x = m_moment2.x - m_amountSum * (centroid.y * centroid.y + centroid.z * centroid.z);
    m_spinMoments.y = m_moment2.y - m_amountSum * (centroid.z * centroid.z + centroid.x * centroid.x);
    m_spinMoments.z = m_moment2.z - m_amountSum * (centroid.x * centroid.x + centroid.y * centroid.y);

    return m_spinMoments;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MeasureGeomCollector::CalculatePrincipalAxes
(
DPoint3dP       principalMomentsP,      // <=  opt: {Ixx, Iyy, Izz} about prin. axes
DPoint3dP       principalDirectionsP,   // <=  opt: vectors along 3 principal axes
DPoint3dCP      i,                      //  => Ixx, Iyy, Izz
double          iXY,                    //  => Ixy
double          iXZ,                    //  => Ixz
double          iYZ                     //  => Iyz
)
    {
    int             nRot, j;
    double          a[3][3], d[3], v[3][3], *doubleP;

    /*  --  define interia tensor as matrix to diagonalize -- */
    a[0][0] = i->x;  a[0][1] = -iXY;  a[0][2] = -iXZ;
    a[1][0] = -iXY;  a[1][1] = i->y;  a[1][2] = -iYZ;
    a[2][0] = -iXZ;  a[2][1] = -iYZ;  a[2][2] = i->z;

    /*  --

        extremize the moments w.r.t. direction cosines

    []  use Lagrangian multipliers to define the problem:
        (a - d.I) . v = {0, 0, 0}

        where a is the {{Ixx, -Ixy, -Ixz}, ...} matrix of moments
              d is a Lagrangian multiplier,
              I is the identity matrix, and
              v is a vector of direction cosines (e.g., x'.x, x'.y, x'.z)

        This has solutions if:

        | a - d.I | = 0

        there are 3 solutions.

        It can be shown that the multipliers are the principal moments
        that we want.  The principal axes are implicit in the direction
        cosines.

        See Shames p. 557.

        This (cubic) could be solved analytically, but ...

    []  This has the FORM of an eigenvector problem: the Lagrangian multiplier
        is an eigenvalue, and the direction cosines make up a corresponding
        eigenvector.

        There will be 3 sets of eigenvalue/vectors.

        We can use Jacobi's method, since the matrix is symmetric, etc., etc.
        Note that Jacobi's method (as implemented) is iterative, but it is NOT
        an approximation -- it runs until it achieves machine precision.)

        Since the eigenvectors (representing the Lagrangian multipliers) will
        be the principal moments, the eigenvectors will describe the principal
        axes

    */
    bsiGeom_jacobi3X3 (d, v, &nRot, a);

    /*  --  return requested principals for xx (0), yy (1) & zz (2) -- */
    for (j=0, doubleP = (double *) principalMomentsP; j<3; j++, doubleP++)
        {
        if (principalMomentsP)
            *doubleP = d[j];

        if (principalDirectionsP)
            {
            principalDirectionsP[j].x = v[0][j];
            principalDirectionsP[j].y = v[1][j];
            principalDirectionsP[j].z = v[2][j];
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MeasureGeomCollector::GetOperationStatus ()
    {
    switch (m_opType)
        {
        case AccumulateLengths:
            return (0.0 == GetLength () ? ERROR : SUCCESS);

        case AccumulateAreas:
            return (0.0 == GetArea () ? ERROR : SUCCESS);

        case AccumulateVolumes:
            return (0.0 == GetVolume () ? ERROR : SUCCESS);

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MeasureGeomCollector::Process (GeometricPrimitiveCR primitive, DgnDbR dgnDb, TransformCP transform)
    {
    m_geomPrimitive = primitive.Clone();
    m_geomTransform = transform ? *transform : Transform::FromIdentity();

    GeometryProcessor::Process (*this, dgnDb); // Calls _OutputGraphics...

    return GetOperationStatus ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MeasureGeomCollector::Process (GeometrySourceCR source)
    {
    GeometryProcessor::Process (*this, source);

    return GetOperationStatus ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MeasureGeomCollectorPtr MeasureGeomCollector::Create (OperationType opType)
    {
    return new MeasureGeomCollector (opType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MeasureGeomCollector::DoMeasure(BeJsValue out, BeJsConst in, DgnDbR db, ICancellablePtr cancel)
    {
    MeasureGeomCollector::Request input(in);
    MeasureGeomCollector::Response output(out);

    output.SetStatus(ERROR);

    if (!input.IsValid())
        return;

    DgnElementIdSet candidates = input.GetCandidates();
    if (0 == candidates.size())
        return;

    OperationType opType = input.GetOperation();
    MeasureGeomCollectorPtr collector = MeasureGeomCollector::Create(opType);

    for (DgnElementId candidateId : candidates)
        {
        if (cancel.IsValid() && cancel->IsCanceled())
            break; // Return what we've processed so far...

        DgnElementCPtr candidateElement = db.Elements().GetElement(candidateId);
        GeometrySourceCP candidateSource = candidateElement.IsValid() ? candidateElement->ToGeometrySource() : nullptr;

        if (nullptr == candidateSource)
            continue;

        if (SUCCESS == collector->Process(*candidateSource))
            output.SetStatus(SUCCESS);
        }

    switch (opType)
        {
        case AccumulateVolumes:
            output.SetVolume(collector->GetVolume());
            output.SetArea(collector->GetArea());
            break;

        case AccumulateAreas:
            output.SetArea(collector->GetArea());
            output.SetPerimeter(collector->GetPerimeter());
            break;

        default:
            output.SetLength(collector->GetLength());
            break;
        }

    output.SetCentroid(collector->GetCentroid());
    output.SetIXY(collector->GetIXY());
    output.SetIXZ(collector->GetIXZ());
    output.SetIYZ(collector->GetIYZ());
    output.SetMoments(collector->GetMoments());
    }
