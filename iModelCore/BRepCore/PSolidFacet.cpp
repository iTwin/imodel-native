/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <BRepCore/SolidKernel.h>
#include <BRepCore/PSolidUtil.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
//! Wrapper class around facets that at least act like Parasolid fin tables.
//=======================================================================================
struct IFacetTopologyTable : BentleyApi::IRefCounted
{
public:

virtual bool        _IsTableValid () = 0;
virtual int         _GetFacetCount () = 0;
virtual int         _GetFinCount () = 0;

virtual DPoint3dCP  _GetPoint () = 0;
virtual int         _GetPointCount () = 0;

virtual int const*  _GetPointIndex () = 0;
virtual int         _GetPointIndexCount () = 0;

virtual DVec3dCP    _GetNormal () = 0;
virtual int         _GetNormalCount () = 0;

virtual int const*  _GetNormalIndex () = 0;
virtual int         _GetNormalIndexCount () = 0;

virtual DPoint2dCP  _GetParamUV () = 0;
virtual int         _GetParamUVCount () = 0;

virtual int const*  _GetParamUVIndex () = 0;
virtual int         _GetParamUVIndexCount () = 0;

virtual int const*  _GetFinData () = 0;
virtual int         _GetFinDataCount () = 0;

virtual int const*  _GetFinFin () = 0;
virtual int         _GetFinFinCount () = 0;

virtual Point2dCP   _GetFacetFin () = 0;
virtual int         _GetFacetFinCount () = 0;

virtual Point2dCP   _GetStripFin () = 0;
virtual int         _GetStripFinCount () = 0;

virtual int const*  _GetStripFaceId () = 0;         // Array of face subElemId (body face index)
virtual int         _GetStripFaceIdCount () = 0;

virtual Point2dCP   _GetFinEdge () = 0;             // NOTE: Parasolid only hidden edge support (fin index, edge tag pair)
virtual int         _GetFinEdgeCount () = 0;

virtual int const*  _GetFacetFace () = 0;           // NOTE: Parasolid only hidden face support (array of face tags)
virtual int         _GetFacetFaceCount () = 0;

virtual bool        _GetEdgeCurveId (CurveTopologyId& edgeId, int32_t edge, bool useHighestId) = 0;

virtual bool        _IsHiddenFace (int32_t entityTag) = 0;
virtual bool        _IsHiddenEdge (int32_t entityTag) = 0;

virtual T_FaceAttachmentsVec const* _GetFaceAttachmentsVec () = 0;
virtual T_FaceToAttachmentIndexMap const* _GetFaceToAttachmentIndexMap () = 0;

public:

//! Translate from IFacetTopologyTable to PolyfaceHeader.
//! @param [in,out] polyface polyface data.  Prior contents are cleared.
//! @param [in] ftt Facet topology table
//! @param [in] facetOptions Facet options
static StatusInt ConvertToPolyface (PolyfaceHeaderR polyface, IFacetTopologyTable& ftt, IFacetOptionsCR facetOptions);

//! Translate from IFacetTopologyTable to multi symbology polyfaces with face ids.
//! @param [in,out] polyfaces Map from face index to the polyfaces.
//! @param [in] facePolyfaces Face color/material attachments nap.
//! @param [in] ftt Facet topology table
//! @param [in] facetOptions Facet options
static StatusInt ConvertToPolyfaces (bvector<PolyfaceHeaderPtr>& polyfaces, bmap<int, PolyfaceHeaderCP>& facePolyfaces, IFacetTopologyTable& ftt, IFacetOptionsCR facetOptions);

}; // IFacetTopologyTable

typedef RefCountedPtr<IFacetTopologyTable> IFacetTopologyTablePtr;

static const double TOLERANCE_ChordAngle        = 0.4;
static const double MESH_TOLERANCE_DIVISOR      = 2.5E2;    // Range diagonal divisor
// unused - static const double HATCH_TOLERANCE_DIVISOR     = 2.5E4;
static const double MIN_MESH_TOLERANCE          = 1e-5;
static const double MIN_STROKE_TOLERANCE        = 0.25;
static const double MAX_STROKE_TOLERANCE        = 1.0;
static const double DEF_STROKE_TOLERANCE        = 0.5;

static const int MAX_FacetsPerStrip             = 4096;     // Max facets per strip returned by ParaSolid facetter.
static const int QV_HIDEFIN                     = -3;       // this finFin value inhibits marking of open edge

static double s_sizeToToleranceRatio            = .5;
static double s_smallFeaturePixels              = 4.0;
static double s_minRangeRelTol                  = 1.0e-5;
static double s_maxRangeIgnoreRelTol            = .3;       // Don't ignore entire body (causes facetter failure).

static double s_maxFacetAngleTol                = 1.00;     // radians
static double s_minFacetAngleTol                = 0.10;     // radians
static double s_defaultFacetAngleTol            = 0.39;     // radians
static double s_facetAngleCurveFactor           = 0.5;
static double s_maxToleranceRatio               = 50000.0;


struct CompareParams { bool operator() (DPoint2dCR param1, DPoint2dCR param2) { return param1.x == param2.x ? (param1.y < param2.y) : (param1.x < param2.x); } };

