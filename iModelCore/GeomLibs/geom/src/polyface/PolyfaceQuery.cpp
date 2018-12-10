/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceQuery.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define ImplementNullArgPolyfaceQueryDispatcher(__type__,__name__) __type__ PolyfaceQuery::__name__##CP () const {return _##__name__##CP ();}
#define ImplementNullArgPolyfaceQueryDispatcher1(__type__,__name__) __type__ PolyfaceQuery::__name__ () const {return _##__name__ ();}

ImplementNullArgPolyfaceQueryDispatcher1 (size_t,GetPointCount)
ImplementNullArgPolyfaceQueryDispatcher1 (size_t,GetNormalCount)
ImplementNullArgPolyfaceQueryDispatcher1 (size_t,GetFaceCount)
ImplementNullArgPolyfaceQueryDispatcher1 (size_t,GetParamCount)
ImplementNullArgPolyfaceQueryDispatcher1 (size_t,GetColorCount)
ImplementNullArgPolyfaceQueryDispatcher1 (size_t,GetPointIndexCount)
ImplementNullArgPolyfaceQueryDispatcher1 (size_t,GetFaceIndexCount)
ImplementNullArgPolyfaceQueryDispatcher1 (size_t,GetEdgeChainCount)

ImplementNullArgPolyfaceQueryDispatcher (DPoint3dCP,GetPoint)
ImplementNullArgPolyfaceQueryDispatcher (DVec3dCP,GetNormal)
ImplementNullArgPolyfaceQueryDispatcher (DPoint2dCP,GetParam)
ImplementNullArgPolyfaceQueryDispatcher (FacetFaceDataCP,GetFaceData)
ImplementNullArgPolyfaceQueryDispatcher (PolyfaceEdgeChainCP, GetEdgeChain)
ImplementNullArgPolyfaceQueryDispatcher (uint32_t const*,GetIntColor)
ImplementNullArgPolyfaceQueryDispatcher (int32_t const*,GetPointIndex)
                                                                                                
                                                                                                
PolyfaceVectors* PolyfaceQuery::_AsPolyfaceVectorsP () const { return nullptr;}
PolyfaceAuxDataCPtr PolyfaceQuery::_GetAuxDataCP() const { return nullptr; }


//! Select an index pointer -- e.g. normal, param, color -- with optional fallback to point index.
//! @param[in] firstChoice first choice (e.g. the available nomal, param, or color index)
//! @param[in] resovleToDefaults true to enable fallback to PointIndex.
//! @param[in] queryObject object to query for point index.
static int32_t const* ResolveIndexPtr (int32_t const * firstChoice, bool resolveToDefaults, PolyfaceQuery const *queryObject)
    {
    if (NULL != firstChoice)
        return firstChoice;
    if (!resolveToDefaults)
        return NULL;
    if (queryObject != NULL)
        return queryObject->GetPointIndexCP ();
    return NULL;
    }
int32_t const*      PolyfaceQuery::GetColorIndexCP  (bool resolveToDefaults) const         { return ResolveIndexPtr (_GetColorIndexCP (),  resolveToDefaults, this);}
int32_t const*      PolyfaceQuery::GetParamIndexCP  (bool resolveToDefaults) const         { return ResolveIndexPtr (_GetParamIndexCP (),  resolveToDefaults, this);}
int32_t const*      PolyfaceQuery::GetNormalIndexCP (bool resolveToDefaults) const         { return ResolveIndexPtr (_GetNormalIndexCP (), resolveToDefaults, this);}
int32_t const*      PolyfaceQuery::GetFaceIndexCP (bool resolveToDefaults) const           { return ResolveIndexPtr (_GetFaceIndexCP (),   resolveToDefaults, this);}

