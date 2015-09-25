/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceElements.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#define ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME    "TestSchema"
#define ELEMENT_PERFORMANCE_ELEMENT1_CLASS      "Element1"
#define ELEMENT_PERFORMANCE_ELEMENT2_CLASS      "Element2"
#define ELEMENT_PERFORMANCE_ELEMENT3_CLASS      "Element3"
#define ELEMENT_PERFORMANCE_ELEMENT4_CLASS      "Element4"
#define ELEMENT_PERFORMANCE_ELEMENT4b_CLASS     "Element4b"
#define ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS "SimpleElement"

namespace ElementInsertPerformanceTestNamespace {

struct PerformanceElement1;
struct PerformanceElement2;
struct PerformanceElement3;
struct PerformanceElement4;
struct PerformanceElement4b;
struct SimpleElement;

DEFINE_REF_COUNTED_PTR(PerformanceElement1)
DEFINE_REF_COUNTED_PTR(PerformanceElement2)
DEFINE_REF_COUNTED_PTR(PerformanceElement3)
DEFINE_REF_COUNTED_PTR(PerformanceElement4)
DEFINE_REF_COUNTED_PTR(PerformanceElement4b)
DEFINE_REF_COUNTED_PTR(SimpleElement)

//---------------------------------------------------------------------------------------
// @bsiclass                                    Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement1 : Dgn::PhysicalElement
    {
    friend struct PerformanceElement1Handler;
    DGNELEMENT_DECLARE_MEMBERS(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, Dgn::PhysicalElement);

    private:
        void GetParams(bvector<Utf8CP>& params);
        DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop1_1;
        int64_t m_prop1_2;
        double m_prop1_3;

    protected:
        PerformanceElement1(CreateParams const& params, Utf8CP prop1_1 = NULL, int64_t prop1_2 = 10000000LL, double prop1_3 = -3.1415) : T_Super(params), m_prop1_1(prop1_1), m_prop1_2(prop1_2), m_prop1_3(prop1_3) {}

        virtual void _GetInsertParams(bvector<Utf8CP>& insertParams) override;
        virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& statement) override;
        virtual void _GetUpdateParams(bvector<Utf8CP>& updateParams) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;
        virtual void _GetSelectParams(bvector<Utf8CP>& params) override { T_Super::_GetSelectParams(params); GetParams(params); }
        virtual Dgn::DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& stmt, BeSQLite::EC::ECSqlSelectParameters const& params) override;

    public:
        static PerformanceElement1Ptr Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category);

        PerformanceElement1CPtr Insert();
        PerformanceElement1CPtr Update();
        void SetString1(Utf8CP str) { m_prop1_1 = str;}

    };

