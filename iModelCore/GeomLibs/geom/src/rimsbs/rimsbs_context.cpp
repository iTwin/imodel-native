/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/rimsbs/rimsbs_context.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// UNUSED static int s_globalNoisy = 0;


RIMSBS_Context::~RIMSBS_Context () {}
RIMSBS_Context::RIMSBS_Context () {}

RIMSBS_CurveChainStruct::RIMSBS_CurveChainStruct() :m_primaryCurveId (0) {}
RIMSBS_CurveChainStruct::RIMSBS_CurveChainStruct (RIMSBS_CurveId primaryCurveId)
    : m_primaryCurveId (primaryCurveId)
    {
    }


RIMSBS_ElementHeader::RIMSBS_ElementHeader () :
    pUserData (nullptr),
    m_chainData(RIMSBS_NULL_CURVE_ID)
    {
    }

RIMSBS_ElementHeader ::RIMSBS_ElementHeader
(
ICurvePrimitivePtr const &_curve,
int _type,
int _userInt,
void *_pUserData,
int _groupId,
int _alternateCurveId,
int64_t _userInt64
)
    :
    curve (_curve),
    type (_type),
    userInt (_userInt),
    pUserData (_pUserData),
    groupId (_groupId),
    alternateCurveId (_alternateCurveId),
    userInt64 (_userInt64)
    {

    }

RIMSBS_CurveIntervalStruct::RIMSBS_CurveIntervalStruct (int _parentId, double _s0, double _s1, int _partialCurveId)
    :
    parentId (_parentId),
    s0 (_s0),
    s1 (_s1),
    partialCurveId (_partialCurveId)
    {
    }

RIMSBS_CurveIntervalStruct::RIMSBS_CurveIntervalStruct ()
    :
    parentId (RIMSBS_NULL_CURVE_ID),
    s0 (0.0),
    s1 (1.0),
    partialCurveId (RIMSBS_NULL_CURVE_ID)
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
    // Scope exit for onTheSpot bvector forces memory release in curve and curveChainStruct ..
    bvector<RIMSBS_ElementHeader>().swap (pContext->m_geometry);
    }

int RIMSBS_Context::AddArc (DEllipse3dCR arc, int userInt, void *pUserData)
    {
    m_geometry.push_back (
        RIMSBS_ElementHeader
            (
            ICurvePrimitive::CreateArc (arc),
            RIMSBS_DEllipse3d,
            userInt,
            pUserData,
            currGroupId));
    return (int)m_geometry.size () - 1;
    }

int RIMSBS_Context::AddDataCarrier (int userInt, void *pUserData)
    {
    m_geometry.push_back (
        RIMSBS_ElementHeader
            (
            nullptr,
            RIMSBS_DataCarrier,
            userInt,
            pUserData,
            currGroupId
            ));
    return (int)m_geometry.size () - 1;
    }

int RIMSBS_Context::AddCurveChain (int primaryCurveId, int userInt, void *pUserData)
    {
    m_geometry.push_back (
        RIMSBS_ElementHeader
            (
            nullptr,
            RIMSBS_CurveChain,
            userInt,
            pUserData,
            currGroupId));
    m_geometry.back ().m_chainData.m_primaryCurveId = primaryCurveId;
    return (int)m_geometry.size () - 1;
    }

int RIMSBS_Context::AddCurve (ICurvePrimitivePtr &curve, int userInt, void *pUserData)
    {
    int rimsbsCurveType = RIMSBS_NoGeometry;
    switch (curve->GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            rimsbsCurveType = RIMSBS_MSBsplineCurve;
            break;
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            rimsbsCurveType = RIMSBS_DEllipse3d;
            break;
        default:
            return -1;      // unsupported curve type . . .
        }
    m_geometry.push_back (
        RIMSBS_ElementHeader
            (
            curve,
            rimsbsCurveType,
            userInt,
            pUserData,
            currGroupId
            ));
    return (int)m_geometry.size () - 1;
    }

void RIMSBS_Context::SetCurrentGroupId (int data)
    {
    currGroupId = data;
    }


int RIMSBS_Context::AddCurveInterval (RIMSBS_CurveIntervalStruct const &data, int userInt, void *pUserData)
    {
    m_geometry.push_back (RIMSBS_ElementHeader (nullptr, RIMSBS_CurveInterval, userInt, pUserData, currGroupId));
    m_geometry.back ().m_partialCurve = data;
    return (int)m_geometry.size () - 1;
    }

MSBsplineCurveCP RIMSBS_Context::GetMSBsplineCurveCP (int curveIndex)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ()
            && m_geometry[index].type == RIMSBS_MSBsplineCurve
            && m_geometry[index].curve.IsValid ()
            )
            {
            return m_geometry[index].curve->GetBsplineCurveCP ();
            }
        }
    return nullptr;
    }


