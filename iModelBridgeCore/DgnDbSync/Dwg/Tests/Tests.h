/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Tests/Tests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#define DWGTOOLKIT_RealDwg 1

#include "../DwgImportInternal.h"

//=======================================================================================
// @bsiclass                                    Sam.Wilson                      04/15
//=======================================================================================
struct ImporterTestsHost : DgnPlatformLib::Host
{
protected:
    virtual void _SupplyProductName (BentleyApi::Utf8StringR name) override {name.assign("DwgImporterTests");}
    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
    //virtual NotificationAdmin& _SupplyNotificationAdmin() override;
    virtual BentleyApi::BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;
};

//=======================================================================================
// @bsiclass                                    Sam.Wilson                      04/15
//=======================================================================================
//struct SyncInfoReader
//    {
//    DgnDbPtr m_dgndb;
//    BentleyApi::BeSQLite::Db m_syncInfo;
//
//    SyncInfoReader();
//    void AttachToDgnDb(BentleyApi::BeFileNameCR);
//    void MustFindFileByName(SyncInfo::V8FileId&, BentleyApi::BeFileNameCR v8FileName, int expectedCount=1);
//    void MustFindModelByV8ModelId(SyncInfo::V8FileId, DgnV8Api::ModelId, int expectedCount=1);
//    void MustFindElementByV8ElementId(DgnElementId&, SyncInfo::V8FileId, DgnV8Api::ElementId, int expectedCount=1);
//    };