//---------------------------------------------------------------------------------------
// @bsiclass                                    Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement2 : PerformanceElement1
    {
    friend struct PerformanceElement2Handler;
    DGNELEMENT_DECLARE_MEMBERS(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, PerformanceElement1);
 
    private:
        void GetParams(bvector<Utf8CP>& params);
        DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop2_1;
        int64_t m_prop2_2;
        double m_prop2_3;

    protected:
        explicit PerformanceElement2(CreateParams const& params,
            Utf8CP prop1_1 = NULL, int64_t prop1_2 = 10000000LL, double prop1_3 = -3.1415,
            Utf8CP prop2_1 = NULL, int64_t prop2_2 = 20000000LL, double prop2_3 = 2.71828) : T_Super(params, prop1_1, prop1_2, prop1_3), m_prop2_1(prop2_1), m_prop2_2(prop2_2), m_prop2_3(prop2_3) {}

        virtual void _GetInsertParams(bvector<Utf8CP>& insertParams) override;
        virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& statement) override;
        virtual void _GetUpdateParams(bvector<Utf8CP>& updateParams) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;
        virtual void _GetSelectParams(bvector<Utf8CP>& params) override { T_Super::_GetSelectParams(params); GetParams(params); }
        virtual Dgn::DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& stmt, BeSQLite::EC::ECSqlSelectParameters const& params) override;

    public:
        static PerformanceElement2Ptr Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category);

        PerformanceElement2CPtr Insert();
        PerformanceElement2CPtr Update();
        void SetString2(Utf8CP str) {m_prop2_1 = str;}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                    Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement3 : PerformanceElement2
    {
    friend struct PerformanceElement3Handler;
    DGNELEMENT_DECLARE_MEMBERS(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, PerformanceElement2);

    private:
        void GetParams(bvector<Utf8CP>& params);
        DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop3_1;
        int64_t m_prop3_2;
        double m_prop3_3;

    protected:
        PerformanceElement3(CreateParams const& params,
            Utf8CP prop1_1 = NULL, int64_t prop1_2 = 10000000LL, double prop1_3 = -3.1415,
            Utf8CP prop2_1 = NULL, int64_t prop2_2 = 20000000LL, double prop2_3 = 2.71828,
            Utf8CP prop3_1 = NULL, int64_t prop3_2 = 30000000LL, double prop3_3 = 1.414121) : T_Super(params, prop1_1, prop1_2, prop1_3, prop2_1, prop2_2, prop2_3), m_prop3_1(prop3_1), m_prop3_2(prop3_2), m_prop3_3(prop3_3) {}

        virtual void _GetInsertParams(bvector<Utf8CP>& insertParams) override;
        virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& statement) override;
        virtual void _GetUpdateParams(bvector<Utf8CP>& updateParams) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;
        virtual void _GetSelectParams(bvector<Utf8CP>& params) override { T_Super::_GetSelectParams(params); GetParams(params); }
        virtual Dgn::DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& stmt, BeSQLite::EC::ECSqlSelectParameters const& params) override;

    public:
        static PerformanceElement3Ptr Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category);

        PerformanceElement3CPtr Insert();
        PerformanceElement3CPtr Update();
        void SetString3(Utf8CP str) { m_prop3_1 = str;}

    };