bool RIMSBS_Context::IsCurveChain (int curveIndex, int &primaryCurveId)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ()
            && m_geometry[index].type == RIMSBS_CurveChain
            )
            {
            primaryCurveId = m_geometry[index].m_chainData.m_primaryCurveId;
            return true;
            }
        }
    return false;
    }

bool RIMSBS_Context::TryGetCurveChainChild (int curveIndex, int childIndex, int &childId)
    {
    RIMSBS_ElementHeader parentHeader;
    childId = -1;
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ()
            && m_geometry[index].type == RIMSBS_CurveChain
            && childIndex >= 0
            )
            {
            size_t ci = (size_t)childIndex;
            if (ci < m_geometry[index].m_chainData.size ())
                {
                childId = m_geometry[index].m_chainData[ci];
                return true;
                }
            }
        }
    return false;
    }

bool RIMSBS_Context::TryAddCurveToCurveChain (int curveIndex, int childCurveIndex)
    {
    if (IsValidCurveIndex (curveIndex))
        {
        auto &desc = GetElementR (curveIndex);
        if (desc.type == RIMSBS_CurveChain)
            {
            desc.m_chainData.push_back ((size_t)childCurveIndex);
            return true;
            }
        }
    return false;
    }


void *RIMSBS_Context::GetUserDataP (int curveIndex)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            return m_geometry[index].pUserData;
        }
    return nullptr;
    }

ICurvePrimitiveCP RIMSBS_Context::GetICurvePrimitiveCP (int curveIndex)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            return m_geometry[index].curve.get ();
        }
    return nullptr;
    }


int RIMSBS_Context::GetUserInt (int curveIndex)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            return m_geometry[index].userInt;
        }
    return 0;
    }


bool RIMSBS_Context::IsValidCurveIndex (int curveIndex)
    {
    return 0 <= curveIndex && (size_t)curveIndex < m_geometry.size ();
    }

RIMSBS_ElementHeader &RIMSBS_Context::GetElementR (int curveId)
    {
    return m_geometry[(size_t)curveId];
    }

bool RIMSBS_Context::TryGetImmediateParent (int curveIndex, int &parentIndex)
    {
    parentIndex = curveIndex;
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            {
            parentIndex = m_geometry.at (index).m_partialCurve.parentId;
            return true;
            }
        }
    return false;
    }

bool RIMSBS_Context::TryGetGroupId (int curveIndex, int &data)
    {
    data = -1;
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            {
            data = m_geometry.at (index).groupId;
            return true;
            }
        }
    return false;
    }

bool RIMSBS_Context::TryGetArc (int curveIndex, DEllipse3dR arc)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            {
            if (m_geometry[index].type == RIMSBS_DEllipse3d)
                return m_geometry.at (index).curve->TryGetArc (arc);
            }
        }
    return false;
    }

bool RIMSBS_Context::TrySetArc (int curveIndex, DEllipse3dCR arc)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            {
            m_geometry.at (index).curve = ICurvePrimitive::CreateArc (arc);
            return true;
            }
        }
    return false;
    }

bool RIMSBS_Context::TryGetUserInt64 (int curveIndex, int64_t &data)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            {
            data = m_geometry.at (index).userInt64;
            return true;
            }
        }
    return false;
    }


bool RIMSBS_Context::TrySetUserInt (int curveIndex, int data)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            {
            m_geometry.at (index).userInt = data;
            return true;
            }
        }
    return false;
    }

bool RIMSBS_Context::TrySetUserInt64 (int curveIndex, int64_t data)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            {
            m_geometry.at (index).userInt64 = data;
            return true;
            }
        }
    return false;
    }

bool RIMSBS_Context::TrySetUserPointer (int curveIndex, void* data)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            {
            m_geometry.at (index).pUserData = data;
            return true;
            }
        }
    return false;
    }

bool RIMSBS_Context::TrySetAlternateCurveId (int curveIndex, int data)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            {
            m_geometry.at (index).alternateCurveId = data;
            return true;
            }
        }
    return false;
    }

bool RIMSBS_Context::TrySetGroupId (int curveIndex, int data)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            {
            m_geometry.at (index).groupId = data;
            return true;
            }
        }
    return false;
    }

