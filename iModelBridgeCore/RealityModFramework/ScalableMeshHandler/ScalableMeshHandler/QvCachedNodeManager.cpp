#include "QvCachedNodeManager.h"
BEGIN_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE
QvCachedNodeManager::CachedNodesList QvCachedNodeManager::m_cachedNodes;

QvCachedNodeManager::QvCachedNodeManager()
    {
    m_maxNbPoints = 3000000;
    m_totalNbPoints = 0;
    }

void QvCachedNodeManager::AddCachedNode(__int64 nodeId, QvElem* qvElem, PhysicalModel* modelRef, size_t nbPoints)
    {
    assert(qvElem != 0);

    while (m_totalNbPoints + nbPoints > m_maxNbPoints)
        {
        assert(m_totalNbPoints >= m_cachedNodes.back().m_nbPoints);
        m_totalNbPoints -= m_cachedNodes.back().m_nbPoints;
        T_HOST.GetGraphicsAdmin()._DeleteQvElem(m_cachedNodes.back().m_qvElem);
        m_cachedNodes.pop_back();
        }

    m_cachedNodes.push_front(QvCachedNode(nodeId, qvElem, modelRef, nbPoints));

    m_totalNbPoints += nbPoints;
    }

void QvCachedNodeManager::ClearCachedNodes(PhysicalModel* modelRef)
    {
    auto cachedNodeIter(m_cachedNodes.begin());
    auto cachedNodeIterEnd(m_cachedNodes.end());

    while (cachedNodeIter != cachedNodeIterEnd)
        {
        if (cachedNodeIter->m_modelRef == modelRef)
            {
            cachedNodeIter = m_cachedNodes.erase(cachedNodeIter);
            }
        else
            {
            cachedNodeIter++;
            }
        }

    m_totalNbPoints = 0;
    }

QvElem* QvCachedNodeManager::FindQvElem(__int64 nodeId, PhysicalModel* modelRef)
    {
    auto cachedNodeIter(m_cachedNodes.begin());
    auto cachedNodeIterEnd(m_cachedNodes.end());

    QvElem* foundElem = 0;

    while (cachedNodeIter != cachedNodeIterEnd)
        {
        if ((cachedNodeIter->m_modelRef == modelRef) &&
            (cachedNodeIter->m_nodeId == nodeId))
            {
            QvCachedNode qvCachedNode(*cachedNodeIter);
            foundElem = cachedNodeIter->m_qvElem;
            m_cachedNodes.erase(cachedNodeIter);
            m_cachedNodes.push_front(qvCachedNode);
            break;
            }

        cachedNodeIter++;
        }

    return foundElem;
    }

QvCachedNodeManager& QvCachedNodeManager::GetManager()
    {
    static QvCachedNodeManager* s_manager = 0;

    if (s_manager == 0)
        {
        s_manager = new QvCachedNodeManager();
        }

    return *s_manager;
    }
END_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE