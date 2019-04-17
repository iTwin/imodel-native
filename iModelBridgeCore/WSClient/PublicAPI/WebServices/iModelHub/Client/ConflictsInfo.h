/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <DgnPlatform/DgnCodesManager.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

struct ConflictsInfo;
typedef std::shared_ptr<ConflictsInfo> ConflictsInfoPtr;

//=======================================================================================
//! This class is used to retrieve codes/locks conflict errors that occured during push
// @bsiclass                                      Algirdas.Mikoliunas            04/2018
//=======================================================================================
struct ConflictsInfo
{
friend struct iModelConnection;

private:
    Dgn::DgnCodeInfoSet      m_codeStates;
    Dgn::DgnLockInfoSet      m_lockStates;

    //! Adds codes/locks conflicts from response to current object.
    void AddFromResponse(Dgn::IBriefcaseManager::Response response);

public:
    //! Get codes conflicts.
    //! @return Returns codes conflicts
    Dgn::DgnCodeInfoSet const&  GetCodesConflicts() { return m_codeStates; }

    //! Get locks conflicts.
    //! @return Returns locks conflicts
    Dgn::DgnLockInfoSet const&  GetLocksConflicts() { return m_lockStates; }

    //! Gets if current object contains any conflicts.
    //! @return Returns if any conflicts exists
    bool Any() { return m_codeStates.size() > 0 || m_lockStates.size() > 0; }
};

END_BENTLEY_IMODELHUB_NAMESPACE
