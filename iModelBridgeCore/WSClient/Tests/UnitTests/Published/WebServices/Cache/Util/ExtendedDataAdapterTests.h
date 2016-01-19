/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Util/ExtendedDataAdapterTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../../../Utils/WebServicesTestsHelper.h"
#include "../../../Utils/SeedFile.h"

class ExtendedDataAdapterTests : public WSClientBaseTest, SeedFile
    {
    protected:
        virtual void SetupSeedFile(BeFileNameCR filePath) override;

    public:
        std::shared_ptr<ObservableECDb> GetTestECDb();
    };
