/*--------------------------------------------------------------------------------------+
|
|     $Source: docs/samplecode/TxnManager.sample.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ TxnManager_TxnMonitor_ElementsByClass_Includes.sampleCode
// Primary header file for the DgnPlatform API 
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/TxnManager.h>

// helper macro for using the BeSQLite namespace
USING_NAMESPACE_BENTLEY_SQLITE

// helper macro for using the DgnPlatform namespace
USING_NAMESPACE_BENTLEY_DGN
//__PUBLISH_EXTRACT_END__

//__PUBLISH_EXTRACT_START__ TxnManager_TxnMonitor_ElementsByClass.sampleCode
/*=================================================================================**//**
* @bsiclass                                                     BentleySystems
+===============+===============+===============+===============+===============+======*/
struct TestTxnMonitor : TxnMonitor
    {
    void _OnCommit(TxnManager&) override;
    void _OnReversedChanges(TxnManager&) override {;}
    void CheckClass(TxnManager&, DgnClassId);
    void CheckClassOrSubClasses(TxnManager&, DgnClassId);
    };

//---------------------------------------------------------------------------------------
// Detect deletions of elements of one or more particular classes. 
// This query is very fast, as the element changes table is indexed on ECClassId
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
void TestTxnMonitor::_OnCommit(TxnManager& txnMgr)
    {
    DgnClassId classOfInterest = DgnClassId(txnMgr.GetDgnDb().Schemas().GetECClassId("namespace", "classname")); // TODO: name of class here

    CheckClass(txnMgr, classOfInterest);
    CheckClassOrSubClasses(txnMgr, classOfInterest);
    }

//---------------------------------------------------------------------------------------
// Detect deletions of elements of one or more particular classes. 
// This query is very fast, as the element changes table is indexed on ECClassId
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
void TestTxnMonitor::CheckClass(TxnManager& txnMgr, DgnClassId classOfInterest)
    {
    auto elements = txnMgr.Elements().MakeIterator();
    elements.Params().SetWhere("ECClassId = ? ");		// Note: Use an IN clause if you want to check for multiple classes at once
    auto statement = elements.GetStatement();
    statement->BindId(1, classOfInterest);

    for (auto element : elements)
        {
        if (TxnTable::ChangeType::Delete == element.GetChangeType())
            {
            DgnElementId eid = element.GetElementId();

            // TODO: do something here
            }
        }
    }

//---------------------------------------------------------------------------------------
// Detect deletions of elements of a particular class or any of its subclasses
// This query must examine each element in the changeset, and so running time is 
// proportional to the number of changed elements.
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
void TestTxnMonitor::CheckClassOrSubClasses(TxnManager& txnMgr, DgnClassId classOfInterest)
    {
    ECN::ECClassCP targetClass = txnMgr.GetDgnDb().Schemas().GetECClass(classOfInterest);

    auto elements = txnMgr.Elements().MakeIterator();

    for (auto element : elements)
        {
        if (TxnTable::ChangeType::Delete == element.GetChangeType())
            {
            DgnElementId eid = element.GetElementId();
            DgnClassId ecclsid = element.GetECClassId();

            ECN::ECClassCP ecclass = txnMgr.GetDgnDb().Schemas().GetECClass(ECN::ECClassId(ecclsid.GetValue()));
            if (ecclass->Is(targetClass))
                {
                // TODO: do something here
                }
            }
        }
    }
//__PUBLISH_EXTRACT_END__
