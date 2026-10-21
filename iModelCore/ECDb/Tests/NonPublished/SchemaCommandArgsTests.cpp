/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECDb/SchemaEditor.h>
#include "../../ECDb/SchemaEditor/SchemaCommandArgParser.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

namespace {
void ExpectInvalid(SchemaCommandResult const& result, Utf8CP path) {
    ASSERT_EQ(SchemaCommandResult::Status::ERROR_INVALID_ARGS, result.GetStatus());
    ASSERT_FALSE(result.GetIssues().empty());
    EXPECT_STREQ("SchemaEditor.InvalidArgs", result.GetIssues().front().code.c_str());
    EXPECT_STREQ(path, result.GetIssues().front().arg.c_str());
    EXPECT_FALSE(result.GetIssues().front().message.empty());
}
}

TEST(SchemaCommandArgsTests, NormalizesDottedKeysWithoutCoercingScalars) {
    SchemaCommandArgs args{{"schema", "Test"}, {"Source.Classes", "Test:A, Test:B"}, {"source.polymorphic", "true"}, {"maxOccurs", "32"}, {"label", " Keep whitespace "}};
    BeJsDocument json;
    ASSERT_TRUE(NormalizeSchemaCommandArgs(args, json).IsOk());
    ASSERT_TRUE(json.isObject());
    ASSERT_TRUE(json["Source"].isObject());
    EXPECT_EQ(2u, json["Source"].size());
    EXPECT_FALSE(json.hasMember("source"));
    EXPECT_STREQ("true", json["Source"]["polymorphic"].asCString());
    EXPECT_TRUE(json["maxOccurs"].isString());
    EXPECT_STREQ(" Keep whitespace ", json["label"].asCString());

    SchemaCommandResult result;
    SchemaCommandArgParser parser(json, result);
    ASSERT_TRUE(parser.CheckKeys({"schema", "source.classes", "source.polymorphic", "maxOccurs", "label"}));
    bvector<Utf8String> classes;
    ASSERT_TRUE(parser.StringList("SOURCE.CLASSES", classes, true));
    ASSERT_EQ(2u, classes.size());
    EXPECT_STREQ("Test:A", classes[0].c_str());
    EXPECT_STREQ("Test:B", classes[1].c_str());
    bool polymorphic = false;
    uint32_t maxOccurs = 0;
    ASSERT_TRUE(parser.Boolean("source.POLYMORPHIC", polymorphic));
    ASSERT_TRUE(parser.UInt32("MAXOCCURS", maxOccurs));
    EXPECT_TRUE(polymorphic);
    EXPECT_EQ(32u, maxOccurs);
    EXPECT_TRUE(result.IsOk());
}

TEST(SchemaCommandArgsTests, RejectsDuplicateAndCollidingKeyValuePaths) {
    bvector<SchemaCommandArgs> cases{
        {{"name", "A"}, {"name", "B"}},
        {{"name", "A"}, {"NAME", "B"}},
        {{"source", "A"}, {"SOURCE.classes", "B"}},
        {{"source.classes", "A"}, {"SOURCE", "B"}},
        {{"source.classes", "A"}, {"SOURCE.CLASSES", "B"}},
        {{"instance", R"({"value":1})"}, {"INSTANCE.value", "2"}}};
    for (auto const& args : cases) {
        SCOPED_TRACE(args.GetEntries().back().first.c_str());
        EXPECT_EQ(2u, args.GetEntries().size());
        BeJsDocument out(R"({"unchanged":true})");
        auto result = NormalizeSchemaCommandArgs(args, out);
        ExpectInvalid(result, args.GetEntries().back().first.c_str());
        EXPECT_TRUE(out["unchanged"].asBool());
        EXPECT_EQ(1u, out.size());
    }
}

TEST(SchemaCommandArgsTests, RejectsMalformedPathsAndCollectsIssues) {
    BeJsDocument out;
    auto result = NormalizeSchemaCommandArgs({{"", "x"}, {".source", "x"}, {"target.", "x"}, {"source..classes", "x"}}, out);
    ExpectInvalid(result, "");
    EXPECT_EQ(4u, result.GetIssues().size());
    EXPECT_TRUE(out.isNull());
}

