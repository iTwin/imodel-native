/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ctime>
#include <random>

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE
#define PRINT_JSON(NAME, JSON) printf(##NAME##": %s\n", JSON.Stringify(StringifyFormat::Indented).c_str());

struct InstanceRepositoryFixture : ECDbTestFixture {
    static std::unique_ptr<BeJsDocument> PraseJson(const char* json) {
        auto doc = std::make_unique<BeJsDocument>();
        doc->Parse(json);
        return doc;
    }
};


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceRepositoryFixture, basic) {
    auto schema = SchemaItem(
        R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                   <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
                    <ECEntityClass typeName="Foo">
                        <ECProperty propertyName="A"    typeName="double"  />
                        <ECProperty propertyName="B"    typeName="double"  />
                        <ECProperty propertyName="Sum"  typeName="double"  />
                    </ECEntityClass>
                    <ECEntityClass typeName="Goo">
                        <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName="Mul"  typeName="double"  />
                    </ECEntityClass>
               </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("first.ecdb", schema));
    m_ecdb.SaveChanges();
    auto& instRepo = m_ecdb.GetInstanceRepository();

    struct FooHandler : InstanceRepository::IClassHandler {
    public:
        FooHandler(ECDbCR ecdb, ECN::ECClassId classId) : IClassHandler(ecdb, classId) {}
        ~FooHandler() override = default;
        PropertyHandlerResult OnBindProperty(InstanceRepository::WriteArgs& args) override {
            if (args.GetProperty().GetName() != "Sum") {
                return PropertyHandlerResult::Continue;
            }

            auto a = args.GetInstance()["a"].asDouble();
            auto b = args.GetInstance()["b"].asDouble();
            args.GetBinder().BindDouble(a + b);
            return PropertyHandlerResult::Handled;
        }
        void OnBeforeInsertInstance(BeJsValue& inst, const BeJsConst&, JsFormat) override {
            inst["sum"] = 0;
        }
    };

    struct GooHandler : InstanceRepository::IClassHandler {
    public:
        GooHandler(ECDbCR ecdb, ECN::ECClassId classId) : IClassHandler(ecdb, classId) {}
        ~GooHandler() override = default;
        PropertyHandlerResult OnBindProperty(InstanceRepository::WriteArgs& args) override {
            if (args.GetProperty().GetName() != "Mul") {
                return PropertyHandlerResult::Continue;
            }

            auto a = args.GetInstance()["a"].asDouble();
            auto b = args.GetInstance()["b"].asDouble();
            args.GetBinder().BindDouble(a * b);
            return PropertyHandlerResult::Handled;
        }
        void OnBeforeInsertInstance(BeJsValue& inst, const BeJsConst&, JsFormat) override {
            inst["mul"] = 0;
        }
    };

    ASSERT_TRUE(instRepo.RegisterClassHandler<FooHandler>("ts:Foo"));
    ASSERT_TRUE(instRepo.RegisterClassHandler<GooHandler>("ts:Goo"));

    auto options = PraseJson(R"json({
        "computeSum": true
        "computeMul": true
    })json");

    auto fooInst1 = PraseJson(R"json({
        "className": "TestSchema.Foo",
        "a": 7,
        "b": 2
    })json");

    auto gooInst1 = PraseJson(R"json({
        "className": "TestSchema.Goo",
        "a": 5,
        "b": 4
    })json");

    ECInstanceKey fooKey, gooKey;
    ASSERT_EQ(BE_SQLITE_DONE, instRepo.Insert(*fooInst1, *options, JsFormat::JsName, fooKey));
    ASSERT_EQ(BE_SQLITE_DONE, instRepo.Insert(*gooInst1, *options, JsFormat::JsName, gooKey));

    m_ecdb.SaveChanges();

    BeJsDocument fooInstRead;
    ASSERT_EQ(BE_SQLITE_ROW, instRepo.Read(fooKey, fooInstRead, *options, JsFormat::JsName));

    BeJsDocument gooInstRead;
    ASSERT_EQ(BE_SQLITE_ROW, instRepo.Read(gooKey, gooInstRead, *options, JsFormat::JsName));

    // out: {
    //    "id": "0x1",
    //    "className": "TestSchema.Foo",
    //    "a": 7.0,
    //    "b": 2.0,
    //    "sum": 9.0
    // }
    // out: {
    //    "id": "0x2",
    //    "className": "TestSchema.Goo",
    //    "a": 5.0,
    //    "b": 4.0,
    //    "sum": 9.0,
    //    "mul": 20.0
    // }

    ASSERT_EQ(7.0, fooInstRead["a"].asDouble());
    ASSERT_EQ(2.0, fooInstRead["b"].asDouble());
    ASSERT_EQ(9.0, fooInstRead["sum"].asDouble());

    ASSERT_EQ(5.0, gooInstRead["a"].asDouble());
    ASSERT_EQ(4.0, gooInstRead["b"].asDouble());
    ASSERT_EQ(9.0, gooInstRead["sum"].asDouble());
    ASSERT_EQ(20.0, gooInstRead["mul"].asDouble());
}
END_ECDBUNITTESTS_NAMESPACE
