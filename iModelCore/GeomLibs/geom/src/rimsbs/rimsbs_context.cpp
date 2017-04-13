/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/rimsbs/rimsbs_context.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// UNUSED static int s_globalNoisy = 0;


RIMSBS_Context::~RIMSBS_Context ()
    {
    }

RIMSBS_Context::RIMSBS_Context ()
    {
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_newContext                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public RIMSBS_Context *jmdlRIMSBS_newContext
(
void
)
    {
    return new RIMSBS_Context ();
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_newContext                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void jmdlRIMSBS_initContext
(
RIMSBS_Context *pContext
)
    {
    // Noop -- assume allocation with constructor
    }


/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_freeContext                                      |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     jmdlRIMSBS_freeContext
(
RIMSBS_Context *pContext
)
    {
    jmdlRIMSBS_releaseMem (pContext);
    delete pContext;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setupRGCallbacks                                 |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     jmdlRIMSBS_setupRGCallbacks
(
RIMSBS_Context *pContext,
RG_Header       *pRG
)
    {
    jmdlRG_setCurveFunctions
                (
                pRG,
                jmdlRIMSBS_getCurveRange,
                jmdlRIMSBS_evaluate,
                jmdlRIMSBS_curveCurveIntersection,
                jmdlRIMSBS_segmentCurveIntersection,
                jmdlRIMSBS_createSubcurve,
                jmdlRIMSBS_sweptProperties,
                jmdlRIMSBS_evaluateDerivatives,
                jmdlRIMSBS_getClosestXYPointOnCurve,
                jmdlRIMSBS_curveCircleIntersectionXY,
                jmdlRIMSBS_getGroupId,
                jmdlRIMSBS_consolidateCoincidentGeometry
                );
    jmdlRG_setCurveFunctions01
                (
                pRG,
                jmdlRIMSBS_appendAllCurveSamplePoints,
                jmdlRIMSBS_transformCurve,
                jmdlRIMSBS_transformAllCurves
                );
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_releaseMem                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     jmdlRIMSBS_releaseMem
(
RIMSBS_Context *pContext
)
    {
    /* Free the type-specfic memor of each curve */
    for(RIMSBS_ElementHeader header : pContext->m_geometry)
        {
        switch (header.type)
            {
            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurve *pCurve = (MSBsplineCurve *)header.pGeometryData;
                bspcurv_freeCurve (pCurve);
                free (pCurve);
                header.pGeometryData = NULL;
                }
                break;
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d *pEllipse = (DEllipse3d *)header.pGeometryData;
                free (pEllipse);
                header.pGeometryData = NULL;
                }
                break;
            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = (RIMSBS_CurveIntervalStruct *)header.pGeometryData;
                free (pInterval);
                header.pGeometryData = NULL;
                break;
                }
            case RIMSBS_CurveChain:
                {
                RIMSBS_CurveChainStruct *pChain = (RIMSBS_CurveChainStruct *)header.pGeometryData;
                delete pChain;
                header.pGeometryData = NULL;
                }
            }
        }
    // Scope exit for onTheSpot bvector forces memory release...
    bvector<RIMSBS_ElementHeader>().swap (pContext->m_geometry);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addElement                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      jmdlRIMSBS_addElement
(
RIMSBS_Context *pContext,
int             type,
int             userInt,
void            *pUserData,
const void      *pGeometry,
int             dataSize
)
    {
    RIMSBS_ElementHeader header;
    header.type          = type;
    header.userInt       = userInt;
    header.pUserData     = pUserData;
    header.alternateCurveId = RIMSBS_NULL_CURVE_ID;
    header.groupId      = pContext->currGroupId;

    if (pGeometry && dataSize > 0)
        {
        header.pGeometryData = malloc (dataSize);
        memcpy (header.pGeometryData, pGeometry, dataSize);
        }
    else if (pGeometry != NULL && dataSize == 0)
        {
        // use the pointer directly -- no copy.
        header.pGeometryData = (void *)pGeometry;
        }
    else
        {
        header.pGeometryData = NULL;
        }

    size_t newId = pContext->m_geometry.size ();
    pContext->m_geometry.push_back (header);

    return  (int)newId;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getElementHeader                                 |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool             jmdlRIMSBS_getElementHeader
(
RIMSBS_Context          *pContext,
RIMSBS_ElementHeader    *pElementHeader,
int                     index
)
    {
    // We do NOT support (-1) as reference to last entry ...
    if (index < 0 || (size_t)index >= pContext->m_geometry.size ())
        return false;
    *pElementHeader = pContext->m_geometry[(size_t)index];
    return true;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getCurveP                                        |
|                                                                       |
| Author:   EarlinLutz                               9/15/09            |
|                                                                       |
+----------------------------------------------------------------------*/
static MSBsplineCurveP jmdlRIMSBS_getCurveP
(
RIMSBS_Context *pContext,
int             index
)
    {
    RIMSBS_ElementHeader header;
    if (!jmdlRIMSBS_getElementHeader (pContext, &header, index))
        return NULL;
    if (header.type != RIMSBS_MSBsplineCurve)
        return NULL;
    return (MSBsplineCurveP)header.pGeometryData;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getUserPointer                                   |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Get the user data pointer part of a curve.                            |
+----------------------------------------------------------------------*/
Public bool     jmdlRIMSBS_getUserPointer
(
RIMSBS_Context  *pContext,
void            **pUserData,
int             index
)
    {
    RIMSBS_ElementHeader header;
    if (jmdlRIMSBS_getElementHeader (pContext, &header, index))
        {
        *pUserData      = header.pUserData;
        return true;
        }

    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getUserInt                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Get the user int part of a curve.                                     |
+----------------------------------------------------------------------*/
Public bool     jmdlRIMSBS_getUserInt
(
RIMSBS_Context  *pContext,
int             *pUserInt,
int             index
)
    {
    RIMSBS_ElementHeader header;
    if (jmdlRIMSBS_getElementHeader (pContext, &header, index))
        {
        *pUserInt      = header.userInt;
        return true;
        }

    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getUserInt64                                     |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Get the user Int64 part of a curve.                                   |
+----------------------------------------------------------------------*/
Public bool     jmdlRIMSBS_getUserInt64
(
RIMSBS_Context  *pContext,
int64_t         *pUserInt64,
int             index
)
    {
    RIMSBS_ElementHeader header;
    if (jmdlRIMSBS_getElementHeader (pContext, &header, index))
        {
        *pUserInt64      = header.userInt64;
        return true;
        }

    return false;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setUserInt                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Set the user int part of a curve.                                     |
+----------------------------------------------------------------------*/
Public bool     jmdlRIMSBS_setUserInt
(
RIMSBS_Context  *pContext,
int             index,
int             userInt
)
    {
    RIMSBS_ElementHeader header;
    if (jmdlRIMSBS_getElementHeader (pContext, &header, index))
        {
        header.userInt = userInt;
        jmdlRIMSBS_setElementHeader (pContext, &header, index);
        return true;
        }

    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setUserInt64                                     |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Set the user Int64 part of a curve.                                   |
+----------------------------------------------------------------------*/
Public bool     jmdlRIMSBS_setUserInt64
(
RIMSBS_Context  *pContext,
int             index,
int64_t         userInt64
)
    {
    RIMSBS_ElementHeader header;
    if (jmdlRIMSBS_getElementHeader (pContext, &header, index))
        {
        header.userInt64 = userInt64;
        jmdlRIMSBS_setElementHeader (pContext, &header, index);
        return true;
        }

    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setUserPointer                                   |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Set the user pointer part of a curve.                                 |
+----------------------------------------------------------------------*/
Public bool     jmdlRIMSBS_setUserPointer
(
RIMSBS_Context  *pContext,
int             index,
void            *pUserData
)
    {
    RIMSBS_ElementHeader header;
    if (jmdlRIMSBS_getElementHeader (pContext, &header, index))
        {
        header.pUserData = pUserData;
        jmdlRIMSBS_setElementHeader (pContext, &header, index);
        return true;
        }

    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setGroupId                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_setGroupId
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId  index,
int             groupId
)
    {
    RIMSBS_ElementHeader header;
    if (jmdlRIMSBS_getElementHeader (pContext, &header, index))
        {
        header.groupId = groupId;
        jmdlRIMSBS_setElementHeader (pContext, &header, index);
        return true;
        }
    return  false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setGroupId                                       |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_getGroupId
(
RIMSBS_Context  *pContext,
int             *pGroupId,
RIMSBS_CurveId  curveId
)
    {
    bool    myResult = false;
    RIMSBS_ElementHeader descr;

    if  (jmdlRIMSBS_getElementHeader (pContext, &descr, curveId))
        {
        *pGroupId = descr.groupId;
        myResult = true;
        }
    return  myResult;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setCurrGroupId                                   |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Set the current group id to be applied to new elements.               |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_setCurrGroupId
(
RIMSBS_Context  *pContext,
int             groupId
)
    {
    bool    myResult = true;
    pContext->currGroupId = groupId;
    return  myResult;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getImmediateParent                               |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Get the immediate parent of a curve.                                  |
| Returns the same index if not a child.                                |
+----------------------------------------------------------------------*/
Public bool     jmdlRIMSBS_getImmediateParent
(
RIMSBS_Context  *pContext,
int             *pParentIndex,
int             index
)
    {
    RIMSBS_ElementHeader header;
    RIMSBS_CurveIntervalStruct *pInterval;
    *pParentIndex = index;

    if (jmdlRIMSBS_getElementHeader (pContext, &header, index))
        {
        if (header.type == RIMSBS_CurveInterval)
            {
            pInterval = (RIMSBS_CurveIntervalStruct *)header.pGeometryData;
            *pParentIndex = pInterval->parentId;
            }
        return true;
        }

    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getFarthestParent                                |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Follow parent pointers as far as possible.                            |
+----------------------------------------------------------------------*/
Public bool     jmdlRIMSBS_getFarthestParent
(
RIMSBS_Context  *pContext,
int             *pParentIndex,
int             index
)
    {
    int oldIndex = *pParentIndex = index;
    do
        {
        oldIndex = *pParentIndex;
        if (!jmdlRIMSBS_getImmediateParent (pContext, pParentIndex, *pParentIndex))
            return false;
        } while (*pParentIndex != oldIndex);

    return true;

    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_setElementHeader                                 |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool             jmdlRIMSBS_setElementHeader
(
RIMSBS_Context          *pContext,
RIMSBS_ElementHeader    *pElementHeader,
int                     index
)
    {
    if (index < 0)
        return false;
    if ((size_t)index >= pContext->m_geometry.size ())
        return false;
    pContext->m_geometry[index] = *pElementHeader;
    return true;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addDEllipse3d                                    |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      jmdlRIMSBS_addDEllipse3d
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData,
const DEllipse3d *pEllipse
)
    {

    if (pEllipse->IsFullEllipse ())
        {
        RG_CurveId fullEllipseCurveId = jmdlRIMSBS_addElement (pContext,
                            RIMSBS_DEllipse3d, userInt, NULL,
                            pEllipse, sizeof (DEllipse3d));
        // RG does not like closed curves.
        // Split in halves, create (and return) chain.
        RIMSBS_CurveId chainId = jmdlRIMSBS_addCurveChain (pContext, userInt, pUserData, fullEllipseCurveId);
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
            RIMSBS_CurveId childCurveId = jmdlRIMSBS_addDEllipse3d (pContext, userInt, NULL, &arc[i]);
            RIMSBS_CurveId curveIntervalId;
            if (jmdlRIMSBS_createSubcurveExt (pContext, &curveIntervalId, chainId, childCurveId, s0, s1))
                jmdlRIMSBS_addCurveToCurveChain (pContext, chainId, curveIntervalId);
            }
        return chainId;
        }
    else
        return jmdlRIMSBS_addElement (pContext,
                            RIMSBS_DEllipse3d, userInt, pUserData,
                            pEllipse, sizeof (DEllipse3d));

    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addDataCarrier                                   |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      jmdlRIMSBS_addDataCarrier
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData
)
    {
    return  jmdlRIMSBS_addElement (pContext,
                            RIMSBS_DataCarrier, userInt, pUserData, NULL, 0);

    }

RIMSBS_CurveChainStruct::RIMSBS_CurveChainStruct (RIMSBS_CurveId primaryCurveId)
    : m_primaryCurveId (primaryCurveId)
    {
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addCurveChain                                    |
|                                                                       |
| Author:   EarlinLutz                               09/09              |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      jmdlRIMSBS_addCurveChain
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData,
RIMSBS_CurveId  primaryCurveId
)
    {
    RIMSBS_CurveChainStruct* chain = new RIMSBS_CurveChainStruct (primaryCurveId);
    return  jmdlRIMSBS_addElement (pContext,
                            RIMSBS_CurveChain, userInt, pUserData, chain, 0);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addCurveToCurveChain                             |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRIMSBS_addCurveToCurveChain
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId  chainId,
RIMSBS_CurveId  childCurveId
)
    {
    RIMSBS_ElementHeader parentHeader;

    if (jmdlRIMSBS_getElementHeader (pContext, &parentHeader, chainId)
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
| Name:     jmdlRIMSBS_isCurveChain                                     |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRIMSBS_isCurveChain
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId  *pPrimaryCurveId,
RIMSBS_CurveId  curveId
)
    {
    RIMSBS_ElementHeader parentHeader;
    if (pPrimaryCurveId != NULL)
        *pPrimaryCurveId = 0;
    if (jmdlRIMSBS_getElementHeader (pContext, &parentHeader, curveId)
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
| Name:     jmdlRIMSBS_getChildCurve                                    |
|                                                                       |
| Author:   EarlinLutz                               09/09              |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRIMSBS_getCurveChainChild
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId  curveId,
int             childIndex,
RIMSBS_CurveId  *pChildId
)
    {
    RIMSBS_ElementHeader parentHeader;
    *pChildId = -1;
    if (jmdlRIMSBS_getElementHeader (pContext, &parentHeader, curveId)
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
| Name:     jmdlRIMSBS_createAlternateMSBsplineCurve                    |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      jmdlRIMSBS_createAlternateMSBsplineCurve
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId  parentId
)
    {
    RIMSBS_CurveId childId = RIMSBS_NULL_CURVE_ID;
    MSBsplineCurve *pCurve;
    RIMSBS_ElementHeader parentHeader;

    if (jmdlRIMSBS_getElementHeader (pContext, &parentHeader, parentId))
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
                    childId = jmdlRIMSBS_addMSBsplineCurve (pContext, -1, NULL, pCurve);
                    parentHeader.alternateCurveId = childId;
                    jmdlRIMSBS_setElementHeader (pContext, &parentHeader, parentId);
                    }
                }
                break;
            }
        }
    return childId;

    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addMSBsplineCurve                                |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Copy the curve structure (bitwise) into the context.  Caller must     |
| NOT free the curve.                                                   |
+----------------------------------------------------------------------*/
Public int      jmdlRIMSBS_addMSBsplineCurveDirect
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData,
MSBsplineCurveP  pCurve
)
    {
    static bool    s_forceOpenCurves = 1;
    if (s_forceOpenCurves && pCurve->params.closed)
        bspcurv_openCurve (pCurve, pCurve, 0.0);
    // This copies bits from the curve struct ...    
    int stat =jmdlRIMSBS_addElement (pContext,
                        RIMSBS_MSBsplineCurve, userInt, pUserData,
                        pCurve, sizeof (MSBsplineCurve));
    memset (pCurve, 0, sizeof (MSBsplineCurve));
    return stat;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addMSBsplineCurve                                |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Copy the curve structure (bitwise) into the context.  Caller must     |
| NOT free the curve.                                                   |
| Returned curveId may be RIMSBS_CurveChain or RIMSBS_MSBsplineCurve    |
+----------------------------------------------------------------------*/
Public int      jmdlRIMSBS_addMSBsplineCurve
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData,
MSBsplineCurveP  pCurve
)
    {
    static int s_normalizationStrategy = 1;    // 0==>do nothing (clearly bad), 1==>normalize fractions stored in partial curve, 2=>prenormalize all
    if (s_normalizationStrategy == 2)
        pCurve->NormalizeKnots ();
    
    DSegment1d knotMapper = pCurve->GetKnotRange ();
    RIMSBS_CurveId refCurveId = jmdlRIMSBS_addMSBsplineCurveDirect (pContext, 0, NULL, pCurve);
    RIMSBS_CurveId chainId = jmdlRIMSBS_addCurveChain (pContext, userInt, pUserData, refCurveId);
    // Hmm.. the curve has been taken away (and zeroed!!!)
    MSBsplineCurveP pSavedCurve = jmdlRIMSBS_getCurveP (pContext, refCurveId);

    /*MSBsplineCurveExtractionContext context;
    double a = 0.0;
    bvector<DPoint3d> poleXYZ;
    bvector<double> poleW;
    bool    isWeighted = pSavedCurve->rational;
    if (context.AttachToCurve (pSavedCurve))
        {
        while (context.GetNextBezier ())
            {
            MSBsplineCurve currCurve;
            if (context.CopyBezierAsMSBspline (currCurve))
                {
                RIMSBS_CurveId childCurveId = jmdlRIMSBS_addMSBsplineCurveDirect (pContext, 0, NULL, &currCurve);
                RIMSBS_CurveId curveIntervalId;
                if (jmdlRIMSBS_createSubcurveExt (pContext, &curveIntervalId, chainId, childCurveId, context.s0, context.s1))
                    jmdlRIMSBS_addCurveToCurveChain (pContext, chainId, curveIntervalId);
                }
            }
        }*/
    BCurveSegment segment;

    for (size_t spanIndex = 0; pSavedCurve->GetBezier (segment, spanIndex); spanIndex++)
        {
         if (!segment.IsNullU ())
            {
            MSBsplineCurve currCurve;
            if (SUCCESS == currCurve.InitFromDPoint4dArray (segment.GetPoleP (), pSavedCurve->params.order, pSavedCurve->params.order))
                {
                RIMSBS_CurveId childCurveId = jmdlRIMSBS_addMSBsplineCurveDirect (pContext, 0, NULL, &currCurve);
                RIMSBS_CurveId curveIntervalId;
                double s0 = segment.UMin ();
                double s1 = segment.UMax ();
                if (s_normalizationStrategy == 1)
                    {
                    double f0, f1;
                    if (knotMapper.PointToFraction (s0, f0) && knotMapper.PointToFraction (s1, f1))
                        {
                        s0 = f0;
                        s1 = f1;
                        }
                    }
                if (jmdlRIMSBS_createSubcurveExt (pContext, &curveIntervalId, chainId, childCurveId, s0, s1))
                    jmdlRIMSBS_addCurveToCurveChain (pContext, chainId, curveIntervalId);
                }
            }
        }
    return chainId;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_addBezier                                        |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
| Create an MSBsplineCurve for the given bezier.                        |
+----------------------------------------------------------------------*/
Public int      jmdlRIMSBS_addBezier
(
RIMSBS_Context  *pContext,
int             userInt,
void            *pUserData,
const DPoint4d  *pPoleArray,
int             order
)
    {
    MSBsplineCurve curve;
    if (SUCCESS == curve.InitFromDPoint4dArray (pPoleArray, order, order))
        {
        return jmdlRIMSBS_addMSBsplineCurveDirect (pContext, userInt, pUserData, &curve);
        }
    else
        return RIMSBS_NULL_CURVE_ID;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_createSubcurveExt                                |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_createSubcurveExt
(
RIMSBS_Context  *pContext,
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

    if  (jmdlRIMSBS_getElementHeader (pContext, &desc0, parentCurveId))
        {
        myResult = true;
        refData.parentId = parentCurveId;
        refData.s0 = s0;
        refData.s1 = s1;
        refData.partialCurveId = partialCurveId;
        newCurveId = jmdlRIMSBS_addElement (pContext,
                            RIMSBS_CurveInterval, 0, NULL,
                            &refData, sizeof (refData));
        jmdlRIMSBS_setGroupId (pContext, newCurveId, desc0.groupId);
        }

    *pNewCurveId = newCurveId;
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_createSubcurve                                   |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool        jmdlRIMSBS_createSubcurve
(
RIMSBS_Context  *pContext,
RIMSBS_CurveId  *pNewCurveId,
RIMSBS_CurveId  parentCurveId,
double          s0,
double          s1
)
    {
    return jmdlRIMSBS_createSubcurveExt (pContext, pNewCurveId, parentCurveId, -1, s0, s1);
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
| Name:     jmdlRIMSBS_consolidateCoincidentGeometry                    |
|                                                                       |
| Author:   EarlinLutz                               6/22/98            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRIMSBS_consolidateCoincidentGeometry
(
RIMSBS_Context  *pContext,
double tolerance
)
    {
    RIMSBS_ElementHeader header0, header1;
    int xyrIndex0, xyrIndex1, elementIndex0, elementIndex1;
    DEllipse3d *pEllipse0, *pEllipse1;
    DPoint3d xyr0;
    int numMatch = 0;
    int numTotalMatch = 0;
    EmbeddedIntArray *pXYRToElementIndexArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedDPoint3dArray *pXYRArray = jmdlEmbeddedDPoint3dArray_grab ();
    EmbeddedIntArray *pBlockedXYRIndexArray = jmdlEmbeddedIntArray_grab ();

    /* Free the type-specfic memor of each curve */
    for (int i0 = 0; jmdlRIMSBS_getElementHeader (pContext, &header0, i0); i0++)
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

        for (int i0 = 0; jmdlEmbeddedIntArray_getInt (pBlockedXYRIndexArray, &xyrIndex0, i0); i0++)
            {
            if (xyrIndex0 >= 0)
                {
                numMatch = 0;
                jmdlEmbeddedIntArray_getInt (pXYRToElementIndexArray, &elementIndex0, xyrIndex0);
                jmdlRIMSBS_getElementHeader (pContext, &header0, elementIndex0);
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
                    jmdlRIMSBS_getElementHeader (pContext, &header1, elementIndex1);
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
END_BENTLEY_GEOMETRY_NAMESPACE