TEST(SchemaCommandArgsTests, EmptyArgumentsReplaceOutputWithAnEmptyObject) {
    BeJsDocument out(R"({"old":1})");
    ASSERT_TRUE(NormalizeSchemaCommandArgs({}, out).IsOk());
    EXPECT_TRUE(out.isObject());
    EXPECT_EQ(0u, out.size());
}

TEST(SchemaCommandArgsTests, ParsesStructuredValuesAndPreservesLegacyStrings) {
    BeJsDocument out;
    ASSERT_TRUE(NormalizeSchemaCommandArgs({{"customAttributes", R"([{"className":"ECDbMap.ClassMap","MapStrategy":"OwnTable"}])"},
                                            {"indexes", R"([{"name":"ix","properties":["Code"],"isUnique":true}])"},
                                            {"enumerators", R"([{"name":"One","value":1}])"},
                                            {"instance", R"({"className":"Test.CA","Value":true})"},
                                            {"description", R"({"not":"parsed"})"}},
                                           out)
                    .IsOk());
    EXPECT_TRUE(out["customAttributes"].isArray());
    EXPECT_TRUE(out["indexes"][0]["isUnique"].isBool());
    EXPECT_TRUE(out["enumerators"][0]["value"].isNumeric());
    EXPECT_TRUE(out["instance"].isObject());
    EXPECT_TRUE(out["description"].isString());
    ASSERT_TRUE(NormalizeSchemaCommandArgs({{"indexes", "ix:Code:unique"}, {"enumerators", "One=1;Two=2"}}, out).IsOk());
    EXPECT_STREQ("ix:Code:unique", out["indexes"].asCString());
    EXPECT_STREQ("One=1;Two=2", out["enumerators"].asCString());
}

TEST(SchemaCommandArgsTests, MalformedStructuredJsonDoesNotFallBackToAString) {
    for (Utf8CP key : {"customAttributes", "indexes", "enumerators", "instance"}) {
        SCOPED_TRACE(key);
        BeJsDocument out(R"({"old":true})");
        ExpectInvalid(NormalizeSchemaCommandArgs({{key, "[{"}}, out), key);
        EXPECT_TRUE(out["old"].asBool());
    }
    BeJsDocument out;
    ExpectInvalid(NormalizeSchemaCommandArgs({{"indexes", R"([{"name":"first","NAME":"second"}])"}}, out), "indexes[0].NAME");
}

TEST(SchemaCommandArgsTests, NativeJsonMustBeAnObjectAndHaveUniqueKeysAtEveryDepth) {
    for (Utf8CP text : {"null", "[]", "true", "5", R"("text")"}) {
        BeJsDocument json(text);
        ExpectInvalid(ValidateSchemaCommandJson(json), "args");
    }
    bvector<std::pair<Utf8CP, Utf8CP>> cases{
        {R"({"name":"A","name":"B"})", "name"},
        {R"({"name":"A","NAME":"B"})", "NAME"},
        {R"({"source":{"classes":[],"CLASSES":[]}})", "source.CLASSES"},
        {R"({"indexes":[{"name":"a"},{"properties":[],"Properties":[]}]})", "indexes[1].Properties"},
        {R"({"customAttributes":[{"className":"A","values":{"a":1,"A":2}}]})", "customAttributes[0].values.A"},
        {R"({"source":{"classes":[]},"source.classes":[]})", "source.classes"},
        {R"({"SOURCE.CLASSES":[],"source":{"classes":[]}})", "SOURCE.CLASSES"}};
    for (auto const& entry : cases) {
        SCOPED_TRACE(entry.first);
        BeJsDocument json(entry.first);
        ASSERT_FALSE(json.hasParseError());
        ExpectInvalid(ValidateSchemaCommandJson(json), entry.second);
        SchemaCommandResult result;
        SchemaCommandArgParser parser(json, result);
        EXPECT_FALSE(parser.CheckKeys({"name", "source.classes", "indexes", "customAttributes"}));
        EXPECT_FALSE(parser.Has("name"));
        ExpectInvalid(result, entry.second);
    }
    BeJsDocument valid(R"({"instance":{"Value":null},"enumerators":[{"name":"A","value":1}]})");
    EXPECT_TRUE(ValidateSchemaCommandJson(valid).IsOk());
}

