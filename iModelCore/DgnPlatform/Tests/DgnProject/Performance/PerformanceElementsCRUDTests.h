/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceElementsCRUDTests.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_DPTEST

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElementsCRUDTestFixture : public PerfTestFixture
    {

    protected:
        static const int s_initialInstanceCount = 1000000;
        static const int s_opCount = 50000;
        static const uint64_t s_firstElementId = UINT64_C(1099511627785);

        PerformanceElementsCRUDTestFixture(){}


        void SetUpTestDgnDb(WCharCP destFileName, Utf8CP testClassName, int initialInstanceCount);
        void CreateElements(int numInstances, Utf8CP className, bvector<DgnElementPtr>& elements, Utf8CP modelName) const;
        void CreateElementsAndInsert(int numInstances, Utf8CP className, Utf8CP modelName) const;
        DgnDbStatus SetPerfElementPropertyValues(DgnElementPtr element, bool update = false) const;
        DgnDbStatus SetPerfElementSub1PropertyValues(DgnElementPtr element, bool update = false) const;
        DgnDbStatus SetPerfElementSub2PropertyValues(DgnElementPtr element, bool update = false) const;
        DgnDbStatus SetPerfElementSub3PropertyValues(DgnElementPtr element, bool update = false) const;
        DgnDbStatus SetPropertyValues(Utf8CP className, DgnElementPtr element, bool update = false) const;
        std::function<DgnDbStatus(Dgn::PhysicalElementPtr& element)> SetPropertyValuesMethod(Utf8CP className, bool update) const;

        Dgn::PhysicalElementPtr CreatePerfElement(Utf8CP className, DgnModelR targetModel, DgnCategoryId catId, DgnElementId parent = DgnElementId(), DgnClassId dgnClassId = DgnClassId()) const;
        std::function<PhysicalElementPtr(void)> CreatePerfElementMethod(Utf8CP className, DgnModelR targetModel, DgnCategoryId catId, DgnElementId parent = DgnElementId(), DgnClassId dgnClassId = DgnClassId()) const;
        static DgnElementId generateTimeBasedId(int counter);
        static DgnElementId generateAlternatingBriefcaseId(int counter);

        DgnDbStatus VerifyPerfElementSelectParams(DgnElementCR element);
        DgnDbStatus VerifyPerfElementSub1SelectParams(DgnElementCR element);
        DgnDbStatus VerifyPerfElementSub2SelectParams(DgnElementCR element);
        DgnDbStatus VerifyPerfElementSub3SelectParams(DgnElementCR element);
        DgnDbStatus GetPropertyValues(DgnElementCR element, Utf8CP className);

        static int DetermineElementIdIncrement(int initialInstanceCount, int opCount) { return initialInstanceCount / opCount; }

        void ApiInsertTime(Utf8CP className, int initialInstanceCount1 = s_initialInstanceCount, int opCount = s_opCount, bool setFederationGuid = false, int idStrategy = 0);
        void ApiSelectTime(Utf8CP className, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);
        void ApiUpdateTime(Utf8CP className, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);
        void ApiDeleteTime(Utf8CP className, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);
        
        void AddGeometry(DgnElementPtr element) const;
        void ExtendGeometry(DgnElementPtr element) const;

        void LogTiming(StopWatch& timer, Utf8CP description, Utf8CP testClassName, bool omitClassIdFilter, int initialInstanceCount, int opCount) const;
        int  GetfirstElementId(Utf8CP className);

    };