typedef bmap <DPoint2d, int32_t, CompareParams>                 T_ParamIndexMap; // Untoleranced parameter to index map used to reindex parameters within a single face.
typedef bmap <int32_t, int32_t>                                 T_IndexRemap;
typedef bmap <int32_t, int32_t>                                 T_FinToEdgeMap;
typedef bmap <CurveTopologyId, bvector<PolyfaceEdge>>           T_EdgeIdToPolyfaceEdgeMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void initPolyface (PolyfaceHeaderR polyface, IFacetTopologyTable& ftt)
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
static void initFinToEdgeMap (T_FinToEdgeMap& finToEdgeMap, IFacetTopologyTable& ftt, bool doEdgeHiding)
    {
    Point2dCP finEdge = ftt._GetFinEdge();

    for (int i=0, count = ftt._GetFinEdgeCount(); i<count; i++)
        if (!doEdgeHiding || !ftt._IsHiddenEdge (finEdge[i].y))
            finToEdgeMap.insert (bpair <int32_t, int32_t> (finEdge[i].x, finEdge[i].y));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void addEdgeChains (BlockedVector<PolyfaceEdgeChain>& edgeChains, T_EdgeIdToPolyfaceEdgeMap& edgeIdToIndicesMap)
    {
    for (T_EdgeIdToPolyfaceEdgeMap::iterator curr = edgeIdToIndicesMap.begin(); curr != edgeIdToIndicesMap.end(); curr++)
        edgeChains.push_back(PolyfaceEdgeChain(curr->first, std::move(curr->second)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveTopologyId     getUnidentifiedEdgeId(PK_EDGE_t edge, bmap<PK_EDGE_t, uint32_t> edgeToIdMap)
    {
    auto    found = edgeToIdMap.find(edge);

    if (found != edgeToIdMap.end())
        return CurveTopologyId(CurveTopologyId::Type::BRepUnIdentifiedEdge, found->second);

    uint32_t        newId = (uint32_t)  edgeToIdMap.size();

    edgeToIdMap.Insert(edge, newId);
    return CurveTopologyId (CurveTopologyId::Type::BRepUnIdentifiedEdge, newId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IFacetTopologyTable::ConvertToPolyface (PolyfaceHeaderR polyface, IFacetTopologyTable& ftt, IFacetOptionsCR facetOptions)
    {
    bool                        edgeChainsRequired = facetOptions.GetEdgeChainsRequired();
    T_FinToEdgeMap              finToEdgeMap;
    T_EdgeIdToPolyfaceEdgeMap   edgeIdToPolyfaceEdgeMap;
    bmap<PK_EDGE_t, uint32_t>   edgeToIdMap;

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
            int32_t   vertexIndex = ftt_finToVertex[ffIndex];
            int32_t   xyzIndex    = ftt_vertexToPoint[vertexIndex];
            int32_t   nextFinIndex;

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

                     if (facetOptions.GetOmitBRepEdgeChainIds() || !ftt._GetEdgeCurveId (curveTopologyId, found->second, true))
                        curveTopologyId = getUnidentifiedEdgeId(found->second, edgeToIdMap);

                    PolyfaceEdge             polyfaceEdge(1 + xyzIndex, 1 + ftt_vertexToPoint[ftt_finToVertex[nextFinIndex]]);
                    bvector<PolyfaceEdge>    indices(1, polyfaceEdge);

                    auto  insertPair = edgeIdToPolyfaceEdgeMap.Insert(curveTopologyId, indices);

                    if (!insertPair.second)
                        insertPair.first->second.push_back(polyfaceEdge);
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
    if (edgeChainsRequired) // Edge chains requested...
        addEdgeChains (polyface.EdgeChain(), edgeIdToPolyfaceEdgeMap);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t remapPointIndex (int32_t pointIndex, T_IndexRemap& pointIndexMap, BlockedVectorDPoint3dR polyfacePoints, DPoint3dCP fttPoints)
    {
    T_IndexRemap::iterator  found = pointIndexMap.find (pointIndex);

    if (found == pointIndexMap.end())
        {
        int32_t pointIndexRemapped = (int32_t) polyfacePoints.size();

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
static StatusInt convertFaceFacetsToPolyface (PolyfaceHeaderR polyface, bmap<int, PolyfaceHeaderCP>& facePolyfaces, IFacetTopologyTable& ftt, T_FinToEdgeMap finToEdgeMap, IFacetOptionsCR facetOptions)
    {
    bool        edgeChainsRequired   = facetOptions.GetEdgeChainsRequired();
    int         numFacetFin          = ftt._GetFacetFinCount ();
    int const*  ftt_finToVertex      = ftt._GetFinData ();
    int const*  ftt_vertexToPoint    = ftt._GetPointIndex ();
    int const*  ftt_vertexToNormal   = ftt._GetNormalIndex ();
    int const*  ftt_vertexToParam    = ftt._GetParamUVIndex ();
    int const*  ftt_facetToFace      = ftt._GetFacetFace ();
    DPoint2dCP  ftt_paramUV          = ftt._GetParamUV ();
    DPoint3dCP  ftt_points           = ftt._GetPoint ();
    DVec3dCP    ftt_normals          = ftt._GetNormal();
    Point2dCP   ftt_facetFin         = ftt._GetFacetFin ();

    T_IndexRemap                pointIndexMap, normalIndexMap, paramIndexMap;
    T_EdgeIdToPolyfaceEdgeMap   edgeIdToPolyfaceEdgeMap;
    bmap<PK_EDGE_t, uint32_t>   edgeToIdMap;


    int32_t thisFace, currentFace = -1;

    for (int facetIndex = 0, ffIndex0 = 0, ffIndex1; ffIndex0 < numFacetFin; facetIndex++, ffIndex0 = ffIndex1)
        {
        bmap <int, PolyfaceHeaderCP>::const_iterator foundPolyface;

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
            int32_t pointIndex = ftt_vertexToPoint[vertexIndex], pointIndexRemapped;

            // POINT INDICES
            pointIndexRemapped = remapPointIndex (pointIndex, pointIndexMap, polyface.Point(), ftt_points);

            int     nextFinIndex;

            if ((nextFinIndex = ++ffIndex) == ffIndex1)
                nextFinIndex = ffIndex0;

            int32_t nextPointIndex = ftt_vertexToPoint[ftt_finToVertex[nextFinIndex]], nextPointIndexRemapped;

            nextPointIndexRemapped = remapPointIndex (nextPointIndex, pointIndexMap, polyface.Point(), ftt_points);

            T_FinToEdgeMap::iterator foundEdge = finToEdgeMap.find (nextFinIndex);

            if (foundEdge == finToEdgeMap.end())
                {
                polyface.PointIndex().push_back (-pointIndexRemapped - 1);
                }
            else
                {
                polyface.PointIndex().push_back (pointIndexRemapped + 1);


                if (edgeChainsRequired)
                    {
                    CurveTopologyId curveTopologyId;

                    if (!ftt._GetEdgeCurveId (curveTopologyId, foundEdge->second, true))
                        curveTopologyId = getUnidentifiedEdgeId(foundEdge->second, edgeToIdMap);

                    PolyfaceEdge             polyfaceEdge(1 + pointIndexRemapped, 1 + nextPointIndexRemapped);
                    bvector<PolyfaceEdge>    indices(1, polyfaceEdge);

                    auto  insertPair = edgeIdToPolyfaceEdgeMap.Insert(curveTopologyId, indices);

                    if (!insertPair.second)
                        insertPair.first->second.push_back(polyfaceEdge);
                    }
                }

            // NORMAL INDICES
            if (NULL != ftt_vertexToNormal)
                {
                int32_t                 normalIndex = ftt_vertexToNormal[vertexIndex], normalIndexRemapped;
                T_IndexRemap::iterator  found = normalIndexMap.find (normalIndex);

                if (found == normalIndexMap.end())
                    {
                    normalIndexRemapped = (int32_t) polyface.Normal().size();
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
                int32_t                 paramIndex = ftt_vertexToParam[vertexIndex], paramIndexRemapped;
                T_IndexRemap::iterator  found = paramIndexMap.find (paramIndex);

                if (found == paramIndexMap.end())
                    {
                    paramIndexRemapped = (int32_t) polyface.Param().size();
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
        addEdgeChains (polyface.EdgeChain(), edgeIdToPolyfaceEdgeMap);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IFacetTopologyTable::ConvertToPolyfaces
(
bvector<PolyfaceHeaderPtr>&     polyfaces,
bmap<int, PolyfaceHeaderCP>&    facePolyfaces,
IFacetTopologyTable&            ftt,
IFacetOptionsCR                 facetOptions
)
    {
    T_FinToEdgeMap  finToEdgeMap;
    bset<uint32_t>  idsWithSymbology;
    StatusInt       status;

    initFinToEdgeMap(finToEdgeMap, ftt, facetOptions.GetEdgeHiding());

    for (PolyfaceHeaderPtr& polyface: polyfaces)
        {
        initPolyface(*polyface, ftt);

        if (SUCCESS != (status = convertFaceFacetsToPolyface(*polyface, facePolyfaces, ftt, finToEdgeMap, facetOptions)))
            return status;
        }

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void enableConcurrentFacetting()
    {
    static char facetFuncStr[] = "PK_TOPOL_facet_2";
    char const* facetFuncName = facetFuncStr;
    PK_FUNCTION_t facetFunc = 0;
    PK_FUNCTION_find_o_t findOpts;

    PK_FUNCTION_find_o_m(findOpts);
    PK_FUNCTION_find(1, &facetFuncName, &findOpts, &facetFunc);

    PK_FUNCTION_run_t facetRun = PK_FUNCTION_run_mutable_conc_c;
    PK_THREAD_set_function_run_o_t facetRunOpts;

    PK_THREAD_set_function_run_o_m(facetRunOpts);
    PK_THREAD_set_function_run(1, &facetFunc, &facetRun, &facetRunOpts);
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  04/09
+===============+===============+===============+===============+===============+======*/
struct PSolidFacetTopologyTable : public RefCounted <IFacetTopologyTable>
{
private:

PK_TOPOL_facet_2_r_t        m_table;
T_FaceAttachmentsVec        m_faceAttachmentsVec;
T_FaceToAttachmentIndexMap  m_faceToAttachmentIndexMap;
bset<int32_t>               m_hiddenFaces;
bset<int32_t>               m_hiddenEdges;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidFacetTopologyTable (IBRepEntityCR in, double pixelSize, DRange1dP pixelSizeRange)
    {
    FacetEntity (in, pixelSize, pixelSizeRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidFacetTopologyTable (IBRepEntityCR in, IFacetOptionsR options)
    {
    FacetEntity (in, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
~PSolidFacetTopologyTable ()
    {
    PK_TOPOL_facet_2_r_f (&m_table);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
T_FaceAttachmentsVec const* _GetFaceAttachmentsVec() override {return (m_faceAttachmentsVec.empty() ? NULL : &m_faceAttachmentsVec);};
T_FaceToAttachmentIndexMap const* _GetFaceToAttachmentIndexMap() override {return (m_faceToAttachmentIndexMap.empty() ? NULL : &m_faceToAttachmentIndexMap);};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool    _IsTableValid () override {return m_table.number_of_facets > 0;}
int     _GetFacetCount () override {return m_table.number_of_facets;}
int     _GetFinCount () override {return m_table.number_of_fins;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
int             FindTableIndex (PK_TOPOL_fctab_t tableId)
    {
    for (int iTable=0; iTable < m_table.number_of_tables; iTable++)
        if (tableId == m_table.tables[iTable].fctab)
            return iTable;

    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCP  _GetPoint () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_point_vec_c)))
        return NULL;

    return (DPoint3dCP) m_table.tables[iTable].table.point_vec->vec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetPointCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_point_vec_c)))
        return 0;

    return m_table.tables[iTable].table.point_vec->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int const*  _GetPointIndex () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_data_point_c)))
        return NULL;

    return m_table.tables[iTable].table.data_point_idx->point;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetPointIndexCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_data_point_c)))
        return 0;

    return m_table.tables[iTable].table.data_point_idx->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3dCP    _GetNormal () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_normal_vec_c)))
        return NULL;

    return (DVec3dP) m_table.tables[iTable].table.normal_vec->vec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetNormalCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_normal_vec_c)))
        return 0;

    return m_table.tables[iTable].table.normal_vec->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int const*  _GetNormalIndex () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_data_normal_c)))
        return NULL;

    return m_table.tables[iTable].table.data_normal_idx->normal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetNormalIndexCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_data_normal_c)))
        return 0;

    return m_table.tables[iTable].table.data_normal_idx->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2dCP  _GetParamUV () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_param_uv_c)))
        return NULL;

    return (DPoint2dP) m_table.tables[iTable].table.param_uv->data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetParamUVCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_param_uv_c)))
        return 0;

    return m_table.tables[iTable].table.param_uv->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int const*  _GetParamUVIndex () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_data_param_c)))
        return NULL;

    return m_table.tables[iTable].table.data_param_idx->param;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetParamUVIndexCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_data_param_c)))
        return 0;

    return m_table.tables[iTable].table.data_param_idx->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int const*  _GetFinData () override // fin vertex...
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_data_c)))
        return NULL;

    return m_table.tables[iTable].table.fin_data->data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetFinDataCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_data_c)))
        return 0;

    return m_table.tables[iTable].table.fin_data->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int const*  _GetFinFin () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_fin_c)))
        return NULL;

    return m_table.tables[iTable].table.fin_fin->fin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetFinFinCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_fin_c)))
        return 0;

    return m_table.tables[iTable].table.fin_fin->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