TEST(SchemaCommandArgsTests, CheckKeysRejectsUnknownNestedKeysAndIncorrectContainers) {
    BeJsDocument json(R"({"schema":"Test","source":{"classes":[],"typo":true},"unknown":1})");
    SchemaCommandResult result;
    SchemaCommandArgParser parser(json, result);
    EXPECT_FALSE(parser.CheckKeys({"schema", "source.classes"}));
    ASSERT_EQ(2u, result.GetIssues().size());
    EXPECT_STREQ("source.typo", result.GetIssues()[0].arg.c_str());
    EXPECT_STREQ("unknown", result.GetIssues()[1].arg.c_str());

    BeJsDocument scalar(R"({"source":5})");
    SchemaCommandResult scalarResult;
    SchemaCommandArgParser scalarParser(scalar, scalarResult);
    EXPECT_FALSE(scalarParser.CheckKeys({"source.classes"}));
    ExpectInvalid(scalarResult, "source");

    BeJsDocument dotted(R"({"source.classes":[]})");
    SchemaCommandResult dottedResult;
    SchemaCommandArgParser dottedParser(dotted, dottedResult);
    EXPECT_FALSE(dottedParser.CheckKeys({"source.classes"}));
    ExpectInvalid(dottedResult, "source.classes");
}

TEST(SchemaCommandArgsTests, ExplicitlyAllowedStructuredKeysOwnTheirSubtrees) {
    BeJsDocument json(R"({"INSTANCE":{"className":"Test.CA","Value":1},"source":{"CLASSES":[]}})");
    SchemaCommandResult result;
    SchemaCommandArgParser parser(json, result);
    EXPECT_TRUE(parser.CheckKeys({"instance", "source.classes"}));
    EXPECT_TRUE(parser.Has("instance"));
    EXPECT_TRUE(parser.Has("SOURCE.classes"));
    EXPECT_TRUE(parser.Value("instance").isObject());
    EXPECT_TRUE(parser.Value("missing").isNull());
    EXPECT_TRUE(parser.Value("missing.child").isNull());
    EXPECT_TRUE(parser.Value("instance.Value.child").isNull());
    EXPECT_TRUE(result.IsOk());
}

TEST(SchemaCommandArgsTests, NestedContainerTypesAreCheckedWithoutCoercion) {
    BeJsDocument json(R"({"source":{"classes":{}},"target":{"classes":[{"name":"Test.A"}]}})");
    SchemaCommandResult result;
    SchemaCommandArgParser parser(json, result);
    ASSERT_TRUE(parser.CheckKeys({"source.classes", "target.classes"}));
    bvector<Utf8String> classes{"unchanged"};
    EXPECT_FALSE(parser.StringList("source.classes", classes));
    EXPECT_FALSE(parser.StringList("target.classes", classes));
    ASSERT_EQ(2u, result.GetIssues().size());
    EXPECT_STREQ("source.classes", result.GetIssues()[0].arg.c_str());
    EXPECT_STREQ("target.classes[0]", result.GetIssues()[1].arg.c_str());
    ASSERT_EQ(1u, classes.size());
    EXPECT_STREQ("unchanged", classes[0].c_str());
}

TEST(SchemaCommandArgsTests, CommandEnvelopeAllowsOpaqueArgsAndNestedWhitelistsRemainStrict) {
    BeJsDocument envelope(R"({"COMMAND":"CreateRelationshipClass","ARGS":{
        "schema":"Test","source":{"classes":["Test:A"],"unexpected":true}}})");
    SchemaCommandResult envelopeResult;
    SchemaCommandArgParser envelopeParser(envelope, envelopeResult);
    ASSERT_TRUE(envelopeParser.CheckKeys({"command", "args"}));
    Utf8String command;
    ASSERT_TRUE(envelopeParser.String("command", command, true));
    EXPECT_STREQ("CreateRelationshipClass", command.c_str());
    auto args = envelopeParser.Value("args");
    ASSERT_TRUE(args.isObject());
    EXPECT_TRUE(envelopeResult.IsOk());

    SchemaCommandResult opaqueResult;
    SchemaCommandArgParser opaqueParser(args, opaqueResult);
    EXPECT_TRUE(opaqueParser.CheckKeys({"schema", "source"}));
    EXPECT_TRUE(opaqueParser.Value("source").isObject());
    EXPECT_TRUE(opaqueResult.IsOk());

    SchemaCommandResult strictResult;
    SchemaCommandArgParser strictParser(args, strictResult);
    EXPECT_FALSE(strictParser.CheckKeys({"schema", "source.classes"}));
    ExpectInvalid(strictResult, "source.unexpected");
    ASSERT_EQ(1u, strictResult.GetIssues().size());
}

