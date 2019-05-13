/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DataCaptureSchema/DataCaptureSchemaApi.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnMarkupProject.h>
#include <DgnPlatform/GenericDomain.h>


#define BEGIN_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE  BEGIN_BENTLEY_DATACAPTURE_NAMESPACE namespace Tests {
#define END_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE    } END_BENTLEY_DATACAPTURE_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DATACAPTURE_UNITTESTS  using namespace BENTLEY_NAMESPACE_NAME::DataCapture::Tests;

BEGIN_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE

struct DataCaptureProjectHostImpl;

//=======================================================================================
//! A DgnPlatformLib host that can be used with "Published" tests
//=======================================================================================
struct DataCaptureProjectHost
{
friend struct DataCaptureTestsFixture;

private:
    DataCaptureProjectHostImpl* m_pimpl;

    void CleanOutputDirectory();
    Dgn::DgnDbStatus ImportDataCaptureSchema(Dgn::DgnDbR dgndb);



public:
    DataCaptureProjectHost();
    ~DataCaptureProjectHost();

    BeFileName GetDocumentsDirectory();
    BeFileName GetOutputDirectory();
    BeFileName GetDgnPlatformAssetsDirectory();
    BeFileName BuildProjectFileName(WCharCP);
    Dgn::DgnDbPtr CreateProject(WCharCP);
    Dgn::DgnDbPtr OpenProject(WCharCP);
    Dgn::DgnModelPtr CreateSpatialModel(Dgn::DgnDbR dgndb, Utf8CP name);
    Dgn::DgnCategoryId CreatePhysicalCategory(Dgn::DgnDbR dgndb, Utf8CP code);
    void CreateDefaultView(Dgn::DgnModelR baseModel);

};

END_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DATACAPTURE
USING_NAMESPACE_BENTLEY_DATACAPTURE_UNITTESTS
