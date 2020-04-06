/*--------------------------------------------------------------------------------------+
|
|     $Source: Presentation/DgnDbECInstanceChangeEventSource.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/RulesDriven/ECInstanceChangeEvents.h>
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
// @bsiclass                                    Grigas.Petraitis                01/2016
//=======================================================================================
struct DgnDbECInstanceChangeEventSource : ECInstanceChangeEventSource, TxnMonitor
{
private:
    bvector<ECInstanceChangeEventSource::ChangedECInstance> m_changes;

private:
    static bset<ChangedECInstance> FindChanges(bvector<ChangedECInstance> const& changes, bset<ECInstanceKey> const& keys);
    void FillWithRelationshipChanges(bvector<ChangedECInstance>&, TxnManager&) const;

protected:
    void _OnCommit(TxnManager&) override;
    void _OnCommitted(TxnManager&) override;
    void _OnClassUsed(ECDbCR, ECClassCR, bool polymorphically) override;

public:
    DgnDbECInstanceChangeEventSource();
    ~DgnDbECInstanceChangeEventSource();
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