TEST(SchemaCommandArgsTests, MissingOptionalArgumentsLeaveOutputsUnchanged) {
    BeJsDocument json("{}");
    SchemaCommandResult result;
    SchemaCommandArgParser parser(json, result);
    Utf8String text("keep");
    bool flag = true;
    uint32_t count = 42;
    bvector<Utf8String> list{"keep"};
    EXPECT_TRUE(parser.String("name", text));
    EXPECT_TRUE(parser.Boolean("flag", flag));
    EXPECT_TRUE(parser.UInt32("count", count));
    EXPECT_TRUE(parser.StringList("list", list));
    EXPECT_STREQ("keep", text.c_str());
    EXPECT_TRUE(flag);
    EXPECT_EQ(42u, count);
    ASSERT_EQ(1u, list.size());
    EXPECT_STREQ("keep", list[0].c_str());
    EXPECT_TRUE(result.IsOk());
    EXPECT_FALSE(parser.String("name", text, true));
    EXPECT_FALSE(parser.Boolean("flag", flag, true));
    EXPECT_FALSE(parser.UInt32("count", count, true));
    EXPECT_FALSE(parser.StringList("list", list, true));
    ExpectInvalid(result, "name");
    EXPECT_EQ(4u, result.GetIssues().size());
}

TEST(SchemaCommandArgsTests, NullIsPresentButNeverCoerced) {
    BeJsDocument json(R"({"value":null})");
    SchemaCommandResult result;
    SchemaCommandArgParser parser(json, result);
    EXPECT_TRUE(parser.Has("VALUE"));
    Utf8String text("unchanged");
    bool flag = true;
    uint32_t count = 9;
    bvector<Utf8String> list{"unchanged"};
    EXPECT_FALSE(parser.String("value", text));
    EXPECT_FALSE(parser.Boolean("value", flag));
    EXPECT_FALSE(parser.UInt32("value", count));
    EXPECT_FALSE(parser.StringList("value", list));
    EXPECT_STREQ("unchanged", text.c_str());
    EXPECT_TRUE(flag);
    EXPECT_EQ(9u, count);
    ASSERT_EQ(1u, list.size());
    EXPECT_EQ(4u, result.GetIssues().size());
}

TEST(SchemaCommandArgsTests, NestedOptionalReadsRejectPresentNonObjectParents) {
    for (Utf8CP text : {"5", "null", "[]", R"("value")"}) {
        BeJsDocument json;
        json.SetEmptyObject();
        BeJsDocument value(text);
        json["source"].From(value);
        SchemaCommandResult result;
        SchemaCommandArgParser parser(json, result);
        Utf8String out("unchanged");
        EXPECT_FALSE(parser.String("source.multiplicity", out));
        ExpectInvalid(result, "source");
        EXPECT_STREQ("unchanged", out.c_str());
    }
}

TEST(SchemaCommandArgsTests, EmbeddedNulCannotTruncateTypedStringReads) {
    BeJsDocument json(R"({"count":"1\u0000x"})");
    SchemaCommandResult result;
    SchemaCommandArgParser parser(json, result);
    uint32_t count = 9;
    EXPECT_FALSE(parser.UInt32("count", count));
    ExpectInvalid(result, "count");
    EXPECT_EQ(9u, count);

    BeJsDocument literal(R"({"name":"\\u0000"})");
    EXPECT_TRUE(ValidateSchemaCommandJson(literal).IsOk());
    BeJsDocument out;
    ExpectInvalid(NormalizeSchemaCommandArgs({{"instance", R"({"count":"1\u0000x"})"}}, out), "instance.count");
}

