/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

void PolyfaceHeader::RemoveTwoEdgeFacesFromVariableSizeOneBasedMesh ()
    {
    if (m_meshStyle == MESH_ELM_STYLE_INDEXED_FACE_LOOPS && GetNumPerFace () <= 1)
        {
        size_t numOriginalIndex = m_pointIndex.size ();
        size_t numAcceptedIndex = 0;
        bool params = m_paramIndex.Active ();
        bool colors = m_colorIndex.Active ();
        bool normals = m_normalIndex.Active ();
        bool faceIndices = m_faceIndex.Active ();
        size_t i0 = 0;
        size_t i1 = 0;
        for (; i0 < numOriginalIndex; i0 = i1)
            {
            for (; i1 < numOriginalIndex; i1++)
                {
                if (m_pointIndex[i1] == 0)
                    break;
                }
            size_t numEdge = i1 - i0;
            if (i1 < numOriginalIndex)  // search terminated at a zero.   "include" the zero in the active block.
                i1 += 1;
            size_t n = i1 - i0;   // total number of indices in block (including trailing zero)
            if (numEdge < 3)
                {
                // skip
                }
            else
                {
                if (i0 > numAcceptedIndex)
                    {
                    // Copy the new block backwards.
                    for (size_t i = 0; i < n; i++)
                        m_pointIndex[numAcceptedIndex + i] = m_pointIndex[i0 + i];
                    if (params)
                        for (size_t i = 0; i < n; i++)
                            m_paramIndex[numAcceptedIndex + i] = m_paramIndex[i0 + i];
                    if (normals)
                        for (size_t i = 0; i < n; i++)
                            m_normalIndex[numAcceptedIndex + i] = m_normalIndex[i0 + i];
                    if (colors)
                        for (size_t i = 0; i < n; i++)
                            m_colorIndex[numAcceptedIndex + i] = m_colorIndex[i0 + i];
                    if (faceIndices)
                        for (size_t i = 0; i < n; i++)
                            m_faceIndex[numAcceptedIndex + i] = m_faceIndex[i0 + i];
                    }
                numAcceptedIndex += n;
                }
            }
        if (numAcceptedIndex < numOriginalIndex)
            {
            m_pointIndex.resize (numAcceptedIndex);
            if (params)
                m_paramIndex.resize (numAcceptedIndex);
            if (normals)
                m_normalIndex.resize (numAcceptedIndex);
            if (colors)
                m_colorIndex.resize (numAcceptedIndex);
            if (faceIndices)
                m_faceIndex.resize (numAcceptedIndex);
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ()
    {
    bool stat = false;
    switch (m_meshStyle)
        {
        case MESH_ELM_STYLE_INDEXED_FACE_LOOPS:
            if (GetNumPerFace () <= 1)
                {
                SetNumPerFace (0);
                }
            else
                {
                if (m_pointIndex.Active ())
                    m_pointIndex.ConvertBlockedToZeroTerminated();
                if (m_normalIndex.Active ())
                    m_normalIndex.ConvertBlockedToZeroTerminated();
                if (m_paramIndex.Active ())
                    m_paramIndex.ConvertBlockedToZeroTerminated();
                if (m_colorIndex.Active ())
                    m_colorIndex.ConvertBlockedToZeroTerminated();
                if (m_faceIndex.Active ())
                    m_faceIndex.ConvertBlockedToZeroTerminated();
                SetNumPerFace (0);
                }
            stat = true;
            break;

        case MESH_ELM_STYLE_TRIANGLE_GRID:
        case MESH_ELM_STYLE_QUAD_GRID:
            {
            //size_t test = m_pointIndex.StructsPerRow ();
            size_t numPerRow = GetNumPerRow ();//
            assert (numPerRow == m_point.StructsPerRow ());
            size_t numCompleteRow = m_point.NumCompleteRows ();
            bool triangulated = m_meshStyle == MESH_ELM_STYLE_TRIANGLE_GRID;
            if (m_point.Active ())
                {
                m_pointIndex.AddTerminatedGridBlocks (numCompleteRow, numPerRow,
                            1, numPerRow,
                            triangulated,
                            true, 1, 0);
                m_point.SetStructsPerRow (1);
                }

            if (m_normal.Active ())
                {
                m_normalIndex.AddTerminatedGridBlocks (numCompleteRow, numPerRow,
                            1, numPerRow,
                            triangulated,
                            true, 1, 0);
                m_normal.SetStructsPerRow (1);
                }

            if (m_param.Active ())
                {
                m_paramIndex.AddTerminatedGridBlocks (numCompleteRow, numPerRow,
                            1, numPerRow,
                            triangulated,
                            true, 1, 0);
                m_param.SetStructsPerRow (1);
                }

            if (m_intColor.Active ())
                {
                m_colorIndex.AddTerminatedGridBlocks (numCompleteRow, numPerRow,
                            1, numPerRow,
                            triangulated,
                            true, 1, 0);
                m_intColor.SetStructsPerRow (1);
                }
            SetNumPerFace (0);
            stat = true;
            }
            break;

        case MESH_ELM_STYLE_COORDINATE_TRIANGLES:
        case MESH_ELM_STYLE_COORDINATE_QUADS:
            {
            size_t numPerRow = 3;
            if (m_meshStyle == MESH_ELM_STYLE_COORDINATE_QUADS)
                numPerRow = 4;
            //size_t numCompleteRow = m_point.NumCompleteRows ();
            // TFS#97187 numPerRow in point array is 1.  Use point count and known divisor.
            size_t numCompleteRow = m_point.size () / numPerRow;

            if (m_point.Active ())
                {
                m_pointIndex.AddTerminatedSequentialBlocks (numCompleteRow, numPerRow, true, 1, 0);
                m_point.SetStructsPerRow (1);
                }

            if (m_normal.Active ())
                {
                m_normalIndex.AddTerminatedSequentialBlocks (numCompleteRow, numPerRow, true, 1, 0);
                m_normal.SetStructsPerRow (1);
                }

            if (m_param.Active ())
                {
                m_paramIndex.AddTerminatedSequentialBlocks (numCompleteRow, numPerRow, true, 1, 0);
                m_param.SetStructsPerRow (1);
                }

            if (m_intColor.Active ())
                {
                m_colorIndex.AddTerminatedSequentialBlocks (numCompleteRow, numPerRow, true, 1, 0);
                m_intColor.SetStructsPerRow (1);
                }

            SetNumPerFace (0);
            stat = true;
            }
            break;
        default:
            break;
        }
    if (stat)
        {
        m_meshStyle = MESH_ELM_STYLE_INDEXED_FACE_LOOPS;
        //ConvertTableColorToColorIndices ();
        }
    
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @description Detect whether the mesh descriptor has auxiliary colors, and return how they are specified (e.g., the color index family).
*
* @param pMeshED            IN      the mesh to query
* @param pColorsAreByVertex IN      whether the mesh has auxiliary colors that are specified at each mesh vertex (or NULL)
* @param pColorsAreByFace   IN      whether the mesh has auxiliary colors that are specified at each mesh face (or NULL)
* @param pColorsAreBySector IN      whether the mesh has auxiliary colors that are specified at each mesh face sector (or NULL)
* @return true if and only if the mesh has valid auxiliary colors
* @group    "Mesh Elements"
* @bsimethod                                    DavidAssaf      12/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool PolyfaceHeader::HasAuxiliaryColors
(
bool &colorsAreByVertex,
bool &colorsAreByFace,
bool &colorsAreBySector
)
    {
    colorsAreByVertex = colorsAreByFace = colorsAreBySector = false;
    int select = 0;
    int indexArrayTag = 0;
    if (IntColor ().Active ())
        {
        indexArrayTag = IntColor ().IndexedBy ();
        select = 2;
        }
    else
        return false;

    // unindexed colors are valid iff there is no vertex index array
    if (MESH_ELM_TAG_NONE == indexArrayTag && !PointIndex ().Active ())
        {
        colorsAreByVertex = true;
        return true;
        }


    int indexFamily = MESH_ELM_INDEX_FAMILY_NONE;
    bool indexFamilyFound = true;
    if (ColorIndex ().Tag () == indexArrayTag)
        indexFamily = ColorIndex ().IndexFamily ();
    else if (PointIndex ().Tag () == indexArrayTag)
        indexFamily = PointIndex ().IndexFamily ();
    else if (NormalIndex ().Tag () == indexArrayTag)
        indexFamily = NormalIndex ().IndexFamily ();
    else if (ParamIndex ().Tag () == indexArrayTag)
        indexFamily = ParamIndex ().IndexFamily ();
    else
        indexFamilyFound = false;

    colorsAreByVertex = MESH_ELM_INDEX_FAMILY_BY_VERTEX     == indexFamily;
    colorsAreByFace   = MESH_ELM_INDEX_FAMILY_BY_FACE       == indexFamily;
    colorsAreByVertex = MESH_ELM_INDEX_FAMILY_BY_FACE_LOOP  == indexFamily;
    return indexFamilyFound;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceHeader::PolyfaceHeader() 
    {
    ClearTags (0, MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
    SetNumPerRow (0);
    }

// DEPRECATED
PolyfaceHeaderPtr PolyfaceHeader::New ()
    {
    PolyfaceHeaderP header = new PolyfaceHeader ();
    return header;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceHeaderPtr PolyfaceHeader::CreateVariableSizeIndexed ()
    {
    PolyfaceHeaderP header = new PolyfaceHeader ();
    return header;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::IsVariableSizeIndexed () const
    {
    return GetMeshStyle () == MESH_ELM_STYLE_INDEXED_FACE_LOOPS && GetNumPerFace () <= 1;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceHeaderPtr PolyfaceHeader::CreateFixedBlockIndexed (int numPerBlock)
    {
    if (numPerBlock < 3)
        return NULL;

    PolyfaceHeaderP header = new PolyfaceHeader ();
    header->ClearTags (numPerBlock, MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
    return header;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceHeaderPtr PolyfaceHeader::CreateIndexedMesh (int numPerFace, bvector<DPoint3d> const &points, bvector<int> const &indexData)
    {
    auto mesh =
        numPerFace > 1 ? CreateFixedBlockIndexed (numPerFace) : CreateVariableSizeIndexed ();

    auto &meshPoint = mesh->Point ();
    auto &meshPointIndex = mesh->PointIndex ();
    meshPointIndex.SetActive (true);
    meshPoint.SetActive (true);

    for (auto &xyz : points)
        meshPoint.push_back (xyz);
    for (auto &index : indexData)
        meshPointIndex.push_back (index);

    return mesh;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceHeaderPtr PolyfaceHeader::CreateFixedBlockCoordinates (int numPerBlock)
    {
    if (numPerBlock < 3 || numPerBlock > 4)
        return NULL;
     

    PolyfaceHeaderP header = new PolyfaceHeader ();
    header->ClearTags (numPerBlock,
                numPerBlock == 3 ? MESH_ELM_STYLE_COORDINATE_TRIANGLES : MESH_ELM_STYLE_COORDINATE_QUADS
                );
    return header;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceHeaderPtr PolyfaceHeader::CreateQuadGrid (int numPerRow)
    {
    if (numPerRow < 2)
        return NULL;
    PolyfaceHeaderP header = new PolyfaceHeader ();
    header->ClearTags (4, MESH_ELM_STYLE_QUAD_GRID);
    header->PointIndex ().SetActive (false);
    header->Point ().SetStructsPerRow (numPerRow);
    header->SetNumPerRow (numPerRow);    
    return header;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceHeaderPtr PolyfaceHeader::CreateTriangleGrid (int numPerRow)
    {
    if (numPerRow < 2)
        return NULL;

    PolyfaceHeaderP header = new PolyfaceHeader ();
    header->ClearTags (3, MESH_ELM_STYLE_TRIANGLE_GRID);
    header->PointIndex ().SetActive (false);
    header->Point ().SetStructsPerRow (numPerRow);
    header->SetNumPerRow (numPerRow);
    return header;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PushTriangle(bvector<int> &indices, int i0, int i1, int i2)
    {
    indices.push_back (i0);
    indices.push_back (i1);
    indices.push_back (i2);
    indices.push_back (0);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
Triangulate faces.
@return SUCCESS if all faces triangulated.
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::Triangulate (size_t maxEdge)
    {
    return Triangulate (maxEdge, true, nullptr);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
Triangulate faces.
@return SUCCESS if all faces triangulated.
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::Triangulate (size_t maxEdge, bool hideNewEdges, IPolyfaceVisitorFilter *tester)
    {
    if (maxEdge < 3)
        maxEdge = 3;
    ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    SetActiveFlagsByAvailableData ();
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, true);
    visitor->SetNumWrap (1);    // force closure.  All count checks have to watch this !!!
    PolyfaceHeaderPtr indices = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<int>    newIndices, auxIndices;
    ::Transform localToWorld, worldToLocal, extentSystem;
    bvector<DPoint3d> &facePoints = visitor->Point ();
    //bvector<int> &outputIndices = indices->PointIndex ();
    size_t errors = 0;
    double planarityTol = Angle::SmallAngle ();
    for (visitor->Reset (); visitor->AdvanceToNextFace (); )
        {
        newIndices.clear ();
        bool triangulateThisFacet = tester == nullptr || tester->TestFacet (*visitor);

        if (   triangulateThisFacet
            && facePoints.size () <= maxEdge + 1
            && DPoint3dOps::PrincipalExtents (facePoints, extentSystem, localToWorld, worldToLocal)
            )
            {
            DVec3d xCol, yCol, zCol;
            DPoint3d origin;
            extentSystem.GetOriginAndVectors (origin, xCol, yCol, zCol);

            double axy = DoubleOps::Hypotenuse (xCol.Magnitude (), yCol.Magnitude ());
            double az = zCol.Magnitude ();
            if (az < planarityTol * axy)
                triangulateThisFacet = false;       // It's planar and low edge count.  Nothing to do.
            }

        if (    !triangulateThisFacet
            ||  facePoints.size () <= (size_t) maxEdge + 1
            )
            {
            for (ptrdiff_t i = 0, n = facePoints.size () - 1; i < n; i++)
                newIndices.push_back ((int)i + 1);  // one based loop.
            newIndices.push_back (0);   
            }
        else if (facePoints.size () == 4)   // Triangle with closure point
            {
            PushTriangle (newIndices, 1,2,3);
            }
        else if (facePoints.size () == 5)   // Quad with closure point
            {
            DVec3d cross012 = DVec3d::FromCrossProductToPoints (facePoints[0], facePoints[1], facePoints[2]);
            DVec3d cross023 = DVec3d::FromCrossProductToPoints (facePoints[0], facePoints[2], facePoints[3]);

            DVec3d cross123 = DVec3d::FromCrossProductToPoints (facePoints[1], facePoints[2], facePoints[3]);
            DVec3d cross130 = DVec3d::FromCrossProductToPoints (facePoints[1], facePoints[3], facePoints[0]);

            double dot0 = cross012.DotProduct (cross023);
            double dot1 = cross123.DotProduct (cross130);
            newIndices.clear ();
            if (dot0 > dot1)
                {
                PushTriangle (newIndices, 1,2,-3);
                PushTriangle (newIndices, -1,3,4);
                }
            else
                {
                PushTriangle (newIndices, 1,-2,4);
                PushTriangle (newIndices, -4,2,3);
                }
            }
        else if (!PolygonOps::FixupAndTriangulateSpaceLoops (&newIndices, NULL, NULL,
                        localToWorld, worldToLocal,
                        &facePoints, 0.0, true))
            {
            errors++;
            continue;
            }


        size_t n = newIndices.size ();
        // Prevalidate all indices ...
        size_t indexErrors = 0;
        for (size_t i = 0; i < n; i++)
            {
            int32_t k1 = newIndices[i];
            int32_t k0 = abs (k1) - 1;
            if (k1 != 0)
                {
                if ((size_t)k0 >= visitor->ClientPointIndex ().size ())
                    indexErrors++;
                if (m_paramIndex.Active () && (size_t)k0 >= visitor->ClientParamIndex ().size ())
                    indexErrors++;
                if (m_normalIndex.Active () && (size_t)k0 >= visitor->ClientNormalIndex ().size ())
                    indexErrors++;
                if (m_colorIndex.Active ()  && (size_t)k0 >= visitor->ClientColorIndex ().size ())
                    indexErrors++;
                if (m_faceIndex.Active ()   && (size_t)k0 >= visitor->ClientFaceIndex().size ())
                    indexErrors++;
                }
            }
        if (indexErrors > 0)
            continue;   // quietly skip the face
        for (size_t i = 0; i < n; i++)
            {
            int32_t k1 = newIndices[i];
            int32_t k0 = abs (k1) - 1;
            if (k1 == 0)
                {
                if (m_pointIndex.Active ())
                    indices->m_pointIndex.push_back (0);
                if (m_paramIndex.Active ())
                    indices->m_paramIndex.push_back (0);
                if (m_normalIndex.Active ())
                    indices->m_normalIndex.push_back (0);
                if (m_colorIndex.Active ())
                    indices->m_colorIndex.push_back (0);
                if (m_faceIndex.Active ())
                    indices->m_faceIndex.push_back (0);
                if (m_auxData.IsValid())
                    auxIndices.push_back(0);
                }
            else
                {
                // Transfer indices .. (as one-based)
                int pointIndex = 1 + visitor->ClientPointIndex()[k0];
                if (hideNewEdges && k1 < 0)  // this is an edge added in the interior.
                    pointIndex = -pointIndex;
                else if (!visitor->Visible () [k0])
                    pointIndex = -pointIndex;   // An original edge, but hidden there.

                if (m_pointIndex.Active ())
                    indices->m_pointIndex.push_back (pointIndex);
                if (m_paramIndex.Active ())
                    indices->m_paramIndex.push_back (1 + visitor->ClientParamIndex()[k0]);
                if (m_normalIndex.Active ())
                    {
                    assert ((size_t)k0 < visitor->ClientNormalIndex().size ());
                    indices->m_normalIndex.push_back (1 + visitor->ClientNormalIndex() [k0]);
                    }
                if (m_colorIndex.Active ())
                    {
                    if ((size_t)k0 < visitor->ClientColorIndex ().size ())
                        { 
                        indices->m_colorIndex.push_back (1 + visitor->ClientColorIndex() [k0]);
                        }
                    else
                        {
                        errors++;
                        m_colorIndex.SetActive (false);
                        m_colorIndex.clear ();
                        }
                    }
                if (m_faceIndex.Active ())
                    indices->m_faceIndex.push_back (1 + visitor->ClientFaceIndex() [k0]);
                
                if (m_auxData.IsValid() && nullptr != visitor->GetClientAuxIndexCP())
                    auxIndices.push_back(1 + visitor->GetClientAuxIndexCP()[k0]);
                    
                }
            }
        }
    // ummm... get the blocking data !!!
    m_pointIndex.CopyVectorFrom (indices->m_pointIndex);
    if (m_paramIndex.Active ())
        m_paramIndex.CopyVectorFrom (indices->m_paramIndex);
    if (m_normalIndex.Active ())
        m_normalIndex.CopyVectorFrom (indices->m_normalIndex);
    if (m_colorIndex.Active ())
        m_colorIndex.CopyVectorFrom (indices->m_colorIndex);
    if (m_faceIndex.Active ())
        m_faceIndex.CopyVectorFrom (indices->m_faceIndex);
    if (m_auxData.IsValid())
        m_auxData->GetIndices() = std::move(auxIndices);

    SetNumPerFace (0);
    SetNumPerRow (0);
    SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);

    return errors == 0;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
Triangulate faces.
@return SUCCESS if all faces triangulated.
+--------------------------------------------------------------------------------------*/
BentleyStatus PolyfaceHeader::Triangulate ()
    {
    return Triangulate (3) ? SUCCESS : ERROR;
    }


// Copy the first numCopy ints from sourceIndex to destIndex (in same array)
// Pad with 0 as indicated by padTo
static void CopyAndPad (bvector<int> &data, size_t destIndex, size_t sourceIndex, size_t numCopy, size_t padTo)
    {
    if (destIndex + padTo > data.size ())
        return;
    for (size_t i = 0; i < numCopy; i++)
        data[destIndex + i] = data[sourceIndex + i];
    for (size_t i = numCopy; i < padTo; i++)
        data[destIndex + i] = 0;
    }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::CompactIndexArrays ()
    {
    if (GetMeshStyle () != MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
        return false;
    size_t numPerFace = GetNumPerFace ();
    if (numPerFace == 3)
        return false;
    size_t minPerFace, maxPerFace;
    CollectPerFaceCounts (minPerFace, maxPerFace);
    bool changed = false;
    if (minPerFace >= 3 && maxPerFace <= 4)
        {
        int newNumPerFace = 4;
        if (maxPerFace == 3)
            newNumPerFace = 3;

        // Old faces have at least newNumPerFace entries (counting terminator 0)
        // (Only 3 and 4 are possible!!!)
        // So the visitor will always run on ahead of the packing !!!
        PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, true);

        bvector<size_t> &indexPosition = visitor->IndexPosition ();

        BlockedVectorIntR pointIndex  = PointIndex ();
        BlockedVectorIntR normalIndex = NormalIndex ();
        BlockedVectorIntR paramIndex  = ParamIndex ();
        BlockedVectorIntR colorIndex  = ColorIndex ();
        BlockedVectorIntR faceIndex   = FaceIndex ();
        bool normalActive = normalIndex.Active ();
        bool paramActive = paramIndex.Active ();
        bool colorActive = colorIndex.Active ();
        bool faceIndexActive = faceIndex.Active ();
        size_t numOut = 0;
        for (visitor->Reset (); visitor->AdvanceToNextFace (); )
            {
            size_t numThisFace = visitor->NumEdgesThisFace ();
            size_t readIndex = indexPosition[0];
            CopyAndPad (pointIndex, numOut, readIndex, numThisFace, newNumPerFace);
            if (normalActive)
                CopyAndPad (normalIndex, numOut, readIndex, numThisFace, newNumPerFace);
            if (paramActive)
                CopyAndPad (paramIndex, numOut, readIndex, numThisFace, newNumPerFace);
            if (colorActive)
                CopyAndPad (colorIndex, numOut, readIndex, numThisFace, newNumPerFace);
            if (faceIndexActive)
                CopyAndPad (faceIndex, numOut, readIndex, numThisFace, newNumPerFace);
            numOut += newNumPerFace;            
            }

        pointIndex.resize (numOut);
        if (normalActive)
            normalIndex.resize (numOut);
        if (paramActive)
            paramIndex.resize (numOut);
        if (colorActive)
            colorIndex.resize (numOut);
        if (faceIndexActive)
            faceIndex.resize (numOut);
        SetNumPerFace (newNumPerFace);
        changed = true;
        }
    return changed;
    }


//! Apply a transform to all coordinates of an array of meshes. Optionally reverse index order (to maintain cross product relationships) 
void PolyfaceHeader::Transform
(
bvector<PolyfaceHeaderPtr> &data,
TransformCR trans,
bool reverseIndicesIfMirrored
)
    {
    for (size_t i = 0; i < data.size (); i++)
        data[i]->Transform (trans, reverseIndicesIfMirrored);
    }


/*--------------------------------------------------------------------------------**//**
* Apply a transform to all coordinates. Optionally reverse index order (to maintain cross product relationships)
*
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::Transform
(
TransformCR transform,
bool        reverseIndicesIfMirrored
)
    {
    RotMatrix   matrix, inverseMatrix;

    matrix.InitFrom (transform);

    double      determinant = matrix.Determinant ();    
    bool reverseIndices = reverseIndicesIfMirrored && determinant < 0.0;

    size_t numPoint = Point ().size ();
    size_t numNormal = Normal ().size ();
    BlockedVectorDPoint3dR point = Point ();
    BlockedVectorDVec3dR normal = Normal ();

    for (size_t k = 0; k < numPoint; k++)
        transform.Multiply(point[k]);

    if (Normal ().Active () && inverseMatrix.InverseOf(matrix))
        {
        for (size_t k = 0; k < numNormal; k++)
            {
            inverseMatrix.MultiplyTranspose(normal[k]);
            normal[k].Normalize ();
            }
        }

    if (reverseIndices)
        ReverseIndicesAllFaces (false, true, true, BlockedVectorInt::ForcePositive);        

    double  transformScale = pow (fabs (determinant), 1.0 / 3.0) * (determinant >= 0.0 ? 1.0 : -1.0);
    for (size_t i=0; i<m_faceData.size(); i++)
        m_faceData[i].ScaleDistances (transformScale);

    if (m_auxData.IsValid())
        m_auxData->Transform(transform);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2013
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::ReverseNormals ()
    {
    size_t numNormal = Normal ().size ();
    BlockedVectorDVec3dR normal = Normal ();
    for (size_t k = 0; k < numNormal; k++)
        {
        normal[k].Negate ();
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
template <typename T>
static void CopyAndActivate (BlockedVector<T> & dest, T const *sourceData, size_t sourceCount)
    {
    dest.clear ();
    if (sourceData != NULL && sourceCount > 0)
        {
        dest.Append (sourceData, sourceCount);
        dest.SetActive (true);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::CopyFrom (PolyfaceQueryCR source)
    {
    int numPerFace = source.GetNumPerFace();
    auto numPerRow = source.GetNumPerRow ();
    SetNumPerFace (numPerFace);
    SetMeshStyle (source.GetMeshStyle ());
    SetTwoSided   (source.GetTwoSided   ());
    SetNumPerRow  (numPerRow);

    CopyAndActivate <DPoint3d>      (m_point,       source.GetPointCP (),               source.GetPointCount ());
    CopyAndActivate <DVec3d>        (m_normal,      source.GetNormalCP (),              source.GetNormalCount ());
    CopyAndActivate <DPoint2d>      (m_param,       source.GetParamCP (),               source.GetParamCount ());
    CopyAndActivate <FacetFaceData> (m_faceData,    source.GetFaceDataCP (),            source.GetFaceCount ());
    CopyAndActivate <uint32_t>      (m_intColor,    (uint32_t const*)source.GetIntColorCP (), source.GetColorCount ());

    CopyAndActivate <int>(m_pointIndex,     source.GetPointIndexCP (),  source.GetPointIndexCount ());
    CopyAndActivate <int>(m_normalIndex,    source.GetNormalIndexCP (), source.GetPointIndexCount ());
    CopyAndActivate <int>(m_paramIndex,     source.GetParamIndexCP (),  source.GetPointIndexCount ());
    CopyAndActivate <int>(m_colorIndex,     source.GetColorIndexCP (),  source.GetPointIndexCount ());
    CopyAndActivate <int>(m_faceIndex,      source.GetFaceIndexCP (),   source.GetPointIndexCount ());
   

    CopyAndActivate <PolyfaceEdgeChain> (m_edgeChain,   source.GetEdgeChainCP (),      source.GetEdgeChainCount ());
    
    m_point.SetStructsPerRow (numPerRow);
    m_normal.SetStructsPerRow (numPerRow);
    m_param.SetStructsPerRow (numPerRow);
    m_intColor.SetStructsPerRow (numPerRow);
    m_faceData.SetStructsPerRow (numPerRow);



    int numIndexPerRow = numPerFace;
    if (numIndexPerRow == 0)
        numIndexPerRow = 1;
    m_pointIndex.SetStructsPerRow (numIndexPerRow);
    m_normalIndex.SetStructsPerRow (numIndexPerRow);
    m_paramIndex.SetStructsPerRow (numIndexPerRow);
    m_colorIndex.SetStructsPerRow (numIndexPerRow);
    m_faceIndex.SetStructsPerRow (numIndexPerRow);
    if (source.GetAuxDataCP().IsValid())
        m_auxData = new PolyfaceAuxData(*source.GetAuxDataCP());   // Do we need to do a deep copy here??
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::ReplicateMissingIndexArrays ()
    {
    size_t numPoint = m_point.size ();
    size_t numParam = m_param.size ();
    size_t numNormal = m_normal.size ();
    size_t numPointIndex = m_pointIndex.size ();
    if (numPointIndex == 0)
        return;
    if (m_paramIndex.size () == 0 && numParam == numPoint)
        {
        CopyAndActivate <int>(m_paramIndex, m_pointIndex.data (), numPointIndex);
        m_paramIndex.Abs ();
        }
    if (m_normalIndex.size () == 0 && numNormal == numPoint)
        {
        CopyAndActivate <int>(m_normalIndex, m_pointIndex.data (), numPointIndex);
        m_normalIndex.Abs ();
        }
    if (m_colorIndex.size () == 0 && IntColor ().size () == numPoint)
        {
        CopyAndActivate <int>(m_colorIndex, m_pointIndex.data (), numPointIndex);
        m_colorIndex.Abs ();
        }
    }

PolyfaceHeaderPtr PolyfaceQuery::CloneAsVariableSizeIndexed (PolyfaceQueryCR source) const
    {
    PolyfaceHeaderPtr clone = PolyfaceHeader::CreateVariableSizeIndexed ();
    clone->CopyFrom (source);
    clone->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    return clone;
    }

PolyfaceHeaderPtr PolyfaceQuery::Clone () const
    {
    PolyfaceHeaderPtr clone = PolyfaceHeader::New();
    clone->CopyFrom (*this);
    return clone;
    }
  

template <typename T>
static bool IsCompatibleActiveState (bool active, T const *source, size_t count)
    {
    if (source == nullptr)
        count = 0;
    if (active)
        {
        return count > 0 && source != nullptr;
        }
    else
        {
        return count == 0;    // ??? should there also be enforced (source != nullptr)??
        }
    }

template <typename T>
static bool TestAndCopyBlockedData (BlockedVector<T> &dest,
    const T *source,
    size_t sourceCount,
    bool doCopy)
    {
    if (!IsCompatibleActiveState (dest.Active (), source, sourceCount))
        return false;
    if (doCopy && source != nullptr)
        {
        for (size_t i = 0; i < sourceCount; i ++)
            dest.push_back (source[i]);
        }
    return true;
    }

// Add all indices from source to dest, with (signed) offset correction.
static bool TestAndCopyOneBasedIndex (BlockedVectorInt &dest, const int *source, size_t sourceCount, int offset, bool doCopy)
    {
    if (!IsCompatibleActiveState (dest.Active (), source, sourceCount))
        return false;
    if (doCopy && source != nullptr)
        {
        for (size_t i = 0; i < sourceCount; i++)
            {
            int index = source[i];
            if (index > 0)
                index += offset;
            else if (index < 0)
                index -= offset;
            dest.push_back (index);
            }
        }
    return true;
    }


// Transfer all possible data (but doCopy param allows first-pass call to test compatibility without changing the destination)
static bool DoContentTransfer (PolyfaceHeaderR dest, PolyfaceQueryCR source, bool doCopy)
    {

    if (source.GetNumPerFace () != source.GetNumPerFace ())
        return false;
    if (source.GetMeshStyle () != source.GetMeshStyle ())
        return false;
    if (source.GetTwoSided () != source.GetTwoSided ())
        return false;

    size_t numIndex = source.GetPointIndexCount ();
    if (!TestAndCopyOneBasedIndex (dest.PointIndex (), source.GetPointIndexCP (), numIndex, (int)dest.Point ().size (), doCopy))
        return false;
    if (!TestAndCopyOneBasedIndex (dest.ParamIndex (), source.GetParamIndexCP (), numIndex, (int)dest.Param ().size (), doCopy))
        return false;
    if (!TestAndCopyOneBasedIndex (dest.NormalIndex (), source.GetNormalIndexCP (), numIndex, (int)dest.Normal ().size (), doCopy))
        return false;

    size_t colorOffset = source.GetColorCount ();

    if (!TestAndCopyOneBasedIndex (dest.ColorIndex (), source.GetColorIndexCP (), numIndex, (int)colorOffset, doCopy))
        return false;

    // Needs work -- what about normalPerVertex shared indexing?
    // Needs work -- edge chains???

    if (!TestAndCopyBlockedData (dest.Point (), source.GetPointCP (), source.GetPointCount (), doCopy))
        return false;
    if (!TestAndCopyBlockedData (dest.Param (), source.GetParamCP (), source.GetParamCount (), doCopy))
        return false;
    if (!TestAndCopyBlockedData (dest.Normal (), source.GetNormalCP (), source.GetNormalCount (), doCopy))
        return false;
    if (!TestAndCopyBlockedData (dest.IntColor (), source.GetIntColorCP (), source.GetColorCount (), doCopy))
        return false;
    if (!TestAndCopyOneBasedIndex (dest.FaceIndex (), source.GetFaceIndexCP (), source.GetFaceIndexCount (), (int)dest.GetFaceCount (), doCopy))
        return false;
    if (!TestAndCopyBlockedData (dest.FaceData (), source.GetFaceDataCP (), source.GetFaceCount (), doCopy))
        return false;
    return true;
    }

 /*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2015
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::AddIfMatchedLayout (PolyfaceQueryCR source)
    {
    if (Point ().size () == 0)
        {
        CopyFrom (source);
        return true;
        }
    if (!DoContentTransfer (*this, source, false))
        return false;
    return DoContentTransfer (*this, source, true);
    }



// TRUE if any pair of the 4 indices is identical.  (Caller applies abs!!)
static bool IsDegenerateQuad (int i0, int i1, int i2, int i3)
    {
    return i0 == i1 || i1 == i2 || i2 == i3 || i3 == i0;
    }
static bool InspectFaces_SpecialCases
(
PolyfaceQueryCP mesh,
size_t &numLoop,
size_t &minPerLoop,
size_t &maxPerLoop,
bool   &hasNonPlanarFacets,
bool   &hasNonConvexFacets
)
    {
    // Look for simple cases ...
    if (mesh->GetMeshStyle () == MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
        {
        size_t numIndex = mesh->GetPointIndexCount ();
        uint32_t numPerFace = mesh->GetNumPerFace ();
        if (numPerFace == 3)
            {
            minPerLoop = maxPerLoop = 3;
            numLoop = numIndex / 3;
            hasNonPlanarFacets = false;
            hasNonConvexFacets = false;
            return true;
            }
        else if (numPerFace < 2)
            {
            int const *index = mesh->GetPointIndexCP ();
            // explicit search for terminators ...
            size_t numThisFace = 0;
            numLoop = 0;
            minPerLoop = numIndex;
            maxPerLoop = 0;
            for (size_t i = 0; i < numIndex; i++)
                {
                if (index[i] == 0 || i + 1 == numIndex)
                    {
                    numLoop++;
                    if (numThisFace < minPerLoop)
                        minPerLoop = numThisFace;
                    if (numThisFace > maxPerLoop)
                        maxPerLoop = numThisFace;
                    numThisFace = 0;
                    }
                else
                    {
                    numThisFace++;
                    }
                }
            hasNonPlanarFacets = false;
            hasNonConvexFacets = false;
            if (minPerLoop == 3 && maxPerLoop == 3)
                return true;
            }
        } 
    return false;    
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceQuery::InspectFaces
(
size_t &numLoop,
size_t &minPerLoop,
size_t &maxPerLoop,
bool   &hasNonPlanarFaces,
bool   &hasNonConvexFaces
) const
    {
    if (InspectFaces_SpecialCases (this, numLoop, minPerLoop, maxPerLoop, hasNonPlanarFaces, hasNonConvexFaces))
        return;
    numLoop = 0;
    minPerLoop = GetPointCount() + 1;
    maxPerLoop = 0;
    hasNonPlanarFaces = false;
    hasNonConvexFaces = false;
    static double s_planarTol = 1e-8;

    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
#ifdef ValidateVisitor
    if (!visitor->IsValid ())
        return;
#else
    if (visitor.IsNull())
        return;
#endif

    bool doHardChecks = true;
    ::Transform localToWorld, worldToLocal;
    bvector <DPoint3d>&point = visitor->Point();
    bvector<int> &indices = visitor->ClientPointIndex ();    // 0 based !!!
    for (visitor->Reset ();visitor->AdvanceToNextFace ();)
        {
        size_t numEdge = visitor->NumEdgesThisFace ();
        numLoop++;
        if (numEdge < minPerLoop)
            minPerLoop = numEdge;
        if (numEdge > maxPerLoop)
            maxPerLoop = numEdge;
        if (numEdge > 3 && doHardChecks)
            {
            if (numEdge == 4 && IsDegenerateQuad (indices[0], indices[1], indices[2], indices[3]))
                //abs(indices[0], abs[indices[1], abs[indices[2], abs[indices[3]))
                {
                
                }
            else if (PolygonOps::CoordinateFrame (&point, localToWorld, worldToLocal))
                {
                DPoint3dOps::Multiply (&point, worldToLocal);
                DRange3d localRange = DPoint3dOps::Range (&point);
                double dz = localRange.high.z - localRange.low.z;
                double a = localRange.low.DistanceXY (localRange.high);
                if (fabs (dz) > s_planarTol * a)
                    hasNonPlanarFaces = true;

                if (!hasNonConvexFaces)
                    {
                    if (0 == bsiGeom_testXYPolygonConvex (&point.at(0), (int)numEdge))
                        hasNonConvexFaces = true;
                    }
                // Suppress further geometric testing if all conditions are already detected ...
                if (hasNonConvexFaces && hasNonPlanarFaces)
                    doHardChecks = false;
                }
            }
        }
    if (maxPerLoop == 0)
        minPerLoop = 0;
    }

/*--------------------------------------------------------------------------------**//**
* Determine the number of indices and pads required for a facet with n vertices.
* Return false if count is incompatible with mesh style and numPerFace
*
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool GetIndexCounts (size_t n, uint32_t meshStyle, uint32_t numPerFace, size_t numIndex, size_t &numPad)
    {
    numIndex = n;
    numPad   = 0;
    if (   (meshStyle == MESH_ELM_STYLE_COORDINATE_TRIANGLES && n == 3)
        || (meshStyle == MESH_ELM_STYLE_COORDINATE_QUADS && n == 4)
        )
        {
        numIndex = 0;   // All fine, but no indices in blocked mesh.
        }
    else if (meshStyle == MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
        {
        if (numPerFace > 1)
            {
            if (n > numPerFace)
                return false;
            numPad = numPerFace - n;
            }
        else
            {
            numPad = 1;
            }
        }
    else
        {
        return false;
        }
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::AddPolygon (DPoint3dCP xyz, size_t n, DVec3dCP normal, DPoint2dCP param)
    {
    // Strip off trailing duplicates
    while (n > 1 && xyz[n-1].IsEqual (xyz[0]))
        n--;

    if (n < 3)
        return false;

    size_t basePointIndex  = m_point.size ();
    size_t numIndex = 0, numPad = 0;
    if (!GetIndexCounts (n, GetMeshStyle (), GetNumPerFace (), numIndex, numPad))
        return false;

    m_pointIndex.AddSequentialBlock ((int)basePointIndex + 1, n, 0, numPad);
    DPoint3dOps::Append (&m_point, xyz, n);

    if (normal != NULL)
        {
        size_t baseNormalIndex = m_normal.size ();
        if (m_normal.Active ())
            DVec3dOps::Append (&m_normal, normal, n);
        if (m_normalIndex.Active ())
            m_normalIndex.AddSequentialBlock ((int)baseNormalIndex + 1, n, 0, numPad);
        }

    if (param != NULL)
        {
        size_t baseParamIndex = m_param.size ();
        if (m_param.Active ())
            DPoint2dOps::Append (&m_param, param, n);
        if (m_paramIndex.Active ())
            m_paramIndex.AddSequentialBlock ((int)baseParamIndex + 1, n, 0, numPad);
        }

    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::AddPolygon (bvector<DPoint3d> const &xyz, bvector<DVec3d> const *normal, bvector<DPoint2d> const *param)
    {
    if (0 == xyz.size ())
        return false;

    return AddPolygon (&xyz[0], xyz.size (), normal ? &normal->front () : NULL, param ? &param->front () : NULL);
    }

// Revise SIGNED ONE BASED polyface indices for new ZERO BASED remap.
static void ReassignIndices (bvector<int> &polyfaceIndices, bvector<size_t>&oldIndexToNewIndex)
    {
    size_t numIndex = polyfaceIndices.size ();
    for (size_t i = 0; i < numIndex; i++)
        {
        int oldSigned = polyfaceIndices[i];
        if (oldSigned != 0)
            {
            size_t oldUnsigned = abs (oldSigned) - 1;
            size_t newUnsigned = oldIndexToNewIndex[oldUnsigned];
            int newSigned = (int)(newUnsigned + 1);
            if (oldSigned < 0)
                newSigned = - newSigned;
            polyfaceIndices[i] = newSigned;
            }
        }
    }

template<typename T>
bool RemoveUnusedSignedOneBased
(
bvector<int> &indices, 
bvector<T> &data,
bvector<size_t> &oldDataToNewData,
bvector<T> &newData
)
    {
    newData.clear ();
    size_t numData = data.size ();
    oldDataToNewData.clear ();
    for (size_t i = 0; i < numData; i++)
        oldDataToNewData.push_back (SIZE_MAX);
    size_t errors = 0;
    for (int &index1 : indices)  // ONE BASED
        {
        if (index1 != 0)
            {
            size_t index0 = abs (index1) - 1;  // 0 BASED
            if (index0 >= numData)
                {
                errors++;
                index1 = 1;     // PANIC
                }
            else
                {
                if (oldDataToNewData[index0] == SIZE_MAX)
                    {
                    oldDataToNewData[index0] = newData.size ();
                    newData.push_back (data[index0]);
                    }
                }
            }
        }

//    if (errors != 0)
//        return false;

    data.swap (newData);
    for (int &index1 : indices) // index1 will be modified in place!!!
        {
        if (index1 != 0)
            {
            int index0 = abs (index1) - 1;  // 0 BASED, but can't be negative
            ptrdiff_t newIndex1 = oldDataToNewData[(size_t)index0] + 1;
            if (index1 < 0)
                newIndex1 = - newIndex1;
            index1 = (int)newIndex1;
            }
        }
    return true;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::Compress ()
    {
    if (GetMeshStyle() == MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
        {
        bvector<size_t>oldIndexToNewIndex;
        bvector<DPoint3d>xyzPack;
        RemoveUnusedSignedOneBased (m_pointIndex, m_point, oldIndexToNewIndex, xyzPack);
        DPoint3dOps::Cluster (m_point, &xyzPack, oldIndexToNewIndex);
        m_point.swap (xyzPack);
        ReassignIndices (m_pointIndex, oldIndexToNewIndex);
        if (Param().Active () && ParamIndex ().Active ())
            {
            bvector<DPoint2d> paramPack;
            RemoveUnusedSignedOneBased (m_paramIndex, m_param, oldIndexToNewIndex, paramPack);
            DPoint2dOps::Cluster (m_param, &paramPack, oldIndexToNewIndex);
            m_param.swap (paramPack);
            ReassignIndices (m_paramIndex, oldIndexToNewIndex);
            }

        if (Normal().Active () && NormalIndex ().Active ())
            {
            bvector<DVec3d> packedNormals;
            RemoveUnusedSignedOneBased (m_normalIndex, m_normal, oldIndexToNewIndex, packedNormals);
            DVec3dOps::Cluster (m_normal, &packedNormals, oldIndexToNewIndex);
            m_normal.swap (packedNormals);
            ReassignIndices (m_normalIndex, oldIndexToNewIndex);
            }
        }
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::TryGetMaxSingleFacetParamLength (DVec2dR uvLength)
    {
    uvLength.Zero ();
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, true);
    bvector<DPoint2d> &params = visitor->Param ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        if (params.size () > 0)
            {
            DRange2d uvRange = DRange2d::From (&params[0], (int)params.size ());
            DoubleOps::UpdateMax (uvLength.x, uvRange.high.x - uvRange.low.x);
            DoubleOps::UpdateMax (uvLength.y, uvRange.high.y - uvRange.low.y);
            }
        }
    return true;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::TryGetMaxSingleFacetLocalXYLength (DVec2dR xyLength)
    {
    xyLength.Zero ();
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, true);
    DVec3d xVec, yVec;
    BentleyApi::Transform localToWorld, worldToLocal;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        if (visitor->TryGetLocalFrame (localToWorld, worldToLocal, LOCAL_COORDINATE_SCALE_01RangeBothAxes))
            {
            localToWorld.GetMatrixColumn (xVec, 0);
            localToWorld.GetMatrixColumn (yVec, 1);
            DoubleOps::UpdateMax (xyLength.x, xVec.Magnitude ());
            DoubleOps::UpdateMax (xyLength.y, yVec.Magnitude ());
            }
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+--------------------------------------------------------------------------------------*/
void    PolyfaceHeader::SetNewFaceData (FacetFaceData* faceDataP, size_t endIndex)
    {
    if (!m_faceIndex.Active() || m_faceIndex.size() >= m_pointIndex.size())
        return;

    if (0 == endIndex)
        endIndex = m_pointIndex.size();

    FacetFaceData           faceData;

    if (NULL != faceDataP)
        faceData = *faceDataP;

    // If parameter range is provided (as by the polyface planeset clipper then
    // use it. 
    bool       setParamRange = faceData.m_paramRange.IsNull() && NULL != GetParamCP();

    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, true);

    if (!visitor->MoveToFacetByReadIndex (m_faceIndex.size()))
        {
        BeAssert (false);
        return;
        }
    do
        {
        for (size_t i=0; i< visitor->NumEdgesThisFace(); i++)
            {
            if (setParamRange && nullptr != visitor->GetParamCP())
                faceData.m_paramRange.Extend (visitor->GetParamCP()[i]);
            }

        } while (visitor->AdvanceToNextFace() && visitor->GetReadIndex() < endIndex);


    if (m_param.Active () && !m_param.empty() && faceData.m_paramDistanceRange.IsNull())
        faceData.SetParamDistanceRangeFromNewFaceData (*this, endIndex);

    m_faceData.push_back (faceData);
    m_faceIndex.reserve(endIndex);
    for (size_t i = m_faceIndex.size(); i <endIndex; i++)
        m_faceIndex.push_back (0 == m_pointIndex[i] ? 0 : (int) m_faceData.size());
    }

BlockedVectorDPoint3dR              PolyfaceHeader::Point ()            { return m_point;}
BlockedVectorDPoint2dR              PolyfaceHeader::Param ()            { return m_param;}
BlockedVectorDVec3dR                PolyfaceHeader::Normal ()           { return m_normal;}
BlockedVectorUInt32R                PolyfaceHeader::IntColor ()         { return m_intColor;}
BlockedVector<FacetFaceData>&       PolyfaceHeader::FaceData ()         { return m_faceData; } 
BlockedVector<PolyfaceEdgeChain>&   PolyfaceHeader::EdgeChain ()        { return m_edgeChain; } 
PolyfaceAuxDataPtr&                 PolyfaceHeader::AuxData()           { return m_auxData; }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::ClearTags (uint32_t numPerFace, uint32_t meshStyle)
    {
    SetNumPerFace (numPerFace);
    SetTwoSided (true);
    SetMeshStyle (meshStyle);
    bool activePointIndex = meshStyle == MESH_ELM_STYLE_INDEXED_FACE_LOOPS;
    uint32_t b = numPerFace > 1 ? numPerFace : 1;
    m_point.SetTags         (3, 1, MESH_ELM_TAG_VERTEX_COORDINATES, MESH_ELM_INDEX_FAMILY_NONE, MESH_ELM_TAG_FACE_LOOP_TO_VERTEX_INDICES, true);
    m_pointIndex.SetTags    (1, b, MESH_ELM_TAG_FACE_LOOP_TO_VERTEX_INDICES, MESH_ELM_INDEX_FAMILY_BY_FACE_LOOP, MESH_ELM_INDEX_FAMILY_NONE, activePointIndex);

    m_normal.SetTags        (3, 1, MESH_ELM_TAG_NORMAL_COORDINATES, MESH_ELM_INDEX_FAMILY_NONE, MESH_ELM_TAG_FACE_LOOP_TO_NORMAL_INDICES, false);
    m_normalIndex.SetTags   (1, b, MESH_ELM_TAG_FACE_LOOP_TO_NORMAL_INDICES, MESH_ELM_INDEX_FAMILY_BY_FACE_LOOP, MESH_ELM_INDEX_FAMILY_NONE, false);

    m_param.SetTags         (2, 1, MESH_ELM_TAG_UV_PARAMETERS, MESH_ELM_INDEX_FAMILY_NONE, MESH_ELM_TAG_FACE_LOOP_TO_UV_PARAMETER_INDICES, false);
    m_paramIndex.SetTags    (1, b, MESH_ELM_TAG_FACE_LOOP_TO_UV_PARAMETER_INDICES, MESH_ELM_INDEX_FAMILY_BY_FACE_LOOP, MESH_ELM_INDEX_FAMILY_NONE, false);

    m_intColor.SetTags      (1, 1, MESH_ELM_TAG_INT_COLOR, MESH_ELM_INDEX_FAMILY_NONE, MESH_ELM_TAG_FACE_LOOP_TO_INT_COLOR_INDICES, false);
    m_colorIndex.SetTags    (1, b, MESH_ELM_TAG_FACE_LOOP_TO_INT_COLOR_INDICES, MESH_ELM_INDEX_FAMILY_BY_FACE_LOOP, MESH_ELM_INDEX_FAMILY_NONE, false);

    m_faceIndex.SetTags     (1, b, MESH_ELM_TAG_FACE_LOOP_TO_FACE_INDICES, MESH_ELM_INDEX_FAMILY_BY_FACE_LOOP, MESH_ELM_INDEX_FAMILY_NONE, false);
    }



// In a POLYFACEVECTORS, all indices are one based.  (Yes, ALL OF THEM).  The Point indices use this
// for the sign.   Normal indices have sometimes been negated for internal reasons.  Others are
// one based just for consistency.
// For the single face of the VISITOR, indices are always defined, are always zero-based and positive,
// and always have wraparound back to the start point.
BlockedVectorIntR               PolyfaceHeader::PointIndex  ()       {return m_pointIndex;}
BlockedVectorIntR               PolyfaceHeader::ParamIndex  ()       {return m_paramIndex;}
BlockedVectorIntR               PolyfaceHeader::NormalIndex  ()      {return m_normalIndex;}
BlockedVectorIntR               PolyfaceHeader::ColorIndex  ()       {return m_colorIndex;}
BlockedVectorIntR               PolyfaceHeader::FaceIndex ()         {return m_faceIndex;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::CopyTo (PolyfaceHeader& dest) const
    {
    dest.m_point                = m_point;
    dest.m_param                = m_param;
    dest.m_normal               = m_normal;
    dest.m_intColor             = m_intColor;
    dest.m_pointIndex           = m_pointIndex;
    dest.m_paramIndex           = m_paramIndex;
    dest.m_normalIndex          = m_normalIndex;
    dest.m_colorIndex           = m_colorIndex;
    dest.m_faceIndex            = m_faceIndex;
    dest.m_faceData             = m_faceData;
    dest.SetNumPerFace (GetNumPerFace ());
    dest.SetTwoSided (GetTwoSided ());
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::ClearAllIndexVectors ()
    {
    m_pointIndex.clear ();
    m_normalIndex.clear ();
    m_paramIndex.clear ();
    m_colorIndex.clear ();
    m_faceIndex.clear ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::TerminateAllActiveIndexVectors ()
    {
    if (m_pointIndex.Active () && !m_pointIndex.empty())
        m_pointIndex.push_back (0);

    if (m_normalIndex.Active () && !m_normalIndex.empty())
        m_normalIndex.push_back (0);

    if (m_paramIndex.Active () && !m_paramIndex.empty())
        m_paramIndex.push_back (0);

    if (m_colorIndex.Active () && !m_colorIndex.empty())
        m_colorIndex.push_back (0);

    if (m_faceIndex.Active () && !m_faceIndex.empty() && 0 != m_faceIndex.back())
        m_faceIndex.push_back (0);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::ActivateVectorsForIndexing (PolyfaceQueryR source)
    {
    Point ().SetActive (true);
    PointIndex ().SetActive (true);

    if (source.GetNormalCP () != NULL)
        {
        Normal ().SetActive (true);
        NormalIndex ().SetActive (true);
        }    

    if (source.GetParamCP () != NULL)
        {
        Param ().SetActive (true);
        ParamIndex ().SetActive (true);
        }    

    if (source.GetIntColorCP () != NULL)
        {
        IntColor ().SetActive (true);
        ColorIndex ().SetActive (true);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2014
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::SetActiveFlagsByAvailableData ()
    {
    Point().SetActive (Point ().size () > 0);
    Normal().SetActive (Normal ().size () > 0);
    Param().SetActive (Param ().size () > 0);
    IntColor ().SetActive (IntColor ().size () > 0);
    FaceData ().SetActive (FaceData ().size () > 0);

    PointIndex ().SetActive (PointIndex ().size () > 0 && Point ().size () > 0);
    NormalIndex ().SetActive (NormalIndex ().size () > 0 && Normal ().size () > 0);
    ParamIndex ().SetActive (ParamIndex ().size () > 0 && Param ().size () > 0);
    FaceIndex ().SetActive (FaceIndex ().size () > 0 && FaceData ().size () > 0);
    ColorIndex ().SetActive (ColorIndex ().size () > 0 && IntColor ().size () > 0);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::ActivateVectorsForPolylineIndexing (PolyfaceQueryR source)
    {
    Point ().SetActive (true);
    PointIndex ().SetActive (true);
    Normal ().SetActive (false);
    NormalIndex ().SetActive (false);

    Param ().SetActive (false);
    ParamIndex ().SetActive (false);

    IntColor ().SetActive (false);
    ColorIndex ().SetActive (false);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceHeader::ClearAllVectors ()
    {
    ClearAllIndexVectors ();
    m_point.clear ();
    m_normal.clear ();
    m_param.clear ();
    m_intColor.clear ();
    m_faceIndex.clear ();
    m_faceData.clear ();
    m_edgeChain.clear ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::AddIndexedFacet
    (
    bvector<int> &pointIndices,
    bvector<int> *normalIndices,
    bvector<int> *paramIndices,
    bvector<int> *colorIndices
    )
    {
    return AddIndexedFacet
        (
        pointIndices.size (),
        &pointIndices.at (0),
        normalIndices == NULL ? NULL : &normalIndices->at (0),
        paramIndices == NULL ? NULL : &paramIndices->at (0),
        colorIndices == NULL ? NULL : &colorIndices->at (0)
        );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceHeader::AddIndexedFacet
    (
    size_t n,
    int *pointIndex,
    int *normalIndex,
    int *paramIndex,
    int *colorIndex
    )
    {
    if (m_pointIndex.AddAndTerminate (pointIndex, n))
        {
        m_normalIndex.AddAndTerminate (normalIndex, n);
        m_paramIndex.AddAndTerminate (paramIndex, n);
        m_colorIndex.AddAndTerminate (colorIndex, n);
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+--------------------------------------------------------------------------------------*/
void  PolyfaceHeader::NormalizeParameters ()
    {
    if (m_paramIndex.size() != m_faceIndex.size() || m_paramIndex.empty())
        {
        BeAssert(false);
        return;
        }
    BlockedVectorInt            oldParamIndices = std::move(m_paramIndex);
    BlockedVector<DPoint2d>     oldParams = std::move (m_param);

    for (size_t i=0; i<oldParamIndices.size(); i++)
        {
        if (0 == oldParamIndices.at(i))
            m_paramIndex.push_back(0);
        else
            {
            DPoint2d        normalizedParam;

            m_faceData.at(m_faceIndex.at(i) - 1).ConvertParamToNormalized(normalizedParam, oldParams.at(oldParamIndices.at(i) - 1));
            m_param.push_back(normalizedParam);
            m_paramIndex.push_back((int32_t) m_param.size());
            }
        }
    for (auto& faceData : m_faceData)
        faceData.m_paramRange = DRange2d::From(0.0, 0.0, 1.0, 1.0);
    
    }


END_BENTLEY_GEOMETRY_NAMESPACE
