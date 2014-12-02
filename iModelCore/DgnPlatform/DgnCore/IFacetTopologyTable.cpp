/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/IFacetTopologyTable.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define COMPARE_INT_VALUES(val0, val1) if (val0 < val1) return true; if (val0 > val1) return false;

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      05/2007
+===============+===============+===============+===============+===============+======*/
struct EdgeIndices 
{
    Int32      m_index0;
    Int32      m_index1;

    EdgeIndices () { }
    EdgeIndices (Int32 index0, Int32 index1) : m_index0 (index0), m_index1 (index1)  { }

    bool operator < (EdgeIndices const& rhs) const { return m_index0 == rhs.m_index0 ? m_index1 < rhs.m_index1 : m_index0 < rhs.m_index0; }
};


struct CompareParams { bool operator() (DPoint2dCR param1, DPoint2dCR param2) { return param1.x == param2.x ? (param1.y < param2.y) : (param1.x < param2.x); } };

typedef     bmap <DPoint2d, Int32, CompareParams>           T_ParamIndexMap;        // Untoleranced parameter to index map used to reindex parameters within a single face.
typedef     bmap <Int32, Int32>                             T_IndexRemap;
typedef     bmap <Int32, Int32>                             T_FinToEdgeMap;
typedef     std::multimap <CurveTopologyId, EdgeIndices>    T_EdgeIdToIndicesMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void    initPolyface (PolyfaceHeaderR polyface, IFacetTopologyTable& ftt)
    {
    polyface.ClearTags (0, MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
    polyface.ClearAllVectors ();

    polyface.PointIndex  ().SetStructsPerRow (1);
    polyface.NormalIndex ().SetStructsPerRow (1);
    polyface.ParamIndex  ().SetStructsPerRow (1);

    polyface.PointIndex  ().SetActive (true);
    polyface.NormalIndex ().SetActive (false);
    polyface.ParamIndex  ().SetActive (false);
    polyface.Point().SetActive (true);
    polyface.Normal ().SetActive (false);

    if (ftt._GetNormalCount () > 0)
        {
        polyface.Normal ().SetActive (true);
        polyface.NormalIndex ().SetActive (true);
        }

    if (ftt._GetParamUVCount () > 0)
        {
        polyface.Param ().SetActive (true);
        polyface.ParamIndex ().SetActive (true);
        polyface.FaceIndex().SetActive (true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void     initFinToEdgeMap (T_FinToEdgeMap& finToEdgeMap, IFacetTopologyTable& ftt, bool doEdgeHiding)
    {
    Point2dCP                       finEdge  = ftt._GetFinEdge();

    for (int i=0, count = ftt._GetFinEdgeCount(); i<count; i++)
        if (!doEdgeHiding || !ftt._IsHiddenEdge (finEdge[i].y))
            finToEdgeMap.insert (bpair <Int32, Int32> (finEdge[i].x, finEdge[i].y));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void     addEdgeChains (BlockedVector<PolyfaceEdgeChain>& edgeChains, T_EdgeIdToIndicesMap& edgeIdToIndicesMap)
    {
    for (T_EdgeIdToIndicesMap::iterator curr = edgeIdToIndicesMap.begin(); curr != edgeIdToIndicesMap.end(); curr++)
        edgeChains.push_back (PolyfaceEdgeChain (curr->first, curr->second.m_index0, curr->second.m_index1));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IFacetTopologyTable::ConvertToPolyface (PolyfaceHeaderR polyface, IFacetTopologyTable& ftt, IFacetOptionsCR facetOptions)
    {
    bool                        edgeChainsRequired = facetOptions.GetEdgeChainsRequired();
    T_FinToEdgeMap              finToEdgeMap;
    bset<Int32>                 edgeSet;
    T_EdgeIdToIndicesMap        edgeIdToIndicesMap;
        
    initPolyface (polyface, ftt);
    initFinToEdgeMap (finToEdgeMap, ftt, facetOptions.GetEdgeHiding());

    DPoint3dOps::Copy (&polyface.Point (), ftt._GetPoint (), (size_t)ftt._GetPointCount ());

    if (ftt._GetNormalCount () > 0)
        DVec3dOps::Copy   (&polyface.Normal (), ftt._GetNormal (), (size_t)ftt._GetNormalCount ());

    if (ftt._GetParamUVCount () > 0)
        DPoint2dOps::Copy  (&polyface.Param (), ftt._GetParamUV(), (size_t)ftt._GetParamUVCount ());

    BlockedVectorIntR polyfacePointIndex  = polyface.PointIndex ();
    BlockedVectorIntR polyfaceNormalIndex = polyface.NormalIndex ();
    BlockedVectorIntR polyfaceParamIndex  = polyface.ParamIndex ();

    // The parasolid terminology is:
    // FIN is an end of an edge, on a a paricular side (like coedge, half edge, or vertex use in various other modelers)
    // Point, ParamUV, and Normal are NUMERIC coordinates
    // VERTEX is a collection of indices into the numeric tables (point, normal, paramUV).
    // several fins can share the same vertex data -- i.e. all the fins that leave a POINT within the same face.

    int         numFacetFin          = ftt._GetFacetFinCount ();
    int const*  ftt_finToVertex      = ftt._GetFinData ();
    int const*  ftt_vertexToPoint    = ftt._GetPointIndex ();
    int const*  ftt_vertexToNormal   = ftt._GetNormalIndex ();
    int const*  ftt_vertexToParam    = ftt._GetParamUVIndex ();
    int const*  ftt_facetToFace      = ftt._GetFacetFace ();                                                                                                                                    
    Point2dCP   ftt_facetFin         = ftt._GetFacetFin ();

    int     thisFace, currentFace  = -1;

    for (int facetIndex = 0, ffIndex0 = 0, ffIndex1; ffIndex0 < numFacetFin; facetIndex++, ffIndex0 = ffIndex1)
        {
        ffIndex1 = ffIndex0 + 1;
        while (ffIndex1 < numFacetFin && ftt_facetFin[ffIndex1].x == ftt_facetFin[ffIndex0].x)
            ffIndex1++;

        if (0 == (thisFace = ftt_facetToFace[facetIndex]) ||
            (facetOptions.GetEdgeHiding() && ftt._IsHiddenFace (thisFace)))
            continue;
        
        if (thisFace != currentFace)
            {
            polyface.SetNewFaceData (NULL);
            currentFace = thisFace;
            }

        for (int ffIndex = ffIndex0; ffIndex < ffIndex1; )
            {
            Int32     vertexIndex = ftt_finToVertex[ffIndex];
            Int32     xyzIndex    = ftt_vertexToPoint[vertexIndex];
            Int32     nextFinIndex;

            if ((nextFinIndex = ++ffIndex) == ffIndex1)
                nextFinIndex = ffIndex0;

            T_FinToEdgeMap::iterator    found = finToEdgeMap.find (nextFinIndex);

            if (found == finToEdgeMap.end())
                {
                polyfacePointIndex.push_back (-xyzIndex - 1);
                }
            else
                {
                polyfacePointIndex.push_back (xyzIndex + 1);
                if (edgeChainsRequired)
                    {
                    CurveTopologyId      curveTopologyId;

                     if (edgeSet.find (found->second) == edgeSet.end() &&
                        ftt._GetEdgeCurveId (curveTopologyId, found->second, true))
                        {
                        edgeSet.insert (found->second);
                        edgeIdToIndicesMap.insert (std::pair <CurveTopologyId, EdgeIndices> (curveTopologyId, EdgeIndices (1 + xyzIndex, 1 + ftt_vertexToPoint[ftt_finToVertex[nextFinIndex]])));
                        }
                    }
                }

            if (NULL != ftt_vertexToNormal)
                polyfaceNormalIndex.push_back (ftt_vertexToNormal[vertexIndex] + 1);

            if (NULL != ftt_vertexToParam)
                polyfaceParamIndex.push_back (ftt_vertexToParam[vertexIndex] + 1);
            }
        polyface.TerminateAllActiveIndexVectors ();
        }

    polyface.SetNewFaceData (NULL);
    if (edgeChainsRequired)           // Edge chains requested...
        addEdgeChains (polyface.EdgeChain(), edgeIdToIndicesMap);

        
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Int32   remapPointIndex (Int32 pointIndex, T_IndexRemap& pointIndexMap, BlockedVectorDPoint3dR polyfacePoints, DPoint3dCP fttPoints)
    {
    T_IndexRemap::iterator  found = pointIndexMap.find (pointIndex);
                
    if (found == pointIndexMap.end())
        {
        Int32       pointIndexRemapped = (Int32) polyfacePoints.size();

        polyfacePoints.push_back (fttPoints [pointIndex]);
        pointIndexMap[pointIndex] = pointIndexRemapped;
        return  pointIndexRemapped;
        }
    else
        {
        return found->second;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt convertFaceFacetsToPolyface (PolyfaceHeaderR polyface, bmap <int, PolyfaceHeaderCP>& facePolyfaces, IFacetTopologyTable& ftt, T_FinToEdgeMap finToEdgeMap, IFacetOptionsCR facetOptions)
    {
    bool        edgeChainsRequired   = facetOptions.GetEdgeChainsRequired();
    int numFacetFin                  = ftt._GetFacetFinCount ();
    int const*  ftt_finToVertex      = ftt._GetFinData ();
    int const*  ftt_vertexToPoint    = ftt._GetPointIndex ();
    int const*  ftt_vertexToNormal   = ftt._GetNormalIndex ();
    int const*  ftt_vertexToParam    = ftt._GetParamUVIndex ();
    int const*  ftt_facetToFace      = ftt._GetFacetFace ();
    DPoint2dCP  ftt_paramUV          = ftt._GetParamUV ();
    DPoint3dCP  ftt_points           = ftt._GetPoint ();
    DVec3dCP    ftt_normals          = ftt._GetNormal(); 
    Point2dCP   ftt_facetFin         = ftt._GetFacetFin ();

    T_IndexRemap            pointIndexMap, normalIndexMap, paramIndexMap;
    T_EdgeIdToIndicesMap    edgeIdToIndicesMap;
    bset<Int32>             edgeSet;

    Int32   thisFace, currentFace  = -1;

    for (int facetIndex = 0, ffIndex0 = 0, ffIndex1; ffIndex0 < numFacetFin; facetIndex++, ffIndex0 = ffIndex1)
        {
        bmap <int, PolyfaceHeaderCP>::const_iterator        foundPolyface;

        ffIndex1 = ffIndex0 + 1;
        while (ffIndex1 < numFacetFin && ftt_facetFin[ffIndex1].x == ftt_facetFin[ffIndex0].x)
            ffIndex1++;

        if (0 == (thisFace = ftt_facetToFace[facetIndex]) ||
            (foundPolyface = facePolyfaces.find (thisFace)) == facePolyfaces.end() ||
            foundPolyface->second != &polyface)
            continue;

        if (thisFace != currentFace)
            {
            polyface.SetNewFaceData (NULL);
            currentFace = thisFace;
            }

        for (int ffIndex = ffIndex0; ffIndex < ffIndex1; )
            {
            int     vertexIndex = ftt_finToVertex[ffIndex];

            // POINT INDICES
            Int32                   pointIndex = ftt_vertexToPoint[vertexIndex], pointIndexRemapped;

            pointIndexRemapped = remapPointIndex (pointIndex, pointIndexMap, polyface.Point(), ftt_points);

            int     nextFinIndex;

            if ((nextFinIndex = ++ffIndex) == ffIndex1)
                nextFinIndex = ffIndex0;

            Int32                   nextPointIndex = ftt_vertexToPoint[ftt_finToVertex[nextFinIndex]], nextPointIndexRemapped;

            nextPointIndexRemapped = remapPointIndex (nextPointIndex, pointIndexMap, polyface.Point(), ftt_points);

            T_FinToEdgeMap::iterator    foundEdge = finToEdgeMap.find (nextFinIndex);

            if (foundEdge == finToEdgeMap.end())
                {
                polyface.PointIndex().push_back (-pointIndexRemapped - 1);
                }
            else
                {
                polyface.PointIndex().push_back (pointIndexRemapped + 1);
                if (edgeChainsRequired)
                    {
                    CurveTopologyId        curveTopologyId;

                    if (edgeSet.find (foundEdge->second) == edgeSet.end() &&
                            ftt._GetEdgeCurveId (curveTopologyId, foundEdge->second, true))
                        {
                        edgeSet.insert (foundEdge->second);
                        edgeIdToIndicesMap.insert (std::pair <CurveTopologyId, EdgeIndices> (curveTopologyId, EdgeIndices (1 + pointIndexRemapped, 1 + nextPointIndexRemapped)));
                        }
                    }
                }

            // NORMAL INDICES
            if (NULL != ftt_vertexToNormal)
                {
                Int32                   normalIndex = ftt_vertexToNormal[vertexIndex], normalIndexRemapped;
                T_IndexRemap::iterator  found = normalIndexMap.find (normalIndex);

                if (found == normalIndexMap.end())
                    {
                    normalIndexRemapped = (Int32) polyface.Normal().size();
                    polyface.Normal().push_back (ftt_normals[normalIndex]);
                    normalIndexMap[normalIndex] = normalIndexRemapped;
                    }
                else
                    {
                    normalIndexRemapped = found->second;
                    }
                polyface.NormalIndex().push_back (normalIndexRemapped + 1);
                }

            if (NULL != ftt_vertexToParam)
                {
                Int32                   paramIndex = ftt_vertexToParam[vertexIndex], paramIndexRemapped;
                T_IndexRemap::iterator  found = paramIndexMap.find (paramIndex);

                if (found == paramIndexMap.end())
                    {
                    paramIndexRemapped = (Int32) polyface.Param().size();
                    polyface.Param().push_back (ftt_paramUV[paramIndex]);
                    paramIndexMap[paramIndex] = paramIndexRemapped;
                    }
                else
                    {
                    paramIndexRemapped = found->second;
                    }
                polyface.ParamIndex().push_back (paramIndexRemapped + 1);
                }
            }
        polyface.TerminateAllActiveIndexVectors ();
        }
    polyface.SetNewFaceData (NULL);
    if (edgeChainsRequired)
        addEdgeChains (polyface.EdgeChain(), edgeIdToIndicesMap);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IFacetTopologyTable::ConvertToPolyfaces
(
bvector <PolyfaceHeaderPtr>&                    polyfaces,
bmap <int, PolyfaceHeaderCP>&                   facePolyfaces,
IFacetTopologyTable&                            ftt,
IFacetOptionsCR                                 facetOptions
)
    {
    T_FinToEdgeMap      finToEdgeMap;
    bset <UInt32>       idsWithSymbology;
    StatusInt           status;

    initFinToEdgeMap (finToEdgeMap, ftt, facetOptions.GetEdgeHiding());

    for (PolyfaceHeaderPtr& polyface: polyfaces)
        {
        initPolyface (*polyface, ftt);

        if (SUCCESS != (status = convertFaceFacetsToPolyface (*polyface, facePolyfaces, ftt, finToEdgeMap, facetOptions)))
            return status;
        }
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
FaceAttachment::FaceAttachment ()
    {
    m_elmColorIndex  = INVALID_COLOR;
    m_elmColorTBGR   = 0;
    m_weight         = 0;
    m_style          = 0;
    m_level          = LEVEL_DEFAULT_LEVEL_ID;
    m_transparency   = 0.0;
    m_material       = NULL;
    m_lineColorTBGR  = 0;
    m_lineColorIndex = DgnColorMap::INDEX_Invalid;
    m_rasterWidth    = 1;
    m_rasterPat      = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
FaceAttachment::FaceAttachment (ElemDisplayParamsCR sourceParams, ViewContextR context)
    {
    ElemDisplayParams elParams;

    elParams = sourceParams;
    // Save "natural" element color index, transparency, and level to resolve effective values...
    m_elmColorIndex  = elParams.GetLineColorIndex ();
    m_elmColorTBGR   = (INVALID_COLOR == m_elmColorIndex ? elParams.GetLineColorTBGR () : 0);
    m_weight         = elParams.GetWeight ();
    m_style          = elParams.GetLineStyle ();
    m_level          = elParams.GetSubLevelId().GetLevel ();
    m_transparency   = elParams.GetTransparency ();

    ElemMatSymb elMatSymb;

    context.CookDisplayParams (elParams, elMatSymb); // Resolve effective values and cook...

    // Store effective color information for use in QvOutput...
    m_lineColorTBGR  = elMatSymb.GetLineColorTBGR ();
    m_lineColorIndex = elMatSymb.GetLineColorIndex ();
    m_rasterWidth    = elMatSymb.GetWidth ();
    m_rasterPat      = elMatSymb.GetRasterPattern ();
    m_material       = elMatSymb.GetMaterial (); // NOTE: Attachments aren't overriden by level material, etc. don't need to store a separate effective value...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void FaceAttachment::ToElemDisplayParams (ElemDisplayParamsR elParams) const
    {
    if (INVALID_COLOR != m_elmColorIndex)
        elParams.SetLineColor (m_elmColorIndex);
    else
        elParams.SetLineColorTBGR (m_elmColorTBGR);

    elParams.SetWeight (m_weight);
    elParams.SetLineStyle (m_style);
    elParams.SetSubLevelId (SubLevelId(m_level));
    elParams.SetTransparency (m_transparency);
    elParams.SetMaterial (m_material, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void FaceAttachment::ToElemMatSymb (ElemMatSymbR elMatSymb) const
    {
    elMatSymb.Init ();
    elMatSymb.SetIndexedLineColorTBGR (m_lineColorIndex, m_lineColorTBGR);
    elMatSymb.SetIndexedFillColorTBGR (m_lineColorIndex, m_lineColorTBGR);
    elMatSymb.SetWidth (m_rasterWidth);
    elMatSymb.SetRasterPattern (m_rasterPat);
    elMatSymb.SetMaterial (m_material); // NOTE: Geometry map should have already been created if needed by CookDisplayParams...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool FaceAttachment::operator==(struct FaceAttachment const& rhs) const
    {
    if (m_elmColorIndex  != rhs.m_elmColorIndex ||
        m_elmColorTBGR   != rhs.m_elmColorTBGR ||
        m_weight         != rhs.m_weight ||
        m_style          != rhs.m_style ||
        m_level          != rhs.m_level ||
        m_transparency   != rhs.m_transparency ||
        m_lineColorTBGR  != rhs.m_lineColorTBGR ||
        m_lineColorIndex != rhs.m_lineColorIndex ||
        m_rasterWidth    != rhs.m_rasterWidth ||
        m_rasterPat      != rhs.m_rasterPat ||
        m_material       != rhs.m_material)
        return false;

#if defined (NEEDS_WORK_MATERIAL)
    if (m_materialDetail.IsValid () && rhs.m_materialDetail.IsValid ())
        {
        if (!(m_materialDetail->Equals (*rhs.m_materialDetail)))
            return false;
        }
    else if (m_materialDetail.IsNull () != rhs.m_materialDetail.IsNull ())
        return false;
#endif

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool FaceAttachment::operator< (struct FaceAttachment const& rhs) const
    {
    return m_elmColorIndex  < rhs.m_elmColorIndex ||
           m_elmColorTBGR   < rhs.m_elmColorTBGR ||
           m_weight         < rhs.m_weight ||
           m_style          < rhs.m_style ||
           m_level          < rhs.m_level ||
           m_transparency   < rhs.m_transparency ||
           m_lineColorTBGR  < rhs.m_lineColorTBGR ||
           m_lineColorIndex < rhs.m_lineColorIndex ||
           m_rasterWidth    < rhs.m_rasterWidth ||
           m_rasterPat      < rhs.m_rasterPat ||
           m_material       < rhs.m_material;
}
