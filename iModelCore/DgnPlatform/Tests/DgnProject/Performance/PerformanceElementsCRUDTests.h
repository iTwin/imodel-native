/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceElementsCRUDTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
#define ELEMENT_ASPECT_CLASS                    "TestMultiAspect"

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
        static void _TEMPORARY_GetAutoHandlerInfo(ECSqlClassParams::HandlingCustomAttributes& params); // *** WIP_AUTO_HANDLED_PROPERTIES
        DgnDbStatus BindParams (BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop1_1;
        int64_t m_prop1_2;
        double m_prop1_3;

    protected:
        PerformanceElement1 (CreateParams const& params) : T_Super (params) {}
        PerformanceElement1 (CreateParams const& params, Utf8CP prop1_1, int64_t prop1_2, double prop1_3) : T_Super (params), m_prop1_1 (prop1_1), m_prop1_2 (prop1_2), m_prop1_3 (prop1_3) {}

        virtual Dgn::DgnDbStatus _BindInsertParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _ReadSelectParams (BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;

    public:
        static PerformanceElement1Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, bool specifyPropertyValues);

        PerformanceElement1CPtr Insert ();
        PerformanceElement1CPtr Update ();
        void AddGeomtry();
        void ExtendGeometry();
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                    Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement2 : PerformanceElement1
    {
    friend struct PerformanceElement2Handler;
    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT2_CLASS, PerformanceElement1);

    private:
        static void _TEMPORARY_GetAutoHandlerInfo(ECSqlClassParams::HandlingCustomAttributes& params); // *** WIP_AUTO_HANDLED_PROPERTIES
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
        virtual Dgn::DgnDbStatus _ReadSelectParams (BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;

    public:
        static PerformanceElement2Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, bool specifyPropertyValues);

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
        static void _TEMPORARY_GetAutoHandlerInfo(ECSqlClassParams::HandlingCustomAttributes& params); // *** WIP_AUTO_HANDLED_PROPERTIES
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
        virtual Dgn::DgnDbStatus _ReadSelectParams (BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;

    public:
        static PerformanceElement3Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, bool specifyPropertyValues);

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
        static void _TEMPORARY_GetAutoHandlerInfo(ECSqlClassParams::HandlingCustomAttributes& params); // *** WIP_AUTO_HANDLED_PROPERTIES
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
        virtual Dgn::DgnDbStatus _ReadSelectParams (BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;

    public:
        static PerformanceElement4Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, bool specifyPropertyValues);

        PerformanceElement4CPtr Insert ();
        PerformanceElement4CPtr Update ();
    };

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestMultiAspect : Dgn::DgnElement::MultiAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect)
private:
    friend struct TestMultiAspectHandler;

    Utf8String m_testMultiAspectProperty;

    explicit TestMultiAspect(Utf8CP prop) : m_testMultiAspectProperty(prop) { ; }

    Utf8CP _GetECSchemaName() const override { return ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME; }
    Utf8CP _GetECClassName() const override { return ELEMENT_ASPECT_CLASS; }
    // Dummy implementation
    Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override ;
    Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el) override { return DgnDbStatus::Success; };

