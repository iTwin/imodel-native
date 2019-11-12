/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_defaultRelTol = 1.0e-12;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceConstruction::PolyfaceConstruction(IFacetOptionsR options, double pointMatchTolerance, double paramMatchTolerance, double normalMatchTolerance)
    : m_facetOptions (&options)
    {
    m_polyfacePtr = PolyfaceHeader::CreateVariableSizeIndexed ();
    if (options.GetNormalsRequired ())
        {
        m_polyfacePtr->Normal ().SetActive (true);
        m_polyfacePtr->NormalIndex ().SetActive (true);
        }
    if (options.GetParamsRequired ())
        {
        m_polyfacePtr->Param ().SetActive (true);
        m_polyfacePtr->ParamIndex ().SetActive (true);
        m_polyfacePtr->FaceIndex ().SetActive (true);
        }
    m_coordinateMapPtr = new PolyfaceCoordinateMap (*m_polyfacePtr, pointMatchTolerance, (0.0 == pointMatchTolerance ? s_defaultRelTol : 0.0), paramMatchTolerance, (0.0 == paramMatchTolerance ? s_defaultRelTol : 0.0), normalMatchTolerance, (0.0 == normalMatchTolerance ? s_defaultRelTol : 0.0));
    SetFaceIndex (0);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceHeaderR PolyfaceConstruction::_GetClientMeshR ()
    {
    return *m_polyfacePtr;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PolyfaceHeaderPtr PolyfaceConstruction::_GetClientMeshPtr ()
    {
    return m_polyfacePtr;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
IFacetOptionsR PolyfaceConstruction::_GetFacetOptionsR()
    {
    return *m_facetOptions;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceConstruction::_CollectCurrentFaceRanges ()
    {
    }

/*--------------------------------------------------------------------------------**//**
* Face construction proceeds without filling FaceIndex.
* Save (push_back) enough copies of the currentFaceIndex to make the faceIndexVector catch up to the pointIndexVector (but include terminators everywhere)
*
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceConstruction::_EndFace ()
    {
    m_polyfacePtr->SetNewFaceData (&m_currentFaceData);
    m_currentFaceData.Init ();

    (*m_coordinateMapPtr).SetCurrentParamZ ((double)_IncrementFaceIndex ());                    // So subsequent face parameters get their own param indices pointing to param values that can be
                                                                                                // remapped independent of parameters of other faces.
            
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceConstruction::_SetFaceIndex (size_t index)
    {
    m_currentFaceData.Init ();
    m_currentFaceIndex = index;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_GetFaceIndex () const {return m_currentFaceIndex;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_IncrementFaceIndex ()
    {
    SetFaceIndex (m_currentFaceIndex + 1);
    return m_currentFaceIndex;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
FacetFaceData   PolyfaceConstruction::_GetFaceData () const                     { return m_currentFaceData; }
void            PolyfaceConstruction::_SetFaceData (FacetFaceDataCR data)       { m_currentFaceData = data; }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_FindOrAddPoint (DPoint3dCR data)
    {
    return  (*m_coordinateMapPtr).AddPoint (MultiplyByLocalToWorld (data));
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_FindOrAddNormal (DVec3dCR data)
    {
    DVec3d data1 = MultiplyNormalByLocalToWorld (data);
    return  (*m_coordinateMapPtr).AddNormal (data1);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_FindOrAddParam (DPoint2dCR data)
    {
    return  (*m_coordinateMapPtr).AddParam (data);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_FindOrAddIntColor (uint32_t color)
    {
    size_t index = (*m_polyfacePtr).IntColor ().size ();
    (*m_polyfacePtr).IntColor ().push_back (color);
    return index;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static size_t AddIndex(BlockedVector<int> &dest, int data)
    {
    size_t position = dest.size ();
    dest.SetActive (true);
    dest.push_back (data);
    return position;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void PolyfaceConstruction::_Clear ()
    {
    m_coordinateMapPtr->ClearData ();
    m_polyfacePtr->ClearAllVectors ();
    _SetFaceIndex (0);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddSignedOneBasedPointIndex (int oneBasedIndex)
    {
    return AddIndex ((*m_polyfacePtr).PointIndex (), oneBasedIndex);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddSignedOneBasedNormalIndex (int oneBasedIndex)
    {
    return AddIndex ((*m_polyfacePtr).NormalIndex (), oneBasedIndex);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddSignedOneBasedParamIndex (int oneBasedIndex)
    {
    return AddIndex ((*m_polyfacePtr).ParamIndex (), oneBasedIndex);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddSignedOneBasedColorIndex (int oneBasedIndex)
    {
    return AddIndex ((*m_polyfacePtr).ColorIndex (), oneBasedIndex);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddPointIndex (size_t zeroBasedIndex, bool visible)
    {
    int oneBasedIndex = (int)zeroBasedIndex + 1;
    if (!visible)
        oneBasedIndex = - oneBasedIndex;
    return AddIndex ((*m_polyfacePtr).PointIndex (), oneBasedIndex);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddNormalIndex (size_t zeroBasedIndex)
    {
    int oneBasedIndex = (int)(zeroBasedIndex + 1);
    return AddIndex ((*m_polyfacePtr).NormalIndex (), oneBasedIndex);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddParamIndex (size_t zeroBasedIndex)
    {
    int oneBasedIndex = (int)(zeroBasedIndex + 1);
    return AddIndex ((*m_polyfacePtr).ParamIndex (), oneBasedIndex);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddColorIndex (size_t zeroBasedIndex)
    {
    int oneBasedIndex = (int)(zeroBasedIndex + 1);
    return AddIndex ((*m_polyfacePtr).ColorIndex (), oneBasedIndex);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddFaceIndex (size_t zeroBasedIndex)
    {
    int oneBasedIndex = (int)(zeroBasedIndex + 1);
    return AddIndex ((*m_polyfacePtr).FaceIndex (), oneBasedIndex);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddPointIndexTerminator ()
    {
    return AddIndex ((*m_polyfacePtr).PointIndex (), 0);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddNormalIndexTerminator ()
    {
    return AddIndex ((*m_polyfacePtr).NormalIndex (), 0);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddParamIndexTerminator ()
    {
    return AddIndex ((*m_polyfacePtr).ParamIndex (), 0);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddColorIndexTerminator ()
    {
    return AddIndex ((*m_polyfacePtr).ColorIndex (), 0);
    }

static double s_printTriangles = 0;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void PrintNormal(char const *name, size_t index0, size_t index1, size_t index2, DVec3dCR normal, bool reverse)
    {
    if (reverse)
        printf("%s- (%d,%d,%d) (%lg,%lg,%lg)\n", name, (int)index0, (int)index2, (int)index1, -normal.x, -normal.y, -normal.z);
    else
        printf("%s+ (%d,%d,%d) (%lg,%lg,%lg)\n", name, (int)index0, (int)index1, (int)index2, normal.x, normal.y, normal.z);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void PrintNormal(char const *name, size_t index, DVec3dCR normal, bool reverse)
    {
    if (reverse)
        printf("%s- (%d) (%lg,%lg,%lg)\n", name, (int)index, normal.x, normal.y, normal.z);
    else
        printf("%s+ (%d) (%lg,%lg,%lg)\n", name, (int)index, normal.x, normal.y, normal.z);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddPointIndexTriangle (size_t index0, bool visible0, size_t index1, bool visible1, size_t index2, bool visible2)
    {
    size_t base;
    if (!GetFacetOptionsR ().GetEdgeHiding ())
        {
        visible0 = visible1 = visible2 = true;
        }
    if (GetReverseNewFacetIndexOrder ())
        {
        base = _AddPointIndex (index0, visible2);
        _AddPointIndex (index2, visible1);
        _AddPointIndex (index1, visible0);
        }
    else
        {
        base = _AddPointIndex (index0, visible0);
        _AddPointIndex (index1, visible1);
        _AddPointIndex (index2, visible2);
        }

    _AddPointIndexTerminator ();

    if (s_printTriangles)
        {
        DPoint3d xyz0, xyz1, xyz2;
        xyz0 = m_polyfacePtr->Point()[index0];
        xyz1 = m_polyfacePtr->Point()[index1];
        xyz2 = m_polyfacePtr->Point()[index2];
        DVec3d normal;
        normal.CrossProductToPoints (xyz0, xyz1, xyz2);
        PrintNormal ("XYZ", index0, index1, index2, normal, GetReverseNewFacetIndexOrder ());
        }


    return base;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddNormalIndexTriangle (size_t index0, size_t index1, size_t index2)
    {
    size_t base = _AddNormalIndex (index0);
    if (GetReverseNewFacetIndexOrder ())
        {
        _AddNormalIndex (index2);
        _AddNormalIndex (index1);
        }
    else
        {
        _AddNormalIndex (index1);
        _AddNormalIndex (index2);
        }
    _AddNormalIndexTerminator ();

    if (s_printTriangles)
        {
        DVec3d normal0, normal1, normal2;
        normal0 = m_polyfacePtr->Normal()[index0];
        normal1 = m_polyfacePtr->Normal()[index1];
        normal2 = m_polyfacePtr->Normal()[index2];
        if (GetReverseNewFacetIndexOrder ())
            {
            PrintNormal ("Normal", index0, normal0, GetReverseNewFacetIndexOrder ());
            PrintNormal ("Normal", index2, normal2, GetReverseNewFacetIndexOrder ());
            PrintNormal ("Normal", index1, normal1, GetReverseNewFacetIndexOrder ());
            }
        else
            {
            PrintNormal ("Normal", index0, normal0, GetReverseNewFacetIndexOrder ());
            PrintNormal ("Normal", index1, normal1, GetReverseNewFacetIndexOrder ());
            PrintNormal ("Normal", index2, normal2, GetReverseNewFacetIndexOrder ());
            }
        }


    return base;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddParamIndexTriangle (size_t index0, size_t index1, size_t index2)
    {
    size_t base = _AddParamIndex (index0);
    if (GetReverseNewFacetIndexOrder ())
        {
        _AddParamIndex (index2);
        _AddParamIndex (index1);
        }
    else
        {
        _AddParamIndex (index1);
        _AddParamIndex (index2);
        }
    _AddParamIndexTerminator ();
    return base;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddColorIndexTriangle (size_t index0, size_t index1, size_t index2)
    {
    size_t base = _AddColorIndex (index0);
    if (GetReverseNewFacetIndexOrder ())
        {
        _AddColorIndex (index2);
        _AddColorIndex (index1);
        }
    else
        {
        _AddColorIndex (index1);
        _AddColorIndex (index2);
        }
    _AddColorIndexTerminator ();
    return base;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddPointIndexQuad (size_t index0, bool visible0, size_t index1, bool visible1, size_t index2, bool visible2, size_t index3, bool visible3)
    {
    size_t base;
    if (!GetFacetOptionsR ().GetEdgeHiding ())
        {
        visible0 = visible1 = visible2 = visible3 = true;
        }
    if (3 == GetFacetOptionsR ().GetMaxPerFace ())
        {
        base = _AddPointIndexTriangle (index0, visible0, index1, visible1, index2, false);
        _AddPointIndexTriangle (index2, visible2, index3, visible3, index0, false);
        return base;
        }

    if (GetReverseNewFacetIndexOrder ())
        {
        base = _AddPointIndex (index0, visible3);
        _AddPointIndex (index3, visible2);
        _AddPointIndex (index2, visible1);
        _AddPointIndex (index1, visible0);
        }
    else
        {
        base = _AddPointIndex (index0, visible0);
        _AddPointIndex (index1, visible1);
        _AddPointIndex (index2, visible2);
        _AddPointIndex (index3, visible3);
        }
    _AddPointIndexTerminator ();
    return base;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddNormalIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3)
    {
    size_t base;
    if (3 == GetFacetOptionsR ().GetMaxPerFace ())
        {
        base = _AddNormalIndexTriangle (index0, index1, index2);
        _AddNormalIndexTriangle (index2, index3, index0);
        return base;
        }

    base = _AddNormalIndex (index0);
    if (GetReverseNewFacetIndexOrder ())
        {
        _AddNormalIndex (index3);
        _AddNormalIndex (index2);
        _AddNormalIndex (index1);
        }
    else
        {
        _AddNormalIndex (index1);
        _AddNormalIndex (index2);
        _AddNormalIndex (index3);
        }
    _AddNormalIndexTerminator ();
    return base;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddParamIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3)
    {
    size_t base;
    if (3 == GetFacetOptionsR ().GetMaxPerFace ())
        {
        base = _AddParamIndexTriangle (index0, index1, index2);
        _AddParamIndexTriangle (index2, index3, index0);
        return base;
        }

    base = _AddParamIndex (index0);
    if (GetReverseNewFacetIndexOrder ())
        {
        _AddParamIndex (index3);
        _AddParamIndex (index2);
        _AddParamIndex (index1);
        }
    else
        {
        _AddParamIndex (index1);
        _AddParamIndex (index2);
        _AddParamIndex (index3);
        }
    _AddParamIndexTerminator ();
    return base;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t PolyfaceConstruction::_AddColorIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3)
    {
    size_t base;
    if (3 == GetFacetOptionsR ().GetMaxPerFace ())
        {
        base = _AddColorIndexTriangle (index0, index1, index2);
        _AddColorIndexTriangle (index2, index3, index0);
        return base;
        }

    base = _AddColorIndex (index0);
    if (GetReverseNewFacetIndexOrder ())
        {
        _AddColorIndex (index3);
        _AddColorIndex (index2);
        _AddColorIndex (index1);
        }
    else
        {
        _AddColorIndex (index1);
        _AddColorIndex (index2);
        _AddColorIndex (index3);
        }
    _AddColorIndexTerminator ();
    return base;
    }
static double DEFAULT_CREASE_DEGREES = 45.0;
static double DEFAULT_CONE_DEGREES   = 90.0;
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceConstruction::_AddPolyface (PolyfaceQueryCR source, size_t drawMethodIndex)
    {
    bool    addNormals  = GetFacetOptionsR ().GetNormalsRequired () && 0 == source.GetNormalCount ();
    bool    addParams   = GetFacetOptionsR ().GetParamsRequired () && 0 == source.GetParamCount ();
    bool    addFaceData = GetFacetOptionsR ().GetParamsRequired () && 0 == source.GetFaceCount();
    bool    addEdgeChains = GetFacetOptionsR().GetEdgeChainsRequired() && 0 == source.GetEdgeChainCount();
    int     destMaxPerFace = GetFacetOptionsR ().GetMaxPerFace ();
    size_t  minPerFace, maxPerFace;

    source.CollectPerFaceCounts (minPerFace, maxPerFace);  
    bool triangulate = destMaxPerFace != 0 && destMaxPerFace < (int)maxPerFace;

    if (!triangulate && !addNormals && !addParams && !addEdgeChains && !addFaceData)
        return AddPolyface_matched (source);
    // we need to make changes in a copy of the source.
    PolyfaceHeaderPtr   workingSource = PolyfaceHeader::CreateVariableSizeIndexed ();
    workingSource->CopyFrom (source);
    workingSource->ReplicateMissingIndexArrays ();
    if (addNormals)
        workingSource->BuildApproximateNormals (Angle::DegreesToRadians (DEFAULT_CREASE_DEGREES), Angle::DegreesToRadians (DEFAULT_CONE_DEGREES), GetFacetOptionsR().GetHideSmoothEdgesWhenGeneratingNormals());
    if (addParams)
        {
        FacetParamMode paramMode = GetFacetOptionsR ().GetParamMode ();
        LocalCoordinateSelect selector = LOCAL_COORDINATE_SCALE_UnitAxesAtStart;
        if (paramMode == FACET_PARAM_01BothAxes)
            selector = LOCAL_COORDINATE_SCALE_01RangeBothAxes;
        else if (paramMode == FACET_PARAM_01LargerAxis)
            selector = LOCAL_COORDINATE_SCALE_01RangeLargerAxis;
        else
            selector = LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft;

        workingSource->BuildPerFaceParameters (selector);
        }

    if (addFaceData)
        workingSource->BuildPerFaceFaceData ();
        
    if (!source.HasConvexFacets() && GetFacetOptionsR().GetConvexFacetsRequired())
        workingSource->Triangulate (3);
    else if (triangulate)
        workingSource->Triangulate (destMaxPerFace);

    if (addEdgeChains)
        workingSource->AddEdgeChains (drawMethodIndex);

    return AddPolyface_matched (*workingSource);
    }

#ifdef checkSizes
void unusedOK (size_t value)
    {
    }
#endif
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceConstruction::AddPolyface_matched (PolyfaceQueryCR source)
    {
    if (m_polyfacePtr->GetNumPerFace() != source.GetNumPerFace())
        {
        int numPerFaceDest = m_polyfacePtr->GetNumPerFace ();
        int numPerFaceSource = source.GetNumPerFace ();
        size_t numDestPoint        = m_polyfacePtr->GetPointCount ();
        size_t numDestPointIndex   = m_polyfacePtr->GetPointIndexCount (); 
        if (numPerFaceDest != numPerFaceSource
            && numDestPoint == 0
            && numDestPointIndex == 0)
            {
            m_polyfacePtr->SetNumPerFace (numPerFaceSource);
            
            }
        else
            return false;
        }

    if (0 == m_polyfacePtr->GetPointCount ())
        m_polyfacePtr->SetTwoSided (source.GetTwoSided ());
    else 
        m_polyfacePtr->SetTwoSided (m_polyfacePtr->GetTwoSided () | source.GetTwoSided ());

    bvector <size_t>        pointRemap(source.GetPointCount());
    bvector <size_t>        normalRemap(source.GetNormalCount());
    bvector <size_t>        paramRemap(source.GetParamCount());
    bvector <size_t>        faceRemap(source.GetFaceCount());

    DPoint3dCP              points = source.GetPointCP();
    DPoint2dCP              params = source.GetParamCP();
    DVec3dCP                normals = source.GetNormalCP();
    //uint32_t const *        intColors = source.GetIntColorCP ();
    FacetFaceDataCP         faceData = source.GetFaceDataCP();
    PolyfaceEdgeChainCP     edgeChain = source.GetEdgeChainCP();
    int32_t const*            pointIndex = source.GetPointIndexCP();
    int32_t const*            paramIndex = source.GetParamIndexCP();
    int32_t const*            normalIndex = source.GetNormalIndexCP();
    int32_t const*            colorIndex = source.GetColorIndexCP();
    int32_t const*            faceIndex = source.GetFaceIndexCP();
    size_t faceIndexCount = source.GetFaceIndexCount ();
    size_t pointIndexCount = source.GetPointIndexCount ();
    if ((0 != faceIndexCount) && (faceIndexCount != pointIndexCount))
        {
        faceIndex = nullptr;
        BeAssert (faceIndexCount == pointIndexCount);
        }
    if (!m_polyfacePtr->NormalIndex ().Active ())
        normalIndex = NULL;
    if (!m_polyfacePtr->ParamIndex ().Active ())
        paramIndex = NULL;
    if (!m_polyfacePtr->ColorIndex ().Active ())
        colorIndex = NULL;

    if (normalIndex == NULL && 
        source.GetPointCount() == source.GetNormalCount())
        normalIndex = pointIndex;

    if (paramIndex == NULL && 
        source.GetPointCount() == source.GetParamCount())
        paramIndex = pointIndex;

    for (size_t i=0; i<source.GetPointCount(); i++)
        pointRemap[i] = (int) FindOrAddPoint (points[i]);

    for (size_t i=0; i<source.GetParamCount(); i++)
        paramRemap[i] = (int) FindOrAddParam (params[i]);

    // If we have no params but they've been requested, fill in.
    if (m_polyfacePtr->Param ().Active () && 0 == source.GetParamCount () && 0 == m_polyfacePtr->GetParamCount ())
        paramRemap.push_back((int)FindOrAddParam (DPoint2d::From (0.0, 0.0)));

    for (size_t i=0; i<source.GetNormalCount(); i++)
        normalRemap[i] = (int) FindOrAddNormal (normals[i]);

    for (size_t i=0; i<source.GetFaceCount(); i++)
        {
        faceRemap[i] = m_polyfacePtr->FaceData().size();
        m_polyfacePtr->FaceData ().push_back (faceData[i]);
        }

    for (size_t i=0; i<source.GetPointIndexCount(); i++)
        {
        if (0 == pointIndex[i])
            {
            m_polyfacePtr->TerminateAllActiveIndexVectors ();
            }
        else
            {
            _AddPointIndex (pointRemap[abs(pointIndex[i])-1], pointIndex[i] >= 0);
            if (NULL != normalIndex)
                _AddNormalIndex (normalRemap[abs(normalIndex[i])-1]);

            if (NULL != paramIndex)
                _AddParamIndex (paramRemap[abs(paramIndex[i])-1]);
            else if (m_polyfacePtr->Param ().Active ())
                _AddParamIndex (0);

            if (NULL != faceIndex)
                _AddFaceIndex (faceRemap[faceIndex[i]-1]);

            if (NULL != colorIndex)
                _AddColorIndex (colorIndex[i] - 1);
            }
        }

    // push_back() with realloc is hot-spot for performance...
    m_polyfacePtr->EdgeChain().reserve(m_polyfacePtr->EdgeChain().size() + source.GetEdgeChainCount());
    for (size_t i=0; i<source.GetEdgeChainCount(); i++)
        {
        int32_t const*            indexCP = edgeChain[i].GetIndexCP();
        PolyfaceEdgeChain       newChain (edgeChain[i].GetId());

        newChain.ReserveIndices(edgeChain[i].GetIndexCount());

        // Needs work - remap ids.
        for (size_t j=0; j<edgeChain[i].GetIndexCount(); j++)
            newChain.AddIndex ((int32_t) (1 + pointRemap[indexCP[j]-1]));

        m_polyfacePtr->EdgeChain().push_back (newChain);
        }
#ifdef checkSizes
    size_t numPointIndex = m_polyfacePtr->PointIndex().size ();
    size_t numParamIndex = m_polyfacePtr->ParamIndex().size ();
    size_t numNormalIndex = m_polyfacePtr->NormalIndex().size ();
    unusedOK (numPointIndex);
    unusedOK (numParamIndex);
    unusedOK (numNormalIndex);
#endif
    assert (m_polyfacePtr->PointIndex().size() == m_polyfacePtr->ParamIndex().size()  || 0 == m_polyfacePtr->ParamIndex().size());
    assert (m_polyfacePtr->PointIndex().size() == m_polyfacePtr->NormalIndex().size() || 0 == m_polyfacePtr->NormalIndex().size());

    return true;
    }




END_BENTLEY_GEOMETRY_NAMESPACE                                  