Point2dCP   _GetFacetFin () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_facet_fin_c)))
        return NULL;

    return (Point2dCP) m_table.tables[iTable].table.facet_fin->data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetFacetFinCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_facet_fin_c)))
        return 0;

    return m_table.tables[iTable].table.facet_fin->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
Point2dCP   _GetStripFin () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_strip_boundary_c)))
        return NULL;

    return (Point2dCP) m_table.tables[iTable].table.strip_boundary->data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetStripFinCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_strip_boundary_c)))
        return 0;

    return m_table.tables[iTable].table.strip_boundary->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int const*  _GetStripFaceId () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_strip_face_c)))
        return NULL;

    return m_table.tables[iTable].table.strip_face->face;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetStripFaceIdCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_strip_face_c)))
        return 0 ;

    return m_table.tables[iTable].table.strip_face->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
Point2dCP   _GetFinEdge () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_edge_c)))
        return NULL;

    return (Point2dCP) m_table.tables[iTable].table.fin_edge->data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetFinEdgeCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_edge_c)))
        return 0;

    return m_table.tables[iTable].table.fin_edge->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int const*  _GetFacetFace () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_facet_face_c)))
        return NULL;

    return m_table.tables[iTable].table.facet_face->face;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int     _GetFacetFaceCount () override
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_facet_face_c)))
        return 0;

    return m_table.tables[iTable].table.facet_face->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