int RIMSBS_Context::GetGeometryType (int curveIndex)
    {
    if (0 <= curveIndex)
        {
        size_t index = (size_t)curveIndex;
        if (index < m_geometry.size ())
            {
            return m_geometry.at (index).type;
            }
        }
    return RIMSBS_NoGeometry;
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
    return const_cast <MSBsplineCurveP> (pContext->GetMSBsplineCurveCP (index));
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
    if (pContext->IsValidCurveIndex (index))
        {
        *pUserData      = pContext->GetUserDataP (index);
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
    if (pContext->IsValidCurveIndex (index))
        {
        *pUserInt      = pContext->GetUserInt (index);
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
    return pContext->TryGetUserInt64 (index, *pUserInt64);
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
    return pContext->TrySetUserInt (index, userInt);
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
    return pContext->TrySetUserInt64(index, userInt64);
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
    return pContext->TrySetUserPointer (index, pUserData);
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
    return pContext->TrySetGroupId (index, groupId);
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
    return pContext->TryGetGroupId (curveId, *pGroupId);
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
    pContext->SetCurrentGroupId (groupId);
    return  true;
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
    return pContext->TryGetImmediateParent (index, *pParentIndex);
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
        RG_CurveId fullEllipseCurveId = pContext->AddArc (*pEllipse, userInt, nullptr);
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
        return pContext->AddArc (*pEllipse, userInt, pUserData);
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
    return  pContext->AddDataCarrier (userInt, pUserData);
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
    return pContext->AddCurveChain (primaryCurveId, userInt, pUserData);
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
    return pContext->TryAddCurveToCurveChain (chainId, childCurveId);
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
    return pContext->IsCurveChain (curveId, *pPrimaryCurveId);
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
    return pContext->TryGetCurveChainChild (curveId, childIndex, *pChildId);
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
    DEllipse3d arc;
    int childId = RIMSBS_NULL_CURVE_ID;
    MSBsplineCurve bcurve;
    if (pContext->TryGetArc (parentId, arc)
        && SUCCESS == bspconv_convertDEllipse3dToCurve (&bcurve, &arc))
        {
        // This takes responsitility for pointers from the curve.
        childId = jmdlRIMSBS_addMSBsplineCurve (pContext, -1, NULL, &bcurve);
        pContext->TrySetAlternateCurveId (parentId, childId);
        return true;
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
    auto cp = ICurvePrimitive::CreateBsplineCurveSwapFromSource (*pCurve);
    return pContext->AddCurve (cp, userInt, pUserData);
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
    RIMSBS_CurveId  newCurveId = -1;
    int groupId;
    if (pContext->TryGetGroupId (parentCurveId, groupId))
        {
        newCurveId = pContext->AddCurveInterval (
                    RIMSBS_CurveIntervalStruct (parentCurveId, s0, s1, partialCurveId),
                    0, nullptr);
        pContext->TrySetGroupId (newCurveId, groupId);
        *pNewCurveId = newCurveId;
        return true;
        }
    return false;
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
            pXYR->Init ( pEllipse->center.x, pEllipse->center.y, r0);
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
    if (pEllipse->vector0.CrossProductXY (pEllipse->vector90) >= 0.0)
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
        pEllipse1->vector0.Scale (radialScale);
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
    int xyrIndex0, xyrIndex1, elementIndex0, elementIndex1;
    int numMatch = 0;
    int numTotalMatch = 0;
    EmbeddedIntArray *pXYRToElementIndexArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedDPoint3dArray *pXYRArray = jmdlEmbeddedDPoint3dArray_grab ();
    EmbeddedIntArray *pBlockedXYRIndexArray = jmdlEmbeddedIntArray_grab ();

    /* Free the type-specfic memor of each curve */
    for (int i0 = 0; pContext->IsValidCurveIndex (i0); i0++)
        {
        DEllipse3d arc;
        pContext->TryGetArc (i0, arc);
        DPoint3d xyr0;
        if (circularParts (&arc, &xyr0, tolerance))
            {
            jmdlEmbeddedDPoint3dArray_addDPoint3d (pXYRArray, &xyr0);
            jmdlEmbeddedIntArray_addInt (pXYRToElementIndexArray, i0);
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
                DPoint3d xyr0;
                DEllipse3d arc0, arc1;
                pContext->TryGetArc (elementIndex0, arc0);
                forceEllipseToExactCircular (&arc0);
                circularParts (&arc0, &xyr0, tolerance);
                // apply the first arc coordinates bit-by-bit to all declared duplicates
                for (
                    i0++;
                    jmdlEmbeddedIntArray_getInt (pBlockedXYRIndexArray, &xyrIndex1, i0)
                        && xyrIndex1 >= 0;
                    i0++
                    )
                    {
                    if (numMatch == 0)
                        pContext->TrySetArc (elementIndex0, arc0);
                    jmdlEmbeddedIntArray_getInt (pXYRToElementIndexArray, &elementIndex1, xyrIndex1);
                    pContext->TryGetArc (elementIndex1, arc1);
                    if (mergeCircularParts (&arc1, &xyr0, tolerance))
                        {
                        pContext->TrySetArc (elementIndex1, arc1);
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