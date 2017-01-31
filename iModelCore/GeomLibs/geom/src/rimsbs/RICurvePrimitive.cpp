/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/rimsbs/RICurvePrimitive.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../memory/msvarray.fdf"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static int s_globalNoisy = 0;

static bool    IsValidCurveId (int curveId)
    {
    return curveId != RIMSBS_NULL_CURVE_ID ? true : false;
    }
struct AnnotatedCurve
{
ICurvePrimitivePtr curve;
int groupId;
int userInt;
void *pUserData;
int alternateCurveId;
int64_t userInt64;
int parentCurveId;   // hm.. does the partial curve usage need INDEX or just pointer?

AnnotatedCurve (ICurvePrimitivePtr &_curve, int _userInt, void *_pUserData, int _alternateCurveId, int _groupId)
    {
    curve = _curve;
    userInt   = _userInt;
    pUserData = _pUserData;
    alternateCurveId = _alternateCurveId;
    groupId = _groupId;
    userInt64 = 0;
    }
};

struct RICurveVectorContext : bvector<AnnotatedCurve>
{
int currGroupId;
bool ValidIndex (size_t i) {return (size_t) i < size ();}
};

typedef RICurveVectorContext const &RICurveVectorContextCR;
typedef RICurveVectorContext &RICurveVectorContextR;
typedef RICurveVectorContext const * RICurveVectorContextCP;
typedef RICurveVectorContext * RICurveVectorContextP;

Public RICurveVectorContext *jmdlRICurveVector_newContext (void)
    {
    RICurveVectorContext *pContext = new RICurveVectorContext ();
    return  pContext;
    }

Public void     jmdlRICurveVector_freeContext (RICurveVectorContext *pContext)
    {
    delete pContext;
    }
#ifdef SETUP_CALLBACKS
Public void     jmdlRICurveVector_setupRGCallbacks
(
RICurveVectorContext *pContext,
RG_Header       *pRG
)
    {
    jmdlRG_setCurveFunctions
                (
                pRG,
                jmdlRICurveVector_getCurveRange,
                jmdlRICurveVector_evaluate,
                jmdlRICurveVector_curveCurveIntersection,
                jmdlRICurveVector_segmentCurveIntersection,
                jmdlRICurveVector_createSubcurve,
                jmdlRICurveVector_sweptProperties,
                jmdlRICurveVector_evaluateDerivatives,
                jmdlRICurveVector_getClosestXYPointOnCurve,
                jmdlRICurveVector_curveCircleIntersectionXY,
                jmdlRICurveVector_getGroupId,
                jmdlRICurveVector_consolidateCoincidentGeometry
                );
    jmdlRG_setCurveFunctions01
                (
                pRG,
                jmdlRICurveVector_appendAllCurveSamplePoints,
                jmdlRICurveVector_transformCurve,
                jmdlRICurveVector_transformAllCurves
                );
    }
