    /*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/BackDoor/TestDomainElements.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_DGNPLATFORM


#define ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME    "TestSchema"
#define ELEMENT_TESTELEMENTSUB1_CLASS      "TestElementSub1"
#define ELEMENT_TESTELEMENTSUB2_CLASS      "TestElementSub2"
#define ELEMENT_TESTELEMENTSUB3_CLASS      "TestElementSub3"
#define ELEMENT_TestElementComplex_CLASS   "TestElementComplex"

struct TestElementSub1;
struct TestElementSub2;
struct TestElementSub3;
struct TestElementComplex;

DEFINE_REF_COUNTED_PTR (TestElementSub1)
DEFINE_REF_COUNTED_PTR (TestElementSub2)
DEFINE_REF_COUNTED_PTR (TestElementSub3)
DEFINE_REF_COUNTED_PTR (TestElementComplex)

struct TestElementSub1 : TestElement
    {
    friend struct TestElementSub1Handler;
    DGNELEMENT_DECLARE_MEMBERS(ELEMENT_TESTELEMENTSUB1_CLASS, TestElement);

    private:
        DgnDbStatus BindParams (BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop1_1;
        int64_t m_prop1_2;
        double m_prop1_3;

    protected:
        TestElementSub1 (CreateParams const& params) : T_Super (params) {}
        TestElementSub1 (CreateParams const& params, Utf8CP prop1_1, int64_t prop1_2, double prop1_3) : T_Super (params), m_prop1_1 (prop1_1), m_prop1_2 (prop1_2), m_prop1_3 (prop1_3) {}

        virtual Dgn::DgnDbStatus _BindInsertParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;

    public:
        static TestElementSub1Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues);

        TestElementSub1CPtr Insert ();
        TestElementSub1CPtr Update ();
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat            12/15
//---------------+---------------+---------------+---------------+---------------+-------
struct TestElementSub2 : TestElementSub1
    {
    friend struct TestElementSub2Handler;
    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_TESTELEMENTSUB2_CLASS, TestElementSub1);

    private:
        DgnDbStatus BindParams (BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop2_1;
        int64_t m_prop2_2;
        double m_prop2_3;

    protected:
        TestElementSub2 (CreateParams const& params) : T_Super (params) {}
        TestElementSub2 (CreateParams const& params,
                             Utf8CP prop1_1, int64_t prop1_2, double prop1_3,
                             Utf8CP prop2_1, int64_t prop2_2, double prop2_3) : T_Super (params, prop1_1, prop1_2, prop1_3), m_prop2_1 (prop2_1), m_prop2_2 (prop2_2), m_prop2_3 (prop2_3)
            {
            }


        virtual Dgn::DgnDbStatus _BindInsertParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _ReadSelectParams (BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;

    public:
        static TestElementSub2Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues);

        TestElementSub2CPtr Insert ();
        TestElementSub2CPtr Update ();
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat            12/15
//---------------+---------------+---------------+---------------+---------------+-------
struct TestElementSub3 : TestElementSub2
    {
    friend struct TestElementSub3Handler;
    DGNELEMENT_DECLARE_MEMBERS (ELEMENT_TESTELEMENTSUB3_CLASS, TestElementSub2);

    private:
        DgnDbStatus BindParams (BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop3_1;
        int64_t m_prop3_2;
        double m_prop3_3;

    protected:
        TestElementSub3 (CreateParams const& params) : T_Super (params) {}
        TestElementSub3 (CreateParams const& params,
                             Utf8CP prop1_1, int64_t prop1_2, double prop1_3,
                             Utf8CP prop2_1, int64_t prop2_2, double prop2_3,
                             Utf8CP prop3_1, int64_t prop3_2, double prop3_3) : T_Super (params, prop1_1, prop1_2, prop1_3, prop2_1, prop2_2, prop2_3), m_prop3_1 (prop3_1), m_prop3_2 (prop3_2), m_prop3_3 (prop3_3)
            {
            }

        virtual Dgn::DgnDbStatus _BindInsertParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _ReadSelectParams (BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;

    public:
        static TestElementSub3Ptr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues);

        TestElementSub3CPtr Insert ();
        TestElementSub3CPtr Update ();
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat            12/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct TestStruct
{
public:
    TestStruct(){}
    TestStruct(int mInt, double mDouble, bool mBoolean) : IntMember(mInt), DoubleMember(mDouble), BoolMember(mBoolean){}
    double  DoubleMember;
    int     IntMember;
    bool    BoolMember;
};

//---------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat            12/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct TestElementComplex : TestElement
    {
    friend struct TestElementComplexHandler;
    DGNELEMENT_DECLARE_MEMBERS(ELEMENT_TestElementComplex_CLASS, TestElement);

    private:
        DgnDbStatus BindParams (BeSQLite::EC::ECSqlStatement& statement);

        DPoint3d    m_prop_DPoint3d;
        TestStruct  m_prop_TestStruct;

    protected:
        TestElementComplex (CreateParams const& params) : T_Super (params) {}
        TestElementComplex (CreateParams const& params, DPoint3d prop1_1, TestStruct prop1_2) : T_Super (params), m_prop_DPoint3d (prop1_1), m_prop_TestStruct (prop1_2) {}

        virtual Dgn::DgnDbStatus _BindInsertParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams (BeSQLite::EC::ECSqlStatement& statement) override;
        virtual Dgn::DgnDbStatus _ReadSelectParams (BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;

    public:
        static TestElementComplexPtr Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id, bool specifyProperyValues);

        TestElementComplexCPtr Insert ();
        TestElementComplexCPtr Update ();
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                     Umar.Hayat            12/15
//---------------+---------------+---------------+---------------+---------------+-------
struct TestElementSub1Handler : TestElementHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_TESTELEMENTSUB1_CLASS, TestElementSub1, TestElementSub1Handler, TestElementHandler, )
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Umar.Hayat            12/15
//---------------+---------------+---------------+---------------+---------------+-------
struct TestElementSub2Handler : TestElementSub1Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_TESTELEMENTSUB2_CLASS, TestElementSub2, TestElementSub2Handler, TestElementSub1Handler, )
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Umar.Hayat            12/15
//---------------+---------------+---------------+---------------+---------------+-------
struct TestElementSub3Handler : TestElementSub2Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_TESTELEMENTSUB3_CLASS, TestElementSub3, TestElementSub3Handler, TestElementSub2Handler, )
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Umar.Hayat            12/15
//---------------+---------------+---------------+---------------+---------------+-------
struct TestElementComplexHandler : TestElementHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (ELEMENT_TestElementComplex_CLASS, TestElementComplex, TestElementComplexHandler, TestElementHandler, )
    };