uint32_t            PolyfaceQuery::GetNumPerFace () const                                  { return _GetNumPerFace ();}
uint32_t            PolyfaceQuery::GetNumPerRow () const                                   { return _GetNumPerRow ();}
bool                PolyfaceQuery::GetTwoSided () const                                    { return _GetTwoSided (); }
uint32_t            PolyfaceQuery::GetMeshStyle () const                                   { return _GetMeshStyle ();}
PolyfaceAuxDataCPtr PolyfaceQuery::GetAuxDataCP() const                                    { return _GetAuxDataCP ();}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
template <typename T>
void ReverseInRange (T *pIndex, size_t iFirst, size_t iLast)
    {
    T temp;
    if (NULL == pIndex)
        return;
    for (; iFirst < iLast; iFirst++, iLast--)
        {
        temp = pIndex [iFirst];
        pIndex[iFirst]= pIndex[iLast];
        pIndex[iLast] = temp;
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AbsInRange(int32_t *pIndex, size_t iFirst, size_t iLast)
    {
    if (NULL == pIndex)
        return;
    for (size_t i = iFirst; i <= iLast; i++)
        {
        pIndex[i] = abs (pIndex[i]);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void NegativeAbsInRange(int32_t *pIndex, size_t iFirst, size_t iLast)
    {
    if (NULL == pIndex)
        return;
    for (size_t i = iFirst; i <= iLast; i++)
        {
        pIndex[i] = -abs (pIndex[i]);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void NegateInRange(int32_t *pIndex, size_t iFirst, size_t iLast)
    {
    if (NULL == pIndex)
        return;
    for (size_t i = iFirst; i <= iLast; i++)
        {
        pIndex[i] = -pIndex[i];
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ShiftSignsFromCyclicPredecessorsInRange(int32_t *pIndex, size_t kFirst, size_t kLast)
    {
    if (NULL == pIndex)
        return;
    int sign0 = pIndex[kLast] >= 0 ? 1 : -1;
    int sign1;
    for (size_t k = kFirst; k <= kLast; k++)
        {
        sign1 = pIndex[k] >= 0 ? 1 : -1;
        pIndex[k] = sign0 * abs (pIndex[k]);
        sign0 = sign1;
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceQuery::ReverseIndicesOneFace
(
size_t        iFirst,
size_t        iLast,
BlockedVectorInt::IndexAction           normalArrayIndexAction
)
    {
    int const*pPointIndex  = (int const*)GetPointIndexCP ();
    int const*pNormalIndex = (int const*)GetNormalIndexCP ();
    int const*pParamIndex  = (int const*)GetParamIndexCP ();
    int const*pColorIndex  = (int const*)GetColorIndexCP ();
    
    //TR #164587: preserve hidden edges in vertex index array
    ShiftSignsFromCyclicPredecessorsInRange (const_cast<int*>(pPointIndex), iFirst, iLast);
    ReverseInRange (const_cast<int*>(pPointIndex), iFirst, iLast);

    
    if (pNormalIndex)
        {
        ReverseInRange (const_cast<int*>(pNormalIndex), iFirst, iLast);

        if (normalArrayIndexAction == BlockedVectorInt::ForcePositive)
            AbsInRange (const_cast<int*>(pNormalIndex), iFirst, iLast);
        else if (normalArrayIndexAction == BlockedVectorInt::ForceNegative)
            NegativeAbsInRange (const_cast<int*>(pNormalIndex), iFirst, iLast);
        else if (normalArrayIndexAction == BlockedVectorInt::Negate)
            NegateInRange (const_cast<int*>(pNormalIndex), iFirst, iLast);
        }

    if (pColorIndex)
        ReverseInRange (const_cast<int*>(pColorIndex), iFirst, iLast);  // No sign effects for color indices.
    if (pParamIndex)
        ReverseInRange (const_cast<int*>(pParamIndex), iFirst, iLast);  // No sign effects for param indices.

    if (GetAuxDataCP().IsValid())
        ReverseInRange(const_cast<int32_t*> (GetAuxDataCP()->GetIndices().data()), iFirst, iLast);
    }

/** From given start position, find final (inclusive) position and position for next start search.
  Initialize iFirst to zero before first call.
  Return false if no more faces. */
static bool DelimitFace (int32_t *pIndex, size_t count, int numPerFace, size_t iFirst, size_t &iLast, size_t &iNext)
    {
    if (iFirst >= count)
        {
        iNext = iFirst;
        if (iFirst >= 1)
            iLast = iFirst - 1;
        return false;
        }
    if (numPerFace > 1)
        {
        iLast = iFirst;
        size_t limit = iFirst + numPerFace;
        while (iLast < limit && pIndex[iLast] != 0)
            iLast++;
        if (iLast >= 1)
            iLast--;
        iNext = iFirst + numPerFace;
        return iLast >= iFirst;
        }
    else
        {
        // zero terminated.
        // skip over leading zeros ..
        while (iFirst < count && pIndex[iFirst] == 0)
            iFirst++;
        if (iFirst >= count)
            {
            iNext = iFirst;
            if (iFirst >= 1)
                iLast = iFirst - 1;
            return false;
            }
        iLast = iFirst + 1;
        while (iLast < count && pIndex[iLast] != 0)
            iLast++;
        // iLast is one after the live stuff for this face.
        iNext = iLast + 1;
        if (iLast >= 1)
            iLast--;
        return true;
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool AllNegativeInRange(int32_t *pIndex, size_t iFirst, size_t iLast)
    {
    for (size_t i = iFirst; i < iLast; i++)
        if (pIndex[i] >= 0)
            return false;
    return true;
    }

template<typename T>
void ReverseByBlock (T *data, size_t n, size_t numPerBlock)
    {
    if (data != nullptr && n > 0 && numPerBlock > 0)
        {
        for (size_t i0 = 0; i0 < n; i0 += numPerBlock)
            {
            size_t numThisBlock = n - i0;
            if (numThisBlock > numPerBlock)
                numThisBlock = numPerBlock;
            for (size_t i = 0, j = numThisBlock - 1; i < j; i++, j--)
                std::swap (data[i0 + i], data[i0 + j]);
            }
        }
    }

bool ReverseMeshByBlock (PolyfaceQueryR mesh, bool negateNormals)
    {
    size_t n = mesh.GetPointCount ();
    size_t numPerBlock = 0;
    if (  mesh.GetMeshStyle () == MESH_ELM_STYLE_QUAD_GRID
       || mesh.GetMeshStyle () == MESH_ELM_STYLE_TRIANGLE_GRID
       )
        {
        // reverse within each row.
        numPerBlock = (size_t)mesh.GetNumPerRow ();
        }
    else if (mesh.GetMeshStyle () == MESH_ELM_STYLE_COORDINATE_QUADS)
        {
        // reverse within each block of 4 coordinates . . .
        numPerBlock = 4;
        }
    else if (mesh.GetMeshStyle () == MESH_ELM_STYLE_COORDINATE_TRIANGLES)
        {
        // reverse within each block of 3 coordinates
        numPerBlock = 3;
        }
    else
        {
        return false;
        }

    DPoint3dP points = const_cast<DPoint3d*>(mesh.GetPointCP ());
    DVec3dP normals = const_cast<DVec3d*>(mesh.GetNormalCP ());
    DPoint2dP params = const_cast<DPoint2d*>(mesh.GetParamCP ());
    uint32_t *intColor = const_cast<uint32_t*>(mesh.GetIntColorCP ());
    ReverseByBlock <DPoint3d> (points, n, numPerBlock);
    ReverseByBlock <DVec3d> (normals, n, numPerBlock);
    ReverseByBlock <DPoint2d> (params, n, numPerBlock);
    ReverseByBlock <uint32_t> (intColor, n, numPerBlock);
    if (negateNormals && nullptr != normals)
        {
        for (size_t i = 0; i < n; i++)
            normals[i].Negate ();
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::ReverseIndicesAllFaces
(
bool           negateNormals,
bool           flipMarked,
bool           flipUnMarked,
BlockedVectorInt::IndexAction normalIndexAction
)
    {
    if (ReverseMeshByBlock (*this,negateNormals))
        return true;


    int const*pPointIndex  = (int const*)GetPointIndexCP ();
    int const*pNormalIndex = (int const*)GetNormalIndexCP ();
    //int *pParamIndex  = (int *)GetParamIndexCP ();
    //int *pColorIndex  = (int *)GetColorIndexCP ();
    DVec3dP pNormal   = const_cast<DVec3dP>(GetNormalCP ());
    size_t numNormal  = GetNormalCount ();
    size_t numIndex = GetPointIndexCount ();

    int numPerFace = GetNumPerFace ();

    if (2 == numPerFace)
        return false;

    size_t iFirst = 0;
    size_t iNext  = 0;
    size_t iLast  = 0;

    if ((flipMarked && flipUnMarked) || pPointIndex != NULL)
        {
        for (;DelimitFace (const_cast<int*>(pPointIndex), numIndex, numPerFace, iFirst, iLast, iNext); iFirst = iNext)
            {
            ReverseIndicesOneFace (iFirst, iLast, normalIndexAction);
            }
        }
    else if (flipMarked || flipUnMarked)
        {
        for (;DelimitFace (const_cast<int*>(pPointIndex), numIndex, numPerFace, iFirst, iLast, iNext); iFirst = iNext)
            {
            if (AllNegativeInRange (const_cast<int*>(pNormalIndex), iFirst, iLast))
                if (flipMarked)
                    ReverseIndicesOneFace (iFirst, iLast, normalIndexAction);
            else
                if (flipUnMarked)
                    ReverseIndicesOneFace (iFirst, iLast, normalIndexAction);
            }
        }
    else
        {
        // no index effects
        }

    if (numNormal > 0 && pNormal != NULL && negateNormals)
        {
        for (size_t i = 0; i < numNormal; i++)
            pNormal [i].Negate ();
        }

    return true;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DRange3d PolyfaceQuery::PointRange () const
    {
    return  DRange3d::From (GetPointCP (), (int)GetPointCount ());
    }


static double s_mediumAbsTol = 1.0e-12;
static double s_mediumRelTol = 1.0e-12;


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double PolyfaceQuery::GetTightTolerance () const
    {
    double smallAngle = Angle::SmallAngle ();
    return DPoint3dOps::Tolerance (
                GetPointCP (), (int)GetPointCount (),
                smallAngle, smallAngle);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double PolyfaceQuery::GetMediumTolerance () const
    {
    return DPoint3dOps::Tolerance (
                GetPointCP (), (int)GetPointCount (),
                s_mediumAbsTol, s_mediumRelTol);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DRange2d PolyfaceQuery::ParamRange () const
    {
    return DRange2d::From (GetParamCP (), (int)GetParamCount ());
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double PolyfaceQuery::SumTetrahedralVolumes (DPoint3dCR origin) const
    {
    double s = 0.0;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    bvector<DPoint3d> &facePoint = visitor->Point ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        size_t numEdge = facePoint.size ();
        for (size_t i = 2; i < numEdge; i++)
            {
            double si = origin.TripleProductToPoints (facePoint[0], facePoint[i-1], facePoint[i]);
            s += si;
            }
        }
    return s / 6.0;
    }

ValidatedDouble PolyfaceQuery::ValidatedVolume () const
    {
    if (GetPointCount () == 0)
        return ValidatedDouble (0,true);
    if (IsClosedByEdgePairing ())
        {
        DPoint3d origin = GetPointCP ()[0];
        DPoint3d origin1 = GetPointCP ()[1];
        double v0 = SumTetrahedralVolumes (origin);
        double v1 = SumTetrahedralVolumes (origin1);
        return ValidatedDouble (v0, DoubleOps::AlmostEqual (v0, v1));
        }
    return ValidatedDouble (0, false);
    }
static void TetrahedralMoments
(
DPoint3dCR pointA,
DPoint3dCR pointB,
DPoint3dCR pointC,
DPoint3dCR pointD,
RotMatrixR products,
DVec3dR    moment1,
double     &volume
)
    {
    DMatrix4d referenceMoments =
        {{
        {2,1,1,5},
        {1,2,1,5},
        {1,1,2,5},
        {5,5,5,1}
        }};
    DVec3d vectorAB = DVec3d::FromStartEnd (pointA, pointB);
    DVec3d vectorAC = DVec3d::FromStartEnd (pointA, pointC);
    DVec3d vectorAD = DVec3d::FromStartEnd (pointA, pointD);
    double detJ = vectorAB.TripleProduct (vectorAC, vectorAD);
    Transform placement = Transform::FromOriginAndVectors (pointA, vectorAB, vectorAC, vectorAD);
    DMatrix4d spaceMoments = DMatrix4d::FromSandwichProduct (placement, referenceMoments, detJ /120.0);
    DVec3d moment1A;
    double a;
    spaceMoments.ExtractAroundPivot (products, moment1A, moment1, a, 3);
    volume = detJ / 6.0;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceQuery::SumTetrahedralMomentProducts (TransformCR worldToLocal, double &volume, DVec3dR moment1, RotMatrixR products) const
    {
    RotMatrix sum2, local2;
    sum2.Zero ();
    DVec3d sum1, local1;
    sum1.Zero ();
    double sum0 = 0.0;
    double local0;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    bvector<DPoint3d> &facePoint = visitor->Point ();
    DPoint3d origin;
    origin.Zero ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        size_t numEdge = facePoint.size ();
        DPoint3d point0 = facePoint[0];
        worldToLocal.Multiply (point0);
        DPoint3d point2 = facePoint[1];
        worldToLocal.Multiply (point2);
        DPoint3d point1;
        for (size_t i = 2; i < numEdge; i++)
            {
            point1 = point2;
            point2 = facePoint[i];
            worldToLocal.Multiply (point2);
            TetrahedralMoments (origin, point0, point1, point2, local2, local1, local0);
            sum0 += local0;
            sum1.Add (local1);
            sum2.Add (local2);
            }
        }
    volume = sum0;
    moment1 = sum1;
    products = sum2;
    }

bool ErrorMoments (double x, double y, double z, double volumeIn,
                              double &volume,
                              DPoint3dR centroid,
                              RotMatrixR axes,
                              DVec3dR momentxyz
                   )
    {
    volume = volumeIn;
    centroid.Init (x,y,z);
    axes.InitIdentity ();
    momentxyz.Init (0,0,0);
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::ComputePrincipalMoments (
                              double &volume,
                              DPoint3dR centroid,
                              RotMatrixR axes,
                              DVec3dR momentxyz,
                              bool forcePositiveVolume
                              ) const
    {
    static bool s_noTranslations = 0;
    if (GetPointCount () == 0)
        return ErrorMoments (0, 0,0,0, volume, centroid, axes, momentxyz);
    DPoint3d origin0 = GetPointCP ()[0];
    if (s_noTranslations)
        origin0.Zero ();
    DVec3d moment0, centroidVector0;

    double volume0 = SumTetrahedralFirstMoments (origin0, moment0);
    if (!centroidVector0.SafeDivide (moment0, volume0))
        return ErrorMoments (origin0.x, origin0.y, origin0.z, volume0, volume, centroid, axes, momentxyz);
    centroid.SumOf (origin0, centroidVector0);
    Transform  worldToLocal = Transform::From (DPoint3d::From (-centroid.x, -centroid.y, -centroid.z));
    double volume1;
    DVec3d moment1;
    RotMatrix product1;
    SumTetrahedralMomentProducts (worldToLocal, volume1, moment1, product1);
    if (forcePositiveVolume && volume1 < 0.0)
        {
        volume1 *= -1.0;
        volume0 *= -1;
        moment0.Scale (-1.0);
        moment1.Scale (-1.0);
        product1.ScaleColumns (-1.0, -1.0, -1.0);
        }
    double xx = product1.form3d[0][0];
    double yy = product1.form3d[1][1];
    double zz = product1.form3d[2][2];
    double xy = product1.form3d[0][1];
    double xz = product1.form3d[0][2];
    double yz = product1.form3d[1][2];
    RotMatrix tensor1 = RotMatrix::FromRowValues
        (
        yy + zz, -xy, -xz,
        -xy, xx + zz, -yz,
        -xz, -yz, yy + xx
        );
    DVec3d eigenvalues;
    RotMatrix workMatrix = tensor1;
    int numIteration;
    bsiGeom_jacobi3X3 (
              (double*)&eigenvalues,
              axes.form3d,
              &numIteration,
              workMatrix.form3d
              );

    momentxyz.Init (eigenvalues.x, eigenvalues.y, eigenvalues.z);
    volume = volume1;
    static double s_volumeRelTol = 1.0e-8;
    return DoubleOps::AlmostEqual (volume0, volume1, s_volumeRelTol * sqrt (fabs (volume0 * volume1)));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2012
+--------------------------------------------------------------------------------------*/
double PolyfaceQuery::SumTetrahedralFirstMoments (DPoint3dCR origin, DVec3dR moment1) const
    {
    double volume = 0.0;
    moment1.Zero ();
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    bvector<DPoint3d> &facePoint = visitor->Point ();
    double factor1 = 1.0 / 24.0;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        size_t numEdge = facePoint.size ();
        DVec3d vector0, vector1, vector2;
        vector0.DifferenceOf (facePoint[0], origin);
        vector2.DifferenceOf (facePoint[1], origin);
        for (size_t i = 2; i < numEdge; i++)
            {
            vector1 = vector2;
            vector2.DifferenceOf (facePoint[i], origin);
            double detJ = vector0.TripleProduct (vector1, vector2);
            double a = factor1 * detJ;
            moment1.SumOf (moment1, vector0, a, vector1, a, vector2, a);
            volume += detJ;
            }
        }
    return volume / 6.0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2012
+--------------------------------------------------------------------------------------*/
double PolyfaceQuery::SumFacetFirstAreaMoments (DPoint3dCR origin, DVec3dR moment1) const
    {
    double meshArea = 0.0;
    double facetArea;
    DVec3d facetNormal;
    DVec3d facetCentroid;
    moment1.Zero ();
    //DVec3d offset;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    bvector<DPoint3d> &points = visitor->Point ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        for (auto &point : points)
            point.Subtract (origin);
        visitor->TryGetFacetCentroidNormalAndArea (facetCentroid, facetNormal, facetArea);
        meshArea += facetArea;
        moment1.SumOf (moment1, facetCentroid, facetArea);
        }
    return meshArea;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceQuery::DirectionalAreaAndVolume
(
DPoint3dCR origin,
DVec3dR areaXYZ,
DVec3dR volumeXYZ,
DVec3dR centroidX,
DVec3dR centroidY,
DVec3dR centroidZ
) const
    {
    DVec3d cross, crossSum;
    areaXYZ.Zero ();
    RotMatrix momentSumsXYZ;
    momentSumsXYZ.Zero ();
    DVec3d facetCentroid;

    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    visitor->SetNumWrap (0);
    bvector<DPoint3d> &points = visitor->Point ();
    double oneThird = 1.0 / 3.0;
    DVec3d originToPoint0, vector01, vector02;
    volumeXYZ.Zero ();
    crossSum.Zero ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        for (size_t i1 = 1; i1 + 1 < points.size (); i1 ++)
            {
            // x,y,z components of the cross product are (2 times) projected areas
            vector01.DifferenceOf (points[i1], points[0]);
            vector02.DifferenceOf (points[i1 + 1], points[0]);
            cross.CrossProduct (vector01, vector02);
            crossSum.Add (cross);
            originToPoint0.DifferenceOf (points[0], origin);
            facetCentroid.SumOf (originToPoint0, vector01, oneThird, vector02, oneThird);
            momentSumsXYZ.AddScaledOuterProductInPlace (facetCentroid, cross, 1.0);
            volumeXYZ.Add (DVec3d::From (
                          cross.x * originToPoint0.x,
                          cross.y * originToPoint0.y,
                          cross.z * originToPoint0.z));
            }
        }
    momentSumsXYZ.GetColumns (centroidX, centroidY, centroidZ);
    // both the numerator and denominator still have 2X factor from cross products . .
    centroidX.SafeDivide (centroidX, crossSum.x);
    centroidY.SafeDivide (centroidY, crossSum.y);
    centroidZ.SafeDivide (centroidZ, crossSum.z);
    areaXYZ.Scale (crossSum, 0.5);
    volumeXYZ.Scale (0.5);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::TryEvaluateEdge (FacetEdgeLocationDetailCR position, DPoint3dR xyz) const
    {
    if (GetMeshStyle () == MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
        {
        uint32_t blockSize = GetNumPerFace ();
        size_t read0 = position.m_readIndex;
        size_t read1 = SIZE_MAX;
        size_t numIndex = GetPointIndexCount ();
        int32_t const *index = GetPointIndexCP ();
        if (blockSize > 1)
            {
            size_t faceCount = read0 / blockSize; // truncates!!!
            size_t faceBase = faceCount * blockSize;
            size_t offset1 = read0 + 1 - faceBase ;
            if (offset1 >= blockSize || index[faceBase + offset1] == 0)
                read1 = faceBase;
            else
                read1 = read0 + 1;
            if (read0 >= numIndex || read1 >= numIndex)
                return false;
            }
        else
            {
            if (read0 >= numIndex)
                return false;
            if (index[read0] == 0)
                return false;
            read1 = read0 + 1;
            if (read1 < numIndex && index[read1] != 0)
                {
                // Accept read1 !!!
                }
            else // Back up to face start
                {
                read1 = read0;
                while (read1 > 0 && index[read1-1] != 0)
                    read1--;
                }
            }
        if (read0 == read1)
            return false;   // singleton facet.  Strange but possible.
        int vertexSignedIndex0 = index[read0];
        int vertexSignedIndex1 = index[read1];
        if (vertexSignedIndex0 == 0 || vertexSignedIndex1 == 0)
            return false;
        size_t vertexIndex0 = abs (vertexSignedIndex0) - 1;
        size_t vertexIndex1 = abs (vertexSignedIndex1) - 1;
        size_t numPoint = GetPointCount ();
        DPoint3dCP points = GetPointCP ();
        if (vertexIndex0 < numPoint && vertexIndex1 < numPoint)
            {
            xyz.Interpolate (points[vertexIndex0], position.m_fraction, points[vertexIndex1]);
            return true;
            }
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2012
+--------------------------------------------------------------------------------------*/
double PolyfaceQuery::SumFacetSecondAreaMomentProducts (DPoint3d origin, DMatrix4dR products) const
    {
    products = DMatrix4d::FromZero ();
    DMatrix4d facetProducts;
    double area = 0.0;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    bvector<DPoint3d> &facePoint = visitor->Point ();
    DPoint3d zero = DPoint3d::From (0,0,0);
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        for (auto &xyz : facePoint)
            xyz.Subtract (origin);
        PolygonOps::SecondAreaMomentProducts (facePoint, zero, facetProducts);
        products.Add (facetProducts);
        area += products.coff[3][3];
        }
    return area;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::ComputePrincipalAreaMoments (double &area, DPoint3dR centroid, RotMatrixR axes, DVec3dR momentTensorDiagonal) const
    {
    if (GetPointCount () > 0)
        {
        DPoint3d origin0 = GetPointCP ()[0];       
        DMatrix4d products;
        DVec3d moment0, centroid1;
        double area0 = SumFacetFirstAreaMoments (origin0, moment0);
        if (centroid1.SafeDivide (moment0, area0))
            {
            DPoint3d origin1 = DPoint3d::FromSumOf (origin0, centroid1, 1.0);
            SumFacetSecondAreaMomentProducts (origin1, products);
            Transform local1ToWorld = Transform::From (origin1);
            DVec3d centroidVector;
            if (products.ConvertInertiaProductsToPrincipalAreaMoments (local1ToWorld, area, centroidVector, axes, momentTensorDiagonal))
                {
                centroid.Init (centroidVector);
                return true;
                }
            }
        }
    return ErrorMoments (0, 0,0,0, area, centroid, axes, momentTensorDiagonal);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double PolyfaceQuery::SumFacetAreas () const
    {
    double sum = 0.0;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    bvector<DPoint3d> &points = visitor->Point ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        DVec3d facetNormal = PolygonOps::AreaNormal (points);
        sum += facetNormal.Magnitude ();
        }
    return sum;
    }
                    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::TryGetPointAtReadIndex (size_t readIndex, DPoint3dR data) const
    {
    size_t                  zeroBasedIndex;
    int32_t const*            pointIndex;
    DPoint3dCP              points;

    if (readIndex > GetPointIndexCount() ||
        NULL == (pointIndex = GetPointIndexCP()) ||
        NULL == (points = GetPointCP()))
        return false;

    zeroBasedIndex = abs (pointIndex[readIndex]) - 1;
    if (zeroBasedIndex >= GetPointCount())
        return false;

    data = points[zeroBasedIndex];
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::TryGetNormalAtReadIndex (size_t readIndex, DVec3dR data) const
    {
    size_t                  zeroBasedIndex;
    int32_t const*            normalIndex;
    DVec3dCP                normals;

    if (readIndex > GetPointIndexCount() ||
        NULL == (normalIndex = GetNormalIndexCP()) ||
        NULL == (normals = GetNormalCP()))
        return false;

    int oneBasedIndex = normalIndex[readIndex];
    if (oneBasedIndex == 0)
        return false;
    zeroBasedIndex = abs (oneBasedIndex) - 1;
    if (zeroBasedIndex >= GetNormalCount())
        return false;

    data = normals[zeroBasedIndex];
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::TryGetParamAtReadIndex (size_t readIndex, DPoint2dR data) const
    {
    size_t                  zeroBasedIndex;
    int32_t const*            paramIndex;
    DPoint2dCP              params;

    if (readIndex > GetPointIndexCount() ||
        NULL == (paramIndex = GetParamIndexCP()) ||
        NULL == (params = GetParamCP()))
        return false;

    zeroBasedIndex = abs (paramIndex[readIndex]) - 1;
    if (zeroBasedIndex >= GetParamCount())
        return false;

    data = params[zeroBasedIndex];
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::TryGetFacetFaceDataAtReadIndex (size_t readIndex, FacetFaceDataR data, size_t& zeroBasedIndex) const
    {
    int32_t const*            faceIndex;
    FacetFaceDataCP         faceData;

    if (readIndex > GetPointIndexCount() ||
        NULL == (faceIndex = GetFaceIndexCP()) ||
        NULL == (faceData = GetFaceDataCP()))
        return false;

    zeroBasedIndex = abs (faceIndex[readIndex]) - 1;
    if (zeroBasedIndex >= GetFaceCount())
        return false;

    data = faceData[zeroBasedIndex];
    return true;

    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceQuery::GetNumFacet () const
    {
    size_t maxPerFace;
    return GetNumFacet (maxPerFace);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceQuery::GetNumFacet (size_t &maxPerFace) const
    {
    size_t numFacet = 0;
    maxPerFace = 0;
    size_t numVertexPerRow, numVertexPerColumn;
    switch (GetMeshStyle())
        {
        case MESH_ELM_STYLE_INDEXED_FACE_LOOPS:
            {
            uint32_t        numPerFace = GetNumPerFace();
            if (numPerFace <= 1)
                {
                size_t     numThisFacet = 0;

                for (int32_t const *index = GetPointIndexCP(), *end = index + GetPointIndexCount(); index < end; index++)
                    {
                    if (0 == *index)
                        {
                        if (numThisFacet > maxPerFace)
                            maxPerFace = numThisFacet;

                        numFacet++;
                        numThisFacet = 0;
                        }
                    else
                        {
                        numThisFacet++;
                        }
                    }
                }
            else
                {
                maxPerFace = numPerFace;
                numFacet = GetPointIndexCount() / maxPerFace;
                }
            break;
            }
        case MESH_ELM_STYLE_COORDINATE_QUADS:
            numFacet = GetPointCount () / 4;
            maxPerFace = 4;
            break;
        case MESH_ELM_STYLE_COORDINATE_TRIANGLES:
            numFacet = GetPointCount () / 3;
            maxPerFace = 3;
            break;
        case MESH_ELM_STYLE_TRIANGLE_GRID:
            numVertexPerRow = GetNumPerRow ();
            numVertexPerColumn = GetPointCount () / numVertexPerRow;
            numFacet = 2 * (numVertexPerRow - 1) * (numVertexPerColumn - 1);
            maxPerFace = 3;
            break;
        case MESH_ELM_STYLE_QUAD_GRID:
            numVertexPerRow = GetNumPerRow ();
            numVertexPerColumn = GetPointCount () / numVertexPerRow;
            numFacet = (numVertexPerRow - 1) * (numVertexPerColumn - 1);
            maxPerFace = 4;
            break;

        default:    // really?  Are there more kinds?
            {
            PolyfaceVisitorPtr visitorPtr = PolyfaceVisitor::Attach (*this, false);
            PolyfaceVisitor & visitor = *visitorPtr.get ();
            for (visitor.Reset ();visitor.AdvanceToNextFace ();)
                {
                numFacet++;
                if (visitor.NumEdgesThisFace () > maxPerFace)
                    maxPerFace = visitor.NumEdgesThisFace();
                }
            break;
            }
        }
    return numFacet;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2013
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::HasConvexFacets () const
    {
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false); visitor->AdvanceToNextFace ();)
        {
        if (visitor->NumEdgesThisFace () > 3 &&
            !bsiGeom_testPolygonConvex (visitor->GetPointCP(), visitor->NumEdgesThisFace()))
            return false;
        }
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double PolyfaceQuery::LargestCoordinate () const
    {
    size_t n = GetPointCount ();
    if (n == 0)
        return 0;
    return DPoint3dOps::LargestCoordinate (GetPointCP (), n);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::HasFacets () const
    {
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*this, false);
    return visitor->AdvanceToNextFace();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::IsTriangulated () const
    {
    size_t numFacet, maxPerFace;
    numFacet = GetNumFacet (maxPerFace);
    return maxPerFace == 3;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley      08/2018
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::IsClosedPlanarRegion(DPlane3dR plane, double planeTolerance, double distanceTolerance) const
    {
    if (!GetTwoSided())
        return false;

    bool            first = true;
    Transform       localToWorld, worldToLocal;
    double          planeDistance = 0.0;

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false); visitor->AdvanceToNextFace ();)
        {
        if (visitor->TryGetLocalFrame(localToWorld, worldToLocal))
            {
            DPlane3d  thisPlane;
            thisPlane.normal = localToWorld.GetMatrixColumn(2);
            thisPlane.normal.Normalize();
            thisPlane.origin = localToWorld.Translation();

            double  thisPlaneDistance = thisPlane.origin.DotProduct(thisPlane.normal);

            if (first)
                {
                first = false;
                plane = thisPlane;
                planeDistance = thisPlaneDistance;
                }
            else
                {
                if (fabs(planeDistance - thisPlaneDistance) > distanceTolerance || !plane.normal.IsEqual(thisPlane.normal, planeTolerance))
                    return false;
                }
            }
        }

    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceQuery::CollectCounts
(
size_t &numVertex,
size_t &numFacet,
size_t &numQuad,
size_t &numTriangle,
size_t &numImplicitTriangle,
size_t &numVisibleEdges,
size_t &numInvisibleEdges
) const
    {
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    numVertex = GetPointCount ();
    numFacet = 0;
    numQuad = 0;
    numTriangle = 0;
    numImplicitTriangle = 0;
    numVisibleEdges = 0;
    numInvisibleEdges = 0;
    
    bvector<bool> &visible = visitor->Visible ();
    for (visitor->Reset (); visitor->AdvanceToNextFace (); )
        {
        numFacet++;
        size_t n = visitor->NumEdgesThisFace ();
        if (n == 3)
            numTriangle++;
        else if (n == 4)
            numQuad++;

        if (n >= 3)
            numImplicitTriangle += n - 2;
        for (size_t i = 0; i < n; i++)
            {
            if (visible[i])
                numVisibleEdges++;
            else
                numInvisibleEdges++;
            }
        }
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceQuery::CollectPerFaceCounts
(
size_t &minPerFace,
size_t &maxPerFace
) const
    {
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    minPerFace = INT_MAX;
    maxPerFace = 0;
    for (visitor->Reset (); visitor->AdvanceToNextFace (); )
        {
        size_t n = visitor->NumEdgesThisFace ();
        if (n < minPerFace)
            minPerFace = n;
        if (n > maxPerFace)
            maxPerFace = n;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2014
+--------------------------------------------------------------------------------------*/
static bool SameIndexArray (int const *dataA, size_t numA, int const * dataB, size_t numB)
    {
    if (numA != numB)
        return false;
    if (nullptr == dataA && nullptr == dataB)
        return true;
    if ((nullptr != dataA) != (nullptr != dataB))
        return false;
    if (nullptr != dataA && nullptr != dataB)
        {
        for (size_t i = 0; i < numA; i++)
            if (dataA[i] != dataB[i])
                return false;
        }
    return true;
    }

#define SAME_FIELDREF(fieldName) (fabs (dataA[i].fieldName - dataB[i].fieldName) <= tolerance)
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2018
+--------------------------------------------------------------------------------------*/
bool AlmostEqualFaceDataParams
(
FacetFaceDataCP dataA,
size_t numA,
FacetFaceDataCP dataB,
size_t numB,
double tolerance
)
    {
    if (numA != numB)
        return false;
    for (size_t i = 0; i < numA; i++)
        {
        if (!SAME_FIELDREF (m_paramDistanceRange.low.x))
            return false;
        if (!SAME_FIELDREF (m_paramDistanceRange.low.y))
            return false;
        if (!SAME_FIELDREF (m_paramDistanceRange.high.x))
            return false;
        if (!SAME_FIELDREF (m_paramDistanceRange.high.y))
            return false;
        }
    return true;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2014
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::IsSameStructureAndGeometry (PolyfaceQueryCR other, double tolerance) const
    {
    if (GetMeshStyle () != other.GetMeshStyle ())
        return false;
    bool checkIndices = false;
    if (GetMeshStyle () == MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
        {
        checkIndices = true;
        }
    else if (GetMeshStyle () == MESH_ELM_STYLE_COORDINATE_TRIANGLES
          || GetMeshStyle () == MESH_ELM_STYLE_COORDINATE_QUADS
          )
        {
        }
    else if (GetMeshStyle () == MESH_ELM_STYLE_QUAD_GRID
          || GetMeshStyle () == MESH_ELM_STYLE_TRIANGLE_GRID
          )
        {
      if (GetNumPerRow () != other.GetNumPerRow ())
          return false;
        }


    if (!DPoint3dOps::AlmostEqual (
          GetPointCP (), GetPointCount (), other.GetPointCP (), other.GetPointCount (), tolerance))
        return false;
    static double s_paramTolerance = 0.0;   // Can't tell if params are 0..1 or real distances.
    if (!DPoint2dOps::AlmostEqual (
          GetParamCP (), GetParamCount (), other.GetParamCP (), other.GetParamCount (), s_paramTolerance))
        return false;
    static double s_normalTolerance = 1.0e-8;
    if (!DVec3dOps::AlmostEqual (
          GetNormalCP (), GetNormalCount (), other.GetNormalCP (), other.GetNormalCount (), s_normalTolerance))
        return false;
    static double s_faceDataTolerance = 1.0e-8;
    if (!AlmostEqualFaceDataParams (
          GetFaceDataCP (), GetFaceCount (), other.GetFaceDataCP (), other.GetFaceCount (), s_faceDataTolerance))
        return false;

    size_t numA = GetPointIndexCount ();
    size_t numB = other.GetPointIndexCount ();
    if (checkIndices)
        {
        if (!SameIndexArray (
              GetPointIndexCP (), numA, other.GetPointIndexCP (), numB))
            return false;

        if (!SameIndexArray (
              GetNormalIndexCP (), numA, other.GetNormalIndexCP (), numB))
            return false;

        if (!SameIndexArray (
              GetParamIndexCP (), numA, other.GetParamIndexCP (), numB))
            return false;

        if (!SameIndexArray (
              GetColorIndexCP (), numA, other.GetColorIndexCP (), numB))
            return false;

        if (!SameIndexArray (
              GetFaceIndexCP (), GetFaceIndexCount (), other.GetFaceIndexCP (), other.GetFaceIndexCount ()))
            return false;
        }
    return true;
    }



static void Validate1BasedIndexArrayZeroStructure (MeshAnnotationVector &description, Utf8CP name,
    const int32_t * indices, size_t numIndex,
    const int32_t * templateIndices
    )
    {
    if (numIndex == 0 || indices == nullptr || templateIndices == 0)
        return;         // higher level has to decide if this ok

    for (size_t i = 0; i < numIndex; i++)
        description.Assert ((indices[i] == 0) == (templateIndices[i] == 0), "Zeros in different places in parallel index arrays");
    }

static void Validate1BasedIndexArrayRange (MeshAnnotationVector &description, Utf8CP name, const int32_t * indices, size_t numIndex, size_t numTarget, bool allowNegative)
    {
    if (indices == nullptr)
        return;
    for (size_t i = 0; i < numIndex; i++)
        {
        int index = indices[i];
        int absIndex = abs (index);
        if (absIndex == 0)
            {
            }
        else
            {
            description.Assert ((size_t)absIndex <= numTarget, name, i, index);
            }
        }
    }
//! Apply various checks for indexing structure.
//! @return true if any errors were found.
bool PolyfaceQuery::HasIndexErrors
(
MeshAnnotationVector &description    //!< (optional) array describing errors checked.  Note that there are descriptions of tests even if no errors found.
)
    {
    PolyfaceVectorsP vectors = _AsPolyfaceVectorsP ();
    description.clear ();

    size_t numPointIndex = GetPointIndexCount ();        // all or nothing !!!
    bool isIndexed = GetMeshStyle () == MESH_ELM_STYLE_INDEXED_FACE_LOOPS;
    if (isIndexed)
        { 
        if (vectors != nullptr)
            {
            // In a header, all index arrays expose individual counts.
            size_t numPointIndex1 = vectors->m_pointIndex.size ();
            size_t numParamIndex = vectors->m_paramIndex.size ();
            size_t numNormalIndex = vectors->m_normalIndex.size ();
            size_t numColorIndex = vectors->m_colorIndex.size ();

            description.Assert (numPointIndex == numPointIndex1, "PointIndexCount matches overall index count");
            if (vectors->m_pointIndex.Active ())
                description.Assert (numPointIndex != 0, "PointIndex().Active() but no indices present");
            if (vectors->m_colorIndex.Active ())
                description.Assert (numColorIndex == numPointIndex, "ColorIndex().Active() but count does not match PointIndex().size()");
            if (vectors->m_normalIndex.Active ())
                description.Assert (numNormalIndex == numPointIndex, "NormalIndex().Active() but count does not match PointIndex().size()");
            if (vectors->m_paramIndex.Active ())
                description.Assert (numParamIndex == numPointIndex, "ParamIndex().Active() but count does not match PointIndex().size()");
            }
        // Don't attempt index access if early failures ...
        if (description.GetTotalFail () > 0)
            return true;

        Validate1BasedIndexArrayRange (description, "PointIndex",  GetPointIndexCP (), GetPointIndexCount (), GetPointCount (), true);
        Validate1BasedIndexArrayRange (description, "NormalIndex", GetNormalIndexCP (), GetPointIndexCount (), GetNormalCount (), false);
        Validate1BasedIndexArrayRange (description, "ParamIndex", GetParamIndexCP (), GetPointIndexCount (), GetParamCount (), false);
        Validate1BasedIndexArrayRange (description, "ColorIndex", GetColorIndexCP (), GetPointIndexCount (), GetColorCount (), false);

        Validate1BasedIndexArrayZeroStructure (description, "ParamIndex Zeros", GetParamIndexCP (), GetPointIndexCount (), GetPointIndexCP ());
        Validate1BasedIndexArrayZeroStructure (description, "NormalIndex Zeros", GetNormalIndexCP (), GetPointIndexCount (), GetPointIndexCP ());
        Validate1BasedIndexArrayZeroStructure (description, "ColorIndex Zeros", GetColorIndexCP (), GetPointIndexCount (), GetPointIndexCP ());
        }
    size_t failures = description.GetTotalFail ();
    return failures > 0;
    }

bool PolyfaceQuery::HasIndexErrors ()
    {
    MeshAnnotationVector descriptions (false);
    return HasIndexErrors (descriptions);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
