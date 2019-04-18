/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/ConflictsInfo.h>

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_DGN

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Algirdas.Mikoliunas              04/18
+---------------+---------------+---------------+---------------+---------------+------*/
void ConflictsInfo::AddFromResponse(Dgn::IBriefcaseManager::Response response)
    {
    for (auto codeItem : response.CodeStates())
        m_codeStates.insert(codeItem);

    for (auto lockItem : response.LockStates())
        m_lockStates.insert(lockItem);
    }
