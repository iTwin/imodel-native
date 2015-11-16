/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceElementsCRUDTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECDbApi.h>
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE

#define ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME    "TestSchema"
#define ELEMENT_PERFORMANCE_ELEMENT1_CLASS      "Element1"
#define ELEMENT_PERFORMANCE_ELEMENT2_CLASS      "Element2"
#define ELEMENT_PERFORMANCE_ELEMENT3_CLASS      "Element3"
#define ELEMENT_PERFORMANCE_ELEMENT4_CLASS      "Element4"

struct PerformanceElement1;
struct PerformanceElement2;
struct PerformanceElement3;
struct PerformanceElement4;

DEFINE_REF_COUNTED_PTR (PerformanceElement1)
DEFINE_REF_COUNTED_PTR (PerformanceElement2)
DEFINE_REF_COUNTED_PTR (PerformanceElement3)
DEFINE_REF_COUNTED_PTR (PerformanceElement4)

struct PerformanceElement1 : Dgn::PhysicalElement
    {
    friend struct PerformanceElement1Handler;
    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT1_CLASS, Dgn::PhysicalElement);

    private:
        static void GetParams (ECSqlClassParams& params);
        DgnDbStatus BindParams (BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop1_1;
        int64_t m_prop1_2;
        double m_prop1_3;

    protected:
        PerformanceElement1 (CreateParams const& params) : T_Super (params) {}
        PerformanceElement1 (CreateParams const& params, Utf8CP prop1_1, int64_t prop1_2, double prop1_3) : T_Super (params), m_prop1_1 (prop1_1), m_prop1_2 (prop1_2), m_prop1_3 (prop1_3) {}

        virtual Dgn::DgnDbStatus _BindInsertParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _ExtractSelectParams (BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;

    public:
        static PerformanceElement1Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues);

        PerformanceElement1CPtr Insert ();
        PerformanceElement1CPtr Update ();
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                    Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement2 : PerformanceElement1
    {
    friend struct PerformanceElement2Handler;
    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT2_CLASS, PerformanceElement1);

    private:
        static void GetParams (ECSqlClassParams& params);
        DgnDbStatus BindParams (BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop2_1;
        int64_t m_prop2_2;
        double m_prop2_3;

    protected:
        PerformanceElement2 (CreateParams const& params) : T_Super (params) {}
        PerformanceElement2 (CreateParams const& params,
                             Utf8CP prop1_1, int64_t prop1_2, double prop1_3,
                             Utf8CP prop2_1, int64_t prop2_2, double prop2_3) : T_Super (params, prop1_1, prop1_2, prop1_3), m_prop2_1 (prop2_1), m_prop2_2 (prop2_2), m_prop2_3 (prop2_3)
            {
            }


        virtual Dgn::DgnDbStatus _BindInsertParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _ExtractSelectParams (BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;

    public:
        static PerformanceElement2Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues);

        PerformanceElement2CPtr Insert ();
        PerformanceElement2CPtr Update ();
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                    Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement3 : PerformanceElement2
    {
    friend struct PerformanceElement3Handler;
    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT3_CLASS, PerformanceElement2);

    private:
        static void GetParams (ECSqlClassParams& params);
        DgnDbStatus BindParams (BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop3_1;
        int64_t m_prop3_2;
        double m_prop3_3;

    protected:
        PerformanceElement3 (CreateParams const& params) : T_Super (params) {}
        PerformanceElement3 (CreateParams const& params,
                             Utf8CP prop1_1, int64_t prop1_2, double prop1_3,
                             Utf8CP prop2_1, int64_t prop2_2, double prop2_3,
                             Utf8CP prop3_1, int64_t prop3_2, double prop3_3) : T_Super (params, prop1_1, prop1_2, prop1_3, prop2_1, prop2_2, prop2_3), m_prop3_1 (prop3_1), m_prop3_2 (prop3_2), m_prop3_3 (prop3_3)
            {
            }

        virtual Dgn::DgnDbStatus _BindInsertParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _ExtractSelectParams (BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;

    public:
        static PerformanceElement3Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues);

        PerformanceElement3CPtr Insert ();
        PerformanceElement3CPtr Update ();
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                    Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement4 : PerformanceElement3
    {
    friend struct PerformanceElement4Handler;
    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT4_CLASS, PerformanceElement3);

    private:
        static void GetParams (ECSqlClassParams& params);
        DgnDbStatus BindParams (BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop4_1;
        int64_t m_prop4_2;
        double m_prop4_3;

    protected:
        PerformanceElement4 (CreateParams const& params) : T_Super (params) {}
        PerformanceElement4 (CreateParams const& params,
                             Utf8CP prop1_1, int64_t prop1_2, double prop1_3,
                             Utf8CP prop2_1, int64_t prop2_2, double prop2_3,
                             Utf8CP prop3_1, int64_t prop3_2, double prop3_3,
                             Utf8CP prop4_1, int64_t prop4_2, double prop4_3) : T_Super (params, prop1_1, prop1_2, prop1_3, prop2_1, prop2_2, prop2_3, prop3_1, prop3_2, prop3_3),
                             m_prop4_1 (prop4_1), m_prop4_2 (prop4_2), m_prop4_3 (prop4_3)
            {
            }

        virtual Dgn::DgnDbStatus _BindInsertParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _ExtractSelectParams (BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;

    public:
        static PerformanceElement4Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues);

        PerformanceElement4CPtr Insert ();
        PerformanceElement4CPtr Update ();
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement1Handler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT1_CLASS, PerformanceElement1, PerformanceElement1Handler, Dgn::dgn_ElementHandler::Physical, )
    protected: virtual void _GetClassParams (Dgn::ECSqlClassParams& params) override;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement2Handler : PerformanceElement1Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT2_CLASS, PerformanceElement2, PerformanceElement2Handler, PerformanceElement1Handler, )
    protected: virtual void _GetClassParams (Dgn::ECSqlClassParams& params) override;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement3Handler : PerformanceElement2Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT3_CLASS, PerformanceElement3, PerformanceElement3Handler, PerformanceElement2Handler, )
    protected: virtual void _GetClassParams (Dgn::ECSqlClassParams& params) override;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement4Handler : PerformanceElement3Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT4_CLASS, PerformanceElement4, PerformanceElement4Handler, PerformanceElement3Handler, )
    protected: virtual void _GetClassParams (Dgn::ECSqlClassParams& params) override;
    };

