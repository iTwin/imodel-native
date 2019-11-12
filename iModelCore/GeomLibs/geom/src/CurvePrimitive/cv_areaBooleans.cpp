/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Mtg/mtgprint.fdf>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static int s_debug = 0;


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool TryGetInt(bvector<int> &source, int &value, int i)
    {
    if (i >= 0 && i < (int)source.size ())
        {
        value = source[i];
        return true;
        }
    return false;
    }

static double s_vertexAbsTol = 1.0e-10;

CurvePrimitivePtrPair::CurvePrimitivePtrPair (ICurvePrimitivePtr _curveA, ICurvePrimitivePtr _curveB)
    : curveA (_curveA), curveB (_curveB)
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct AreaBooleanContext
    {
    RIMSBS_Context  *m_rimsbsCurves;
    RG_Header       *m_rgContext;
    // As the Load() methods build up RG structure from input curves:
    //  1) the input curve pointer is added to the m_indexedParentCurve array.
    //  2) its index is saved as the parentId in the RIMSBS curve.
    //  3) The MTG graph parentIndex is the RIMSBS curve index.
    // The RG merge process promises (we hope?) that it will carry the MTG node parentIndex through all of its contortions.
    // This is not trivial to trust -- the RG/MTG can have linear edges fully defined by their endpoints but lacking an associated curve for their bounded length.
    //  (In the initial setup, these get carrier curves.  But can be lost)
    //
    bvector<ICurvePrimitivePtr>m_indexedParentCurve;
    CurvePrimitivePtrPairVector *m_newCurveToOldCurve; // In "new to old" order !!!

    int m_nextGroupId;
private:
    MTGGraphP GetGraph () { return jmdlRG_getGraph (m_rgContext);}

    void Merge (bool buildSearchStructures)
        {
        jmdlRG_mergeWithGapTolerance (m_rgContext, s_vertexAbsTol, s_vertexAbsTol);
        if (buildSearchStructures)
            {
            jmdlRG_buildFaceRangeTree (m_rgContext, 0.0, 0.0);
            jmdlRG_buildFaceHoleArray (m_rgContext);
            }
        }

    void SetNextGroupId (int i)
        {
        m_nextGroupId = i;
        jmdlRIMSBS_setCurrGroupId (m_rimsbsCurves, i);
        }
        
    int GetNextGroupId (){return m_nextGroupId;}
    void IncrementNextGroupId ()
        {
        SetNextGroupId (m_nextGroupId + 1);
        }
#define ParentCurveOffsetCheck 9000
    void RecordNewCurve (ICurvePrimitivePtr newCurve, int oldIndex)
        {
        if (m_newCurveToOldCurve != NULL && newCurve.IsValid () && oldIndex >= 0)
            {
            size_t indexA = (size_t) oldIndex;
            if (indexA >= ParentCurveOffsetCheck && indexA < ParentCurveOffsetCheck + m_indexedParentCurve.size ())
                {
                size_t indexB = indexA - ParentCurveOffsetCheck;
                m_newCurveToOldCurve->push_back (CurvePrimitivePtrPair (newCurve, m_indexedParentCurve[indexB]));
                }
            }
        }
public:
    bool MakeMaximalLineStrings ()
        {
        return m_newCurveToOldCurve == NULL;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
int AddParentCurve(ICurvePrimitivePtr parent)
    {
    int index = (int)m_indexedParentCurve.size ();
    m_indexedParentCurve.push_back (parent);
    return index + ParentCurveOffsetCheck;
    }

AreaBooleanContext (CurvePrimitivePtrPairVector *newCurveToOldCurve)
    {
    m_rimsbsCurves = jmdlRIMSBS_newContext ();
    m_rgContext = jmdlRG_new ();
    jmdlRIMSBS_setupRGCallbacks (m_rimsbsCurves, m_rgContext);
    jmdlRG_setFunctionContext (m_rgContext, m_rimsbsCurves);
    m_newCurveToOldCurve = newCurveToOldCurve;
    m_nextGroupId = 0;
    }

~AreaBooleanContext ()
    {
    jmdlRG_free (m_rgContext);
    jmdlRIMSBS_freeContext (m_rimsbsCurves);
    }

private:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void LoadPolyline(bvector<DPoint3d> const &points, int sourceIndex)
    {
    if (points.size () > 1)
        {
        int parentCurveId = jmdlRIMSBS_addDataCarrier (m_rimsbsCurves, sourceIndex, NULL);
        jmdlRG_addLinear (m_rgContext, &points[0], (int)points.size (), false, parentCurveId);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void LoadDSegment3d(DSegment3dCR segment, int sourceIndex)
    {
    int parentCurveId = jmdlRIMSBS_addDataCarrier (m_rimsbsCurves, sourceIndex, NULL);
    jmdlRG_addLinear (m_rgContext, segment.point, 2, false, parentCurveId);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void LoadDEllipse3d(DEllipse3dCR ellipse, int sourceIndex)
    {
    int curveId = jmdlRIMSBS_addDEllipse3d (m_rimsbsCurves, sourceIndex, NULL, &ellipse);
    jmdlRG_addCurve (m_rgContext, curveId, curveId);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void LoadMSBsplineCurve(MSBsplineCurveCR curve, int sourceIndex)
    {
    // WOW ...this function takes over the memory management ....
    MSBsplineCurve curve1;
    curve1.CopyFrom (curve);
    int curveId = jmdlRIMSBS_addMSBsplineCurve (m_rimsbsCurves, sourceIndex, NULL, &curve1);
    assert (curve1.poles == NULL);
    jmdlRG_addCurve (m_rgContext, curveId, curveId);
    }

public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void Load(CurveVectorCR source, bool advanceGroupAfterPath = true)
    {
    if (source.IsClosedPath ())
        {
        for (size_t i = 0; i < source.size (); i++)
            {
            MSBsplineCurveCP proxyCurve;
            ICurvePrimitive::CurvePrimitiveType curveType = source[i]->GetCurvePrimitiveType ();

            if (curveType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
                LoadPolyline (*source[i]->GetLineStringCP (), AddParentCurve (source[i]));
            else if (curveType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc)
                LoadDEllipse3d (*source[i]->GetArcCP (), AddParentCurve (source[i]));
            else if (curveType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
                LoadDSegment3d (*source[i]->GetLineCP (), AddParentCurve (source[i]));
            else if (NULL != (proxyCurve = source[i]->GetProxyBsplineCurveCP ()))
                LoadMSBsplineCurve (*proxyCurve, AddParentCurve (source[i]));
            }
        if (advanceGroupAfterPath)
            IncrementNextGroupId ();
        }
    else if (source.IsUnionRegion ())
        {
        for (size_t i = 0; i < source.size (); i++)
            {
            CurveVectorCP child = source.at (i)->GetChildCurveVectorCP ();
            if (NULL != child)
                Load (*child, true);
            }
        }
    else if (source.IsParityRegion ())
        {
        for (size_t i = 0; i < source.size (); i++)
            {
            CurveVectorCP child = source.at (i)->GetChildCurveVectorCP ();
            if (NULL != child)
                Load (*child, false);
            }
        IncrementNextGroupId ();
        }
    if (s_debug > 0)
     jmdlRG_print (m_rgContext, "After Load");
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/2013
+--------------------------------------------------------------------------------------*/
struct LineStringBuilder
{
bvector<DPoint3d> m_points;
static const int s_defaultParentId = -1;
int  m_parentId;
bool m_makeMaximalLineStrings;

LineStringBuilder (bool makeMakimalLineStrings)
    {
    m_makeMaximalLineStrings = makeMakimalLineStrings;
    }

void Init ()
    {
    m_points.clear ();
    m_parentId = s_defaultParentId;
    }

void Flush (AreaBooleanContext &context, CurveVectorR curves)
    {
    if (m_points.size () > 0)
        {
        curves.push_back (ICurvePrimitive::CreateLineString (m_points));
        context.RecordNewCurve (curves.back (), m_parentId);
        }
    Init ();
    }

bool TryAppendLineSegment (DPoint3dCR point0, DPoint3dCR point1, int parentId)
    {
    if (m_points.size () == 0)
        {
        m_points.push_back (point0);
        m_points.push_back (point1);
        m_parentId = parentId;
        return true;
        }
    DPoint3d lastPoint = m_points.back ();
    if (point0.IsEqual (lastPoint))
        {
        if (m_makeMaximalLineStrings || parentId == m_parentId)
            {
            m_points.push_back (point1);
            m_parentId = parentId;    // keep it current even though not using it.
            return true;
            }
        }
    return false;
    }
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TryExtractEdge(
CurveVectorR  dest,
LineStringBuilder &lineString,
MTGNodeId     nodeId
)
    {
    bool isReversed;
    bool myStat = false;
    int curveIndex;
    DPoint3d point[2];
    //int parentId;
    /* MSBsplineCurve curve; */
    DEllipse3d ellipse;
    MSBsplineCurve curve;
    static int s_numChords = 0; // to trigger length debug.
    double quickLength = 0.0;
    double realLength = 0.0;
    if (s_numChords > 0)
        quickLength = jmdlRG_quickChordLength (m_rgContext, nodeId, s_numChords);

    if (   jmdlRG_getCurveData
	    (m_rgContext, &curveIndex, &isReversed, &point[0], &point[1], nodeId)
        )
        {
        int rimsbsParentCurveIndex, callerCurveIndex;
        jmdlRG_getParentCurveIndex (m_rgContext, &rimsbsParentCurveIndex, nodeId);
        jmdlRIMSBS_getUserInt (m_rimsbsCurves, &callerCurveIndex, rimsbsParentCurveIndex);


        if (curveIndex == RG_NULL_CURVEID)
	          {
            if (lineString.TryAppendLineSegment (point[0], point[1], callerCurveIndex))
                {
                }
            else
                {
                lineString.Flush (*this, dest);
                lineString.TryAppendLineSegment (point[0], point[1], callerCurveIndex);
                }
 	          myStat = true;
	          }
        else if (jmdlRIMSBS_getDEllipse3d (m_rimsbsCurves, &ellipse, curveIndex, isReversed))
	          {
	          if (s_numChords > 0)
	              realLength = ellipse.ArcLength ();
            lineString.Flush (*this, dest);
            dest.push_back (ICurvePrimitive::CreateArc (ellipse));
            RecordNewCurve (dest.back (), callerCurveIndex);
	          myStat = true;
	          }
        else if (jmdlRIMSBS_getMSBsplineCurve (m_rimsbsCurves, &curve, curveIndex, isReversed))
	          {
            lineString.Flush (*this, dest);
            dest.push_back (ICurvePrimitive::CreateBsplineCurve (curve));
            curve.ReleaseMem ();
            RecordNewCurve (dest.back (), callerCurveIndex);
            myStat = true;
	          }
        }
    return myStat;    
    }




/*---------------------------------------------------------------------------------**//**
* Extract a single face loop from the graph structure to an output CurveVector
+--------------------------------------------------------------------------------------*/
bool TryExtractFace
(
CurveVectorR    dest,
int             startNodeId,
bool	        reverse
)
    {
    LineStringBuilder builder (MakeMaximalLineStrings ());
    MTGGraph *pGraph = GetGraph ();

    if (!reverse)
        {
        MTGARRAY_FACE_LOOP (currNodeId, pGraph, startNodeId)
            {
            if (!TryExtractEdge (dest, builder,  currNodeId))
                return false;
            }
        MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, startNodeId)
        }
    else
        {
        MTGNodeId currNodeId = startNodeId;
        do {
            if (!TryExtractEdge (dest, builder, currNodeId))
                return false;
            currNodeId = jmdlMTGGraph_getFPred (pGraph, currNodeId);
            } while (currNodeId != startNodeId);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Extract the loops (outer and hole) which surround a point by flood fill logic.
* @param OUT dest This array is emptied and filled with the geometry of the face
*				loop that surrounds the seed point.
* @param IN boundary   source geometry array.
* @param IN seedPoint seed point for search
+--------------------------------------------------------------------------------------*/
static bool  ExtractLoopsContainingPoint
(
CurveVectorR		dest,
CurveVectorCR        boundary,
DPoint3dCR    seedPoint
)
    {
    AreaBooleanContext context (NULL);
    MTGNodeId outerFaceNodeId, holeNodeId;
    int i;
    bvector<int> holeNodeIdArray;
    dest.clear ();

    context.Load (boundary);

    context.Merge (true);

    /* Get the outer face */
    jmdlRG_smallestContainingFace (context.m_rgContext, &outerFaceNodeId, &seedPoint);
    context.TryExtractFace (dest, outerFaceNodeId, false);

    /* And each inner face */
    jmdlRG_searchFaceHoleArray (context.m_rgContext, &holeNodeIdArray, outerFaceNodeId);

    for (i = 0; TryGetInt (holeNodeIdArray, holeNodeId, i); i++)
	{
	context.TryExtractFace (dest, holeNodeId, true);
	}

    return false;
    }


/*---------------------------------------------------------------------------------**//**
* Extract "all" loops in a region
* @param loops     <= This array is emptied and filled with the loop geometry.
* @param boundary   <= source geometry array.
* @param gapTolerance   <= add lines to close gaps this size.
* @param includePositive => true to include positive areas
* @param includeNegative => true to include negative areas
* @bsimethod 							EarlinLutz 	05/99
+--------------------------------------------------------------------------------------*/
static bool  ExtractAllLoops
(
CurveVectorR        loops,
CurveVectorCR        boundary,
double				gapTolerance,
bool				positiveAreas,
bool				negativeAreas
)
    {
    AreaBooleanContext context (NULL);
    bool active;
    double area;
    bool defaultActive = positiveAreas && negativeAreas;
    bvector<MTGNodeId> faceNodeIdArray;

    context.Load (boundary);

    context.Merge (false);

    MTGGraphP pGraph = context.GetGraph ();
    jmdlMTGGraph_collectAndNumberFaceLoops (pGraph, &faceNodeIdArray, NULL);

    for (size_t i = 0; i < faceNodeIdArray.size (); i++)
        {
        MTGNodeId seedNodeId = faceNodeIdArray[i];
        active = defaultActive;

        if (!active && jmdlRG_getFaceSweepProperties
		        (context.m_rgContext, &area, NULL, NULL, seedNodeId))
	          {
	          active =  (area >  0.0 && positiveAreas)
		          || (area <= 0.0 && negativeAreas);
	          }

	      if (active)
	          context.TryExtractFace (loops, seedNodeId, false);
        }
    return true;
    }

private:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TryAssembleLoop(
bvector<int> &sequenceArray,
int &sequencerReadIndex,
bvector<int> &nodeIdToDepthArray,
CurveVectorPtr &dest
)
    {
    bool myStat = true;
    MTGGraph* pGraph = GetGraph();
    LineStringBuilder builder (MakeMaximalLineStrings ());
    bvector<DPoint3d>points;
    MTGNodeId nodeId;
    dest = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    for (;myStat && jmdlEmbeddedIntArray_getInt (&sequenceArray, &nodeId, sequencerReadIndex);)
        {
        sequencerReadIndex += 1;
        if (   nodeId == MTG_MARKER_END_FACE)
            {
            break;
            }
        else if (!jmdlMTGGraph_isValidNodeId (pGraph, nodeId))
	          {
	          myStat = false;
	          }
        else
	          {
            if (!TryExtractEdge (*dest, builder, nodeId))
                myStat = false;
            }
        }
    builder.Flush (*this, *dest);
    return myStat;
    }

private:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TryAssembleOuterInner(
bvector<int> &sequenceArray,
int &sequencerReadIndex,
bvector<int> &nodeIdToDepthArray,
CurveVectorPtr &result
)
    {
    result = CurveVectorPtr ();
    MTGNodeId nodeId, nextNodeId;   // or coded sequence token.
    int numLoop = 0;
    for (;TryGetInt (sequenceArray, nodeId, sequencerReadIndex);)
        {
        sequencerReadIndex++;
        if (   nodeId == MTG_MARKER_START_FACE)
	          {
            CurveVectorPtr loops;
            if (!TryAssembleLoop (sequenceArray, sequencerReadIndex, nodeIdToDepthArray, loops))
                return false;
            numLoop++;
            if (  !result.IsValid () 
                && TryGetInt (sequenceArray, nextNodeId, sequencerReadIndex)
                && nextNodeId == MTG_MARKER_END_COMPONENT
                )
                {
                // single loop.   Take it directly as the result
                sequencerReadIndex++;
                result = loops;
                return true;
                }
            // good loop.  Append to (possibly new) growing parity region.
            if (!result.IsValid ())
                result = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
            if (result->size () == 0)
                {
                // first loop.  Leave it as outer
                }
            else
                {
                // Additional loops are holes.
                loops->SetBoundaryType (CurveVector::BOUNDARY_TYPE_Inner);
                }
            result->push_back (
                    ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*loops)
                    );
            }
        else if (nodeId == MTG_MARKER_END_COMPONENT)
	          {
            return true;
	          }
        else
	          {
            return false;
            }
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TryAssembleComponents(
bvector<int> &sequenceArray,
bvector<int> &nodeIdToDepthArray,
CurveVectorPtr &loops
)
    {
    int i;
    MTGNodeId nodeId;
    int numComponent = 0;
    CurveVectorPtr thisComponent;
    loops = CurveVectorPtr ();
    for (i = 0; TryGetInt (sequenceArray, nodeId, i++);)
	{
	if (   nodeId == MTG_MARKER_START_COMPONENT
	    && TryAssembleOuterInner (sequenceArray, i, nodeIdToDepthArray, thisComponent)
            )
            {
            numComponent++;
            if (numComponent == 1)
                {
                // Very first component.  Save its as if it is alone at the top....
                loops = thisComponent;
                }
            else
                {
                if (numComponent == 2)
                    {
                    // Second component.   The first is a loner;  move it down below a new top level.
                    CurveVectorPtr firstComponent = loops;
                    loops = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
                    loops->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*firstComponent));
                    }
                // And push the second or later behind all the prior ones ....
                loops->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*thisComponent));
                }
            }
        else
            {
            return false;
            }
        }
    return true;
    }

public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static CurveVectorPtr doBoolop
(
CurveVectorCR        regionA,
CurveVectorCR        regionB,
RGBoolSelect         opcode,
CurvePrimitivePtrPairVector *newCurveToOldCurve,
const char * name = "op"
)
    {
    AreaBooleanContext context (newCurveToOldCurve);
    
    CurveVectorPtr result;
    bvector<int> faceNodeIdArray;
    bool myStat = false;
    MTGGraph * pGraph = context.GetGraph ();

    context.SetNextGroupId (0);
    context.Load (regionA);
    
    if (s_debug)
        jmdlMTGGraph_printFaceLoops (pGraph);
            
    int numGroupA = context.GetNextGroupId ();
    context.Load (regionB);
    context.Merge (true);

    if (s_debug)
        jmdlMTGGraph_printFaceLoops (pGraph);
    MTG_MarkSet faceSet (pGraph, MTG_ScopeFace);
    jmdlRG_buildFaceRangeTree (context.m_rgContext, 0.0, 0.0);
    jmdlRG_buildFaceHoleArray (context.m_rgContext);
    if (s_debug)
        jmdlMTGGraph_printFaceLoops (pGraph);

    if (jmdlRG_collectBooleanFaces (context.m_rgContext, opcode, numGroupA - 1, &faceSet))
        {
        bvector<int> startArray;
        bvector<int> sequenceArray;
        bvector<int> nodeIdToDepthArray;
        jmdlRG_collectAndNumberExtendedFaceLoops (context.m_rgContext, &startArray, &sequenceArray, &faceSet);
        jmdlRG_setMarksetDepthByInwardSearch (context.m_rgContext, &nodeIdToDepthArray, &faceSet);
        myStat = context.TryAssembleComponents (sequenceArray, nodeIdToDepthArray, result);
	}

    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static CurveVectorPtr doFixup
(
CurveVectorCR        regionA,
CurvePrimitivePtrPairVector *newCurveToOldCurve,
const char * name = "op"
)
    {
    AreaBooleanContext context (newCurveToOldCurve);
    
    CurveVectorPtr result;
    bvector<int> faceNodeIdArray;
    bool myStat = false;
    context.SetNextGroupId (0);
    context.Load (regionA);
    int numGroup = context.GetNextGroupId ();
    context.Merge (true);

    MTGGraph * pGraph = context.GetGraph ();
    //jmdlMTGGraph_printFaceLoops (pGraph);
    MTG_MarkSet faceSet (pGraph, MTG_ScopeFace);
    jmdlRG_buildFaceRangeTree (context.m_rgContext, 0.0, 0.0);
    jmdlRG_buildFaceHoleArray (context.m_rgContext);
    //jmdlMTGGraph_printFaceLoops (pGraph);

    if (jmdlRG_collectBooleanFaces (context.m_rgContext, RGBoolSelect_Parity, numGroup - 1, &faceSet))
        {
        bvector<int> startArray;
        bvector<int> sequenceArray;
        bvector<int> nodeIdToDepthArray;
        jmdlRG_collectAndNumberExtendedFaceLoops (context.m_rgContext, &startArray, &sequenceArray, &faceSet);
        jmdlRG_setMarksetDepthByInwardSearch (context.m_rgContext, &nodeIdToDepthArray, &faceSet);
        myStat = context.TryAssembleComponents (sequenceArray, nodeIdToDepthArray, result);
	}

    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2013
+--------------------------------------------------------------------------------------*/
static CurveVectorPtr doAnalysis
(
CurveVectorCR        regionA,
AreaSelect          groupOp,
BoolSelect          boolOp,
bool                reverseSense,
CurvePrimitivePtrPairVector *newCurveToOldCurve,
const char * name = "op"
)
    {
    AreaBooleanContext context (newCurveToOldCurve);
    
    CurveVectorPtr result;
    bvector<int> faceNodeIdArray;
    bool myStat = false;
    context.SetNextGroupId (0);

    BoolSelect finalBoolOp = boolOp;
    if (boolOp == BoolSelect_FromStructure)
        {
        boolOp = BoolSelect_Summed_Parity;
        if (regionA.IsUnionRegion ())
            boolOp = BoolSelect_Union;
        }
    context.Load (regionA);
    context.Merge (true);

    MTGGraph * pGraph = context.GetGraph ();
    //jmdlMTGGraph_printFaceLoops (pGraph);
    MTG_MarkSet faceSet (pGraph, MTG_ScopeFace);
    jmdlRG_buildFaceRangeTree (context.m_rgContext, 0.0, 0.0);
    jmdlRG_buildFaceHoleArray (context.m_rgContext);
    //jmdlMTGGraph_printFaceLoops (pGraph);
  

    if (jmdlRG_collectAnalysisFaces (context.m_rgContext, groupOp, finalBoolOp, &faceSet, reverseSense))
        {
        bvector<int> startArray;
        bvector<int> sequenceArray;
        bvector<int> nodeIdToDepthArray;
        jmdlRG_collectAndNumberExtendedFaceLoops (context.m_rgContext, &startArray, &sequenceArray, &faceSet);
        jmdlRG_setMarksetDepthByInwardSearch (context.m_rgContext, &nodeIdToDepthArray, &faceSet);
        myStat = context.TryAssembleComponents (sequenceArray, nodeIdToDepthArray, result);
	}

    return result;
    }


};


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::AreaAnalysis (CurveVectorCR  region, AreaSelect select1, BoolSelect select2, bool reverse )
    {
    return AreaBooleanContext::doAnalysis (region, select1, select2, reverse,  NULL, "WindingFixup");
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::AreaIntersection (CurveVectorCR regionA, CurveVectorCR regionB, CurvePrimitivePtrPairVector *newToOld)
    {return AreaBooleanContext::doBoolop (regionA, regionB, RGBoolSelect_Intersection, newToOld, "Intersection");}

CurveVectorPtr CurveVector::AreaUnion (CurveVectorCR regionA, CurveVectorCR regionB, CurvePrimitivePtrPairVector *newToOld)
    {return AreaBooleanContext::doBoolop (regionA, regionB, RGBoolSelect_Union, newToOld, "Union");}

CurveVectorPtr CurveVector::AreaDifference (CurveVectorCR regionA, CurveVectorCR regionB, CurvePrimitivePtrPairVector *newToOld)
    {return AreaBooleanContext::doBoolop (regionA, regionB, RGBoolSelect_Difference, newToOld, "Difference");}

CurveVectorPtr CurveVector::AreaParity (CurveVectorCR regionA, CurveVectorCR regionB, CurvePrimitivePtrPairVector *newToOld)
    {return AreaBooleanContext::doBoolop (regionA, regionB, RGBoolSelect_Parity, newToOld, "Parity");}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::ReduceToCCWAreas (CurveVectorCR regionA)
    {
    return AreaBooleanContext::doFixup (regionA, NULL);
    }



END_BENTLEY_GEOMETRY_NAMESPACE