//---------------------------------------------------------------------------------------
// @bsiclass                                    Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement4 : PerformanceElement3
    {
    friend struct PerformanceElement4Handler;
    DGNELEMENT_DECLARE_MEMBERS(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, PerformanceElement3);

    private:
        void GetParams(bvector<Utf8CP>& params);
        DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop4_1;
        int64_t m_prop4_2;
        double m_prop4_3;

    protected:
        PerformanceElement4(CreateParams const& params,
            Utf8CP prop1_1 = NULL, int64_t prop1_2 = 10000000LL, double prop1_3 = -3.1415,
            Utf8CP prop2_1 = NULL, int64_t prop2_2 = 20000000LL, double prop2_3 = 2.71828,
            Utf8CP prop3_1 = NULL, int64_t prop3_2 = 30000000LL, double prop3_3 = 1.414121,
            Utf8CP prop4_1 = NULL, int64_t prop4_2 = 40000000LL, double prop4_3 = 1.61803398874) : T_Super(params, prop1_1, prop1_2, prop1_3, prop2_1, prop2_2, prop2_3, prop3_1, prop3_2, prop3_3), 
            m_prop4_1(prop4_1), m_prop4_2(prop4_2), m_prop4_3(prop4_3) {}

        virtual void _GetInsertParams(bvector<Utf8CP>& insertParams) override;
        virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& statement) override;
        virtual void _GetUpdateParams(bvector<Utf8CP>& updateParams) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;
        virtual void _GetSelectParams(bvector<Utf8CP>& params) override { T_Super::_GetSelectParams(params); GetParams(params); }
        virtual Dgn::DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& stmt, BeSQLite::EC::ECSqlSelectParameters const& params) override;

    public:
        static PerformanceElement4Ptr Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category);

        PerformanceElement4CPtr Insert();
        PerformanceElement4CPtr Update();
        void SetString4(Utf8CP str) { m_prop4_1 = str;}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                    Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement4b : PerformanceElement3
    {
    friend struct PerformanceElement4bHandler;
    DGNELEMENT_DECLARE_MEMBERS(ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, PerformanceElement3);

    private:
        void GetParams(bvector<Utf8CP>& params);
        DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& statement);

        Utf8String m_prop4b_1;
        int64_t m_prop4b_2;
        double m_prop4b_3;
        DPoint3d m_prop4b_4;

    protected:
        PerformanceElement4b(CreateParams const& params,
            Utf8CP prop1_1 = NULL, int64_t prop1_2 = 10000000LL, double prop1_3 = -3.1415,
            Utf8CP prop2_1 = NULL, int64_t prop2_2 = 20000000LL, double prop2_3 = 2.71828,
            Utf8CP prop3_1 = NULL, int64_t prop3_2 = 30000000LL, double prop3_3 = 1.414121,
            Utf8CP prop4b_1 = NULL, int64_t prop4b_2 = 45000000LL, double prop4b_3 = 6.022140857, DPoint3d prop4b_4 = DPoint3d::From(1.0, 2.0, 3.0)) : 
            T_Super(params, prop1_1, prop1_2, prop1_3, prop2_1, prop2_2, prop2_3, prop3_1, prop3_2, prop3_3), 
            m_prop4b_1(prop4b_1), m_prop4b_2(prop4b_2), m_prop4b_3(prop4b_3), m_prop4b_4(prop4b_4) {}

        virtual void _GetInsertParams(bvector<Utf8CP>& insertParams) override;
        virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& statement) override;
        virtual void _GetUpdateParams(bvector<Utf8CP>& updateParams) override;
        virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;
        virtual void _GetSelectParams(bvector<Utf8CP>& params) override { T_Super::_GetSelectParams(params); GetParams(params); }
        virtual Dgn::DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& stmt, BeSQLite::EC::ECSqlSelectParameters const& params) override;

    public:
        static PerformanceElement4bPtr Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category);

        PerformanceElement4bCPtr Insert();
        PerformanceElement4bCPtr Update();
        void SetString4b(Utf8CP str) { m_prop4b_1 = str;}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct SimpleElement: Dgn::DgnElement
    {
    public:
        DGNELEMENT_DECLARE_MEMBERS("SimpleElement", Dgn::DgnElement);
        friend struct SimpleElementHandler;

    protected:
        SimpleElement(CreateParams const& params) : T_Super(params) {}

    public:
        static SimpleElementPtr Create(Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category);

        SimpleElementCPtr Insert();
        SimpleElementCPtr Update();
        //! Query the DgnClassId for the SimpleElement ECClass in the specified DgnDb.
        static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetECClassId(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS));}

    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct SimpleElementHandler : Dgn::dgn_ElementHandler::Element
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(ELEMENT_PERFORMANCE_SIMPLEELEMENT_CLASS, SimpleElement, SimpleElementHandler, Dgn::dgn_ElementHandler::Element, )
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement1Handler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(ELEMENT_PERFORMANCE_ELEMENT1_CLASS, PerformanceElement1, PerformanceElement1Handler, Dgn::dgn_ElementHandler::Physical, )
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement2Handler : PerformanceElement1Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(ELEMENT_PERFORMANCE_ELEMENT2_CLASS, PerformanceElement2, PerformanceElement2Handler, PerformanceElement1Handler, )
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement3Handler : PerformanceElement2Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(ELEMENT_PERFORMANCE_ELEMENT3_CLASS, PerformanceElement3, PerformanceElement3Handler, PerformanceElement2Handler, )
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement4Handler : PerformanceElement3Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(ELEMENT_PERFORMANCE_ELEMENT4_CLASS, PerformanceElement4, PerformanceElement4Handler, PerformanceElement3Handler, )
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElement4bHandler : PerformanceElement3Handler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(ELEMENT_PERFORMANCE_ELEMENT4b_CLASS, PerformanceElement4b, PerformanceElement4bHandler, PerformanceElement3Handler, )
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
struct PerformanceElementsTestDomain : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(PerformanceElementsTestDomain, )
    public:
        PerformanceElementsTestDomain();
        static void RegisterDomainAndImportSchema(DgnDbR db, ECN::ECSchemaR schema);
    };

}; // namespace 

