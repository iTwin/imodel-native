/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ctime>
#include <initializer_list>
#include <random>
USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE
#define PRINT_JSON(NAME, JSON) printf(##NAME##": %s\n", JSON.Stringify(StringifyFormat::Indented).c_str());

struct InstanceRepositoryFixture : ECDbTestFixture {
    static std::unique_ptr<BeJsDocument> ParseJson(const char* json) {
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
                        <ECProperty propertyName="a0"    typeName="double"  />
                        <ECProperty propertyName="a1"    typeName="double"  />
                        <ECProperty propertyName="unhandled_prop"    typeName="double"  />
                    </ECEntityClass>
                    <ECEntityClass typeName="Goo">
                        <BaseClass>Foo</BaseClass>
                        <ECProperty propertyName="b0"    typeName="double"  />
                        <ECProperty propertyName="b1"    typeName="double"  />
                    </ECEntityClass>
               </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("first.ecdb", schema));
    m_ecdb.SaveChanges();
    auto& instRepo = m_ecdb.GetInstanceRepository();

    struct FooHandler : InstanceRepository::IClassHandler {
        uint64_t id = 100;
        void OnNextId(ECInstanceId& k) { k = ECInstanceId(id++); };

        PropertyHandlerResult OnBindECProperty(ECN::ECPropertyCR property, BeJsConst val, IECSqlBinder& binder, ECSqlStatus& rc) {
            return PropertyHandlerResult::Continue;
        }
        PropertyHandlerResult OnBindUserProperty(Utf8CP property, BeJsConst val, PropertyBinder::Finder finder, ECSqlStatus& rc) {
            if (BeStringUtilities::StricmpAscii(property, "a") == 0) {
                if (val.isNumericMember("a0")) {
                    rc = finder("a0")->GetBinder().BindDouble(val["a0"].asDouble());
                    if (rc != ECSqlStatus::Success) {
                        return PropertyHandlerResult::Handled;
                    }
                }
                if (val.isNumericMember("a1")) {
                    rc = finder("a1")->GetBinder().BindDouble(val["a1"].asDouble());
                    if (rc != ECSqlStatus::Success) {
                        return PropertyHandlerResult::Handled;
                    }
                }
                return PropertyHandlerResult::Handled;
            }
            return PropertyHandlerResult::Continue;
        }
        PropertyHandlerResult OnReadECProperty(ECN::ECPropertyCR property, const IECSqlValue& valueReader, BeJsValue val, ECSqlStatus& rc) {
            return PropertyHandlerResult::Continue;
        }
        ECSqlStatus OnReadComplete(BeJsValue& instance, PropertyReader::Finder finder) {
            auto aVal = instance["a"];
            aVal.toObject();
            aVal["a0"] = finder("a0")->GetReader().GetDouble();
            aVal["a1"] = finder("a1")->GetReader().GetDouble();
            return ECSqlStatus::Success;
        }
    };

