/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECInstanceChangeEvents.h>
#include <DgnPlatform/TxnManager.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
//! DgnDb-based ECInstanceChange event source which provides ECInstanceChange events for
//! dgndb elements and models.
// @bsiclass
//=======================================================================================
struct DgnDbECInstanceChangeEventSource : ECInstanceChangeEventSource, TxnMonitor
{
private:
    bvector<ECInstanceChangeEventSource::ChangedECInstance> m_changes;

protected:
    void _OnCommit(TxnManager&) override;
    void _OnCommitted(TxnManager&) override;
    void _OnAppliedChanges(TxnManager&) override;
    void _OnClassUsed(ECDbCR, ECClassCR, bool polymorphically) override;

public:
    DgnDbECInstanceChangeEventSource();
    ~DgnDbECInstanceChangeEventSource();
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