*
*     Note:  This method  assumes that the body still exists.   This is true
*            for the instances where they are currently used, but if that changes
*            the curve Ids and hidden states will have to actually be stored in the
*            topology table.
*
+---------------+---------------+---------------+---------------+---------------+------*/
bool    _GetEdgeCurveId (CurveTopologyId& curveTopologyId, int32_t edge, bool useHighestId) override
    {
    return SUCCESS == PSolidTopoId::CurveTopologyIdFromEdge (curveTopologyId, (PK_EDGE_t) edge, useHighestId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool    _IsHiddenEdge (int32_t entity) override  {return m_hiddenEdges.find(entity) != m_hiddenEdges.end();}
bool    _IsHiddenFace (int32_t entity) override  {return m_hiddenFaces.find(entity) != m_hiddenFaces.end();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetFacetTolerances (double *chordTolP, double *angleTolP, DPoint3dCP lowP, DPoint3dCP highP, double pixelSize)
    {
    if (0.0 == pixelSize)
        {
        double  strokeTolerance = DEF_STROKE_TOLERANCE; // tcb->strokeTolerance;

        LIMIT_RANGE (MIN_STROKE_TOLERANCE, MAX_STROKE_TOLERANCE, strokeTolerance);

        *angleTolP  = (strokeTolerance * TOLERANCE_ChordAngle);
        *chordTolP = 0.0;

        if (lowP && highP)
            *chordTolP = (strokeTolerance * lowP->Distance (*highP) / MESH_TOLERANCE_DIVISOR);

        if (*chordTolP <= 0.0)
            *chordTolP = 0.0;
        else
            *chordTolP = (*chordTolP < MIN_MESH_TOLERANCE ? MIN_MESH_TOLERANCE : *chordTolP);
        }
    else
        {
        *chordTolP = s_sizeToToleranceRatio * pixelSize;
        *angleTolP  = msGeomConst_2pi / 7.0;
        }

    // Don't allow a tolerance that is really small compared to the range diagonal.
    // This is redundant of the strokeTol * distance / MESH_TOLERANCE_DIVISOR test...
    // if strokeTol is "relative" that one is ok, if "absolute" that one is confused.
    if (lowP != NULL && highP != NULL)
        {
        double diag = lowP->Distance (*highP);
        double minTol = s_minRangeRelTol * diag;

        if (*chordTolP < minTol)
            *chordTolP = minTol;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
double          RestrictAngleTol (double radians, double defaultRadians, double minRadians, double maxRadians)
    {
    if (radians <= 0.0)
        radians = defaultRadians;

    else if (radians < minRadians)
        radians = minRadians;

    else if (radians > maxRadians)
        radians = maxRadians;

    return radians;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            RemoveHiddenEdges (PK_ENTITY_t entityTag)
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_edge_c)))
        return;

    PK_TOPOL_fcstr_fin_edge_t*  finEdgeP = m_table.tables[iTable].table.fin_edge->data;
    int                         numFinEdge = m_table.tables[iTable].table.fin_edge->length;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_fin_c)))
        return;

    int*        finFinIdxP = m_table.tables[iTable].table.fin_fin->fin;
    int         fin, coFin;

    for (int iFinEdge = 0; iFinEdge < numFinEdge; iFinEdge++, finEdgeP++)
        {
        // if this fin's exterior edge is hidden, mark both co-fin entries so that QV knows not to infer an edge
        fin   = finEdgeP->fin;
        coFin = finFinIdxP[fin];

        if (coFin != QV_HIDEFIN && PSolidAttrib::IsEntityHidden (finEdgeP->edge))
            {
            finFinIdxP[fin] = QV_HIDEFIN;

            if (coFin >= 0)
                finFinIdxP[coFin] = QV_HIDEFIN;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            RemoveHiddenFaces (PK_ENTITY_t entityTag)
    {
    int         iTable;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_strip_boundary_c)))
        return;

    PK_TOPOL_fcstr_strip_boundary_t*    stripFinP = m_table.tables[iTable].table.strip_boundary->data;
    int                                 numStripFin = m_table.tables[iTable].table.strip_boundary->length;
    int*                                numStripFinP = &m_table.tables[iTable].table.strip_boundary->length;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_strip_face_c)))
        return;

    int*        stripFaceP = m_table.tables[iTable].table.strip_face->face;
    int*        numStripFaceP = &m_table.tables[iTable].table.strip_face->length;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_fin_c)))
        return;

    int*        finFinIdxP = m_table.tables[iTable].table.fin_fin->fin;
    int         prevStrip = -1, prevFace = -1, currStrip, currFace, numStripFinNew = 0, numStripFaceNew = 0, fin, coFin;
    bool        stripIsHidden = false;

    for (int iStripFin = 0; iStripFin < numStripFin; iStripFin++)
        {
        currStrip = stripFinP[iStripFin].strip;
        currFace  = stripFaceP[currStrip];

        // find a new strip...
        if (prevStrip != currStrip)
            {
            // ...in a new face...
            if (prevFace != currFace)
                {
                stripIsHidden = PSolidAttrib::IsEntityHidden (currFace);
                prevFace = currFace;
                }

            // if strip is visible, reassign its face (overwrite hidden face)
            if (!stripIsHidden)
                {
                stripFaceP[numStripFaceNew] = currFace;
                numStripFaceNew++;
                }

            prevStrip = currStrip;
            }

        if (stripIsHidden)
            {
            // if this hidden fin's co-fin has a co-fin, remove it
            fin   = stripFinP[iStripFin].fin;
            coFin = finFinIdxP[fin];

            if (coFin >= 0)
                {
                finFinIdxP[fin] = -1;

                if (finFinIdxP[coFin] >= 0)
                    finFinIdxP[coFin] = -1;
                }
            }
        else
            {
            // append a new fin to the visible strip (overwrite hidden strip)
            stripFinP[numStripFinNew].strip = numStripFaceNew - 1;
            stripFinP[numStripFinNew].fin   = stripFinP[iStripFin].fin;
            numStripFinNew++;
            }
        }

    // Update counts to reflect removed faces...
    *numStripFinP  = numStripFinNew;
    *numStripFaceP = numStripFaceNew;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    CompleteTable (PK_ENTITY_t entityTag, IFaceMaterialAttachmentsCP attachments, bool hasHiddenEdge, bool hasHiddenFace)
    {
    // Process hidden edges so they don't display...
    if (hasHiddenEdge)
        {
        RemoveHiddenEdges (entityTag);
        PSolidAttrib::GetHiddenBodyEdges (m_hiddenEdges, entityTag);
        }

    // Process hidden faces so they don't display...
    if (hasHiddenFace)
        {
        RemoveHiddenFaces (entityTag);
        PSolidAttrib::GetHiddenBodyFaces (m_hiddenFaces, entityTag);
        }

    if (nullptr != attachments)
        {
        m_faceAttachmentsVec = attachments->_GetFaceAttachmentsVec();
        PSolidAttrib::PopulateFaceMaterialIndexMap(m_faceToAttachmentIndexMap, entityTag, m_faceAttachmentsVec.size());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            FacetEntity (IBRepEntityCR in, double pixelSize, DRange1dP pixelSizeRange)
    {
    PK_ENTITY_t entityTag = PSolidUtil::GetEntityTag(in);
    Transform   entityTransform = in.GetEntityTransform();

    if (0.0 != pixelSize && !PSolidUtil::HasCurvedFaceOrEdge(entityTag))
        pixelSize = 0.0; // Don't return pixelSizeRange or use pixelSize to exclude small features when not creating size-dependent representation...

    if (nullptr != pixelSizeRange)
        pixelSizeRange->InitNull();

    bool        rangeIsValid = false;
    PK_BOX_t    box;
    DRange3d    range;

    if (PK_ERROR_no_errors == PK_TOPOL_find_box (entityTag, &box))
        {
        range.InitFrom (box.coord[0], box.coord[1], box.coord[2], box.coord[3], box.coord[4], box.coord[5]);
        rangeIsValid = true;
        }

    if (0.0 != pixelSize)
        {
        DVec3d  xColumn;

        entityTransform.GetMatrixColumn (xColumn, 0);

        if (rangeIsValid && nullptr != pixelSizeRange)
            {
            static double sizeDependentRatio = 5.0;
            static double pixelToChordRatio = 0.5;
            static double minRangeRelTol = 1.0e-4;
            static double maxRangeRelTol = 1.5e-2;
            double maxDimension = range.DiagonalDistance() * xColumn.Magnitude(); // Scale to world units
            double minChordTol = minRangeRelTol * maxDimension;
            double maxChordTol = maxRangeRelTol * maxDimension;
            double chordTol = pixelToChordRatio * pixelSize;
            bool isMin = false, isMax = false;

            if (isMin = (chordTol < minChordTol))
                chordTol = minChordTol; // Don't allow chord to get too small relative to BRep size...
            else if (isMax = (chordTol > maxChordTol))
                chordTol = maxChordTol; // Don't keep creating coarser and coarser graphics as you zoom out, at a certain point it just wastes memory/time...

            if (isMin)
                *pixelSizeRange = DRange1d::FromLowHigh(0.0, chordTol * sizeDependentRatio); // Finest tessellation, keep using this as we zoom in...
            else if (isMax)
                *pixelSizeRange = DRange1d::FromLowHigh(chordTol / sizeDependentRatio, DBL_MAX); // Coarsest tessellation, keep using this as we zoom out...
            else
                *pixelSizeRange = DRange1d::FromLowHigh(chordTol / sizeDependentRatio, chordTol * sizeDependentRatio);
            }

        pixelSize /= xColumn.Magnitude (); // Scale pixelSize to kernel units
        }

    double chordTol, angleTol;

    if (rangeIsValid)
        GetFacetTolerances (&chordTol, &angleTol, &range.low, &range.high, pixelSize);
    else
        GetFacetTolerances (&chordTol, &angleTol, nullptr, nullptr, pixelSize);

    bool hasHiddenEdge = PSolidAttrib::HasHiddenEdge (entityTag);
    bool hasHiddenFace = PSolidAttrib::HasHiddenFace (entityTag);

    PK_TOPOL_facet_2_o_t options;

    PK_TOPOL_facet_mesh_2_o_m (options.control);

    if (0.0 != chordTol)
        {
        options.control.is_curve_chord_tol   = PK_LOGICAL_true;
        options.control.curve_chord_tol      = chordTol;

        options.control.is_surface_plane_tol = PK_LOGICAL_true;
        options.control.surface_plane_tol    = chordTol;
        }

    if (0.0 != angleTol)
        {
        options.control.is_surface_plane_ang = PK_LOGICAL_true;
        options.control.surface_plane_ang    = angleTol;

        options.control.is_curve_chord_ang   = PK_LOGICAL_true;
        options.control.curve_chord_ang      = angleTol;
        }

    if (0.0 != pixelSize && rangeIsValid)
        {
        double  maximumIgnore = s_maxRangeIgnoreRelTol * range.low.Distance (range.high);

        options.control.ignore = PK_facet_ignore_absolute_c;
        options.control.ignore_value = s_smallFeaturePixels * pixelSize;

        if (options.control.ignore_value > maximumIgnore)
            options.control.ignore_value = maximumIgnore;
        }

    options.control.max_facet_sides = 3;
    options.control.match = PK_facet_match_topol_c;

    PK_TOPOL_facet_choice_2_o_m (options.choice);

    // NOTE: These options were needed for qv_addFaceTriStrips, which we are no longer using...
    // options.choice.strip_face           = PK_LOGICAL_true;
    // options.choice.fin_fin              = PK_LOGICAL_true;

    options.choice.data_point_idx       = PK_LOGICAL_true;
    options.choice.data_normal_idx      = PK_LOGICAL_true;
    options.choice.data_param_idx       = PK_LOGICAL_true;
    options.choice.point_vec            = PK_LOGICAL_true;
    options.choice.normal_vec           = PK_LOGICAL_true;
    options.choice.param_uv             = PK_LOGICAL_true;

    options.choice.facet_face           = PK_LOGICAL_true;
    options.choice.fin_data             = PK_LOGICAL_true;
    options.choice.facet_fin            = PK_LOGICAL_true;
    options.choice.fin_edge             = PK_LOGICAL_true;

    options.choice.consistent_parms     = PK_facet_consistent_parms_yes_c;
    options.choice.split_strips         = PK_facet_split_strip_yes_c; // TR# 269038.
    options.choice.strip_boundary       = PK_LOGICAL_true;
    options.choice.max_facets_per_strip = MAX_FacetsPerStrip;

    memset (&m_table, 0, sizeof (m_table)); // Not initialized if error?!?

    enableConcurrentFacetting();
    if (SUCCESS != PK_TOPOL_facet_2 (1, &entityTag, NULL, &options, &m_table) || !_IsTableValid ())
        return;

    IFaceMaterialAttachmentsCP attachments = in.GetFaceMaterialAttachments();

    CompleteTable (entityTag, attachments, hasHiddenEdge, hasHiddenFace);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            FacetEntity (IBRepEntityCR in, IFacetOptionsR facetOptions)
    {
    PK_ENTITY_t entityTag = PSolidUtil::GetEntityTag(in);
    Transform   entityTransform = in.GetEntityTransform();

    PK_TOPOL_facet_2_o_t    options;

    PK_TOPOL_facet_mesh_2_o_m (options.control);

    Transform   solidToOutputTransform = entityTransform, outputToSolidTransform;
    DPoint3d    tolerancePoint;

    tolerancePoint.Init (facetOptions.GetChordTolerance (), 0.0, 0.0);
    outputToSolidTransform.InverseOf (solidToOutputTransform);
    outputToSolidTransform.MultiplyMatrixOnly (tolerancePoint);

    double      solidTolerance = tolerancePoint.Magnitude ();
    double      angleTol = RestrictAngleTol (facetOptions.GetAngleTolerance (), s_defaultFacetAngleTol, s_minFacetAngleTol, s_maxFacetAngleTol);

    DRange3d range = in.GetLocalEntityRange();
    if (!range.IsNull())
        {
        double rangeSize = range.low.Distance (range.high);

        if (0.0 != solidTolerance && rangeSize / solidTolerance > s_maxToleranceRatio)
            solidTolerance = rangeSize / s_maxToleranceRatio;

        options.control.curve_chord_tol = options.control.surface_plane_tol = solidTolerance;
        }

    if (0.0 != solidTolerance)
        {
        options.control.is_curve_chord_tol   = PK_LOGICAL_true;
        options.control.is_surface_plane_tol = PK_LOGICAL_true;
        }

    if (0.0 != facetOptions.GetMaxEdgeLength ())
        {
        DPoint3d maxWidthPoint;
        maxWidthPoint.Init (facetOptions.GetMaxEdgeLength (), 0.0, 0.0);
        outputToSolidTransform.MultiplyMatrixOnly (maxWidthPoint);
        options.control.is_max_facet_width  = PK_LOGICAL_true;
        options.control.max_facet_width     = maxWidthPoint.Magnitude ();
        }

    if (0.0 != facetOptions.GetBRepIgnoredFeatureSize())
        {
        DPoint3d ignoredFeatureSizePoint;
        ignoredFeatureSizePoint.Init(facetOptions.GetBRepIgnoredFeatureSize(), 0.0, 0.0);
        outputToSolidTransform.MultiplyMatrixOnly(ignoredFeatureSizePoint);
        options.control.ignore          = PK_facet_ignore_absolute_c;
        options.control.ignore_value    = ignoredFeatureSizePoint.Magnitude();
        }

    options.control.is_curve_chord_ang   = PK_LOGICAL_true;
    options.control.curve_chord_ang      = angleTol * s_facetAngleCurveFactor;

    options.control.is_surface_plane_ang = PK_LOGICAL_true;
    options.control.surface_plane_ang    = angleTol;

    int maxPerFace = facetOptions.GetMaxPerFace();
    if (facetOptions.GetCurvedSurfaceMaxPerFace() != maxPerFace && in.HasCurvedFaceOrEdge())
        maxPerFace = facetOptions.GetCurvedSurfaceMaxPerFace();

    options.control.match                = PK_facet_match_topol_c;
    options.control.max_facet_sides      = (maxPerFace > 3 ? maxPerFace : 3);
    options.control.shape                = facetOptions.GetConvexFacetsRequired () ? PK_facet_shape_convex_c : PK_facet_shape_cut_c;

    PK_TOPOL_facet_choice_2_o_m (options.choice);

    options.choice.data_point_idx   = PK_LOGICAL_true;
    options.choice.data_normal_idx  = facetOptions.GetNormalsRequired ();
    options.choice.data_param_idx   = facetOptions.GetParamsRequired ();
    options.choice.point_vec        = PK_LOGICAL_true;
    options.choice.normal_vec       = facetOptions.GetNormalsRequired ();
    options.choice.param_uv         = facetOptions.GetParamsRequired ();
    options.choice.facet_face       = PK_LOGICAL_true;
    options.choice.fin_data         = PK_LOGICAL_true;
    options.choice.facet_fin        = PK_LOGICAL_true;
    options.choice.fin_edge         = PK_LOGICAL_true;

    if (facetOptions.GetParamsRequired ())
        {
        options.choice.consistent_parms = PK_facet_consistent_parms_yes_c;
        options.choice.split_strips     = PK_facet_split_strip_yes_c; // TR# 269038
        options.choice.strip_boundary   = PK_LOGICAL_true;            // TR# 276122
        }

    memset (&m_table, 0, sizeof (m_table)); // Not initialized if error?!?

    // PK_TOPOL_facet_2 in concurrent mode is always significantly slower in our testing,
    // regardless of how many threads are used. The actual concurrency inside PK_TOPOL_facet_2
    // seems to be very poor with many fine-grained locks acquired and released - this overhead
    // eliminates any potential gains. See Parasolid IR 8415939 for more info.
    if (facetOptions.GetBRepConcurrentFacetting())
        enableConcurrentFacetting();

    if (SUCCESS != PK_TOPOL_facet_2 (1, &entityTag, NULL, &options, &m_table) || !_IsTableValid ())
        return;

    IFaceMaterialAttachmentsCP attachments = in.GetFaceMaterialAttachments();

    bool            hasHiddenEdge = false, hasHiddenFace = false;

    if (!facetOptions.GetIgnoreHiddenBRepEntities())
        {
        hasHiddenEdge = PSolidAttrib::HasHiddenEdge (entityTag);
        hasHiddenFace = PSolidAttrib::HasHiddenFace (entityTag);
        }

    CompleteTable (entityTag, attachments, hasHiddenEdge, hasHiddenFace);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static PSolidFacetTopologyTable* CreateNewFacetTable (IBRepEntityCR in, double pixelSize, DRange1dP pixelSizeRange)
    {
    return new PSolidFacetTopologyTable (in, pixelSize, pixelSizeRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
static PSolidFacetTopologyTable* CreateNewFacetTable (IBRepEntityCR in, IFacetOptionsR options)
    {
    return new PSolidFacetTopologyTable (in, options);
    }

}; // PSolidFacetTopologyTable;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr PSolidUtil::FacetEntity(IBRepEntityCR entity, double pixelSize, DRange1dP pixelSizeRange)
    {
    IFacetTopologyTablePtr facetTopo = PSolidFacetTopologyTable::CreateNewFacetTable(entity, pixelSize, pixelSizeRange);

    if (!facetTopo->_IsTableValid())
        return nullptr;

    PolyfaceHeaderPtr mesh = PolyfaceHeader::New();
    IFacetOptionsPtr  facetOptions = IFacetOptions::Create(); // Doesn't matter...

    if (SUCCESS != IFacetTopologyTable::ConvertToPolyface(*mesh, *facetTopo, *facetOptions))
        return nullptr;

    mesh->SetTwoSided(IBRepEntity::EntityType::Solid != entity.GetEntityType());
    mesh->Transform(entity.GetEntityTransform());

    return mesh;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr PSolidUtil::FacetEntity(IBRepEntityCR entity, IFacetOptionsR facetOptions)
    {
    IFacetTopologyTablePtr facetTopo = PSolidFacetTopologyTable::CreateNewFacetTable(entity, facetOptions);

    if (!facetTopo->_IsTableValid())
        return nullptr;

    PolyfaceHeaderPtr mesh = PolyfaceHeader::New();

    if (SUCCESS != IFacetTopologyTable::ConvertToPolyface(*mesh, *facetTopo, facetOptions))
        return nullptr;

    mesh->SetTwoSided(IBRepEntity::EntityType::Solid != entity.GetEntityType());
    mesh->Transform(entity.GetEntityTransform());

    return mesh;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool facetTableToPolyfaces(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<FaceAttachment>& params, IFacetOptionsR facetOptions, IFacetTopologyTable& facetTopo)
    {
    T_FaceToAttachmentIndexMap  const& faceToAttachmentIndexMap = *facetTopo._GetFaceToAttachmentIndexMap();
    T_FaceAttachmentsVec const& faceAttachmentsVec = *facetTopo._GetFaceAttachmentsVec();
    bmap<int, PolyfaceHeaderCP> faceToPolyfaces;
    bmap<FaceAttachment, PolyfaceHeaderCP> uniqueFaceAttachments;

    for (T_FaceToAttachmentIndexMap::const_iterator curr = faceToAttachmentIndexMap.begin(); curr != faceToAttachmentIndexMap.end(); ++curr)
        {
        BeAssert(curr->second < faceAttachmentsVec.size());
        FaceAttachment faceAttachment = faceAttachmentsVec.at(curr->second);
        bmap<FaceAttachment, PolyfaceHeaderCP>::iterator found = uniqueFaceAttachments.find(faceAttachment);

        if (found == uniqueFaceAttachments.end())
            {
            PolyfaceHeaderPtr polyface = PolyfaceHeader::New();

            polyfaces.push_back(polyface);
            params.push_back(faceAttachment);

            faceToPolyfaces[curr->first] = uniqueFaceAttachments[faceAttachment] = polyface.get();
            }
        else
            {
            faceToPolyfaces[curr->first] = found->second;
            }
        }

    if (SUCCESS != IFacetTopologyTable::ConvertToPolyfaces(polyfaces, faceToPolyfaces, facetTopo, facetOptions))
        return false;

    bool isTwoSided = IBRepEntity::EntityType::Solid != entity.GetEntityType();
    for (size_t i=0; i<polyfaces.size(); i++)
        {
        polyfaces[i]->SetTwoSided(isTwoSided);
        polyfaces[i]->Transform(entity.GetEntityTransform());
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool PSolidUtil::FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<FaceAttachment>& params, double pixelSize, DRange1dP pixelSizeRange)
    {
    if (nullptr == entity.GetFaceMaterialAttachments())
        return false; // No reason to call this method when there aren't attachments...can't return params...

    IFacetTopologyTablePtr facetTopo = PSolidFacetTopologyTable::CreateNewFacetTable(entity, pixelSize, pixelSizeRange);

    if (!facetTopo->_IsTableValid())
        return false;

    IFacetOptionsPtr facetOptions = IFacetOptions::Create(); // Doesn't matter...

    return facetTableToPolyfaces(entity, polyfaces, params, *facetOptions, *facetTopo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool PSolidUtil::FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<FaceAttachment>& params, IFacetOptionsR facetOptions)
    {
    if (nullptr == entity.GetFaceMaterialAttachments())
        return false; // No reason to call this method when there aren't attachments...can't return params...

    IFacetTopologyTablePtr facetTopo = PSolidFacetTopologyTable::CreateNewFacetTable(entity, facetOptions);

    if (!facetTopo->_IsTableValid())
        return false;

    return facetTableToPolyfaces(entity, polyfaces, params, facetOptions, *facetTopo);
    }