    struct GooHandler : InstanceRepository::IClassHandler {
    public:
        PropertyHandlerResult OnBindECProperty(ECN::ECPropertyCR property, BeJsConst val, IECSqlBinder& binder, ECSqlStatus& rc) {
            return PropertyHandlerResult::Continue;
        }
        PropertyHandlerResult OnBindUserProperty(Utf8CP property, BeJsConst val, PropertyBinder::Finder finder, ECSqlStatus& rc) {
            if (BeStringUtilities::StricmpAscii(property, "b") == 0) {
                if (val.isNumericMember("b0")) {
                    rc = finder("b0")->GetBinder().BindDouble(val["b0"].asDouble());
                    if (rc != ECSqlStatus::Success) {
                        return PropertyHandlerResult::Handled;
                    }
                }
                if (val.isNumericMember("b1")) {
                    rc = finder("b1")->GetBinder().BindDouble(val["b1"].asDouble());
                    if (rc != ECSqlStatus::Success) {
                        return PropertyHandlerResult::Handled;
                    }
                }
                return PropertyHandlerResult::Handled;
            }
            return PropertyHandlerResult::Continue;
        }
        PropertyHandlerResult OnReadECProperty(ECN::ECPropertyCR property, const IECSqlValue& valueReader, BeJsValue val, ECSqlStatus& rc) {
            if (property.GetName().EqualsIAscii("b0")) {
                auto ob = val["b"];
                ob.toObject();
                ob["b0"] = valueReader.GetDouble();
                return PropertyHandlerResult::Handled;
            }
            if (property.GetName().EqualsIAscii("b1")) {
                auto ob = val["b"];
                ob.toObject();
                ob["b1"] = valueReader.GetDouble();
                return PropertyHandlerResult::Handled;
            }

            return PropertyHandlerResult::Continue;
        }
        ECSqlStatus OnReadComplete(BeJsValue& instance, PropertyReader::Finder finder) {
            auto aVal = instance["b"];
            aVal.toObject();
            aVal["b0"] = finder("b0")->GetReader().GetDouble();
            aVal["b1"] = finder("b1")->GetReader().GetDouble();
            return ECSqlStatus::Success;
        }
    };

    ASSERT_TRUE(instRepo.RegisterClassHandler<FooHandler>("ts:Foo", std::vector<Utf8CP>{"a0", "a1"}));
    ASSERT_TRUE(instRepo.RegisterClassHandler<GooHandler>("ts:Goo", std::vector<Utf8CP>{"b0", "b1"}));

    auto options = ParseJson(R"json({
        "computeSum": true
        "computeMul": true
    })json");

    auto fooInst1 = ParseJson(R"json({
        "className": "TestSchema.Foo",
        "a": {
            "a0": 17,
            "a1": 43
        },
        "unhandled_prop": 100
    })json");

    auto gooInst1 = ParseJson(R"json({
        "className": "TestSchema.Goo",
        "a": {
            "a0": 50,
            "a1": 42
        },
        "b": {
            "b0": 15,
            "b1": 76
        },
        "unhandled_prop": 200
    })json");

    ECInstanceKey fooKey, gooKey;
    ASSERT_EQ(BE_SQLITE_DONE, instRepo.Insert(*fooInst1, *options, JsFormat::JsName, fooKey));
    ASSERT_EQ(BE_SQLITE_DONE, instRepo.Insert(*gooInst1, *options, JsFormat::JsName, gooKey));

    m_ecdb.SaveChanges();

    BeJsDocument fooInstRead;
    ASSERT_EQ(BE_SQLITE_ROW, instRepo.Read(fooKey, fooInstRead, *options, JsFormat::JsName));

    BeJsDocument gooInstRead;
    ASSERT_EQ(BE_SQLITE_ROW, instRepo.Read(gooKey, gooInstRead, *options, JsFormat::JsName));

    auto expectedFoo = ParseJson(R"json({
        "id": "0x64",
        "classFullName": "TestSchema.Foo",
        "unhandled_prop": 100.0,
        "a": {
            "a0": 17.0,
            "a1": 43.0
        }
    })json");

    auto expectedGoo = ParseJson(R"json({
        "id": "0x65",
        "classFullName": "TestSchema.Goo",
        "unhandled_prop": 200.0,
        "a": {
            "a0": 50.0,
            "a1": 42.0
        },
        "b": {
            "b0": 15.0,
            "b1": 76.0
        }
    })json");
    ASSERT_FALSE(expectedFoo->hasParseError());
    ASSERT_FALSE(expectedGoo->hasParseError());

    ASSERT_STREQ(expectedFoo->Stringify(StringifyFormat::Indented).c_str(), fooInstRead.Stringify(StringifyFormat::Indented).c_str());
    ASSERT_STREQ(expectedGoo->Stringify(StringifyFormat::Indented).c_str(), gooInstRead.Stringify(StringifyFormat::Indented).c_str());
}
END_ECDBUNITTESTS_NAMESPACE