TEST(SchemaCommandArgsTests, ScalarsAreStrictAndBooleanStringsAreRecognized) {
    BeJsDocument json(R"({"yes":true,"no":"false","literal":"true","name":"Name","number":1,"object":{}})");
    SchemaCommandResult result;
    SchemaCommandArgParser parser(json, result);
    bool flag = false;
    ASSERT_TRUE(parser.Boolean("YES", flag));
    EXPECT_TRUE(flag);
    ASSERT_TRUE(parser.Boolean("no", flag));
    EXPECT_FALSE(flag);
    ASSERT_TRUE(parser.Boolean("literal", flag));
    EXPECT_TRUE(flag);
    Utf8String name;
    ASSERT_TRUE(parser.String("NAME", name));
    EXPECT_STREQ("Name", name.c_str());
    EXPECT_FALSE(parser.String("yes", name));
    EXPECT_FALSE(parser.String("number", name));
    EXPECT_FALSE(parser.Boolean("number", flag));
    EXPECT_FALSE(parser.Boolean("object", flag));
    EXPECT_EQ(4u, result.GetIssues().size());
    EXPECT_STREQ("Name", name.c_str());
    EXPECT_TRUE(flag);
}

TEST(SchemaCommandArgsTests, BooleanStringsMustMatchAnEntireLowercaseLiteral) {
    for (Utf8CP text : {"TRUE", "False", "1", "0", " true", "false ", "truex", ""}) {
        SCOPED_TRACE(text);
        BeJsDocument json;
        json.SetEmptyObject();
        json["flag"] = text;
        SchemaCommandResult result;
        SchemaCommandArgParser parser(json, result);
        bool flag = true;
        EXPECT_FALSE(parser.Boolean("flag", flag));
        EXPECT_TRUE(flag);
        ExpectInvalid(result, "flag");
    }
}

TEST(SchemaCommandArgsTests, UInt32ChecksTheEntireStringAndNumericRange) {
    for (Utf8CP text : {"0", "4294967295", "1.0", R"("0")", R"("4294967295")"}) {
        SCOPED_TRACE(text);
        BeJsDocument json;
        json.SetEmptyObject();
        BeJsDocument value(text);
        json["value"].From(value);
        SchemaCommandResult result;
        SchemaCommandArgParser parser(json, result);
        uint32_t number = 7;
        EXPECT_TRUE(parser.UInt32("value", number));
        EXPECT_TRUE(result.IsOk());
    }
    for (Utf8CP text : {"-1", "4294967296", "1.5", "true", "[]", "{}",
                        R"("")", R"("-1")", R"("+1")", R"(" 1")", R"("1 ")",
                        R"("1x")", R"("1.0")", R"("1e2")", R"("0x10")", R"("4294967296")"}) {
        SCOPED_TRACE(text);
        BeJsDocument json;
        json.SetEmptyObject();
        BeJsDocument value(text);
        json["value"].From(value);
        SchemaCommandResult result;
        SchemaCommandArgParser parser(json, result);
        uint32_t number = 7;
        EXPECT_FALSE(parser.UInt32("value", number));
        EXPECT_EQ(7u, number);
        ExpectInvalid(result, "value");
    }
}

TEST(SchemaCommandArgsTests, StringListsValidateEachElementWithoutPartialOutput) {
    BeJsDocument json(R"({"array":["a","b"],"csv":" a, b ","empty":"","bad":["a",false,2],"badCsv":"a,,b"})");
    SchemaCommandResult result;
    SchemaCommandArgParser parser(json, result);
    bvector<Utf8String> list;
    ASSERT_TRUE(parser.StringList("array", list));
    ASSERT_EQ(2u, list.size());
    EXPECT_STREQ("a", list[0].c_str());
    ASSERT_TRUE(parser.StringList("csv", list));
    ASSERT_EQ(2u, list.size());
    EXPECT_STREQ("b", list[1].c_str());
    EXPECT_FALSE(parser.StringList("bad", list));
    ASSERT_EQ(2u, list.size());
    ASSERT_EQ(2u, result.GetIssues().size());
    EXPECT_STREQ("bad[1]", result.GetIssues()[0].arg.c_str());
    EXPECT_STREQ("bad[2]", result.GetIssues()[1].arg.c_str());
    EXPECT_FALSE(parser.StringList("badCsv", list));
    EXPECT_EQ(2u, list.size());
    ASSERT_TRUE(parser.StringList("empty", list));
    EXPECT_TRUE(list.empty());
}

