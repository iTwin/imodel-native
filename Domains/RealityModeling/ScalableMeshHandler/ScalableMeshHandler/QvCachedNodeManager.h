/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshHandler/ScalableMeshHandler/QvCachedNodeManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <ScalableMeshHandler/ScalableMeshModel.h>
#include <list>

BEGIN_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE
class QvCachedNodeManager
    {
    private:

        size_t m_maxNbPoints;
        size_t m_totalNbPoints;

        struct QvCachedNode
            {
            QvCachedNode(__int64 nodeId, QvElem* qvElem, PhysicalModel* modelRef, size_t nbPoints)
                {
                m_nodeId = nodeId;
                m_modelRef = modelRef;
                m_qvElem = qvElem;
                m_nbPoints = nbPoints;
                }

            __int64     m_nodeId;
            PhysicalModel* m_modelRef;
            QvElem*     m_qvElem;
            size_t      m_nbPoints;
            };

        typedef std::list<QvCachedNode> CachedNodesList;
        static CachedNodesList m_cachedNodes;

        QvCachedNodeManager();

    public:

        void AddCachedNode(__int64 nodeId, QvElem* qvElem, PhysicalModel* modelRef, size_t nbPoints);

        void ClearCachedNodes(PhysicalModel* modelRef);

        QvElem* FindQvElem(__int64 nodeId, PhysicalModel* modelRef);

        static QvCachedNodeManager& GetManager();
    };
END_BENTLEY_SCALABLE_MESH_MODEL_NAMESPACE