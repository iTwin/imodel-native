/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ExtendedDataAdapter.h>
#include "../Instances/ObjectInfoManager.h"
#include "../Instances/RelationshipInfoManager.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExtendedDataDelegate : ExtendedDataAdapter::IDelegate
    {
    private:
        IECDbAdapter& m_dbAdapter;
        ObjectInfoManager& m_objectInfoManager;
        RelationshipInfoManager& m_relationshipInfoManager;

    public:
        ExtendedDataDelegate(IECDbAdapterR dbAdapter, ObjectInfoManager& m_objectInfoManager, RelationshipInfoManager& m_relationshipInfoManager) :
            m_dbAdapter(dbAdapter),
            m_objectInfoManager(m_objectInfoManager),
            m_relationshipInfoManager(m_relationshipInfoManager)
            {}

        ECClassCP GetExtendedDataClass(ECInstanceKeyCR ownerKey) override;
        ECRelationshipClassCP GetExtendedDataRelationshipClass(ECInstanceKeyCR ownerKey) override;
        ECInstanceKey GetHolderKey(ECInstanceKeyCR ownerKey) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