public:
    static RefCountedPtr<TestMultiAspect> Create(Utf8CP prop) { return new TestMultiAspect(prop); }

    static ECN::ECClassCP GetECClass(Dgn::DgnDbR db) { return db.Schemas().GetECClass(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_ASPECT_CLASS); }

    Utf8StringCR GetTestMultiAspectProperty() const { return m_testMultiAspectProperty; }
    void SetTestMultiAspectProperty(Utf8CP s) { m_testMultiAspectProperty = s; }
};

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestMultiAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
    DOMAINHANDLER_DECLARE_MEMBERS(ELEMENT_ASPECT_CLASS, TestMultiAspectHandler, Dgn::dgn_AspectHandler::Aspect, )
        RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new TestMultiAspect(""); }
};
typedef RefCountedPtr<TestMultiAspect> TestMultiAspectPtr;
//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement1Handler : Dgn::dgn_ElementHandler::Geometric3d
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_PERFORMANCE_ELEMENT1_CLASS, PerformanceElement1, PerformanceElement1Handler, Dgn::dgn_ElementHandler::Geometric3d, )
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
protected:
    static const int s_initialInstanceCount = 1000000;
    static const int s_opCount = 50000;
    static const uint64_t s_firstElementId = UINT64_C(6);
    static Utf8CP const s_testSchemaXml;
    //we need to explicitly supply ElementId in case of insertion using Sql and ECSql otherwise we will get Unique_Constraint_Error
    static uint64_t s_elementId;

    void SetUpTestDgnDb(WCharCP destFileName, Utf8CP testClassName, int initialInstanceCount);

    void CreateElements(int numInstances, Utf8CP className, bvector<DgnElementPtr>& elements, Utf8String modelCode, bool specifyPropertyValues) const;

    static int DetermineElementIdIncrement(int initialInstanceCount, int opCount) { return initialInstanceCount / opCount; }

    //Generate Sql CRUD Statements.
    void GetInsertSql(Utf8CP className, Utf8StringR insertSql, DgnClassId classId) const;
    void GetSelectSql(Utf8CP className, Utf8StringR selectSql, bool asTranslatedFromECSql, bool omitClassIdFilter) const;
    void GetUpdateSql(Utf8CP className, Utf8StringR updateSql, bool omitClassIdFilter) const;
    void GetDeleteSql(Utf8CP className, Utf8StringR deleteSql, bool omitClassIdFilter) const;

    //Generate ECSql CRUD Statements. 
    void GetInsertECSql(Utf8CP className, Utf8StringR insertECSql) const;
    void GetSelectECSql(Utf8CP className, Utf8StringR selectECSql, bool omitClassIdFilter) const;
    void GetUpdateECSql(Utf8CP className, Utf8StringR updateECSql, bool omitClassIdFilter) const;
    void GetDeleteECSql(Utf8CP className, Utf8StringR deleteECSql, bool omitClassIdFilter) const;

    //Methods to Bind Business Property Values for Sql Statements
    static DgnDbStatus BindElement1PropertyParams(BeSQLite::Statement& stmt, bool updateParams);
    static DgnDbStatus BindElement2PropertyParams(BeSQLite::Statement& stmt, bool updateParams);
    static DgnDbStatus BindElement3PropertyParams(BeSQLite::Statement& stmt, bool updateParams);
    static DgnDbStatus BindElement4PropertyParams(BeSQLite::Statement& stmt, bool updateParams);
    static void BindParams(DgnElementPtr& element, BeSQLite::Statement& stmt, Utf8CP className);
    static void BindUpdateParams(BeSQLite::Statement& stmt, Utf8CP className);

    //ECsql Overloads to Bind business Property values
    static DgnDbStatus BindElement1PropertyParams(ECSqlStatement& stmt, bool updateParams);
    static DgnDbStatus BindElement2PropertyParams(ECSqlStatement& stmt, bool updateParams);
    static DgnDbStatus BindElement3PropertyParams(ECSqlStatement& stmt, bool updateParams);
    static DgnDbStatus BindElement4PropertyParams(ECSqlStatement& stmt, bool updateParams);
    static void BindParams(DgnElementPtr& element, ECSqlStatement& stmt, Utf8CP className);
    static void BindUpdateParams(DgnElementPtr& element, ECSqlStatement& stmt, Utf8CP className);

    //Methods to verify Business Property Values returned by Sql Statements. 
    static DgnDbStatus ExtractElement1SelectParams(BeSQLite::Statement& stmt);
    static DgnDbStatus ExtractElement2SelectParams(BeSQLite::Statement& stmt);
    static DgnDbStatus ExtractElement3SelectParams(BeSQLite::Statement& stmt);
    static DgnDbStatus ExtractElement4SelectParams(BeSQLite::Statement& stmt);
    static void ExtractSelectParams(BeSQLite::Statement& stmt, Utf8CP className);

    //OverLoaded Methods to Verify Business property Values returned by ECSql Statements. 
    static DgnDbStatus ExtractElement1SelectParams(ECSqlStatement& stmt);
    static DgnDbStatus ExtractElement2SelectParams(ECSqlStatement& stmt);
    static DgnDbStatus ExtractElement3SelectParams(ECSqlStatement& stmt);
    static DgnDbStatus ExtractElement4SelectParams(ECSqlStatement& stmt);
    static void ExtractSelectParams(ECSqlStatement& stmt, Utf8CP className);

    void LogTiming(StopWatch&, Utf8CP description, Utf8CP testClassName, bool omitClassIdFilter, int initialCount, int opCount) const;

    void ApiInsertTime(Utf8CP className, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);
    void ApiSelectTime(Utf8CP className, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);
    void ApiUpdateTime(Utf8CP className, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);
    void ApiDeleteTime(Utf8CP className, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);

    void ECSqlInsertTime(Utf8CP className, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);
    void ECSqlSelectTime(Utf8CP className, bool omitClassIdFilter, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);
    void ECSqlUpdateTime(Utf8CP className, bool omitClassIdFilter, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);
    void ECSqlDeleteTime(Utf8CP className, bool omitClassIdFilter, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);

    void SqlInsertTime(Utf8CP className, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);
    void SqlSelectTime(Utf8CP className, bool asTranslatedByECSql, bool omitClassIdFilter, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);
    void SqlUpdateTime(Utf8CP className, bool omitClassIdFilter, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);
    void SqlDeleteTime(Utf8CP className, bool omitClassIdFilter, int initialInstanceCount = s_initialInstanceCount, int opCount = s_opCount);

    PerformanceElementsCRUDTestFixture() : DgnDbTestFixture()
        {
        DgnDomains::RegisterDomain(PerformanceElementTestDomain::GetDomain());
        }
};