#endif
Public void     jmdlRICurveVector_releaseMem
(
RICurveVectorContext *pContext
)
    {
    pContext->clear ();
    pContext->currGroupId = 0;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_addElement                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
static int      jmdlRICurveVector_addElement
(
RICurveVectorContext *pContext,
int             type,
int             userInt,
void            *pUserData,
ICurvePrimitivePtr &curve
)
    {
    pContext->push_back (AnnotatedCurve (curve, userInt, pUserData, RIMSBS_NULL_CURVE_ID, pContext->currGroupId));
    return  (int)pContext->size () - 1;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_getCurveP                                        |
|                                                                       |
| Author:   EarlinLutz                               9/15/09            |
|                                                                       |
+----------------------------------------------------------------------*/
static MSBsplineCurveP jmdlRICurveVector_getCurveP
(
RICurveVectorContext *pContext,
int             index
)
    {
    if (pContext->ValidIndex (index))
        {
        return const_cast <MSBsplineCurveP>(pContext->at((size_t)index).curve->GetBsplineCurveCP ());
        }
    return NULL;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_getUserPointer                                   |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Get the user data pointer part of a curve.                            |
+----------------------------------------------------------------------*/
Public bool     jmdlRICurveVector_getUserPointer
(
RICurveVectorContext  *pContext,
void            **pUserData,
int             index
)
    {
    if (pContext->ValidIndex (index))
        {
        *pUserData      = pContext->at((size_t)index).pUserData;
        return true;
        }
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_getUserInt                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Get the user int part of a curve.                                     |
+----------------------------------------------------------------------*/
Public bool     jmdlRICurveVector_getUserInt
(
RICurveVectorContext  *pContext,
int             *pUserInt,
int             index
)
    {
    if (pContext->ValidIndex (index))
        {
        *pUserInt      = pContext->at((size_t)index).userInt;
        return true;
        }
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_getUserInt64                                     |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Get the user Int64 part of a curve.                                   |
+----------------------------------------------------------------------*/
Public bool     jmdlRICurveVector_getUserInt64
(
RICurveVectorContext  *pContext,
int64_t         *pUserInt64,
int             index
)
    {
    if (pContext->ValidIndex (index))
        {
        *pUserInt64      = pContext->at((size_t)index).userInt64;
        return true;
        }
    return false;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_setUserInt                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Set the user int part of a curve.                                     |
+----------------------------------------------------------------------*/
Public bool     jmdlRICurveVector_setUserInt
(
RICurveVectorContext  *pContext,
int             index,
int             userInt
)
    {
    if (pContext->ValidIndex (index))
        {
        pContext->at((size_t)index).userInt = userInt;
        return true;
        }
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_setUserInt64                                     |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Set the user Int64 part of a curve.                                   |
+----------------------------------------------------------------------*/
Public bool     jmdlRICurveVector_setUserInt64
(
RICurveVectorContext  *pContext,
int             index,
int64_t         userInt64
)
    {
    if (pContext->ValidIndex (index))
        {
        pContext->at((size_t)index).userInt64 = userInt64;
        return true;
        }
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_setUserPointer                                   |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Set the user pointer part of a curve.                                 |
+----------------------------------------------------------------------*/
Public bool     jmdlRICurveVector_setUserPointer
(
RICurveVectorContext  *pContext,
int             index,
void            *pUserData
)
    {
    if (pContext->ValidIndex (index))
        {
        pContext->at((size_t)index).pUserData = pUserData;
        return true;
        }
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_setGroupId                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRICurveVector_setGroupId
(
RICurveVectorContext  *pContext,
RIMSBS_CurveId  index,
int             groupId
)
    {
    if (pContext->ValidIndex (index))
        {
        pContext->at((size_t)index).groupId = groupId;
        return true;
        }
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_setGroupId                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRICurveVector_getGroupId
(
RICurveVectorContext  *pContext,
int             *pGroupId,
RIMSBS_CurveId  index
)
    {
    if (pContext->ValidIndex (index))
        {
        *pGroupId      = pContext->at((size_t)index).groupId;
        return true;
        }
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_setCurrGroupId                                   |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Set the current group id to be applied to new elements.               |
+----------------------------------------------------------------------*/
Public bool        jmdlRICurveVector_setCurrGroupId
(
RICurveVectorContext  *pContext,
int             groupId
)
    {
    bool    myResult = true;
    pContext->currGroupId = groupId;
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_getImmediateParent                               |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Get the immediate parent of a curve.                                  |
| Returns the same index if not a child.                                |
+----------------------------------------------------------------------*/
Public bool     jmdlRICurveVector_getImmediateParent
(
RICurveVectorContext  *pContext,
int             *pParentIndex,
int             index
)
    {
    if (pContext->ValidIndex (index))
        {
        *pParentIndex      = pContext->at((size_t)index).parentCurveId;
        return IsValidCurveId (*pParentIndex) ? true : false;
        }
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_getFarthestParent                                |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Follow parent pointers as far as possible.                            |
+----------------------------------------------------------------------*/
Public bool     jmdlRICurveVector_getFarthestParent
(
RICurveVectorContext  *pContext,
int             *pParentIndex,
int             index
)
    {
    int oldIndex = *pParentIndex = index;
    do
        {
        oldIndex = *pParentIndex;
        if (!jmdlRICurveVector_getImmediateParent (pContext, pParentIndex, *pParentIndex))
            return false;
        } while (*pParentIndex != oldIndex);

    return true;

    }
#ifdef __SetElementHeader
/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_setElementHeader                                 |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool             jmdlRICurveVector_setElementHeader
(
RICurveVectorContext          *pContext,
RIMSBS_ElementHeader    *pElementHeader,
int                     index
)
    {
    return SUCCESS == omdlVArray_set
                        (
                        &pContext->vbaElements,
                        (char *)pElementHeader,
                        index
                        )
            ? true : false;
    }
#endif

#ifdef abc
/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_addDEllipse3d                                    |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      jmdlRICurveVector_addDEllipse3d
(
RICurveVectorContext  *pContext,
int             userInt,
void            *pUserData,
const DEllipse3d *pEllipse
)
    {

    if (pEllipse->isFullEllipse ())
        {
        RG_CurveId fullEllipseCurveId = jmdlRICurveVector_addElement (pContext,
                            RIMSBS_DEllipse3d, 0, NULL,
                            pEllipse, sizeof (DEllipse3d));
        // RG does not like closed curves.
        // Split in halves, create (and return) chain.
        RIMSBS_CurveId chainId = jmdlRICurveVector_addCurveChain (pContext, userInt, pUserData, fullEllipseCurveId);
        DEllipse3d arc[2];
        arc[0] = arc[1] = *pEllipse;
        static double splitFraction = 0.5;
        double fraction0 = splitFraction;
        double fraction1 = 1.0 - fraction0;
        arc[0].start = pEllipse->start;
        arc[0].sweep = fraction0 * pEllipse->sweep;
        arc[1].start = arc[0].start + arc[0].sweep;
        arc[1].sweep = fraction1 * pEllipse->sweep;

        double s0 = 0.0;
        double s1 = 0.5;

        for (int i = 0; i < 2; i++, s0 = s1, s1 += 0.5)
            {
            RIMSBS_CurveId childCurveId = jmdlRICurveVector_addDEllipse3d (pContext, 0, NULL, &arc[i]);
            RIMSBS_CurveId curveIntervalId;
            if (jmdlRICurveVector_createSubcurveExt (pContext, &curveIntervalId, chainId, childCurveId, s0, s1))
                jmdlRICurveVector_addCurveToCurveChain (pContext, chainId, curveIntervalId);
            }
        return chainId;
        }
    else
        return jmdlRICurveVector_addElement (pContext,
                            RIMSBS_DEllipse3d, userInt, pUserData,
                            pEllipse, sizeof (DEllipse3d));

    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_addDataCarrier                                   |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      jmdlRICurveVector_addDataCarrier
(
RICurveVectorContext  *pContext,
int             userInt,
void            *pUserData
)
    {
    return  jmdlRICurveVector_addElement (pContext,
                            RIMSBS_DataCarrier, userInt, pUserData, NULL, 0);

    }

RIMSBS_CurveChainStruct::RIMSBS_CurveChainStruct (RIMSBS_CurveId primaryCurveId)
    : m_primaryCurveId (primaryCurveId)
    {
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_addCurveChain                                    |
|                                                                       |
| Author:   EarlinLutz                               09/09              |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      jmdlRICurveVector_addCurveChain
(
RICurveVectorContext  *pContext,
int             userInt,
void            *pUserData,
RIMSBS_CurveId  primaryCurveId
)
    {
    RIMSBS_CurveChainStruct* chain = new RIMSBS_CurveChainStruct (primaryCurveId);
    return  jmdlRICurveVector_addElement (pContext,
                            RIMSBS_CurveChain, userInt, pUserData, chain, 0);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_addCurveToCurveChain                             |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRICurveVector_addCurveToCurveChain
(
RICurveVectorContext  *pContext,
RIMSBS_CurveId  chainId,
RIMSBS_CurveId  childCurveId
)
    {
    RIMSBS_ElementHeader parentHeader;

    if (jmdlRICurveVector_getElementHeader (pContext, &parentHeader, chainId)
        && parentHeader.type == RIMSBS_CurveChain)
        {
        RIMSBS_CurveChainStruct *pChain = (RIMSBS_CurveChainStruct*)parentHeader.pGeometryData;
        pChain->push_back (childCurveId);
        return true;
        }
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_isCurveChain                                     |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRICurveVector_isCurveChain
(
RICurveVectorContext  *pContext,
RIMSBS_CurveId  *pPrimaryCurveId,
RIMSBS_CurveId  curveId
)
    {
    RIMSBS_ElementHeader parentHeader;
    if (pPrimaryCurveId != NULL)
        *pPrimaryCurveId = 0;
    if (jmdlRICurveVector_getElementHeader (pContext, &parentHeader, curveId)
        && parentHeader.type == RIMSBS_CurveChain)
        {
        RIMSBS_CurveChainStruct *pChain = (RIMSBS_CurveChainStruct*)parentHeader.pGeometryData;
        *pPrimaryCurveId = pChain->m_primaryCurveId;
        return true;
        }
    return false;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_getChildCurve                                    |
|                                                                       |
| Author:   EarlinLutz                               09/09              |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRICurveVector_getCurveChainChild
(
RICurveVectorContext  *pContext,
RIMSBS_CurveId  curveId,
int             childIndex,
RIMSBS_CurveId  *pChildId
)
    {
    RIMSBS_ElementHeader parentHeader;
    *pChildId = -1;
    if (jmdlRICurveVector_getElementHeader (pContext, &parentHeader, curveId)
        && parentHeader.type == RIMSBS_CurveChain)
        {
        RIMSBS_CurveChainStruct *pChain = (RIMSBS_CurveChainStruct*)parentHeader.pGeometryData;
        if (NULL != pChain && childIndex >= 0 && (size_t)childIndex < pChain->size ())
            {
            *pChildId = pChain->at(childIndex);
            return true;
            }
        }
    return false;
    }



/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_createAlternateMSBsplineCurve                    |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      jmdlRICurveVector_createAlternateMSBsplineCurve
(
RICurveVectorContext  *pContext,
RIMSBS_CurveId  parentId
)
    {
    RIMSBS_CurveId childId = RIMSBS_NULL_CURVE_ID;
    MSBsplineCurve *pCurve;
    RIMSBS_ElementHeader parentHeader;

    if (jmdlRICurveVector_getElementHeader (pContext, &parentHeader, parentId))
        {
        switch (parentHeader.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)parentHeader.pGeometryData;
                pCurve = (MSBsplineCurve *)malloc (sizeof(MSBsplineCurve));
                if (   pCurve
                    && SUCCESS == bspconv_convertDEllipse3dToCurve (pCurve, pEllipse)
                    )
                    {
                    childId = jmdlRICurveVector_addMSBsplineCurve (pContext, -1, NULL, pCurve);
                    parentHeader.alternateCurveId = childId;
                    jmdlRICurveVector_setElementHeader (pContext, &parentHeader, parentId);
                    }
                }
                break;
            }
        }
    return childId;

    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_addMSBsplineCurve                                |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Copy the curve structure (bitwise) into the context.  Caller must     |
| NOT free the curve.                                                   |
+----------------------------------------------------------------------*/
Public int      jmdlRICurveVector_addMSBsplineCurveDirect
(
RICurveVectorContext  *pContext,
int             userInt,
void            *pUserData,
MSBsplineCurveP  pCurve
)
    {
    static bool    s_forceOpenCurves = 1;
    if (s_forceOpenCurves && pCurve->params.closed)
        bspcurv_openCurve (pCurve, pCurve, 0.0);
    // This copies bits from the curve struct ...    
    int stat =jmdlRICurveVector_addElement (pContext,
                        RIMSBS_MSBsplineCurve, userInt, pUserData,
                        pCurve, sizeof (MSBsplineCurve));
    memset (pCurve, 0, sizeof (MSBsplineCurve));
    return stat;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_addMSBsplineCurve                                |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Copy the curve structure (bitwise) into the context.  Caller must     |
| NOT free the curve.                                                   |
| Returned curveId may be RIMSBS_CurveChain or RIMSBS_MSBsplineCurve    |
+----------------------------------------------------------------------*/
Public int      jmdlRICurveVector_addMSBsplineCurve
(
RICurveVectorContext  *pContext,
int             userInt,
void            *pUserData,
MSBsplineCurveP  pCurve
)
    {
    RIMSBS_CurveId refCurveId = jmdlRICurveVector_addMSBsplineCurveDirect (pContext, 0, NULL, pCurve);
    RIMSBS_CurveId chainId = jmdlRICurveVector_addCurveChain (pContext, userInt, pUserData, refCurveId);
    // Hmm.. the curve has been taken away (and zeroed!!!)
    MSBsplineCurveP pSavedCurve = jmdlRICurveVector_getCurveP (pContext, refCurveId);

    BCurveSegment segment;


    double a = 0.0;
    bool    isWeighted = pSavedCurve->rational;
    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
        if (!segment.IsNullU ())
            {
            MSBsplineCurve currCurve;
            if (currCurve.InitFromPoles (segment.GetPoleP (), segment.GetOrder ())
                {
                RIMSBS_CurveId childCurveId = jmdlRICurveVector_addMSBsplineCurveDirect (pContext, 0, NULL, &currCurve);
                RIMSBS_CurveId curveIntervalId;
                if (jmdlRICurveVector_createSubcurveExt (pContext, &curveIntervalId, chainId, childCurveId, context.s0, context.s1))
                    jmdlRICurveVector_addCurveToCurveChain (pContext, chainId, curveIntervalId);
                }
            }
        }
    }
    return chainId;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_convertDPoint4dBezierToCurve                    |
|                                                                       |
| author        LuHan                                   3/95            |
|                                                                       |
| Captured from bspicurv 6/9/99 by EDL.   Delete this copy              |
| when merge problems are over?                                         |
+----------------------------------------------------------------------*/
static int         convertDPoint4dBezierToCurve
(
MSBsplineCurve      *pCurve,
const DPoint4d      *pPoles,
int                 order
)
    {
    int     i, status;

    if (pCurve == NULL || pPoles == NULL || order < 2)
        return  ERROR;

    memset (pCurve, 0, sizeof(MSBsplineCurve));
    for (i = 0; i < order; i++)
        {
        if (fabs(pPoles[i].w - 1.0) > 1.0e-12)
            {
            pCurve->rational = true;
            break;
            }
        }

    pCurve->params.order =
    pCurve->params.numPoles = order;
    if (SUCCESS == (status = bspcurv_allocateCurve (pCurve)))
        {
        for (i = 0; i < order; i++)
            {
            pCurve->poles[i].x = pPoles[i].x;
            pCurve->poles[i].y = pPoles[i].y;
            pCurve->poles[i].z = pPoles[i].z;
            if (pCurve->rational == true)
                pCurve->weights[i] = pPoles[i].w;
            }
        status = bspknot_computeKnotVector (pCurve->knots, &pCurve->params, NULL);
        }

    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_addBezier                                        |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Create an MSBsplineCurve for the given bezier.                        |
+----------------------------------------------------------------------*/
Public int      jmdlRICurveVector_addBezier
(
RICurveVectorContext  *pContext,
int             userInt,
void            *pUserData,
const DPoint4d  *pPoleArray,
int             order
)
    {
    MSBsplineCurve curve;
    if (SUCCESS == convertDPoint4dBezierToCurve (&curve, pPoleArray, order))
        {
        return jmdlRICurveVector_addMSBsplineCurveDirect (pContext, userInt, pUserData, &curve);
        }
    else
        return RIMSBS_NULL_CURVE_ID;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_createSubcurveExt                                |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRICurveVector_createSubcurveExt
(
RICurveVectorContext  *pContext,
RIMSBS_CurveId  *pNewCurveId,
RIMSBS_CurveId  parentCurveId,
RIMSBS_CurveId  partialCurveId,
double          s0,
double          s1
)
    {
    bool    myResult = false;
    RIMSBS_CurveId  newCurveId = -1;
    RIMSBS_ElementHeader desc0;
    RIMSBS_CurveIntervalStruct refData;

    if  (jmdlRICurveVector_getElementHeader (pContext, &desc0, parentCurveId))
        {
        myResult = true;
        refData.parentId = parentCurveId;
        refData.s0 = s0;
        refData.s1 = s1;
        refData.partialCurveId = partialCurveId;
        newCurveId = jmdlRICurveVector_addElement (pContext,
                            RIMSBS_CurveInterval, 0, NULL,
                            &refData, sizeof (refData));
        jmdlRICurveVector_setGroupId (pContext, newCurveId, desc0.groupId);
        }

    *pNewCurveId = newCurveId;
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_createSubcurve                                   |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRICurveVector_createSubcurve
(
RICurveVectorContext  *pContext,
RIMSBS_CurveId  *pNewCurveId,
RIMSBS_CurveId  parentCurveId,
double          s0,
double          s1
)
    {
    return jmdlRICurveVector_createSubcurveExt (pContext, pNewCurveId, parentCurveId, -1, s0, s1);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     circularParts                                               |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Return centerX, centerY, and radius as dpoint3d if true circle.       |
+----------------------------------------------------------------------*/
static bool    circularParts
(
const DEllipse3d *pEllipse,
DPoint3d        *pXYR,
double tol
)
    {
    double  dx0, dy0, dx90, dy90, r0, r90;
    dx0 = pEllipse->vector0.x;
    dy0 = pEllipse->vector0.y;
    dx90 = pEllipse->vector90.x;
    dy90 = pEllipse->vector90.y;
    r0  = sqrt (dx0  * dx0  + dy0  * dy0);
    r90 = sqrt (dx90 * dx90 + dy90 * dy90);
    if (fabs (r0 - r90) < tol)
        {
        /* + 90 rotation? dx90 = -dy0, dy90 = dx0
           - 90 rotation? dx90 = dy0, dy90 = - dx0
        */
        if (  (fabs (dx90 + dy0) < tol &&  fabs (dy90 - dx0) < tol)
           || (fabs (dx90 - dy0) < tol &&  fabs (dy90 + dx0) < tol)
           )
            {
            bsiDPoint3d_setXYZ (pXYR, pEllipse->center.x, pEllipse->center.y, r0);
            return true;
            }
        }
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     forceEllipseToExactCircular                                 |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Replace the 90 degree vector by the 0 degree vector rotated exactly   |
| 90 degrees (but z and preserve sign of orientation)                   |
+----------------------------------------------------------------------*/
static void forceEllipseToExactCircular
(
DEllipse3d  *pEllipse
)
    {
    if (bsiDPoint3d_crossProductXY (&pEllipse->vector0, &pEllipse->vector90) >= 0.0)
        {
        pEllipse->vector90.x = -pEllipse->vector0.y;
        pEllipse->vector90.y =  pEllipse->vector0.x;
        }
    else
        {
        pEllipse->vector90.x =  pEllipse->vector0.y;
        pEllipse->vector90.y = -pEllipse->vector0.x;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     matchCircularParts                                          |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Compare ellipse with circle.                                          |
+----------------------------------------------------------------------*/
static bool    mergeCircularParts
(
DEllipse3d  *pEllipse1,
const DPoint3d  *pXYR0,
double tol
)
    {
    DPoint3d xyr1;
    double radialScale;
    bool    myResult = false;
    static double s_minRadiusFactor = 100.0;
    if (    fabs (pEllipse1->center.x - pXYR0->x) < tol
        &&  fabs (pEllipse1->center.y - pXYR0->y) < tol
        && circularParts (pEllipse1, &xyr1, tol )
        && fabs (pXYR0->z - xyr1.z) < tol
        && fabs (xyr1.z) > s_minRadiusFactor * tol
        )
        {
        pEllipse1->center.x = pXYR0->x;
        pEllipse1->center.y = pXYR0->y;
        radialScale = pXYR0->z / xyr1.z;
        /* Scale to the right length.  Might be off by a bit, but we preserve the
            old direction. */
        bsiDPoint3d_scaleInPlace (&pEllipse1->vector0, radialScale);
        forceEllipseToExactCircular (pEllipse1);
        myResult = true;
        }
    return myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRICurveVector_consolidateCoincidentGeometry                    |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRICurveVector_consolidateCoincidentGeometry
(
RICurveVectorContext  *pContext,
double tolerance
)
    {
    int i0, xyrIndex0, xyrIndex1, elementIndex0, elementIndex1;
    RIMSBS_ElementHeader header0, header1;
    DEllipse3d *pEllipse0, *pEllipse1;
    double tol2 = tolerance * tolerance;
    DPoint3d xyr0;
    int numMatch = 0;
    int numTotalMatch = 0;
    EmbeddedIntArray *pXYRToElementIndexArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedDPoint3dArray *pXYRArray = jmdlEmbeddedDPoint3dArray_grab ();
    EmbeddedIntArray *pBlockedXYRIndexArray = jmdlEmbeddedIntArray_grab ();

    /* Free the type-specfic memor of each curve */
    for (i0 = 0; SUCCESS == omdlVArray_get (&pContext->vbaElements, (char *)&header0, i0) ;i0++ )
        {
        if (header0.type == RIMSBS_DEllipse3d)
            {
            pEllipse0 = (DEllipse3d *)header0.pGeometryData;
            if (circularParts (pEllipse0, &xyr0, tolerance))
                {
                jmdlEmbeddedDPoint3dArray_addDPoint3d (pXYRArray, &xyr0);
                jmdlEmbeddedIntArray_addInt (pXYRToElementIndexArray, i0);
                }
            }
        }

    if (jmdlEmbeddedDPoint3dArray_getCount (pXYRArray) > 0)
        {
        jmdlVArrayDPoint3d_identifyMatchedVertices (pXYRArray, NULL, pBlockedXYRIndexArray, tolerance, 0.0);

        for (i0 = 0; jmdlEmbeddedIntArray_getInt (pBlockedXYRIndexArray, &xyrIndex0, i0); i0++)
            {
            if (xyrIndex0 >= 0)
                {
                numMatch = 0;
                jmdlEmbeddedIntArray_getInt (pXYRToElementIndexArray, &elementIndex0, xyrIndex0);
                omdlVArray_get (&pContext->vbaElements, (char *)&header0, elementIndex0);
                pEllipse0 = (DEllipse3d *)header0.pGeometryData;
                circularParts (pEllipse0, &xyr0, tolerance);
                for (
                    i0++;
                    jmdlEmbeddedIntArray_getInt (pBlockedXYRIndexArray, &xyrIndex1, i0)
                        && xyrIndex1 >= 0;
                    i0++
                    )
                    {
                    jmdlEmbeddedIntArray_getInt (pXYRToElementIndexArray, &elementIndex1, xyrIndex1);
                    omdlVArray_get (&pContext->vbaElements, (char *)&header1, elementIndex1);
                    pEllipse1 = (DEllipse3d *)header1.pGeometryData;
                    if (mergeCircularParts (pEllipse1, &xyr0, tolerance))
                        {
                        if (numMatch == 0)
                            forceEllipseToExactCircular (pEllipse0);
                        numMatch++;
                        numTotalMatch++;
                        }
                    }
                }
            }
        }

    jmdlEmbeddedIntArray_drop (pXYRToElementIndexArray);
    jmdlEmbeddedIntArray_drop (pBlockedXYRIndexArray);
    jmdlEmbeddedDPoint3dArray_drop (pXYRArray);

    return true;
    }
#endif
END_BENTLEY_GEOMETRY_NAMESPACE