/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/Tests.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../ConverterInternal.h"
#include "V8FileEditor.h"
#include <Bentley/BeTest.h>

#define ASSERT_SUCCESS(val) ASSERT_TRUE (SUCCESS == val)
#define EXPECT_SUCCESS(val) EXPECT_TRUE (SUCCESS == val)

//=======================================================================================
// @bsiclass                                    Sam.Wilson                      04/15
//=======================================================================================
struct ConverterTestsHost : DgnViewLib::Host
{
protected:
    virtual void _SupplyProductName (BentleyApi::Utf8StringR name) override {name.assign("SampleDgnV8ConverterTests");}
    virtual ViewManager& _SupplyViewManager() override;
#if defined (NOT_NOW_PARASOLID)
    virtual SolidsKernelAdmin& _SupplySolidsKernelAdmin() override {return *new PSolidKernelAdmin();}
#endif
    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
    virtual NotificationAdmin& _SupplyNotificationAdmin() override;
    virtual BentleyApi::BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;
};

//=======================================================================================
// @bsiclass                                    Sam.Wilson                      04/15
//=======================================================================================
struct SyncInfoReader
    {
    DgnDbPtr m_dgndb;
    BentleyApi::BeSQLite::Db m_syncInfo;
    Converter::Params* m_params;

    SyncInfoReader(Converter::Params&);
    void AttachToDgnDb(BentleyApi::BeFileNameCR);
    void MustFindFileByName(RepositoryLinkId&, BentleyApi::BeFileNameCR v8FileName, int expectedCount=1);
    void MustFindModelByV8ModelId(iModelExternalSourceAspectID&, RepositoryLinkId, DgnV8Api::ModelId, int expectedCount=1);
    void MustFindElementByV8ElementId(DgnElementId&, iModelExternalSourceAspectID, DgnV8Api::ElementId, int expectedCount=1);
    };