//---------------------------------------------------------------------------------------
// @bsiClass                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElementTestDomain : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS (PerformanceElementTestDomain, )
    public:
        PerformanceElementTestDomain ();
    };

//---------------------------------------------------------------------------------------
// @bsiClass                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElementsCRUDTestFixture : public DgnDbTestFixture
    {
    private:
        static const int64_t s_firstInstanceId = INT64_C (6);
        static Utf8CP const s_testSchemaXml;

        void CreateElements (int numInstances, Utf8CP className, bvector<DgnElementPtr>& elements, Utf8String modelCode, bool specifyProperyValues);
        void SetUpTestDgnDb (WCharCP destFileName, Utf8CP testClassName, int initialInstanceCount);

        //Generate Sql CRUD Statements.
        void GetInsertSql (Utf8CP className, Utf8StringR insertSql, DgnClassId classId);
        void GetSelectSql (Utf8CP className, Utf8StringR selectSql);
        void GetUpdateSql (Utf8CP className, Utf8StringR updateSql);
        void GetDeleteSql (Utf8StringR deleteSql);

        //Generate ECSql CRUD Statements. 
        void GetInsertECSql (Utf8CP className, Utf8StringR insertECSql);
        void GetSelectECSql (Utf8CP className, Utf8StringR selectECSql);
        void GetUpdateECSql (Utf8CP className, Utf8StringR updateECSql);
        void GetDeleteECSql (Utf8CP className, Utf8StringR deleteECSql);

        //Methods to Bind Business Property Values for Sql Statements
        DgnDbStatus BindElement1PropertyParams (BeSQLite::Statement& stmt, bool updateParams);
        DgnDbStatus BindElement2PropertyParams (BeSQLite::Statement& stmt, bool updateParams);
        DgnDbStatus BindElement3PropertyParams (BeSQLite::Statement& stmt, bool updateParams);
        DgnDbStatus BindElement4PropertyParams (BeSQLite::Statement& stmt, bool updateParams);
        void BindParams (DgnElementPtr& element, BeSQLite::Statement& stmt, Utf8CP className);
        void BindUpdateParams (BeSQLite::Statement& stmt, Utf8CP className);

        //ECsql Overloads to Bind business Property values
        DgnDbStatus BindElement1PropertyParams (ECSqlStatement& stmt, bool updateParams);
        DgnDbStatus BindElement2PropertyParams (ECSqlStatement& stmt, bool updateParams);
        DgnDbStatus BindElement3PropertyParams (ECSqlStatement& stmt, bool updateParams);
        DgnDbStatus BindElement4PropertyParams (ECSqlStatement& stmt, bool updateParams);
        void BindParams (DgnElementPtr& element, ECSqlStatement& stmt, Utf8CP className);
        void BindUpdateParams (ECSqlStatement& stmt, Utf8CP className);

        //Methods to verify Business Property Values returned by Sql Statements. 
        DgnDbStatus ExtractElement1SelectParams (BeSQLite::Statement& stmt);
        DgnDbStatus ExtractElement2SelectParams (BeSQLite::Statement& stmt);
        DgnDbStatus ExtractElement3SelectParams (BeSQLite::Statement& stmt);
        DgnDbStatus ExtractElement4SelectParams (BeSQLite::Statement& stmt);
        void ExtractSelectParams (BeSQLite::Statement& stmt, Utf8CP className);

        //OverLoaded Methods to Verify Business property Values returned by ECSql Statements. 
        DgnDbStatus ExtractElement1SelectParams (ECSqlStatement& stmt);
        DgnDbStatus ExtractElement2SelectParams (ECSqlStatement& stmt);
        DgnDbStatus ExtractElement3SelectParams (ECSqlStatement& stmt);
        DgnDbStatus ExtractElement4SelectParams (ECSqlStatement& stmt);
        void ExtractSelectParams (ECSqlStatement& stmt, Utf8CP className);

    protected:
        static const int s_initialInstanceCount = 1000;
        static const int s_opCount = 300;
        static const int varyInitialCount = 3;

        void DgnApiIsertTime (int instanceCount, Utf8CP className, int initialInstanceCount);
        void DgnApiSelectTime (Utf8CP className, int initialInstanceCount);
        void DgnApiUpdateTime (int instanceCount, Utf8CP className, int initialInstanceCount);
        void DgnApiDeleteTime (int instanceCount, Utf8CP className, int initialInstanceCount);

        void ECSqlInsertTime (int instanceCount, Utf8CP className, int initialInstanceCount);
        void ECSqlSelectTime (Utf8CP className, int initialInstanceCount);
        void ECSqlUpdateTime (int instanceCount, Utf8CP className, int initialInstanceCount);
        void ECSqlDeleteTime (int instanceCount, Utf8CP className, int initialInstanceCount);

        void SqlInsertTime (int instanceCount, Utf8CP className, int initialInstanceCount);
        void SqlSelectTime (Utf8CP className, int initialInstanceCount);
        void SqlUpdateTime (int instanceCount, Utf8CP className, int initialInstanceCount);
        void SqlDeleteTime (int instanceCount, Utf8CP className, int initialInstanceCount);

    public:
        PerformanceElementsCRUDTestFixture ()
            {
            DgnDomains::RegisterDomain (PerformanceElementTestDomain::GetDomain ());
            }
    };