TEST(SchemaCommandArgsTests, ResultsSerializeIssuesFailureContextAndBlockingInstances) {
    SchemaCommandResult success;
    BeJsDocument json;
    success.ToJson(json);
    EXPECT_STREQ("OK", json["status"].asCString());
    EXPECT_TRUE(json["issues"].isArray());
    EXPECT_TRUE(json["blockingInstances"].isArray());
    EXPECT_EQ(0u, json["issues"].size());
    EXPECT_FALSE(json.hasMember("failedCommand"));

    auto result = SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_DATA_DELETION_REQUIRED,
                                               "SchemaEditor.DataDeletionRequired", "class", "Instances must be removed first.");
    result.AddIssue({"SchemaEditor.Validation", "schema", "Additional detail."});
    result.SetFailedCommand("DropEntityClass", 2);
    ECInstanceKeyMultiMap instances;
    instances.insert(ECInstanceKeyMultiMapPair(ECN::ECClassId(uint64_t(1)), ECInstanceId(uint64_t(2))));
    instances.insert(ECInstanceKeyMultiMapPair(ECN::ECClassId(uint64_t(1)), ECInstanceId(uint64_t(3))));
    result.SetBlockingInstances(instances);
    EXPECT_FALSE(result.IsOk());
    EXPECT_EQ(2u, result.GetBlockingInstances().size());
    result.ToJson(json);
    EXPECT_STREQ("ERROR_DATA_DELETION_REQUIRED", json["status"].asCString());
    EXPECT_STREQ("SchemaEditor.DataDeletionRequired", json["issues"][0]["code"].asCString());
    EXPECT_STREQ("class", json["issues"][0]["arg"].asCString());
    EXPECT_STREQ("Instances must be removed first.", json["issues"][0]["message"].asCString());
    EXPECT_EQ(2u, json["issues"].size());
    EXPECT_STREQ("DropEntityClass", json["failedCommand"].asCString());
    EXPECT_TRUE(json["failedCommandIndex"].isNumeric());
    EXPECT_EQ(2, json["failedCommandIndex"].asInt());
    EXPECT_EQ(2u, json["blockingInstances"].size());
    EXPECT_STREQ("0x1", json["blockingInstances"][0]["classId"].asCString());
    EXPECT_STREQ("0x2", json["blockingInstances"][0]["instanceId"].asCString());
    EXPECT_STREQ("0x3", json["blockingInstances"][1]["instanceId"].asCString());

    success.AddIssue({"SchemaEditor.Error", "", "A failure cannot appear successful."});
    EXPECT_FALSE(success.IsOk());
    EXPECT_EQ(SchemaCommandResult::Status::ERROR, success.GetStatus());
}

TEST(SchemaCommandArgsTests, CopyingResultsDoesNotAliasMutableState) {
    auto original = SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_INVALID_ARGS,
                                                 "SchemaEditor.InvalidArgs", "name", "A name is required.");
    original.SetFailedCommand("CreateEntityClass", 1);
    ECInstanceKeyMultiMap instances;
    instances.insert(ECInstanceKeyMultiMapPair(ECN::ECClassId(uint64_t(1)), ECInstanceId(uint64_t(2))));
    original.SetBlockingInstances(instances);
    auto copy = original;
    copy.AddIssue({"SchemaEditor.InvalidArgs", "schema", "A schema is required."});
    copy.SetFailedCommand("AddProperty", 3);
    copy.SetBlockingInstances({});
    SchemaCommandResult assigned;
    assigned = original;
    assigned.AddIssue({"SchemaEditor.InvalidArgs", "kind", "A kind is required."});
    EXPECT_EQ(1u, original.GetIssues().size());
    EXPECT_EQ(2u, copy.GetIssues().size());
    EXPECT_EQ(2u, assigned.GetIssues().size());
    EXPECT_EQ(1u, original.GetBlockingInstances().size());
    EXPECT_TRUE(copy.GetBlockingInstances().empty());
    BeJsDocument json;
    original.ToJson(json);
    EXPECT_STREQ("CreateEntityClass", json["failedCommand"].asCString());
    EXPECT_EQ(1, json["failedCommandIndex"].asInt());
    copy.ToJson(json);
    EXPECT_STREQ("AddProperty", json["failedCommand"].asCString());
    EXPECT_EQ(3, json["failedCommandIndex"].asInt());
}

END_ECDBUNITTESTS_NAMESPACE
