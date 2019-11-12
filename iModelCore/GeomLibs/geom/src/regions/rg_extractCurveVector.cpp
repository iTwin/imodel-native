/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "Regions/rg_intern.h"
#include <Mtg/mtgprint.fdf>
#include <stdio.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE





struct Printer
{
int m_noisy;
int m_depth;
Printer (int noisy) : m_noisy (noisy), m_depth (0) {}

void PrintIndent ()
    {
    GEOMAPI_PRINTF ("\n");
    for (int i = 0; i < m_depth; i++)
        GEOMAPI_PRINTF ("  ");
    }
void Begin (char const*name, int id)
    {
    if (m_noisy)
      {
      m_depth++;
      PrintIndent ();
      GEOMAPI_PRINTF ("<%s id=\"%d\">", name, id);
      }
    }
void End (char const*name, int id)
    {
    if (m_noisy)
      {
      PrintIndent ();
      GEOMAPI_PRINTF ("</%s id=\"%d\">", name, id);
      m_depth--;
      }
    }
void Data (char const*name, int id)
    {
    if (m_noisy)
        GEOMAPI_PRINTF ("(%s %d)", name, id);
    }
};
static int s_noisyParse = 0;
struct ExtendedFaceParser
{
RG_Header*          m_pRG;
RIMSBS_Context*     m_pCurves;
bvector<MTGNodeId>  m_startArray;
bvector<MTGNodeId>  m_sequenceArray;

ExtendedFaceParser (RG_Header *pRG, RIMSBS_Context* pCurves)
    : m_pRG (pRG), m_pCurves (pCurves)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GetCurveAndParent (MTGNodeId nodeId, RegionCurveRef& curveRef)
    {
    int         curveId, parentCurveId;
    bool        reversed;
    double      startParam, endParam;

    curveRef.m_curveId       = RG_NULL_CURVEID;
    curveRef.m_reversed      = false;
    curveRef.m_parentCurveId = RG_NULL_CURVEID;
    curveRef.m_startParam    = 0.0;
    curveRef.m_endParam      = 1.0;

    if (!jmdlRG_getCurveData (m_pRG, &curveId, &reversed, NULL, NULL, nodeId))
        return false;

    curveRef.m_curveId  = curveId;
    curveRef.m_reversed = reversed;

    if (RG_NULL_CURVEID != curveId)
        {
        if (jmdlRIMSBS_getCurveInterval (m_pCurves, &parentCurveId, &startParam, &endParam, curveId, reversed))
            {
            curveRef.m_parentCurveId = parentCurveId;
            curveRef.m_startParam    = startParam;
            curveRef.m_endParam      = endParam;
            }         
        else
            {
            curveRef.m_parentCurveId = curveId;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AlignEllipses (DEllipse3dCR ellipseA, DEllipse3dCR ellipseB, double tolerance, double* totalSweep)
    {
    if (ellipseA.center.Distance (ellipseB.center) > tolerance)
        return false;

    // Don't even try to shuffle orientations -- look for exact match of vectors.
    if (ellipseA.vector0.Distance (ellipseB.vector0) > tolerance)
        return false;

    if (ellipseA.vector90.Distance (ellipseB.vector90) > tolerance)
        return false;

    if (totalSweep)
        {
        double  endRadians = bsiTrig_adjustAngleToSweep (ellipseB.start + ellipseB.sweep, ellipseA.start, ellipseA.sweep);

        *totalSweep = endRadians - ellipseA.start;

        // We have closed back to start. Turn it into a full sweep
        if (bsiTrig_isAngleNearZero (*totalSweep) && (!bsiTrig_isAngleNearZero (ellipseA.sweep) || !bsiTrig_isAngleNearZero (ellipseB.sweep)))
            *totalSweep = (ellipseA.sweep < 0.0 ? -msGeomConst_2pi : msGeomConst_2pi);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SameUnderlyingCurve (int curveIndexA, int curveIndexB)
    {
    if (curveIndexA == curveIndexB)
        return true;

    DEllipse3d  ellipseA, ellipseB;

    // uh oh -- reversal issues.
    if (!jmdlRIMSBS_getDEllipse3d (m_pCurves, &ellipseA, curveIndexA, false) ||
        !jmdlRIMSBS_getDEllipse3d (m_pCurves, &ellipseB, curveIndexB, false))
        return false;

    double      tolerance = jmdlRG_getTolerance (m_pRG);

    return AlignEllipses (ellipseA, ellipseB, tolerance, NULL);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            InitSequencerFromBreak (RegionSequencer& sequencer, bvector<int>* pSequenceArray, int& sequenceIndex)
    {
    int             breakStrength = 0;
    int             breakIndex = sequenceIndex;
    int             i = sequenceIndex;
    int             i0 = i;
    int             i1 = i - 1;
    MTGNodeId       currNodeId;
    RegionCurveRef  oldCurve, newCurve;

    // Get the initial node, curve, and parent info...
    if (jmdlEmbeddedIntArray_getInt (pSequenceArray, &currNodeId, i++) &&
        jmdlMTGGraph_isValidNodeId (jmdlRG_getGraph (m_pRG), currNodeId) &&
        GetCurveAndParent (currNodeId, oldCurve))
        {
        i1 = i - 1; // If nothing else happens, remember this as the start.

        // Look for adjacent nodes with different curves; Loop executes to entry with null node id --- i1 falls out.
        for (; jmdlEmbeddedIntArray_getInt (pSequenceArray, &currNodeId, i++) &&
               jmdlMTGGraph_isValidNodeId (jmdlRG_getGraph (m_pRG), currNodeId) &&
               GetCurveAndParent (currNodeId, newCurve); oldCurve = newCurve)
            {
            i1 = i - 1;

            if (breakStrength < 2)
                {
                if (SameUnderlyingCurve (oldCurve.m_parentCurveId, newCurve.m_parentCurveId))
                    {
                    if (breakStrength == 0)
                        {
                        breakStrength = 1;
                        breakIndex = i - 1;
                        }
                    }
                else if ((oldCurve.m_curveId == RG_NULL_CURVEID && newCurve.m_curveId != RG_NULL_CURVEID) ||
                         (oldCurve.m_curveId != RG_NULL_CURVEID && newCurve.m_curveId == RG_NULL_CURVEID))
                    {
                    // curve adjacent to to non-curve.  Strong break; start at newCurve
                    breakStrength = 2;
                    breakIndex = i - 1;
                    }
                else if (oldCurve.m_parentCurveId != newCurve.m_parentCurveId)
                    {
                    // dissimilar curves.  Strong break, start at newCurve
                    breakStrength = 2;
                    breakIndex = i - 1;
                    }
                else if (breakStrength == 0 &&
                         oldCurve.m_parentCurveId != oldCurve.m_curveId &&  // partial curve
                         newCurve.m_parentCurveId != newCurve.m_curveId &&  // partial curve
                         oldCurve.m_parentCurveId == newCurve.m_parentCurveId && // same parent
                         fabs (newCurve.m_startParam - oldCurve.m_endParam) < 1.0e-3) // parametric successors
                    {
                    breakStrength = 1;
                    breakIndex = i - 1;
                    }
                }
            }
        }

    sequencer.Init (i0, i1, breakIndex);
    sequenceIndex = i;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LookAheadForEllipseSequence
(
RegionSequencer&    sequencer,
double&             area,
DEllipse3d&         ellipse,
int&                numEllipse,
bvector<int>*   pSequenceArray,
DPoint3dCR          refPoint
)
    {
    int             i;
    RG_CurveId      currCurveId;
    MTGNodeId       currNodeId;
    DPoint3d        point[2];
    bool            currCurveIsReversed;
    double          areaToCurve = 0.0;
    DEllipse3d      baseEllipse, currEllipse;
    RegionSequencer lookaheadSequencer, savedSequencer;
    double          currSweep, totalSweep, currArea;

    lookaheadSequencer.Copy (sequencer);
    lookaheadSequencer.Backup ();

    if (lookaheadSequencer.Advance (i) &&
        jmdlEmbeddedIntArray_getInt (pSequenceArray, &currNodeId, i) &&
        jmdlMTGGraph_isValidNodeId (jmdlRG_getGraph (m_pRG), currNodeId) &&
        jmdlRG_getCurveData (m_pRG, &currCurveId, &currCurveIsReversed, &point[0], &point[1], currNodeId) &&
        RG_NULL_CURVEID != currCurveId &&
        jmdlRIMSBS_getDEllipse3d (m_pCurves, &baseEllipse, currCurveId, currCurveIsReversed))
        {
        jmdlRG_getEdgeSweepProperties (m_pRG, &currArea, NULL, const_cast<DPoint3dP>(&refPoint), currNodeId);

        areaToCurve += currArea;
        totalSweep = baseEllipse.sweep;
        numEllipse = 1;

        savedSequencer.Copy (lookaheadSequencer);

        while (lookaheadSequencer.Advance (i) && 
               jmdlEmbeddedIntArray_getInt (pSequenceArray, &currNodeId, i) &&
               jmdlMTGGraph_isValidNodeId (jmdlRG_getGraph (m_pRG), currNodeId) && 
               jmdlRG_getCurveData (m_pRG, &currCurveId, &currCurveIsReversed, &point[0], &point[1], currNodeId) && 
               RG_NULL_CURVEID != currCurveId && 
               jmdlRIMSBS_getDEllipse3d (m_pCurves, &currEllipse, currCurveId, currCurveIsReversed) && 
               AlignEllipses (baseEllipse, currEllipse, 1.0e-12, &currSweep))
            {
            jmdlRG_getEdgeSweepProperties (m_pRG, &currArea, NULL, const_cast<DPoint3dP>(&refPoint), currNodeId);

            areaToCurve += currArea;
            totalSweep = currSweep;
            numEllipse++;

            savedSequencer.Copy (lookaheadSequencer);
            }

        sequencer.Copy (savedSequencer);
        area = areaToCurve;

        ellipse = baseEllipse; // Return the base ellipse with its sweep extended...
        ellipse.sweep = totalSweep;

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LookAheadForForSectorSequence
(
RegionSequencer&        sequencer,
double&                 area,
RegionSectorSequence&   sectorSequence,
bvector<int>*       pSequenceArray,
DPoint3dCR              refPoint
)
    {
    int             i;
    RG_CurveId      currCurveId, currParentId;
    double          currStartFraction, currEndFraction;
    MTGNodeId       currNodeId;
    DPoint3d        point[2];
    bool            currCurveIsReversed;
    double          prevEndFraction;
    double          areaToCurve = 0.0, currArea;
    RegionSequencer lookaheadSequencer, savedSequencer;

    lookaheadSequencer.Copy (sequencer);
    lookaheadSequencer.Backup ();

    if (lookaheadSequencer.Advance (i) && 
        jmdlEmbeddedIntArray_getInt (pSequenceArray, &currNodeId, i) &&
        jmdlMTGGraph_isValidNodeId (jmdlRG_getGraph (m_pRG), currNodeId) &&
        jmdlRG_getCurveData (m_pRG, &currCurveId, &currCurveIsReversed, &point[0], &point[1], currNodeId) && 
        RG_NULL_CURVEID != currCurveId && 
        jmdlRIMSBS_getCurveInterval (m_pCurves, &currParentId, &currStartFraction, &currEndFraction, currCurveId, currCurveIsReversed))
        {
        jmdlRG_getEdgeSweepProperties (m_pRG, &areaToCurve, NULL, const_cast<DPoint3dP>(&refPoint), currNodeId);

        sectorSequence.m_parentCurveId = currParentId;
        sectorSequence.m_s00           = currStartFraction;
        sectorSequence.m_s01           = currEndFraction;
        sectorSequence.m_curveId0      = currCurveId;
        sectorSequence.m_nodeId0 = currNodeId;
        sectorSequence.m_numSector     = 1;
        prevEndFraction                = currEndFraction;

        while (lookaheadSequencer.Advance (i) &&
               jmdlEmbeddedIntArray_getInt (pSequenceArray, &currNodeId, i) &&
               jmdlMTGGraph_isValidNodeId (jmdlRG_getGraph (m_pRG), currNodeId) &&
               jmdlRG_getCurveData (m_pRG, &currCurveId, &currCurveIsReversed, &point[0], &point[1], currNodeId) &&
               RG_NULL_CURVEID != currCurveId && 
               jmdlRIMSBS_getCurveInterval (m_pCurves, &currParentId, &currStartFraction, &currEndFraction, currCurveId, currCurveIsReversed) &&
               currParentId == sectorSequence.m_parentCurveId &&
               /* Require that the fractions pick up right where the previous one left off.
                  This prevents steps across the start point of a spline that comes back to its start point (good)
                  It also prevents jumping through a self intersection. (good)
                  It also prevents continuing smoothly along an ellipse (not good, not real bad) or periodic spline */
               fabs (currStartFraction - prevEndFraction) < 1.0e-8)
            {
            jmdlRG_getEdgeSweepProperties (m_pRG, &currArea, NULL, const_cast<DPoint3dP>(&refPoint), currNodeId);

            areaToCurve += currArea;
            sectorSequence.m_s10       = currStartFraction;
            sectorSequence.m_s11       = currEndFraction;
            sectorSequence.m_curveId1  = currCurveId;
            prevEndFraction            = currEndFraction;
            sectorSequence.m_numSector++;

            savedSequencer.Copy (lookaheadSequencer);
            }

        if (sectorSequence.m_numSector < 2)
            return false;

        area = areaToCurve;
        sequencer.Copy (savedSequencer);

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AppendSectorSequenceToLoop (CurveVectorPtr& loopCurve, RegionSectorSequence& sectorSequence)
    {
    DEllipse3d      ellipse;
    MSBsplineCurve  bcurve;

    if (jmdlRIMSBS_getDEllipse3d (m_pCurves, &ellipse, sectorSequence.m_parentCurveId, false))
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (ellipse);

        primitive->GetCurvePrimitiveInfoW () = CurveNodeInfo::Create (sectorSequence.m_nodeId0); // Store nodeId of first edge in sequence...
        loopCurve->push_back (primitive);

        return SUCCESS;
        }
    else if (jmdlRIMSBS_getMappedMSBsplineCurve (m_pCurves, &bcurve, sectorSequence.m_parentCurveId, sectorSequence.m_s00, sectorSequence.m_s11))
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateBsplineCurve (bcurve);

        primitive->GetCurvePrimitiveInfoW () = CurveNodeInfo::Create (sectorSequence.m_nodeId0); // Store nodeId of first edge in sequence...
        loopCurve->push_back (primitive);
        bcurve.ReleaseMem ();

        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AppendCurveToLoop (CurveVectorPtr& loopCurve, ICurvePrimitivePtr& linearPrimitive, bvector<int>* pSequenceArray, int nodeId)
    {
    int             curveIndex;
    bool            isReversed;
    DPoint3d        point[2];
    DEllipse3d      ellipse;
    MSBsplineCurve  bcurve;

    jmdlRG_getCurveData (m_pRG, &curveIndex, &isReversed, &point[0], &point[1], nodeId);

    if (RG_NULL_CURVEID == curveIndex)
        {
        if (!linearPrimitive.IsValid ())
            {
            linearPrimitive = ICurvePrimitive::CreateLineString (&point[0], 1);

            linearPrimitive->GetCurvePrimitiveInfoW () = CurveNodeInfo::Create (nodeId);
            }
        else
            {
            CurveNodeInfo* nodeInfo = dynamic_cast <CurveNodeInfo*> (linearPrimitive->GetCurvePrimitiveInfoW ().get ());

            if (nodeInfo)
                nodeInfo->m_nodeIds.push_back (nodeId);
            }

        linearPrimitive->GetLineStringP ()->push_back (point[1]);

        return SUCCESS;
        }
    else if (jmdlRIMSBS_getDEllipse3d (m_pCurves, &ellipse, curveIndex, isReversed))
        {
        FlushLinearBuffer (loopCurve, linearPrimitive);

        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (ellipse);

        primitive->GetCurvePrimitiveInfoW () = CurveNodeInfo::Create (nodeId);
        loopCurve->push_back (primitive);

        return SUCCESS;
        }
    else if (jmdlRIMSBS_getMSBsplineCurve (m_pCurves, &bcurve, curveIndex, isReversed))
        {
        FlushLinearBuffer (loopCurve, linearPrimitive);

        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateBsplineCurve (bcurve);

        primitive->GetCurvePrimitiveInfoW () = CurveNodeInfo::Create (nodeId);
        loopCurve->push_back (primitive);
        bcurve.ReleaseMem ();

        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            FlushLinearBuffer (CurveVectorPtr& loopCurve, ICurvePrimitivePtr& linearPrimitive)
    {
    if (!linearPrimitive.IsValid ())
        return;

    loopCurve->push_back (linearPrimitive);
    linearPrimitive = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GetRegionLoop (CurveVectorPtr& loopCurve, bvector<int>* pSequenceArray, int& sequenceIndex)
    {
    int             i = sequenceIndex;
    RegionSequencer sequencer;

    InitSequencerFromBreak (sequencer, pSequenceArray, sequenceIndex);

    int     nodeId, count = 0;
    double  totalArea = 0.0;

    ICurvePrimitivePtr  linearPrimitive;

    loopCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);

    for ( ; sequencer.Advance (i) && jmdlEmbeddedIntArray_getInt (pSequenceArray, &nodeId, i); )
        {
        if (!jmdlMTGGraph_isValidNodeId (jmdlRG_getGraph (m_pRG), nodeId))
            break;

        DPoint3d    refPoint;

        if (count++ == 0)
            jmdlRG_getVertexData (m_pRG, &refPoint, 0, NULL, nodeId, 0.0);

        double                  areaToCurve = 0.0;
        int                     numStep;
        DEllipse3d              ellipse;
        RegionSectorSequence    sectorSequence;

        if (LookAheadForEllipseSequence (sequencer, areaToCurve, ellipse, numStep, pSequenceArray, refPoint))
            {
            FlushLinearBuffer (loopCurve, linearPrimitive);

            ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (ellipse);

            primitive->GetCurvePrimitiveInfoW () = CurveNodeInfo::Create (nodeId);
            loopCurve->push_back (primitive);
            }
        else if (LookAheadForForSectorSequence (sequencer, areaToCurve, sectorSequence, pSequenceArray, refPoint))
            {
            FlushLinearBuffer (loopCurve, linearPrimitive);

            if (SUCCESS != AppendSectorSequenceToLoop (loopCurve, sectorSequence))
                return ERROR;
            }
        else
            {
            jmdlRG_getEdgeSweepProperties (m_pRG, &areaToCurve, NULL, &refPoint, nodeId);

            if (SUCCESS != AppendCurveToLoop (loopCurve, linearPrimitive, pSequenceArray, nodeId))
                return ERROR;
            }

        totalArea += areaToCurve;
        }

    FlushLinearBuffer (loopCurve, linearPrimitive);

    if (0 == loopCurve->size ())
        return ERROR;

    loopCurve->SetBoundaryType (totalArea > 0.0 ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Inner);

#ifdef RegionUntilTransform_NEEDSWORK
    TransformCP placementTransP = m_context->GetCurrLocalToFrustumTransformCP ();

    if (placementTransP)
        loopCurve->TransformInPlace (*placementTransP);

    // Flatten to plane defined by "average" geometry depth...
    if (ComputePostFlattenTransform (*loopCurve))
        loopCurve->TransformInPlace (m_flattenTrans);
#endif
    return SUCCESS;
    }

size_t Load (MTG_MarkSet *pMarkSet)
    {
    m_startArray.clear ();
    m_sequenceArray.clear ();
    return (size_t)jmdlRG_collectAndNumberExtendedFaceLoops (m_pRG, &m_startArray, &m_sequenceArray, pMarkSet);
    }

ICurvePrimitivePtr ExpandCurve (MTGNodeId seedNodeId)
    {
    int             curveIndex;
    bool            isReversed;
    DPoint3d        point[2];
    DEllipse3d      ellipse;
    MSBsplineCurve  bcurve;

    jmdlRG_getCurveData (m_pRG, &curveIndex, &isReversed, &point[0], &point[1], seedNodeId);
    ICurvePrimitivePtr curve;
    if (RG_NULL_CURVEID == curveIndex)
        {
        curve =ICurvePrimitive::CreateLine (DSegment3d::From (point[0], point[1]));
        }
    else if (jmdlRIMSBS_getDEllipse3d (m_pCurves, &ellipse, curveIndex, isReversed))
        {
        curve = ICurvePrimitive::CreateArc (ellipse);
        }
    else if (jmdlRIMSBS_getMSBsplineCurve (m_pCurves, &bcurve, curveIndex, isReversed))
        {
        curve = ICurvePrimitive::CreateBsplineCurve (bcurve);
        bcurve.ReleaseMem ();
        return curve;
        }    
    if (curve.IsValid ())
        curve->SetTag (seedNodeId);
    return NULL;
    }

CurveVectorPtr ExpandAllComponents ()
    {
    CurveVectorPtr allComponents = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    int componentCount = 0;
    Printer printer (s_noisyParse);
    for (size_t i = 0; i < m_sequenceArray.size (); i++)
        {
        printer.Begin ("Component", componentCount);
        if (m_sequenceArray[i] == MTG_MARKER_START_COMPONENT)
            {
            int faceCount = 0;
            CurveVectorPtr component = NULL;
            for (i++; m_sequenceArray[i] == MTG_MARKER_START_FACE;)
                {
                printer.Begin ("Face", faceCount);
                int indexA = (int)i + 1;
                CurveVectorPtr newLoop;
                if (SUCCESS != GetRegionLoop (newLoop, &m_sequenceArray, indexA)
                    || !newLoop.IsValid ())
                    {
                    return NULL;
                    }
                i = (size_t)indexA;
                printer.End ("Face", faceCount);
                faceCount++;
                if (faceCount == 1)  // first face of component -- it's just an outer loop
                    {
                    component = newLoop;
                    }
                else
                    {
                    if (faceCount > 2)
                        {
                        component->Add (newLoop);
                        }
                    else  // promote to parity region...
                        {
                        CurveVectorPtr outerLoop = component;
                        component = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
                        component->Add (outerLoop);
                        component->Add (newLoop);
                        }
                    }
                }
            if (component.IsValid ())
                allComponents->push_back (ICurvePrimitive::CreateChildCurveVector (component));
            printer.End ("Component", componentCount);
            }
        }
    if (allComponents->size () == 0)
        return NULL;
    if (allComponents->size () == 1)
        return allComponents->at (0)->GetChildCurveVectorP ();
    return allComponents;
    }
};

CurveVectorPtr jmdlRG_collectExtendedFaces
(
RG_Header           *pRG,
RIMSBS_Context*     pCurves,
MTG_MarkSet         *pMarkSet
)
    {
    ExtendedFaceParser parser (pRG, pCurves);
    if (parser.Load (pMarkSet))
        return parser.ExpandAllComponents ();    
    return NULL;
    }

CurveVectorPtr jmdlRG_collectSimpleFace
(
RG_Header           *pRG,
RIMSBS_Context*     pCurves,
MTGNodeId           faceNodeId
)
    {
    CurveVectorPtr loop = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    MTGGraphP graph = jmdlRG_getGraph (pRG);
    MTGARRAY_FACE_LOOP (currNodeId, graph, faceNodeId)
        {
        int             curveIndex;
        bool            isReversed;
        DPoint3d        point[2];
        DEllipse3d      ellipse;
        MSBsplineCurve  bcurve;

        jmdlRG_getCurveData (pRG, &curveIndex, &isReversed, &point[0], &point[1], currNodeId);

        if (RG_NULL_CURVEID == curveIndex)
            {
            loop->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (point[0], point[1])));
            }
        else if (jmdlRIMSBS_getDEllipse3d (pCurves, &ellipse, curveIndex, isReversed))
            {
            loop->push_back (ICurvePrimitive::CreateArc (ellipse));
            }
        else if (jmdlRIMSBS_getMSBsplineCurve (pCurves, &bcurve, curveIndex, isReversed))
            {
            loop->push_back (ICurvePrimitive::CreateBsplineCurve (bcurve));
            bcurve.ReleaseMem ();
            }
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, graph, faceNodeId)

    return loop;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
