/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SchemaUpdateTestFixture.h"
#include <BeRapidJson/BeJsValue.h>
#include <Bentley/Base64Utilities.h>
#include <GeomSerialization/GeomSerializationApi.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Property types and kinds, enumerations, KindOfQuantities, units, formats and DateTimeInfo.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PropertyTypeUpdateTests : SchemaUpdateTestFixture
    {
    static SchemaUpdateBaseline const& GetBaseline()
        {
        static SchemaUpdateBaseline s_baseline("PropertyTypes", 
        R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA" />
                <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
                <ECSchemaReference name="SchemaUpgradeCustomAttributes" version="01.00.00" alias="SchemaUpgradeCA" />
                <ECEnumeration typeName="Status" backingTypeName="int" isStrict="false">
                    <ECEnumerator name="Active" value="1" />
                    <ECEnumerator name="Retired" value="2" />
                </ECEnumeration>
                <ECEnumeration typeName="Switch" backingTypeName="int" isStrict="true">
                    <ECEnumerator name="On" value="1" />
                    <ECEnumerator name="Off" value="2" />
                </ECEnumeration>
                <ECEnumeration typeName="Color" backingTypeName="string" isStrict="false">
                    <ECEnumerator name="Red" value="red" />
                    <ECEnumerator name="Blue" value="blue" />
                </ECEnumeration>
                <ECEnumeration typeName="Shade" backingTypeName="string" isStrict="true">
                    <ECEnumerator name="Light" value="light" />
                    <ECEnumerator name="Dark" value="dark" />
                </ECEnumeration>
                <ECEnumeration typeName="Phase" backingTypeName="int" isStrict="false">
                    <ECEnumerator name="Design" value="1" />
                    <ECEnumerator name="Build" value="2" />
                </ECEnumeration>
                <KindOfQuantity typeName="Length" persistenceUnit="u:M" relativeError="0.001" presentationUnits="f:DefaultRealU(4)[u:M]" />
                <KindOfQuantity typeName="LengthInFeet" persistenceUnit="u:FT" relativeError="0.001" presentationUnits="f:DefaultRealU(4)[u:FT]" />
                <KindOfQuantity typeName="Angle" persistenceUnit="u:RAD" relativeError="0.001" presentationUnits="f:DefaultRealU(4)[u:ARC_DEG]" />
                <KindOfQuantity typeName="SiteLength" persistenceUnit="SiteMeter" relativeError="0.001" />
                <UnitSystem typeName="SiteSystem" />
                <UnitSystem typeName="SpareSystem" />
                <Phenomenon typeName="SitePhenomenon" definition="u:LENGTH" />
                <Phenomenon typeName="SparePhenomenon" definition="u:LENGTH" />
                <Unit typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" />
                <Unit typeName="UnusedUnit" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" numerator="2.0" />
                <Format typeName="SiteFormat" type="decimal" precision="4" formatTraits="keepSingleZero|keepDecimalPoint" />
                <Format typeName="UnusedFormat" type="decimal" precision="2" />
                <Format typeName="CompositeFormat" type="decimal" precision="2">
                    <Composite spacer=" ">
                        <Unit label="m">SiteMeter</Unit>
                    </Composite>
                </Format>
                <PropertyCategory typeName="Dimensions" priority="1" />
                <PropertyCategory typeName="Metadata" priority="2" />
                <ECStructClass typeName="Extent">
                    <ECProperty propertyName="Width" typeName="double" />
                    <ECProperty propertyName="Height" typeName="double" />
                </ECStructClass>
                <ECStructClass typeName="Extent3d">
                    <ECProperty propertyName="Width" typeName="double" />
                    <ECProperty propertyName="Height" typeName="double" />
                    <ECProperty propertyName="Depth" typeName="double" />
                </ECStructClass>
                <ECEntityClass typeName="Element" modifier="Abstract">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00.00">
                            <MaxSharedColumnsBeforeOverflow>6</MaxSharedColumnsBeforeOverflow>
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                        </ShareColumns>
                        <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />
                    </ECCustomAttributes>
                    <ECProperty propertyName="Code" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="Widget">
                    <BaseClass>Element</BaseClass>
                    <ECProperty propertyName="Flag" typeName="boolean" />
                    <ECProperty propertyName="Payload" typeName="binary" />
                    <ECProperty propertyName="Recorded" typeName="dateTime" />
                    <ECProperty propertyName="Weight" typeName="double" />
                    <ECProperty propertyName="Count" typeName="int" />
                    <ECProperty propertyName="BigCount" typeName="long" />
                    <ECProperty propertyName="Origin" typeName="point2d" />
                    <ECProperty propertyName="Position" typeName="point3d" />
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECProperty propertyName="Shape" typeName="Bentley.Geometry.Common.IGeometry" />
                </ECEntityClass>
                <ECEntityClass typeName="Gadget">
                    <BaseClass>Element</BaseClass>
                    <ECProperty propertyName="State" typeName="Status" />
                    <ECProperty propertyName="Power" typeName="Switch" />
                    <ECProperty propertyName="Paint" typeName="Color" />
                    <ECArrayProperty propertyName="Counts" typeName="int" />
                    <ECArrayProperty propertyName="Tags" typeName="string" />
                    <ECArrayProperty propertyName="Bounded" typeName="int" minOccurs="1" maxOccurs="5" />
                    <ECStructProperty propertyName="Size" typeName="Extent" />
                    <ECStructArrayProperty propertyName="Sizes" typeName="Extent" />
                    <ECNavigationProperty propertyName="Owner" relationshipName="WidgetOwnsGadgets" direction="Backward" />
                </ECEntityClass>
                <ECRelationshipClass typeName="WidgetOwnsGadgets" strength="referencing" modifier="Sealed">
                    <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="false">
                        <Class class="Widget" />
                    </Source>
                    <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="false">
                        <Class class="Gadget" />
                    </Target>
                </ECRelationshipClass>
                <ECEntityClass typeName="Sensor">
                    <BaseClass>Element</BaseClass>
                    <ECProperty propertyName="Span" typeName="double" kindOfQuantity="Length" />
                    <ECProperty propertyName="Tilt" typeName="double" kindOfQuantity="Angle" />
                    <ECProperty propertyName="SiteSpan" typeName="double" kindOfQuantity="SiteLength" />
                    <ECProperty propertyName="Reading" typeName="double" minimumValue="0" maximumValue="1000" category="Dimensions" />
                    <ECProperty propertyName="Label" typeName="string" maximumLength="100" />
                    <ECProperty propertyName="InstalledOn" typeName="dateTime">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00.03">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                    <ECProperty propertyName="CalibratedOn" typeName="dateTime">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00.03">
                                <DateTimeComponent>Date</DateTimeComponent>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECEntityClass>
                <ECEntityClass typeName="Note" modifier="Sealed">
                    <ECCustomAttributes>
                        <DbIndexList xmlns="ECDbMap.02.00.00">
                            <Indexes>
                                <DbIndex>
                                    <Name>ix_note_priority</Name>
                                    <Properties>
                                        <string>Priority</string>
                                    </Properties>
                                </DbIndex>
                            </Indexes>
                        </DbIndexList>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Text" typeName="string" />
                    <ECProperty propertyName="Priority" typeName="int" />
                    <ECProperty propertyName="CreatedOn" typeName="dateTime" />
                </ECEntityClass>
            </ECSchema>)xml");
        return s_baseline;
        }

    //! Expects the import to be accepted, then verifies all data.
    void ExpectAccepted(Utf8StringCR schemaXml, SchemaVersionChange versionChange = SchemaVersionChange::Minor, ExpectedDataChanges const& expectedChanges = ExpectedDataChanges(),
        SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade)
        {
        EXPECT_EQ(SUCCESS, UpdateSchema(schemaXml, versionChange, options));
        VerifyData(expectedChanges);
        }

    //! Expects the import to be rejected with the given issue, then verifies that nothing changed.
    void ExpectRejected(Utf8StringCR schemaXml, Utf8CP issueId, SchemaVersionChange versionChange = SchemaVersionChange::Minor,
        SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade)
        {
        EXPECT_EQ(ERROR, UpdateSchema(schemaXml, versionChange, options));
        ExpectRejectionReason(issueId);
        VerifyData();
        }

    static SchemaManager::SchemaImportOptions DoNotFail() { return SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade | SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications; }

    //! The options every iModel schema import uses (DgnDomains::DoImportSchemas), plus the data transform flag DgnDb sets when the schema lock is held.
    static SchemaManager::SchemaImportOptions IModelImportOptions()
        {
        return SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade | SchemaManager::SchemaImportOptions::AllowMajorSchemaUpgradeForDynamicSchemas
            | SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade;
        }

    //! Marks the schema as dynamic. SchemaWriter checks the flag on the incoming schema, so adding it in the same import counts.
    static void MakeDynamic(Utf8StringR schemaXml)
        {
        ReplaceInSchema(schemaXml, R"xml(<ECSchemaReference name="SchemaUpgradeCustomAttributes" version="01.00.00" alias="SchemaUpgradeCA" />)xml",
            R"xml(<ECSchemaReference name="SchemaUpgradeCustomAttributes" version="01.00.00" alias="SchemaUpgradeCA" />
                <ECCustomAttributes>
                    <DynamicSchema xmlns="CoreCustomAttributes.01.00.03" />
                </ECCustomAttributes>)xml");
        }

    //! Replaces the type of a property. The property element must start with <ECProperty propertyName="X" typeName="old".
    static void ChangePropertyType(Utf8StringR schemaXml, Utf8CP className, Utf8CP propertyName, Utf8CP oldType, Utf8CP newType)
        {
        Utf8PrintfString oldText(R"xml(propertyName="%s" typeName="%s")xml", propertyName, oldType);
        Utf8PrintfString newText(R"xml(propertyName="%s" typeName="%s")xml", propertyName, newType);
        ReplaceInClass(schemaXml, className, oldText.c_str(), newText.c_str());
        }

    //! Replaces a whole XML element, from the start tag that begins with @p startTagPrefix up to and including
    //! the next @p endTag. Independent of the indentation in the baseline, unlike ReplaceInSchema with a multi-line text.
    static void ReplaceElement(Utf8StringR schemaXml, Utf8CP startTagPrefix, Utf8CP endTag, Utf8CP newXml)
        {
        const size_t start = schemaXml.find(startTagPrefix);
        ASSERT_NE(Utf8String::npos, start) << "Start tag '" << startTagPrefix << "' not found in the schema XML.";
        ASSERT_EQ(Utf8String::npos, schemaXml.find(startTagPrefix, start + 1)) << "Start tag '" << startTagPrefix << "' is not unique in the schema XML.";
        const size_t endTagStart = schemaXml.find(endTag, start);
        ASSERT_NE(Utf8String::npos, endTagStart) << "End tag '" << endTag << "' not found after '" << startTagPrefix << "'.";
        schemaXml.replace(start, endTagStart + strlen(endTag) - start, newXml);
        }

    //! Removes the Status enumeration from the schema XML.
    static void RemoveStatusEnumeration(Utf8StringR schemaXml) { ReplaceElement(schemaXml, R"xml(<ECEnumeration typeName="Status")xml", "</ECEnumeration>", ""); }

    //! Schema items for the unit algebra tests (U13, U14, U28-U30). Added by an update of their own, so the baseline stays small.
    static Utf8CP ConstantAndInvertedUnitXml()
        {
        return R"xml(<Constant typeName="SiteConstant" phenomenon="SitePhenomenon" definition="u:M" numerator="3.0" />
            <InvertedUnit typeName="SiteInverse" invertsUnit="SiteMeter" unitSystem="SiteSystem" />)xml";
        }

    //! A composite format with three unit slots (major, middle, minor) for the composite slot tests (F24, F25, F31).
    static Utf8CP ThreeUnitCompositeFormatXml()
        {
        return R"xml(<Format typeName="SiteComposite3" type="decimal" precision="2"><Composite spacer=" "><Unit label="m">u:M</Unit><Unit label="cm">u:CM</Unit><Unit label="mm">u:MM</Unit></Composite></Format>)xml";
        }

    //! Imports the baseline plus @p itemsXml as a first, accepted update and returns the XML that is now persisted.
    //! The test then modifies that XML for the update it is about.
    Utf8String ImportBaselinePlus(Utf8CP itemsXml)
        {
        Utf8String schemaXml = GetBaselineSchemaXml();
        AppendToSchema(schemaXml, itemsXml);
        EXPECT_EQ(SUCCESS, UpdateSchema(schemaXml));
        return schemaXml;
        }

    //! Replaces a dateTime property that carries a DateTimeInfo custom attribute by a plain one.
    static void RemoveDateTimeInfo(Utf8StringR schemaXml, Utf8CP propertyName)
        {
        Utf8PrintfString startTag(R"xml(<ECProperty propertyName="%s" typeName="dateTime">)xml", propertyName);
        Utf8PrintfString plainProperty(R"xml(<ECProperty propertyName="%s" typeName="dateTime" />)xml", propertyName);
        ReplaceElement(schemaXml, startTag.c_str(), "</ECProperty>", plainProperty.c_str());
        }

    // Value converters, expressed as JSON the way JsonECSqlSelectAdapter renders it.
    static Utf8String ToJson(double value) { BeJsDocument json; json = value; return json.Stringify(); }
    static Utf8String ToJson(int64_t value) { BeJsDocument json; json = value; return json.Stringify(); }
    // Stringify() renders a bare string value without quotes, ECDb's JSON adapter output has them.
    static Utf8String ToJson(Utf8CP value) { return Utf8String("\"") + value + "\""; }
    static Utf8String Unquote(Utf8StringCR json) { return json.size() >= 2 && json[0] == '"' ? Utf8String(json.substr(1, json.size() - 2)) : Utf8String(json); }

    static Utf8String IntegerAsDouble(Utf8StringCR json) { return ToJson((double) std::stoll(json.c_str())); }
    static Utf8String IntegerAsString(Utf8StringCR json) { return ToJson(json.c_str()); }
    static Utf8String BooleanAsInteger(Utf8StringCR json) { return ToJson((int64_t) (json == "true" ? 1 : 0)); }
    // Leaves values that are not integers as they are, so the comparison reports them instead of throwing.
    static Utf8String StringAsInteger(Utf8StringCR json)
        {
        Utf8String text = Unquote(json);
        char* end = nullptr;
        const int64_t value = std::strtoll(text.c_str(), &end, 10);
        return (end != text.c_str() && *end == '\0') ? ToJson(value) : json;
        }

    // What SQLite makes of the old bytes when ECDb reads them under the new type.
    static Utf8String StringReadAsInteger(Utf8StringCR json) { Utf8String converted = StringAsInteger(json); return converted == json ? ToJson((int64_t) 0) : converted; }
    // SQLite converts text to a number by its numeric prefix, so "true" reads as 0 and only "1" (or any other non-zero number) as true.
    static Utf8String StringReadAsBoolean(Utf8StringCR json) { return StringReadAsInteger(json) == ToJson((int64_t) 0) ? "false" : "true"; }
    static Utf8String DoubleReadAsInteger(Utf8StringCR json) { return ToJson((int64_t) std::stod(json.c_str())); }
    // sqlite3_column_int keeps the low 32 bits of a stored 64-bit integer.
    static Utf8String LongReadAsInteger(Utf8StringCR json) { return ToJson((int64_t) (int32_t) std::stoll(json.c_str())); }
    // Inline struct value rendered as {"Height":9.5,"Width":8.5}: the Width number is read as an int, the fraction is dropped.
    static Utf8String StructWidthReadAsInteger(Utf8StringCR json)
        {
        const Utf8String key = "\"Width\":";
        const size_t keyPosition = json.find(key);
        if (keyPosition == Utf8String::npos)
            return json;

        const size_t numberStart = keyPosition + key.size();
        char* numberEnd = nullptr;
        const double width = std::strtod(json.c_str() + numberStart, &numberEnd);
        const size_t numberLength = (size_t) (numberEnd - (json.c_str() + numberStart));
        return json.substr(0, numberStart) + ToJson((int64_t) width) + json.substr(numberStart + numberLength);
        }
    static Utf8String StringReadAsBinary(Utf8StringCR json) { Utf8String text = Unquote(json); Utf8String base64; Base64Utilities::Encode(base64, (Byte const*) text.c_str(), text.size()); return ToJson(("encoding=base64;" + base64).c_str()); }
    // The JSON adapter renders a blob as "encoding=base64;<base64>".
    static Utf8String BinaryToBase64Json(Byte const* bytes, size_t size) { Utf8String base64; Base64Utilities::Encode(base64, bytes, size); return ToJson(("encoding=base64;" + base64).c_str()); }
    static bvector<Byte> Base64JsonToBinary(Utf8StringCR json)
        {
        Utf8String text = Unquote(json);
        const Utf8CP prefix = "encoding=base64;";
        if (text.StartsWith(prefix))
            text = text.substr(strlen(prefix));
        bvector<Byte> bytes;
        Base64Utilities::Decode(bytes, text);
        return bytes;
        }
    // sqlite3_column_text hands the blob bytes out as they are. Only valid for blobs that hold UTF-8 text without NUL bytes.
    static Utf8String BinaryReadAsString(Utf8StringCR json) { bvector<Byte> bytes = Base64JsonToBinary(json); return ToJson(Utf8String((Utf8CP) bytes.data(), bytes.size()).c_str()); }
    // IGeometry is stored as a flatbuffer blob. Read as binary, that blob is rendered as base64.
    static Utf8String GeometryReadAsBinary(Utf8StringCR json)
        {
        BeJsDocument geometryJson;
        geometryJson.Parse(json);
        IGeometryPtr geometry = ECJsonUtilities::JsonToIGeometry(geometryJson);
        if (geometry == nullptr)
            return json;
        bvector<Byte> bytes;
        BentleyGeometryFlatBuffer::GeometryToBytes(*geometry, bytes);
        return BinaryToBase64Json(bytes.data(), bytes.size());
        }
    // A blob that holds a geometry flatbuffer, read as IGeometry, is rendered as geometry JSON.
    static Utf8String BinaryReadAsGeometry(Utf8StringCR json)
        {
        bvector<Byte> bytes = Base64JsonToBinary(json);
        IGeometryPtr geometry = BentleyGeometryFlatBuffer::BytesToGeometrySafe(bytes.data(), bytes.size(), true);
        if (geometry == nullptr)
            return json;
        BeJsDocument geometryJson;
        ECJsonUtilities::IGeometryToJson(geometryJson, *geometry);
        return geometryJson.Stringify();
        }
    static Utf8String IntegerReadAsDateTime(Utf8StringCR json)
        {
        DateTime dateTime;
        DateTime::FromJulianDay(dateTime, (double) std::stoll(json.c_str()), DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified));
        return ToJson(dateTime.ToString().c_str());
        }
    static Utf8String WithoutUtcMarker(Utf8StringCR json) { Utf8String text = Unquote(json); if (!text.empty() && text[text.size() - 1] == 'Z') text.resize(text.size() - 1); return ToJson(text.c_str()); }
    static Utf8String WithUtcMarker(Utf8StringCR json) { return ToJson((Unquote(json) + "Z").c_str()); }
    static Utf8String DateOnly(Utf8StringCR json) { return ToJson(Unquote(json).substr(0, 10).c_str()); }
    // The JSON adapter renders longs with extended type "Id" as hex strings.
    static Utf8String IntegerAsHexId(Utf8StringCR json) { return ToJson(BeInt64Id((uint64_t) std::stoll(json.c_str())).ToHexStr().c_str()); }

    ECPropertyCP GetProperty(Utf8CP className, Utf8CP propertyName) const
        {
        ECClassCP ecClass = m_ecdb.Schemas().GetClass("TestSchema", className);
        return ecClass != nullptr ? ecClass->GetPropertyP(propertyName) : nullptr;
        }

    ECEnumerationCP GetEnumeration(Utf8CP name) const { return m_ecdb.Schemas().GetEnumeration("TestSchema", name); }
    KindOfQuantityCP GetKindOfQuantity(Utf8CP name) const { return m_ecdb.Schemas().GetKindOfQuantity("TestSchema", name); }

    //! Overwrites Widget.Name of every instance, one ECSQL update each (an UPDATE cannot copy one property into another).
    void SeedWidgetNames(std::function<Utf8String(int)> nameOf)
        {
        bvector<Utf8String> updates;
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Widget ORDER BY ECInstanceId"));
            int index = 0;
            while (BE_SQLITE_ROW == stmt.Step())
                updates.push_back(Utf8PrintfString("UPDATE ts.Widget SET Name = '%s' WHERE ECInstanceId = %s", nameOf(index++).c_str(), stmt.GetValueId<ECInstanceId>(0).ToHexStr().c_str()));
            }
        ASSERT_FALSE(updates.empty());
        for (Utf8StringCR update : updates)
            ChangeDataBeforeUpdate(update.c_str());
        }

    //! Overwrites Widget.Payload of every instance with the given bytes. ECSQL has no blob literal, so this binds
    //! the blobs directly and then re-reads the baseline data the same way ChangeDataBeforeUpdate does.
    void SeedWidgetPayloads(std::function<bvector<Byte>(int)> payloadOf)
        {
        bvector<ECInstanceId> ids;
            {
            ECSqlStatement stmt;
            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ts.Widget ORDER BY ECInstanceId"));
            while (BE_SQLITE_ROW == stmt.Step())
                ids.push_back(stmt.GetValueId<ECInstanceId>(0));
            }
        ASSERT_FALSE(ids.empty());

        ECSqlStatement update;
        ASSERT_EQ(ECSqlStatus::Success, update.Prepare(m_ecdb, "UPDATE ts.Widget SET Payload = ? WHERE ECInstanceId = ?"));
        for (size_t i = 0; i < ids.size(); i++)
            {
            const bvector<Byte> payload = payloadOf((int) i);
            ASSERT_EQ(ECSqlStatus::Success, update.BindBlob(1, payload.data(), (int) payload.size(), IECSqlBinder::MakeCopy::Yes));
            ASSERT_EQ(ECSqlStatus::Success, update.BindId(2, ids[i]));
            ASSERT_EQ(BE_SQLITE_DONE, update.Step());
            update.Reset();
            update.ClearBindings();
            }
        // A no-op statement so the fixture commits and snapshots the data again.
        ChangeDataBeforeUpdate("UPDATE ts.Widget SET Code = Code");
        }

    static bvector<Byte> TextBytes(Utf8StringCR text) { return bvector<Byte>((Byte const*) text.c_str(), (Byte const*) text.c_str() + text.size()); }
    static bvector<Byte> LineGeometryBytes(int index)
        {
        IGeometryPtr geometry = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, (double) index, (double) index + 1.0, (double) index + 2.0)));
        bvector<Byte> bytes;
        BentleyGeometryFlatBuffer::GeometryToBytes(*geometry, bytes);
        return bytes;
        }

    // dateTime values are stored as julian days. Text read under a dateTime type is converted by SQLite to its numeric prefix.
    static Utf8String StringReadAsDateTime(Utf8StringCR json)
        {
        DateTime dateTime;
        DateTime::FromJulianDay(dateTime, std::strtod(Unquote(json).c_str(), nullptr), DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified));
        return ToJson(dateTime.ToString().c_str());
        }
    // A julian day read as text is SQLite's text rendering of the stored real.
    static Utf8String DateTimeReadAsString(Utf8StringCR json)
        {
        DateTime dateTime;
        double julianDay = 0.0;
        DateTime::FromString(dateTime, Unquote(json).c_str());
        dateTime.ToJulianDay(julianDay);
        return ToJson(Utf8PrintfString("%.15g", julianDay).c_str());
        }
    static ExpectedDataChanges::ValueConverter Compose(ExpectedDataChanges::ValueConverter first, ExpectedDataChanges::ValueConverter second)
        {
        return [first, second] (Utf8StringCR json) { return second(first(json)); };
        }
    };

//***************************************************************************************
// Primitive type changes (T01-T08). Every change needs AllowDataTransformDuringSchemaUpgrade
// and a major version bump; ECDb does not look at the stored values.
//***************************************************************************************

//---------------------------------------------------------------------------------------
// T01. int to long is lossless. Values read the same.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, WidenIntToLong)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "long");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// int to double is lossless. Values read as doubles.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, WidenIntToDouble)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "double");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Count", IntegerAsDouble));
    }

//---------------------------------------------------------------------------------------
// int to string is lossless. Values read as their decimal text.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertIntToStringInSharedColumn)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "string");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Count", IntegerAsString));
    }

//---------------------------------------------------------------------------------------
// Same change in a table of its own, where the column has INTEGER affinity.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertIntToStringInOwnTable)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Note", "Priority", "int", "string");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Note", "Priority", IntegerAsString));
    }

//---------------------------------------------------------------------------------------
// boolean to int is lossless: true reads as 1, false as 0.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertBooleanToInt)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Flag", "boolean", "int");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Flag", BooleanAsInteger));
    }

//---------------------------------------------------------------------------------------
// A string to int change is fine ONLY when every stored value is an integer.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertNumericStringsToInt)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    bvector<Utf8String> updates;
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, [Count] FROM ts.Widget"));
        while (BE_SQLITE_ROW == stmt.Step())
            updates.push_back(Utf8PrintfString("UPDATE ts.Widget SET Name = '%d' WHERE ECInstanceId = %s", stmt.GetValueInt(1), stmt.GetValueId<ECInstanceId>(0).ToHexStr().c_str()));
        }
    ASSERT_FALSE(updates.empty());
    for (Utf8StringCR update : updates)
        ChangeDataBeforeUpdate(update.c_str());

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Name", "string", "int");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Name", StringAsInteger));
    }

//---------------------------------------------------------------------------------------
// T01. The stored names are not numbers. Records current behaviour (D2): ECDb accepts the
// change without looking at the data, SQLite reads the old text as 0.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertNonNumericStringsToInt_ReadAsZero)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Name", "string", "int");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Name", StringReadAsInteger));
    }

//---------------------------------------------------------------------------------------
// T01. Same in a table of its own. Records current behaviour (D2): the column keeps its TEXT
// affinity and the values read as 0.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertNonNumericStringsToIntInOwnTable_ReadAsZero)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Note", "Text", "string", "int");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Note", "Text", StringReadAsInteger));
    }

//---------------------------------------------------------------------------------------
// T01. Records current behaviour (D2): double to int is accepted, the fraction is dropped on read.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, NarrowDoubleToInt_FractionDropped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Weight", "double", "int");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Weight", DoubleReadAsInteger));
    }

//---------------------------------------------------------------------------------------
// T01. Records current behaviour (D2): long to int is accepted. The baseline values fit in 32
// bits, so they read the same; a larger value would be read by its low 32 bits.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, NarrowLongToInt)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "BigCount", "long", "int");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "BigCount", LongReadAsInteger));
    }

//---------------------------------------------------------------------------------------
// T01. Records current behaviour (D2): double to long is accepted, the fraction is dropped on read.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, NarrowDoubleToLong_FractionDropped)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Weight", "double", "long");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Weight", DoubleReadAsInteger));
    }

//---------------------------------------------------------------------------------------
// T01. Records current behaviour (D2): int to dateTime is accepted, the ints are read as
// julian days, i.e. dates around 4713 BC. The existing test
// MajorSchemaUpgradeVerifyDataAfterTypeChange asserts the same result.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertIntToDateTime_ReadAsJulianDay)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "dateTime");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Count", IntegerReadAsDateTime));
    }

//---------------------------------------------------------------------------------------
// T01. Records current behaviour (D2): string to binary is accepted, the UTF-8 bytes of the
// old text are read as the blob.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertStringToBinary_ReadAsUtf8Bytes)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Name", "string", "binary");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Name", StringReadAsBinary));
    }

//---------------------------------------------------------------------------------------
// T01. Records current behaviour (D2): binary to string is accepted, the blob bytes are handed
// out as text. Payloads are seeded with UTF-8 text so the result is readable; the generated
// 8-byte integer payloads would contain NUL bytes and read as truncated garbage.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertBinaryToString_ReadAsText)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    SeedWidgetPayloads([] (int index) { return TextBytes(Utf8PrintfString("Payload %d", index)); });

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Payload", "binary", "string");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Payload", BinaryReadAsString));
    }

//---------------------------------------------------------------------------------------
// T01. Records current behaviour (D2): string to boolean is accepted. SQLite converts the text
// by its numeric prefix: "1" reads as true, "0" as false and "true" as false too.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertStringToBoolean_TextTrueReadsAsFalse)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    SeedWidgetNames([] (int index) { static Utf8CP const s_names[] = {"1", "0", "true"}; return Utf8String(s_names[index % 3]); });

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Name", "string", "boolean");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Name", StringReadAsBoolean));
    }

//---------------------------------------------------------------------------------------
// T01. IGeometry to binary is lossless: the stored flatbuffer is read as the blob it is.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertIGeometryToBinary_ReadAsFlatBuffer)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Shape", "Bentley.Geometry.Common.IGeometry", "binary");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Shape", GeometryReadAsBinary));
    }

//---------------------------------------------------------------------------------------
// T01. Binary to IGeometry is fine ONLY when every stored blob is a geometry flatbuffer, so
// the payloads are seeded with one. The generated 8-byte payloads would make the read fail.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertBinaryHoldingFlatBuffersToIGeometry)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    SeedWidgetPayloads(LineGeometryBytes);

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Payload", "binary", "Bentley.Geometry.Common.IGeometry");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Payload", BinaryReadAsGeometry));
    }

//---------------------------------------------------------------------------------------
// T01 (customer case). A string property that holds ISO 8601 date times becomes dateTime.
// Valid for ECDb, but the column is not rewritten: the text is read as a julian day, so the
// customer sees dates around 4707 BC instead of their data. Records current behaviour (D2).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertIsoDateTimeStringsToDateTime_ReadAsWrongDate)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    SeedWidgetNames([] (int index) { return Utf8PrintfString("2020-03-%02dT10:30:00.000Z", 10 + index); });

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Name", "string", "dateTime");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Name", StringReadAsDateTime));
    }

//---------------------------------------------------------------------------------------
// T01. The other direction: dateTime to string. The reals are read as their text, "2451545.25"
// instead of an ISO date. Records current behaviour (D2).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertDateTimeToString_ReadAsJulianDayText)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Recorded", "dateTime", "string");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Recorded", DateTimeReadAsString));
    }

//---------------------------------------------------------------------------------------
// T01, two changes in a row (Affan's case). string -> dateTime -> string where the text already
// is a date time. In between the text is read as a julian day (SQLite takes the numeric prefix
// "2013"), after the way back the original text is there again: nothing was rewritten.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, RoundTripDateTimeTextToDateTimeAndBack_TextRestored)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    SeedWidgetNames([] (int) { return Utf8String("2013-12-05T13:11:22"); });

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Name", "string", "dateTime");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major));
    VerifyData(ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Name", StringReadAsDateTime));

    Utf8String backXml = GetBaselineSchemaXml();
    ASSERT_EQ(SUCCESS, UpdateSchema(backXml, SchemaVersionChange::Major));
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// T01, two changes in a row. int -> string -> int: nothing is rewritten, so the original
// values come back.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, RoundTripIntToStringAndBack_ValuesRestored)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "string");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major));

    Utf8String backXml = GetBaselineSchemaXml();
    ASSERT_EQ(SUCCESS, UpdateSchema(backXml, SchemaVersionChange::Major));
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// T01, two changes in a row. double -> int -> double: the fraction is only hidden while the
// property is an int. Records current behaviour (D2).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, RoundTripDoubleToIntAndBack_FractionRestored)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Weight", "double", "int");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major));

    Utf8String backXml = GetBaselineSchemaXml();
    ASSERT_EQ(SUCCESS, UpdateSchema(backXml, SchemaVersionChange::Major));
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// T01, two changes in a row. string -> int -> string: non-numeric text is read as 0 in
// between, but it is still stored and comes back.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, RoundTripStringToIntAndBack_TextRestored)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Name", "string", "int");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major));

    Utf8String backXml = GetBaselineSchemaXml();
    ASSERT_EQ(SUCCESS, UpdateSchema(backXml, SchemaVersionChange::Major));
    VerifyData();
    }

//---------------------------------------------------------------------------------------
// T01, two changes in a row. int -> string -> dateTime: the stored ints end up read as
// julian days, the same as a direct int -> dateTime change.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChainIntToStringToDateTime_ReadAsJulianDay)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "string");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major));

    Utf8String finalXml = GetBaselineSchemaXml();
    ChangePropertyType(finalXml, "Widget", "Count", "int", "dateTime");
    ASSERT_EQ(SUCCESS, UpdateSchema(finalXml, SchemaVersionChange::Major));
    VerifyData(ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Count", IntegerReadAsDateTime));
    }

//---------------------------------------------------------------------------------------
// T01, rows written between two changes. int -> string, a row with text is added, string -> int:
// the old rows come back as ints, the new row reads 0.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, TextWrittenBetweenTypeChangesReadsAsZero)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "string");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major));
    ChangeDataBeforeUpdate("INSERT INTO ts.Widget (Code, [Count]) VALUES ('late', 'not a number')");

    Utf8String backXml = GetBaselineSchemaXml();
    ASSERT_EQ(SUCCESS, UpdateSchema(backXml, SchemaVersionChange::Major));
    VerifyData(ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Count", StringReadAsInteger));
    }

//---------------------------------------------------------------------------------------
// T02. Points take several columns, nothing can become one.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertingIntToPoint3dIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "point3d");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T02. point2d to string.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertingPoint2dToStringIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Origin", "point2d", "string");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T01. Even a lossless type change is a major change for the schema's consumers.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, TypeChangeWithoutMajorVersionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "long");
    ExpectRejected(schemaXml, "ECDb_0336");
    }

//---------------------------------------------------------------------------------------
// T01. Without rows, no value can become unreadable, so any type change is a metadata change.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertStringToIntWithoutRows)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    // Gadget.Owner points at Widgets, clear it first so check_nav_ids stays clean.
    ChangeDataBeforeUpdate("UPDATE ts.Gadget SET Owner = NULL");
    ChangeDataBeforeUpdate("DELETE FROM ts.Widget");

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Name", "string", "int");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T13. Array element type changes are rejected even when the scalar equivalent is accepted.
// Records current behaviour (D2).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, WideningArrayElementIntToLongIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "Counts", "int", "long");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T14. Array element int to enumeration.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertingArrayElementIntToEnumerationIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "Counts", "int", "Status");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T15. Struct property pointing to a different struct class.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingStructPropertyTypeIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECStructProperty propertyName="Size" typeName="Extent" />)xml", R"xml(<ECStructProperty propertyName="Size" typeName="Extent3d" />)xml");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T16. Struct array property pointing to a different struct class.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingStructArrayPropertyTypeIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECStructArrayProperty propertyName="Sizes" typeName="Extent" />)xml", R"xml(<ECStructArrayProperty propertyName="Sizes" typeName="Extent3d" />)xml");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//***************************************************************************************
// Property kind changes (T17-T23). SchemaWriter rejects every kind change with ECDb_0337.
//***************************************************************************************

//---------------------------------------------------------------------------------------
// T17. primitive -> primitive array
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingPrimitiveToPrimitiveArrayIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Widget", R"xml(<ECProperty propertyName="Count" typeName="int" />)xml", R"xml(<ECArrayProperty propertyName="Count" typeName="int" />)xml");
    ExpectRejected(schemaXml, "ECDb_0337", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T17. primitive array -> primitive
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingPrimitiveArrayToPrimitiveIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECArrayProperty propertyName="Counts" typeName="int" />)xml", R"xml(<ECProperty propertyName="Counts" typeName="int" />)xml");
    ExpectRejected(schemaXml, "ECDb_0337", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T19. primitive -> struct
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingPrimitiveToStructIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Widget", R"xml(<ECProperty propertyName="Count" typeName="int" />)xml", R"xml(<ECStructProperty propertyName="Count" typeName="Extent" />)xml");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T19. struct -> primitive
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingStructToPrimitiveIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECStructProperty propertyName="Size" typeName="Extent" />)xml", R"xml(<ECProperty propertyName="Size" typeName="string" />)xml");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T18. struct -> struct array
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingStructToStructArrayIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECStructProperty propertyName="Size" typeName="Extent" />)xml", R"xml(<ECStructArrayProperty propertyName="Size" typeName="Extent" />)xml");
    ExpectRejected(schemaXml, "ECDb_0337", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T18. struct array -> struct
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingStructArrayToStructIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECStructArrayProperty propertyName="Sizes" typeName="Extent" />)xml", R"xml(<ECStructProperty propertyName="Sizes" typeName="Extent" />)xml");
    ExpectRejected(schemaXml, "ECDb_0337", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T20. primitive array -> struct array
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingPrimitiveArrayToStructArrayIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECArrayProperty propertyName="Counts" typeName="int" />)xml", R"xml(<ECStructArrayProperty propertyName="Counts" typeName="Extent" />)xml");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T23. primitive -> struct array
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingPrimitiveToStructArrayIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Widget", R"xml(<ECProperty propertyName="Count" typeName="int" />)xml", R"xml(<ECStructArrayProperty propertyName="Count" typeName="Extent" />)xml");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T22. primitive array -> struct
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingPrimitiveArrayToStructIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECArrayProperty propertyName="Counts" typeName="int" />)xml", R"xml(<ECStructProperty propertyName="Counts" typeName="Extent" />)xml");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T17. Kind change is rejected before the version check, so even a major bump plus
// DoNotFail does not get it through silently: the property is skipped and reported.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, KindChangeWithDoNotFailKeepsOldProperty)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Widget", R"xml(<ECProperty propertyName="Count" typeName="int" />)xml", R"xml(<ECArrayProperty propertyName="Count" typeName="int" />)xml");
    ExpectRejected(schemaXml, "ECDb_0337", SchemaVersionChange::Major, DoNotFail());
    }

//---------------------------------------------------------------------------------------
// T01 / B7. With DoNotFailForDeletionsOrModifications, the illegal type change is skipped and the
// legal label change on the same property should still be applied.
// DISABLED: UpdateProperty returns before writing anything else, so the label stays "Count".
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DISABLED_SkippedTypeChangeKeepsOtherChangesOfTheProperty)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Widget", R"xml(propertyName="Count" typeName="int")xml", R"xml(propertyName="Count" typeName="string" displayLabel="Item count")xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges(), DoNotFail());

    ECPropertyCP count = GetProperty("Widget", "Count");
    ASSERT_TRUE(count != nullptr);
    EXPECT_STREQ("int", count->GetTypeName().c_str());
    EXPECT_STREQ("Item count", count->GetDisplayLabel().c_str());
    }

//***************************************************************************************
// Enumerations
//***************************************************************************************

//---------------------------------------------------------------------------------------
// T03. A non-strict enumeration accepts any value, so an int property can become one.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertIntToNonStrictEnumeration)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "Status");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// T04. The stored counts are not enumerators of the strict enumeration.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertingIntToStrictEnumerationWithOtherValuesIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "Switch");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T04. All stored values are enumerators of the strict enumeration, yet ECDb rejects the
// change without looking at the data. Records current behaviour (D2).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertingIntToStrictEnumerationIsRejectedEvenWithEnumeratorValues)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ChangeDataBeforeUpdate("UPDATE ts.Widget SET [Count] = 1");

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "Switch");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T07.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertEnumerationToItsBackingType)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "int");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// T08. Without a major version.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertingEnumerationToAnotherBackingTypeIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "string");
    ExpectRejected(schemaXml, "ECDb_0336");
    }

//---------------------------------------------------------------------------------------
// T10. Every stored value is an enumerator of the strict enumeration too.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, SwitchToStrictEnumerationWithEnumeratorValues)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "Switch");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// T10 / B5. 99 is not an enumerator of the strict enumeration Switch. Records current
// behaviour (D2): IsPropertyTypeChangeSupported builds the error message for this case but
// returns true, so ECDb accepts the change and 99 is kept.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, SwitchToStrictEnumerationWithOtherValues_Accepted)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ChangeDataBeforeUpdate("UPDATE ts.Gadget SET State = 99");

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "Switch");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// T10 / B5. The expected behaviour of the test above. Enable once the check is fixed.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DISABLED_SwitchingToStrictEnumerationWithOtherValuesIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ChangeDataBeforeUpdate("UPDATE ts.Gadget SET State = 99");

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "Switch");
    ExpectRejected(schemaXml, "ECDb_0336");
    }

//---------------------------------------------------------------------------------------
// E10.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, AddEnumerator)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Retired" value="2" />)xml", R"xml(<ECEnumerator name="Retired" value="2" /><ECEnumerator name="Planned" value="3" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// E12. Instances use the enumerator.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeletingEnumeratorInUseIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Retired" value="2" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0402", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// E12. No instance uses the enumerator anymore, ECDb still rejects (does not look at data).
// Records current behaviour (D2).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeletingUnusedEnumeratorIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ChangeDataBeforeUpdate("UPDATE ts.Gadget SET State = 1");

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Retired" value="2" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0402", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// E12 / B6. With DoNotFail, ECDb reports that it keeps the enumerator. It has to.
// DISABLED: VerifyEnumeratorChanges logs "enumerator not deleted", then UpdateEnumeration
// stores the new enumerator list without it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DISABLED_SkippedEnumeratorDeletionKeepsTheEnumerator)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Retired" value="2" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges(), DoNotFail());

    ECEnumerationCP status = GetEnumeration("Status");
    ASSERT_TRUE(status != nullptr);
    EXPECT_EQ(2u, status->GetEnumeratorCount());
    }

//---------------------------------------------------------------------------------------
// E12 / B6. Records what DoNotFail does today: the enumerator is gone from the schema while
// the instances still hold its value.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, SkippedEnumeratorDeletion_EnumeratorIsDeletedAnyway)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Retired" value="2" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges(), DoNotFail());

    ECEnumerationCP status = GetEnumeration("Status");
    ASSERT_TRUE(status != nullptr);
    EXPECT_EQ(1u, status->GetEnumeratorCount());
    }

//---------------------------------------------------------------------------------------
// E13.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingEnumeratorValueIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Retired" value="2" />)xml", R"xml(<ECEnumerator name="Retired" value="20" />)xml");
    ExpectRejected(schemaXml, "ECDb_0400", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// E17 / E18. Enumerator label and description are free to change.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeEnumeratorLabelAndDescription)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Retired" value="2" />)xml", R"xml(<ECEnumerator name="Retired" value="2" displayLabel="Retired item" description="No longer in use" />)xml");
    ExpectAccepted(schemaXml);

    ECEnumerationCP status = GetEnumeration("Status");
    ASSERT_TRUE(status != nullptr);
    ECEnumeratorCP retired = status->FindEnumerator(2);
    ASSERT_TRUE(retired != nullptr);
    EXPECT_STREQ("Retired item", retired->GetDisplayLabel().c_str());
    EXPECT_STREQ("No longer in use", retired->GetDescription().c_str());
    }

//---------------------------------------------------------------------------------------
// E07.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, MakeEnumerationNonStrict)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="Switch" backingTypeName="int" isStrict="true")xml", R"xml(typeName="Switch" backingTypeName="int" isStrict="false")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// E08. 99 is not an enumerator.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, MakingEnumerationStrictWithOtherValuesIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ChangeDataBeforeUpdate("UPDATE ts.Gadget SET State = 99");

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="Status" backingTypeName="int" isStrict="false")xml", R"xml(typeName="Status" backingTypeName="int" isStrict="true")xml");
    ExpectRejected(schemaXml, "ECDb_0399", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// E08. All stored values are enumerators, ECDb still rejects (does not look at data).
// Records current behaviour (D2).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, MakingEnumerationStrictIsRejectedEvenWithEnumeratorValues)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="Status" backingTypeName="int" isStrict="false")xml", R"xml(typeName="Status" backingTypeName="int" isStrict="true")xml");
    ExpectRejected(schemaXml, "ECDb_0399", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// E06.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingEnumerationBackingTypeIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="Color" backingTypeName="string" isStrict="false")xml", R"xml(typeName="Color" backingTypeName="int" isStrict="false")xml");
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Red" value="red" />)xml", R"xml(<ECEnumerator name="Red" value="1" />)xml");
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Blue" value="blue" />)xml", R"xml(<ECEnumerator name="Blue" value="2" />)xml");
    ExpectRejected(schemaXml, "ECDb_0397", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// E02 / T07. The enumeration goes, the property keeps its values as plain ints.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeleteEnumerationAndKeepValuesAsInt)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveStatusEnumeration(schemaXml);
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "int");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);

    ECPropertyCP state = GetProperty("Gadget", "State");
    ASSERT_TRUE(state != nullptr && state->GetIsPrimitive());
    EXPECT_EQ(PRIMITIVETYPE_Integer, state->GetAsPrimitiveProperty()->GetType());
    EXPECT_TRUE(state->GetAsPrimitiveProperty()->GetEnumeration() == nullptr);
    }

//---------------------------------------------------------------------------------------
// E02. Deleting an enumeration needs a major version.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeletingEnumerationWithoutMajorVersionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveStatusEnumeration(schemaXml);
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "int");
    ExpectRejected(schemaXml, "ECDb_0678");
    }

//***************************************************************************************
// KindOfQuantity
//***************************************************************************************

//---------------------------------------------------------------------------------------
// P27. Same persistence unit, so the values keep their meaning.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ReplaceKindOfQuantityWithSamePersistenceUnit)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(<KindOfQuantity typeName="Distance" persistenceUnit="u:M" relativeError="0.01" presentationUnits="f:DefaultRealU(2)[u:KM]" />)xml");
    ReplaceInClass(schemaXml, "Sensor", R"xml(kindOfQuantity="Length")xml", R"xml(kindOfQuantity="Distance")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// P28. Meters are not feet.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ReplacingKindOfQuantityWithOtherPersistenceUnitIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(kindOfQuantity="Length")xml", R"xml(kindOfQuantity="LengthInFeet")xml");
    ExpectRejected(schemaXml, "ECDb_0344", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// P28. AllowUnitChange is the explicit opt-in. Values are not converted, by design.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ReplaceKindOfQuantityInPropertyPersistenceUnitWithAllowUnitChange)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(<ECProperty propertyName="Span" typeName="double" kindOfQuantity="Length" />)xml",
        R"xml(<ECProperty propertyName="Span" typeName="double" kindOfQuantity="LengthInFeet">
            <ECCustomAttributes>
                <AllowUnitChange xmlns="SchemaUpgradeCustomAttributes.01.00.00">
                    <From>u:M</From>
                    <To>u:FT</To>
                </AllowUnitChange>
            </ECCustomAttributes>
        </ECProperty>)xml");
    ExpectAccepted(schemaXml);

    ECPropertyCP span = GetProperty("Sensor", "Span");
    ASSERT_TRUE(span != nullptr && span->GetKindOfQuantity() != nullptr);
    EXPECT_STREQ("LengthInFeet", span->GetKindOfQuantity()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// P28. The opt-in names the wrong unit, so it does not apply.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, AllowUnitChangeWithWrongFromUnitIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(<ECProperty propertyName="Span" typeName="double" kindOfQuantity="Length" />)xml",
        R"xml(<ECProperty propertyName="Span" typeName="double" kindOfQuantity="LengthInFeet">
            <ECCustomAttributes>
                <AllowUnitChange xmlns="SchemaUpgradeCustomAttributes.01.00.00">
                    <From>u:KM</From>
                    <To>u:FT</To>
                </AllowUnitChange>
            </ECCustomAttributes>
        </ECProperty>)xml");
    ExpectRejected(schemaXml, "ECDb_0344");
    }

//---------------------------------------------------------------------------------------
// P29. Removing the KoQ needs AllowUnitChange.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, RemovingKindOfQuantityWithoutAllowUnitChangeIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml( kindOfQuantity="Length")xml", "");
    ExpectRejected(schemaXml, "ECDb_0344");
    }

//---------------------------------------------------------------------------------------
// P29. AllowUnitChange with an empty To unit.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, RemoveKindOfQuantityWithAllowUnitChange)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(<ECProperty propertyName="Span" typeName="double" kindOfQuantity="Length" />)xml",
        R"xml(<ECProperty propertyName="Span" typeName="double">
            <ECCustomAttributes>
                <AllowUnitChange xmlns="SchemaUpgradeCustomAttributes.01.00.00">
                    <From>u:M</From>
                    <To></To>
                </AllowUnitChange>
            </ECCustomAttributes>
        </ECProperty>)xml");
    ExpectAccepted(schemaXml);

    ECPropertyCP span = GetProperty("Sensor", "Span");
    ASSERT_TRUE(span != nullptr);
    EXPECT_TRUE(span->GetKindOfQuantity() == nullptr);
    }

//---------------------------------------------------------------------------------------
// P26. Assigns a unit meaning to existing numbers, no rescaling.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, AddKindOfQuantityToProperty)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(propertyName="Reading" typeName="double")xml", R"xml(propertyName="Reading" typeName="double" kindOfQuantity="Length")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// K13. Changing the persistence unit of the KoQ itself changes the meaning of every property using it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingKindOfQuantityPersistenceUnitIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="Length" persistenceUnit="u:M")xml", R"xml(typeName="Length" persistenceUnit="u:FT")xml");
    ExpectRejected(schemaXml, "ECDb_0391", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// K12 / K17. Relative error and presentation formats are display metadata.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeKindOfQuantityRelativeErrorAndPresentationFormats)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="Length" persistenceUnit="u:M" relativeError="0.001" presentationUnits="f:DefaultRealU(4)[u:M]")xml",
        R"xml(typeName="Length" persistenceUnit="u:M" relativeError="0.01" presentationUnits="f:DefaultRealU(2)[u:M];f:DefaultRealU(2)[u:CM]")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// K08. No property uses LengthInFeet.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeleteUnusedKindOfQuantity)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<KindOfQuantity typeName="LengthInFeet" persistenceUnit="u:FT" relativeError="0.001" presentationUnits="f:DefaultRealU(4)[u:FT]" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// K08. Deleting an unused KoQ touches no data, yet ECDb requires a major version for every
// KoQ deletion. Records current behaviour (D2).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeletingUnusedKindOfQuantityWithoutMajorVersionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<KindOfQuantity typeName="LengthInFeet" persistenceUnit="u:FT" relativeError="0.001" presentationUnits="f:DefaultRealU(4)[u:FT]" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0677");
    }

//---------------------------------------------------------------------------------------
// K16. Replace the presentation format list entirely.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ReplaceKindOfQuantityPresentationFormats)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(presentationUnits="f:DefaultRealU(4)[u:M]")xml", R"xml(presentationUnits="f:DefaultRealU(2)[u:CM]")xml");
    ExpectAccepted(schemaXml);

    KindOfQuantityCP length = GetKindOfQuantity("Length");
    ASSERT_TRUE(length != nullptr);
    EXPECT_EQ(1u, length->GetPresentationFormats().size());
    }

//---------------------------------------------------------------------------------------
// K15. Remove all presentation formats.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, RemoveKindOfQuantityPresentationFormats)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml( presentationUnits="f:DefaultRealU(4)[u:M]")xml", "");
    ExpectAccepted(schemaXml);

    KindOfQuantityCP length = GetKindOfQuantity("Length");
    ASSERT_TRUE(length != nullptr);
    EXPECT_TRUE(length->GetPresentationFormats().empty());
    }

// //---------------------------------------------------------------------------------------
// // P29 / B9. Removing the KoQ while AllowUnitChange names a non-empty To unit. SchemaWriter
// // dereferences the (missing) new KoQ to compare it against To.
// // DISABLED: may crash (null deref) instead of reporting ECDb_0344.
// // Already confirmed and reported by Rob!
// // @bsimethod
// //+---------------+---------------+---------------+---------------+---------------+------
// TEST_F(PropertyTypeUpdateTests, DISABLED_RemovingKindOfQuantityWithNonEmptyToUnitIsRejected)
//     {
//     ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

//     Utf8String schemaXml = GetBaselineSchemaXml();
//     ReplaceInClass(schemaXml, "Sensor", R"xml(<ECProperty propertyName="Span" typeName="double" kindOfQuantity="Length" />)xml",
//         R"xml(<ECProperty propertyName="Span" typeName="double">
//             <ECCustomAttributes>
//                 <AllowUnitChange xmlns="SchemaUpgradeCustomAttributes.01.00.00">
//                     <From>u:M</From>
//                     <To>u:FT</To>
//                 </AllowUnitChange>
//             </ECCustomAttributes>
//         </ECProperty>)xml");
//     ExpectRejected(schemaXml, "ECDb_0344");
//     }

//---------------------------------------------------------------------------------------
// K13 / K11 / B7. With DoNotFail, the illegal persistence unit change is skipped and the legal
// description change on the same KoQ should be applied.
// DISABLED: UpdateKindOfQuantities skips the whole KoQ, the description stays empty.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DISABLED_SkippedPersistenceUnitChangeKeepsOtherChangesOfTheKindOfQuantity)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="Length" persistenceUnit="u:M")xml", R"xml(typeName="Length" description="Horizontal extent" persistenceUnit="u:FT")xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges(), DoNotFail());

    KindOfQuantityCP length = GetKindOfQuantity("Length");
    ASSERT_TRUE(length != nullptr);
    EXPECT_STREQ("u:M", length->GetPersistenceUnit()->GetQualifiedName(length->GetSchema()).c_str());
    EXPECT_STREQ("Horizontal extent", length->GetDescription().c_str());
    }

//***************************************************************************************
// DateTimeInfo (X08). SchemaWriter treats DateTimeInfo as a plain custom attribute: every
// change is accepted, no data is touched, the values are just read differently afterwards.
// All tests in this section record current behaviour (D2) unless marked DISABLED_.
//***************************************************************************************

//---------------------------------------------------------------------------------------
// X08. Utc -> Unspecified: the values lose their Z marker on read.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeDateTimeKindUtcToUnspecified_ValuesLoseUtcMarker)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", "<DateTimeKind>Utc</DateTimeKind>", "<DateTimeKind>Unspecified</DateTimeKind>");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges().ConvertValuesOf("TestSchema:Sensor", "InstalledOn", WithoutUtcMarker));
    }

//---------------------------------------------------------------------------------------
// X08. Removing the custom attribute has the same effect.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, RemoveDateTimeInfo_ValuesLoseUtcMarker)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveDateTimeInfo(schemaXml, "InstalledOn");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges().ConvertValuesOf("TestSchema:Sensor", "InstalledOn", WithoutUtcMarker));
    }

//---------------------------------------------------------------------------------------
// X08. Declaring plain values as UTC after the fact: they gain a Z marker on read.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, AddDateTimeInfoUtc_ValuesGainUtcMarker)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="CreatedOn" typeName="dateTime" />)xml",
        R"xml(<ECProperty propertyName="CreatedOn" typeName="dateTime">
            <ECCustomAttributes>
                <DateTimeInfo xmlns="CoreCustomAttributes.01.00.03">
                    <DateTimeKind>Utc</DateTimeKind>
                </DateTimeInfo>
            </ECCustomAttributes>
        </ECProperty>)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges().ConvertValuesOf("TestSchema:Note", "CreatedOn", WithUtcMarker));
    }

//---------------------------------------------------------------------------------------
// X08. Without rows, DateTimeInfo is metadata.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeDateTimeKindWithoutRows)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ChangeDataBeforeUpdate("DELETE FROM ts.Sensor");

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", "<DateTimeKind>Utc</DateTimeKind>", "<DateTimeKind>Unspecified</DateTimeKind>");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// X08. ECDb cannot bind local date times at all, so a property must not become one.
// DISABLED: accepted, and the write probe fails afterwards with "ECDb does not support to
// bind local date times".
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DISABLED_ChangingDateTimeKindToLocalIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", "<DateTimeKind>Utc</DateTimeKind>", "<DateTimeKind>Local</DateTimeKind>");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// X08. Date-only values are valid date times, so widening the component is lossless.
// The adapter renders the values with a time of day afterwards.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, WidenDateTimeComponentToDateAndTime)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveDateTimeInfo(schemaXml, "CalibratedOn");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor,
        ExpectedDataChanges().ConvertValuesOf("TestSchema:Sensor", "CalibratedOn", [] (Utf8StringCR json) { return ToJson((Unquote(json) + "T00:00:00.000").c_str()); }));
    }

//---------------------------------------------------------------------------------------
// X08. Stored values have a time of day, which a date-only property cannot show. Accepted,
// the time of day is hidden on read (still stored).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, NarrowDateTimeComponentToDate_TimeOfDayHidden)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="CreatedOn" typeName="dateTime" />)xml",
        R"xml(<ECProperty propertyName="CreatedOn" typeName="dateTime">
            <ECCustomAttributes>
                <DateTimeInfo xmlns="CoreCustomAttributes.01.00.03">
                    <DateTimeComponent>Date</DateTimeComponent>
                </DateTimeInfo>
            </ECCustomAttributes>
        </ECProperty>)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges().ConvertValuesOf("TestSchema:Note", "CreatedOn", DateOnly));
    }

//***************************************************************************************
// Units, formats, phenomena, unit systems
//***************************************************************************************

//---------------------------------------------------------------------------------------
// U12 / F01.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, AddUnitAndFormat)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(<Unit typeName="SiteDecimeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="SiteMeter" numerator="0.1" />
        <Format typeName="SiteFormatCompact" type="decimal" precision="1" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// U15. Nothing references UnusedUnit, ECDb still rejects every unit deletion, also with a
// major version. Records current behaviour (D2).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeletingUnusedUnitIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Unit typeName="UnusedUnit" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" numerator="2.0" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0416", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// F02. Records current behaviour (D2): every format deletion is rejected.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeletingUnusedFormatIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Format typeName="UnusedFormat" type="decimal" precision="2" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0421", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// U19. SiteMeter is the persistence unit of SiteLength, which Sensor.SiteSpan uses. Redefining
// it changes what every stored SiteSpan means.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, RedefiningPersistenceUnitIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M")xml",
        R"xml(typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:FT")xml");
    ExpectRejected(schemaXml, "ECDb_0419", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// U19. UnusedUnit is no persistence unit, ECDb still rejects every definition change.
// Records current behaviour (D2).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, RedefiningUnusedUnitIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(definition="u:M" numerator="2.0")xml", R"xml(definition="u:FT" numerator="2.0")xml");
    ExpectRejected(schemaXml, "ECDb_0419", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// U23. Numerator change of a unit.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeUnitNumerator)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(definition="u:M" numerator="2.0")xml", R"xml(definition="u:M" numerator="3.0")xml");
    ExpectRejected(schemaXml, "ECDb_0419", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// U20. Moving a unit to another phenomenon.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeUnitPhenomenon)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="UnusedUnit" phenomenon="SitePhenomenon")xml", R"xml(typeName="UnusedUnit" phenomenon="u:LENGTH")xml");
    ExpectRejected(schemaXml, "ECDb_0419", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// F09. Formats do not touch stored values.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeFormatPrecision)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="SiteFormat" type="decimal" precision="4")xml", R"xml(typeName="SiteFormat" type="decimal" precision="6")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// F08. Format type decimal -> fractional.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeFormatType)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="SiteFormat" type="decimal" precision="4")xml", R"xml(typeName="SiteFormat" type="fractional" precision="8")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// F04 / F05. Format label and description.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeFormatLabelAndDescription)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="SiteFormat" type="decimal")xml", R"xml(typeName="SiteFormat" displayLabel="Site format" description="Used on site" type="decimal")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// F20. Adding a composite to a format counts as a composite unit change.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, AddingFormatCompositeIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Format typeName="UnusedFormat" type="decimal" precision="2" />)xml",
        R"xml(<Format typeName="UnusedFormat" type="decimal" precision="2">
            <Composite spacer=" ">
                <Unit label="m">SiteMeter</Unit>
            </Composite>
        </Format>)xml");
    ExpectRejected(schemaXml, "ECDb_0424", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// F22. Composite spacer is metadata.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeFormatCompositeSpacer)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Composite spacer=" ">)xml", R"xml(<Composite spacer="-">)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// F27. Composite unit label is metadata.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeFormatCompositeUnitLabel)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Unit label="m">SiteMeter</Unit>)xml", R"xml(<Unit label="meter">SiteMeter</Unit>)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// F23. Swapping the composite unit.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingFormatCompositeUnitIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Unit label="m">SiteMeter</Unit>)xml", R"xml(<Unit label="m">UnusedUnit</Unit>)xml");
    ExpectRejected(schemaXml, "ECDb_0424", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// F14. Format traits are part of the numeric spec, metadata.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeFormatTraits)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(formatTraits="keepSingleZero|keepDecimalPoint")xml", R"xml(formatTraits="trailZeroes")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// U17. Unit label is metadata.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeUnitLabel)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="SiteMeter" phenomenon)xml", R"xml(typeName="SiteMeter" displayLabel="site m" phenomenon)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// U11. Phenomenon definition change.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingPhenomenonDefinitionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Phenomenon typeName="SparePhenomenon" definition="u:LENGTH" />)xml", R"xml(<Phenomenon typeName="SparePhenomenon" definition="u:LENGTH(2)" />)xml");
    ExpectRejected(schemaXml, "ECDb_0409", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// U09. Phenomenon label is metadata.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangePhenomenonLabel)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Phenomenon typeName="SitePhenomenon" definition="u:LENGTH" />)xml", R"xml(<Phenomenon typeName="SitePhenomenon" displayLabel="Site length" definition="u:LENGTH" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// U07. Nothing references SparePhenomenon, deletion is still rejected.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeletingUnusedPhenomenonIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Phenomenon typeName="SparePhenomenon" definition="u:LENGTH" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0406", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// U04. Unit system label is metadata.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeUnitSystemLabel)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<UnitSystem typeName="SiteSystem" />)xml", R"xml(<UnitSystem typeName="SiteSystem" displayLabel="Site" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// U01. Add a unit system.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, AddUnitSystem)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(<UnitSystem typeName="OtherSystem" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// U02. Nothing references SpareSystem, deletion is still rejected.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeletingUnusedUnitSystemIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<UnitSystem typeName="SpareSystem" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0411", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// U21. Moving a unit to another unit system.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeUnitSystemOfUnit)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="UnusedUnit" phenomenon="SitePhenomenon" unitSystem="SiteSystem")xml", R"xml(typeName="UnusedUnit" phenomenon="SitePhenomenon" unitSystem="u:SI")xml");
    ExpectRejected(schemaXml, "ECDb_0419", SchemaVersionChange::Major);
    }

//***************************************************************************************
// Min/max, length, category
//***************************************************************************************

//---------------------------------------------------------------------------------------
// P21. ECDb does not enforce min/max values, so the bounds are metadata even when stored values
// fall outside the new range. Whether that is the behaviour we want is a spike question.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, NarrowMaximumValueBelowStoredValues)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(minimumValue="0" maximumValue="1000")xml", R"xml(minimumValue="0" maximumValue="1")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// P19. Same for the maximum length of strings.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, NarrowMaximumLengthBelowStoredValues)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(maximumLength="100")xml", R"xml(maximumLength="2")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// P25.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangePropertyCategory)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(category="Dimensions")xml", R"xml(category="Metadata")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// K02 / P25.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeletePropertyCategoryAndRemoveItsReference)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    // K02: a category is deletable once nothing references it. A dangling reference is dropped by ECObjects on load.
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<PropertyCategory typeName="Dimensions" priority="1" />)xml", "");
    ReplaceInClass(schemaXml, "Sensor", R"xml( category="Dimensions")xml", "");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// K02. Nothing references the category.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeleteUnusedPropertyCategory)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<PropertyCategory typeName="Metadata" priority="2" />)xml", "");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// K02. Deleting a category that is still referenced: ECObjects drops the dangling reference
// on load, so ECDb sees "category deleted" plus "Sensor.Reading lost its category". With
// DoNotFail the deletion is skipped, the category stays.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DISABLED_DeletingReferencedPropertyCategoryWithDoNotFailKeepsCategory)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<PropertyCategory typeName="Dimensions" priority="1" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges(), DoNotFail());

    EXPECT_TRUE(m_ecdb.Schemas().GetPropertyCategory("TestSchema", "Dimensions") != nullptr);
    }

//***************************************************************************************
// Extended type names (P24), struct members
//***************************************************************************************

//---------------------------------------------------------------------------------------
// P24. Plain extended type names are metadata.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, AddPlainExtendedTypeName)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Widget", R"xml(propertyName="Name" typeName="string")xml", R"xml(propertyName="Name" typeName="string" extendedTypeName="Json")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// P24. "Id" changes how longs are bound and rendered. Records current behaviour (D2): the
// change is accepted and the stored longs are rendered as hex ids afterwards.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, AddIdExtendedTypeName_ValuesRenderedAsHex)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Widget", R"xml(propertyName="BigCount" typeName="long")xml", R"xml(propertyName="BigCount" typeName="long" extendedTypeName="Id")xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor);
    }

//---------------------------------------------------------------------------------------
// P24. "BeGuid" on a binary property expects 16-byte blobs. The generated payloads are not,
// so the adapter cannot render them as GUIDs afterwards. Records current behaviour (D2).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, AddBeGuidExtendedTypeName_Accepted)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Widget", R"xml(propertyName="Payload" typeName="binary")xml", R"xml(propertyName="Payload" typeName="binary" extendedTypeName="BeGuid")xml");
    EXPECT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Minor));

    ECPropertyCP payload = GetProperty("Widget", "Payload");
    ASSERT_TRUE(payload != nullptr);
    EXPECT_STREQ("BeGuid", payload->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str());
    }

//---------------------------------------------------------------------------------------
// T01 on a struct member. Extent.Width is mapped once per use (Gadget.Size, Gadget.Sizes
// is JSON). Same rule as a scalar: widening is accepted with a major version.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DISABLED_ChangeStructMemberTypeDoubleToString)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Extent", "Width", "double", "string");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T01 on a struct member, lossy. Records current behaviour (D2).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DISABLED_ChangeStructMemberTypeDoubleToInt)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Extent", "Width", "double", "int");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Gadget", "Size", StructWidthReadAsInteger));
    }

//---------------------------------------------------------------------------------------
// Struct member type change without a major version.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, StructMemberTypeChangeWithoutMajorVersionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Extent", "Width", "double", "string");
    ExpectRejected(schemaXml, "ECDb_0336");
    }

//***************************************************************************************
// Matrix 3.3: editing a reused struct definition. Extent is used inline (Gadget.Size, flattened
// columns) and as a struct array (Gadget.Sizes, one JSON column).
//***************************************************************************************

//! Removes every "name":<scalar> member from a JSON text, together with one adjacent comma.
static Utf8String RemoveJsonMember(Utf8StringCR json, Utf8CP name)
    {
    Utf8String result = json;
    const Utf8String key = Utf8PrintfString("\"%s\":", name);
    size_t position = 0;
    while ((position = result.find(key)) != Utf8String::npos)
        {
        size_t valueEnd = position + key.size();
        while (valueEnd < result.size() && result[valueEnd] != ',' && result[valueEnd] != '}')
            valueEnd++;

        size_t eraseStart = position;
        size_t eraseEnd = valueEnd;
        if (eraseEnd < result.size() && result[eraseEnd] == ',')
            eraseEnd++;
        else if (eraseStart > 0 && result[eraseStart - 1] == ',')
            eraseStart--;
        result.erase(eraseStart, eraseEnd - eraseStart);
        }
    return result;
    }

//---------------------------------------------------------------------------------------
// 3.3 add member. Existing rows (inline columns and JSON elements) have no value for it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, AddStructMember_ExistingValuesHaveNoMember)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceElement(schemaXml, R"xml(<ECStructClass typeName="Extent">)xml", "</ECStructClass>",
        R"xml(<ECStructClass typeName="Extent">
                    <ECProperty propertyName="Width" typeName="double" />
                    <ECProperty propertyName="Height" typeName="double" />
                    <ECProperty propertyName="Depth" typeName="double" />
                </ECStructClass>)xml");
    ExpectAccepted(schemaXml);

    ECPropertyCP depth = GetProperty("Extent", "Depth");
    ASSERT_TRUE(depth != nullptr);
    EXPECT_EQ(PRIMITIVETYPE_Double, depth->GetAsPrimitiveProperty()->GetType());
    }

//---------------------------------------------------------------------------------------
// 3.3 delete member. Needs gate G, so a minor version is rejected.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeletingStructMemberWithoutMajorVersionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceElement(schemaXml, R"xml(<ECStructClass typeName="Extent">)xml", "</ECStructClass>",
        R"xml(<ECStructClass typeName="Extent">
                    <ECProperty propertyName="Width" typeName="double" />
                </ECStructClass>)xml");
    ExpectRejected(schemaXml, "ECDb_0384");
    }

//---------------------------------------------------------------------------------------
// 3.3 delete member. With a major version it is accepted. The member disappears from both use
// sites on read. The matrix says the use-site ec_PropertyPath row for Size.Height survives
// (orphaned), recorded here.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeleteStructMember_UseSitePropertyPathIsOrphaned)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceElement(schemaXml, R"xml(<ECStructClass typeName="Extent">)xml", "</ECStructClass>",
        R"xml(<ECStructClass typeName="Extent">
                    <ECProperty propertyName="Width" typeName="double" />
                </ECStructClass>)xml");
    auto withoutHeight = [] (Utf8StringCR json) { return RemoveJsonMember(json, "Height"); };
    ExpectAccepted(schemaXml, SchemaVersionChange::Major,
        ExpectedDataChanges().ConvertValuesOf("TestSchema:Gadget", "Size", withoutHeight).ConvertValuesOf("TestSchema:Gadget", "Sizes", withoutHeight));

    EXPECT_TRUE(GetProperty("Extent", "Height") == nullptr);

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ec_PropertyPath WHERE AccessString='Size.Height'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(1, stmt.GetValueInt(0)) << "Matrix 3.3: the property path of the deleted member is expected to survive.";
    }

//---------------------------------------------------------------------------------------
// 3.3 delete member with DoNotFail. The matrix says the deletion is skipped (Info ECDb_0641),
// so Height stays in the persisted struct and the data is unchanged.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeletingStructMemberWithDoNotFailKeepsMember)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceElement(schemaXml, R"xml(<ECStructClass typeName="Extent">)xml", "</ECStructClass>",
        R"xml(<ECStructClass typeName="Extent">
                    <ECProperty propertyName="Width" typeName="double" />
                </ECStructClass>)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges(), DoNotFail());

    EXPECT_TRUE(GetProperty("Extent", "Height") != nullptr);
    }

//---------------------------------------------------------------------------------------
// 3.3 delete then re-add the member. Unproven in the matrix: does the re-added member reuse the
// orphaned column and show the stale values again? Both imports must succeed; what the rows show
// afterwards is logged, not asserted.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ReAddDeletedStructMember_StaleValuesObserved)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String withoutHeight = GetBaselineSchemaXml();
    ReplaceElement(withoutHeight, R"xml(<ECStructClass typeName="Extent">)xml", "</ECStructClass>",
        R"xml(<ECStructClass typeName="Extent">
                    <ECProperty propertyName="Width" typeName="double" />
                </ECStructClass>)xml");
    ASSERT_EQ(SUCCESS, UpdateSchema(withoutHeight, SchemaVersionChange::Major, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    Utf8String withHeightAgain = GetBaselineSchemaXml();
    ASSERT_EQ(SUCCESS, UpdateSchema(withHeightAgain, SchemaVersionChange::Minor, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.Gadget WHERE Size.Height IS NOT NULL"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    GTEST_LOG_(INFO) << "Gadget rows showing a Size.Height after delete and re-add: " << stmt.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// 3.3 member label, category and range annotations are metadata.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeStructMemberLabelCategoryAndRange)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceElement(schemaXml, R"xml(<ECStructClass typeName="Extent">)xml", "</ECStructClass>",
        R"xml(<ECStructClass typeName="Extent">
                    <ECProperty propertyName="Width" typeName="double" displayLabel="Breadth" category="Dimensions" minimumValue="0" maximumValue="100" />
                    <ECProperty propertyName="Height" typeName="double" />
                </ECStructClass>)xml");
    ExpectAccepted(schemaXml);

    ECPropertyCP width = GetProperty("Extent", "Width");
    ASSERT_TRUE(width != nullptr);
    EXPECT_STREQ("Breadth", width->GetDisplayLabel().c_str());
    }

//***************************************************************************************
// Batch A: gaps from the matrix cross-check. Primitive <-> enumeration with a different
// backing type (T05, T06, T08), enumeration <-> enumeration (T09, T11, T12), the reverse
// directions of the kind changes (T20-T23), array bounds (A01-A04), unit defaults made
// explicit (U22, U24, U26) and length/value facets (P18, P20).
//***************************************************************************************

//---------------------------------------------------------------------------------------
// T05. int -> non-strict string enumeration. The stored integers are read as text.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertIntToNonStrictStringEnumeration)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "Color");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Count", IntegerAsString));
    }

//---------------------------------------------------------------------------------------
// T05. Without a major version.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertingIntToNonStrictStringEnumerationWithoutMajorVersionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "Color");
    ExpectRejected(schemaXml, "ECDb_0336");
    }

//---------------------------------------------------------------------------------------
// T06. int -> strict string enumeration. No stored integer can be an enumerator.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertingIntToStrictStringEnumerationIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "Shade");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T05 on a string property: string -> non-strict int enumeration. Text that is not a number
// reads as 0 afterwards.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertStringToNonStrictIntEnumeration_ReadAsZero)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Name", "string", "Status");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Name", StringReadAsInteger));
    }

//---------------------------------------------------------------------------------------
// T08. int enumeration -> string, with a major version. Enumerator values read as text.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertEnumerationToOtherPrimitiveWithMajorVersion)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "string");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Gadget", "State", IntegerAsString));
    }

//---------------------------------------------------------------------------------------
// T08. int enumeration -> double. Enumerator values read as doubles.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertEnumerationToDouble)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "double");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Gadget", "State", IntegerAsDouble));
    }

//---------------------------------------------------------------------------------------
// T09. Non-strict int enumeration -> another non-strict int enumeration. Same bytes, new
// meaning. No major version needed.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, SwitchToOtherNonStrictEnumerationWithSameBackingType)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "Phase");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// T09. Values outside both enumerations are kept as they are.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, SwitchToOtherNonStrictEnumerationKeepsForeignValues)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ChangeDataBeforeUpdate("UPDATE ts.Gadget SET State = 99");

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "Phase");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// T11. int enumeration -> non-strict string enumeration, major version. Ints read as text.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertEnumerationToNonStrictEnumerationWithOtherBackingType)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "Color");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Gadget", "State", IntegerAsString));
    }

//---------------------------------------------------------------------------------------
// T11. Without a major version.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertingEnumerationToNonStrictEnumerationWithOtherBackingTypeWithoutMajorVersionIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "Color");
    ExpectRejected(schemaXml, "ECDb_0336");
    }

//---------------------------------------------------------------------------------------
// T12. int enumeration -> strict string enumeration. Rejected even with a major version.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ConvertingEnumerationToStrictEnumerationWithOtherBackingTypeIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "Shade");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T20. struct array -> primitive array.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingStructArrayToPrimitiveArrayIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECStructArrayProperty propertyName="Sizes" typeName="Extent" />)xml", R"xml(<ECArrayProperty propertyName="Sizes" typeName="int" />)xml");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T22. struct -> primitive array. The type name changes too, so 0336 wins over 0337.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingStructToPrimitiveArrayIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECStructProperty propertyName="Size" typeName="Extent" />)xml", R"xml(<ECArrayProperty propertyName="Size" typeName="double" />)xml");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T23. struct array -> primitive. The type name changes too, so 0336 wins over 0337.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingStructArrayToPrimitiveIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECStructArrayProperty propertyName="Sizes" typeName="Extent" />)xml", R"xml(<ECProperty propertyName="Sizes" typeName="string" />)xml");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T21. navigation -> primitive. The FK column holds ids, so long would be the natural target.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingNavigationToPrimitiveIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECNavigationProperty propertyName="Owner" relationshipName="WidgetOwnsGadgets" direction="Backward" />)xml", R"xml(<ECProperty propertyName="Owner" typeName="long" />)xml");
    ExpectRejected(schemaXml, "ECDb_0337", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T21. primitive -> navigation. A new relationship is added for the navigation property.
// A navigation property has type long, so from int the type check (0336) fires before the
// kind check. ChangingNavigationToPrimitiveIsRejected goes long -> long and gets 0337.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingPrimitiveToNavigationIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="Priority" typeName="int" />)xml", R"xml(<ECNavigationProperty propertyName="Priority" relationshipName="WidgetHasNotes" direction="Backward" />)xml");
    ReplaceInSchema(schemaXml, "</ECSchema>", R"xml(    <ECRelationshipClass typeName="WidgetHasNotes" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="has" polymorphic="false">
            <Class class="Widget" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="belongs to" polymorphic="false">
            <Class class="Note" />
        </Target>
    </ECRelationshipClass>
</ECSchema>)xml");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// T21 / kind change on a navigation property: changing its relationship.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingNavigationRelationshipIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(relationshipName="WidgetOwnsGadgets" direction="Backward")xml", R"xml(relationshipName="WidgetHoldsGadgets" direction="Backward")xml");
    ReplaceInSchema(schemaXml, "</ECSchema>", R"xml(    <ECRelationshipClass typeName="WidgetHoldsGadgets" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="holds" polymorphic="false">
            <Class class="Widget" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is held by" polymorphic="false">
            <Class class="Gadget" />
        </Target>
    </ECRelationshipClass>
</ECSchema>)xml");
    ExpectRejected(schemaXml, "ECDb_0339", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// A01. Increasing minOccurs narrows the array bounds.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, IncreasingArrayMinOccursIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(minOccurs="1" maxOccurs="5")xml", R"xml(minOccurs="2" maxOccurs="5")xml");
    ExpectRejected(schemaXml, "ECDb_0338", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// A02. Decreasing minOccurs widens the array bounds. SchemaWriter allows it; the matrix
// says "No". Records current behaviour.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DecreaseArrayMinOccurs)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(minOccurs="1" maxOccurs="5")xml", R"xml(minOccurs="0" maxOccurs="5")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// A03. Increasing a finite maxOccurs widens the array bounds.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, IncreaseArrayMaxOccurs)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(minOccurs="1" maxOccurs="5")xml", R"xml(minOccurs="1" maxOccurs="10")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// A03. maxOccurs finite -> unbounded is a widening and is accepted. (Predicted a rejection from
// reading SchemaWriter, where a NULL new maximum counts as narrowing; ECObjects evidently does
// not hand the writer a NULL for "unbounded".)
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangeArrayMaxOccursToUnbounded)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(minOccurs="1" maxOccurs="5")xml", R"xml(minOccurs="1" maxOccurs="unbounded")xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// A04. Decreasing maxOccurs narrows the array bounds.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DecreasingArrayMaxOccursIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(minOccurs="1" maxOccurs="5")xml", R"xml(minOccurs="1" maxOccurs="3")xml");
    ExpectRejected(schemaXml, "ECDb_0338", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// A04. Setting a finite bound on an unbounded array (unbounded is NULL, so it is "narrowing").
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, BoundingUnboundedArrayIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECArrayProperty propertyName="Counts" typeName="int" />)xml", R"xml(<ECArrayProperty propertyName="Counts" typeName="int" maxOccurs="5" />)xml");
    ExpectRejected(schemaXml, "ECDb_0338", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// U22 / U24 / U26. Making the implicit numerator, denominator and offset explicit does not
// change the conversion. Tested one at a time.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, SetImplicitUnitNumeratorExplicitly)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Unit typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" />)xml",
        R"xml(<Unit typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" numerator="1.0" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// U24.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, SetImplicitUnitDenominatorExplicitly)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Unit typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" />)xml",
        R"xml(<Unit typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" denominator="1.0" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// U26.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, SetImplicitUnitOffsetExplicitly)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Unit typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" />)xml",
        R"xml(<Unit typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" offset="0.0" />)xml");
    ExpectAccepted(schemaXml);
    }

//---------------------------------------------------------------------------------------
// U25. Setting the denominator to something other than 1 is a conversion change.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingUnitDenominatorIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Unit typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" />)xml",
        R"xml(<Unit typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" denominator="2.0" />)xml");
    ExpectRejected(schemaXml, "ECDb_0419", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// U27. Setting a non-zero offset is a conversion change.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, ChangingUnitOffsetIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Unit typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" />)xml",
        R"xml(<Unit typeName="SiteMeter" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" offset="273.15" />)xml");
    ExpectRejected(schemaXml, "ECDb_0419", SchemaVersionChange::Major);
    }

//---------------------------------------------------------------------------------------
// P18. minimumLength is metadata only. Existing shorter strings stay as they are.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, SetMinimumLength)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ChangeDataBeforeUpdate("UPDATE ts.Sensor SET Label = 'x'");

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(<ECProperty propertyName="Label" typeName="string" maximumLength="100" />)xml", R"xml(<ECProperty propertyName="Label" typeName="string" minimumLength="5" maximumLength="100" />)xml");
    ExpectAccepted(schemaXml);

    ECPropertyCP label = GetProperty("Sensor", "Label");
    ASSERT_TRUE(label != nullptr);
    EXPECT_EQ(5u, label->GetMinimumLength());
    }

//---------------------------------------------------------------------------------------
// P20. minimumValue is metadata only. Existing values below it stay as they are.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, RaiseMinimumValueAboveStoredValues)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ChangeDataBeforeUpdate("UPDATE ts.Sensor SET Reading = 1.0");

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml(minimumValue="0" maximumValue="1000")xml", R"xml(minimumValue="500" maximumValue="1000")xml");
    ExpectAccepted(schemaXml);

    ECPropertyCP reading = GetProperty("Sensor", "Reading");
    ASSERT_TRUE(reading != nullptr);
    ECValue minimum;
    ASSERT_EQ(ECObjectsStatus::Success, reading->GetMinimumValue(minimum));
    EXPECT_DOUBLE_EQ(500.0, minimum.GetDouble());
    }

//---------------------------------------------------------------------------------------
// P20 / P21. Removing minimumValue / maximumValue.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, RemoveMinimumAndMaximumValue)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Sensor", R"xml( minimumValue="0" maximumValue="1000")xml", "");
    ExpectAccepted(schemaXml);

    ECPropertyCP reading = GetProperty("Sensor", "Reading");
    ASSERT_TRUE(reading != nullptr);
    EXPECT_FALSE(reading->IsMinimumValueDefined());
    EXPECT_FALSE(reading->IsMaximumValueDefined());
    }

//***************************************************************************************
// Type change combined with a property move (matrix "T": data movement). A type change on its
// own only updates ec_Property, the column stays. Data is only moved when the RemapManager frees
// a column, e.g. because a property moved up the hierarchy. Widget is in a joined table with
// shared columns, Element.Count becomes a column of the Element table, so the values cross tables.
//***************************************************************************************

//---------------------------------------------------------------------------------------
// T01 / T. Widget.Count (int) is deleted and Element.Count (double) added in the same update.
// Widget values move to the Element table and read as doubles; Gadget and Sensor read NULL.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, MovePropertyToBaseClassWithOtherType)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Widget", R"xml(<ECProperty propertyName="Count" typeName="int" />)xml", "");
    ReplaceInClass(schemaXml, "Element", R"xml(<ECProperty propertyName="Code" typeName="string" />)xml",
        R"xml(<ECProperty propertyName="Code" typeName="string" />
                    <ECProperty propertyName="Count" typeName="double" />)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Count", IntegerAsDouble));

    ECPropertyCP count = GetProperty("Element", "Count");
    ASSERT_TRUE(count != nullptr);
    EXPECT_EQ(PRIMITIVETYPE_Double, count->GetAsPrimitiveProperty()->GetType());
    EXPECT_TRUE(GetProperty("Widget", "Count") == count) << "Widget inherits Count from Element";
    }

//---------------------------------------------------------------------------------------
// Same update without AllowDataTransformDuringSchemaUpgrade. The move needs the transform,
// so the import is rejected with ECDb_0590 and nothing moves.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, MovingPropertyToBaseClassWithOtherTypeWithoutDataTransformIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Widget", R"xml(<ECProperty propertyName="Count" typeName="int" />)xml", "");
    ReplaceInClass(schemaXml, "Element", R"xml(<ECProperty propertyName="Code" typeName="string" />)xml",
        R"xml(<ECProperty propertyName="Code" typeName="string" />
                    <ECProperty propertyName="Count" typeName="double" />)xml");
    ExpectRejected(schemaXml, "ECDb_0590", SchemaVersionChange::Major, SchemaManager::SchemaImportOptions::None);
    }

// E03. Renaming an unreferenced enumeration is a delete + add (no identity-preserving rename).
TEST_F(PropertyTypeUpdateTests, RenameUnreferencedEnumeration)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumeration typeName="Phase")xml", R"xml(<ECEnumeration typeName="Stage")xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

// E03. Renaming a referenced enumeration (Status is used by Gadget.State) together with its references is accepted
// with Major: delete + add, and the data is untouched. Records current behaviour (D2).
TEST_F(PropertyTypeUpdateTests, RenameReferencedEnumeration_Accepted)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumeration typeName="Status")xml", R"xml(<ECEnumeration typeName="State")xml");
    ReplaceInSchema(schemaXml, R"xml(typeName="Status")xml", R"xml(typeName="State")xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

// K09. Renaming an unreferenced KindOfQuantity (LengthInFeet).
TEST_F(PropertyTypeUpdateTests, RenameUnreferencedKindOfQuantity)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<KindOfQuantity typeName="LengthInFeet")xml", R"xml(<KindOfQuantity typeName="LengthFeet")xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

// K09. Renaming a referenced KindOfQuantity (Length is used by Sensor.Span) together with its references is accepted
// with Major. Records current behaviour (D2).
TEST_F(PropertyTypeUpdateTests, RenameReferencedKindOfQuantity_Accepted)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<KindOfQuantity typeName="Length" persistenceUnit="u:M")xml", R"xml(<KindOfQuantity typeName="Distance" persistenceUnit="u:M")xml");
    ReplaceInClass(schemaXml, "Sensor", R"xml(kindOfQuantity="Length")xml", R"xml(kindOfQuantity="Distance")xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

// K03. Renaming an unreferenced property category (Metadata).
TEST_F(PropertyTypeUpdateTests, RenameUnreferencedPropertyCategory)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<PropertyCategory typeName="Metadata")xml", R"xml(<PropertyCategory typeName="Notes")xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

// K03. Renaming a referenced property category (Dimensions is used by Sensor.Reading) together with its references
// is accepted with Major. Records current behaviour (D2).
TEST_F(PropertyTypeUpdateTests, RenameReferencedPropertyCategory_Accepted)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<PropertyCategory typeName="Dimensions")xml", R"xml(<PropertyCategory typeName="Sizes")xml");
    ReplaceInClass(schemaXml, "Sensor", R"xml(category="Dimensions")xml", R"xml(category="Sizes")xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

// F03. Renaming a Format is a delete + add and formats cannot be deleted: rejected with ECDb_0421, even unreferenced.
TEST_F(PropertyTypeUpdateTests, RenameUnreferencedFormatIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Format typeName="UnusedFormat")xml", R"xml(<Format typeName="RenamedFormat")xml");
    ExpectRejected(schemaXml, "ECDb_0421", SchemaVersionChange::Major);
    }

// E09. Removing isStrict lets the parser default it to true, which is the forbidden non-strict -> strict change (ECDb_0399, same as E08).
TEST_F(PropertyTypeUpdateTests, RemoveIsStrictFromNonStrictEnumerationIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumeration typeName="Status" backingTypeName="int" isStrict="false">)xml",
        R"xml(<ECEnumeration typeName="Status" backingTypeName="int">)xml");
    ExpectRejected(schemaXml, "ECDb_0399", SchemaVersionChange::Major);
    }

// E19. Changing an enumerator value to another kind (int -> string literal) under an int backing type.
TEST_F(PropertyTypeUpdateTests, ChangeEnumeratorValueKindIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Design" value="1" />)xml", R"xml(<ECEnumerator name="Design" value="one" />)xml");
    ExpectRejected(schemaXml, "ECDb_0400", SchemaVersionChange::Major);
    }

// E20. Reordering enumerators is not tracked as a change.
TEST_F(PropertyTypeUpdateTests, ReorderEnumerators)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Design" value="1" />
                    <ECEnumerator name="Build" value="2" />)xml",
        R"xml(<ECEnumerator name="Build" value="2" />
                    <ECEnumerator name="Design" value="1" />)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor);
    }

// U13 / U14. Adding a constant and an inverted unit.
TEST_F(PropertyTypeUpdateTests, AddConstantAndInvertedUnit)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, ConstantAndInvertedUnitXml());
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor);
    }

// U28. A constant becomes a plain unit of the same name.
TEST_F(PropertyTypeUpdateTests, ChangeConstantToUnitIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = ImportBaselinePlus(ConstantAndInvertedUnitXml());
    ReplaceInSchema(schemaXml, R"xml(<Constant typeName="SiteConstant" phenomenon="SitePhenomenon" definition="u:M" numerator="3.0" />)xml",
        R"xml(<Unit typeName="SiteConstant" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" numerator="3.0" />)xml");
    ExpectRejected(schemaXml, "ECDb_0419", SchemaVersionChange::Major);
    }

// U29. Changing the unit an inverted unit inverts.
TEST_F(PropertyTypeUpdateTests, ChangeInvertedUnitInvertsUnitIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = ImportBaselinePlus(ConstantAndInvertedUnitXml());
    ReplaceInSchema(schemaXml, R"xml(invertsUnit="SiteMeter")xml", R"xml(invertsUnit="UnusedUnit")xml");
    ExpectRejected(schemaXml, "ECDb_0419", SchemaVersionChange::Major);
    }

// U30. An inverted unit becomes a non-inverted unit of the same name.
TEST_F(PropertyTypeUpdateTests, ChangeInvertedUnitToUnitIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = ImportBaselinePlus(ConstantAndInvertedUnitXml());
    ReplaceInSchema(schemaXml, R"xml(<InvertedUnit typeName="SiteInverse" invertsUnit="SiteMeter" unitSystem="SiteSystem" />)xml",
        R"xml(<Unit typeName="SiteInverse" phenomenon="SitePhenomenon" unitSystem="SiteSystem" definition="u:M" />)xml");
    ExpectRejected(schemaXml, "ECDb_0419", SchemaVersionChange::Major);
    }

// F06 / F11 / F18 / F19. Several numeric-spec settings on an existing format in one update.
// If rejected, split the settings to see which one is refused.
TEST_F(PropertyTypeUpdateTests, ChangeNumericSpecSettingsOfFormat)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Format typeName="UnusedFormat" type="decimal" precision="2" />)xml",
        R"xml(<Format typeName="UnusedFormat" type="decimal" precision="2" roundFactor="0.5" showSignOption="signAlways" decimalSeparator="," thousandSeparator="." minWidth="6" />)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor);
    }

// F08 + F32. type="ratio" needs ratioType, so this is a presentation type change (F08, accepted) combined with
// the ratioType facet (F32). The matrix says ratioType is neither compared nor read from ECXML 3.2: if this fails in
// ECObjects, F32 cannot be exercised through a schema update at all; if VerifyPersistedSchemas fails, the change is
// accepted but not stored.
TEST_F(PropertyTypeUpdateTests, ChangeFormatRatioType)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Format typeName="UnusedFormat" type="decimal" precision="2" />)xml",
        R"xml(<Format typeName="UnusedFormat" type="ratio" ratioType="OneToN" precision="2" />)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);
    }

// F33-F35. Azimuth settings on an existing format.
TEST_F(PropertyTypeUpdateTests, ChangeFormatAzimuthSettings)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<Format typeName="UnusedFormat" type="decimal" precision="2" />)xml",
        R"xml(<Format typeName="UnusedFormat" type="decimal" precision="2" azimuthBase="90" azimuthBaseUnit="u:ARC_DEG" azimuthCounterClockwise="true" />)xml");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor);
    }

// F24. Changing the unit of the first composite slot (m -> km keeps the required large-to-small order).
TEST_F(PropertyTypeUpdateTests, ChangeCompositeSlotUnitIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = ImportBaselinePlus(ThreeUnitCompositeFormatXml());
    ReplaceInSchema(schemaXml, R"xml(<Unit label="m">u:M</Unit>)xml", R"xml(<Unit label="km">u:KM</Unit>)xml");
    ExpectRejected(schemaXml, "ECDb_0424", SchemaVersionChange::Major);
    }

// F25. Removing a composite slot.
TEST_F(PropertyTypeUpdateTests, RemoveCompositeSlotIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = ImportBaselinePlus(ThreeUnitCompositeFormatXml());
    ReplaceInSchema(schemaXml, R"xml(<Unit label="mm">u:MM</Unit>)xml", "");
    ExpectRejected(schemaXml, "ECDb_0424", SchemaVersionChange::Major);
    }

// F28. The label of the middle composite slot is metadata, like the major slot label (F27, ChangeFormatCompositeUnitLabel).
TEST_F(PropertyTypeUpdateTests, ChangeCompositeMiddleSlotLabel)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = ImportBaselinePlus(ThreeUnitCompositeFormatXml());
    ReplaceInSchema(schemaXml, R"xml(<Unit label="cm">u:CM</Unit>)xml", R"xml(<Unit label="centi">u:CM</Unit>)xml");
    ExpectAccepted(schemaXml);
    }

// F31. Not testable through ECXML: ECObjects requires composite units in descending order (m, cm, mm) and
// refuses to read a schema whose slots are reordered ("failed to create composite value spec"), so a reorder
// never reaches SchemaWriter. Any order change that is still valid is a unit change of each slot (F23-F26).

// K13. The property-level AllowUnitChange opt-in does not apply to the KoQ definition: changing the KoQ's own
// persistence unit is rejected even when every property using it carries a matching AllowUnitChange.
TEST_F(PropertyTypeUpdateTests, ChangingKindOfQuantityPersistenceUnitIsRejectedDespiteAllowUnitChange)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(typeName="Length" persistenceUnit="u:M")xml", R"xml(typeName="Length" persistenceUnit="u:FT")xml");
    ReplaceInClass(schemaXml, "Sensor", R"xml(<ECProperty propertyName="Span" typeName="double" kindOfQuantity="Length" />)xml",
        R"xml(<ECProperty propertyName="Span" typeName="double" kindOfQuantity="Length">
            <ECCustomAttributes>
                <AllowUnitChange xmlns="SchemaUpgradeCustomAttributes.01.00.00">
                    <From>u:M</From>
                    <To>u:FT</To>
                </AllowUnitChange>
            </ECCustomAttributes>
        </ECProperty>)xml");
    ExpectRejected(schemaXml, "ECDb_0391", SchemaVersionChange::Major);
    }

// P24. Removing extendedTypeName="Id" again: the longs render as plain numbers once more. Nothing was rewritten.
TEST_F(PropertyTypeUpdateTests, RemoveIdExtendedTypeName_ValuesRenderedAsNumbersAgain)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String withId = GetBaselineSchemaXml();
    ReplaceInClass(withId, "Widget", R"xml(propertyName="BigCount" typeName="long")xml", R"xml(propertyName="BigCount" typeName="long" extendedTypeName="Id")xml");
    ASSERT_EQ(SUCCESS, UpdateSchema(withId));

    Utf8String backXml = GetBaselineSchemaXml();
    ASSERT_EQ(SUCCESS, UpdateSchema(backXml));
    VerifyData();
    }

// P26 on a primitive array. Assigns a unit meaning to the stored array elements, no rescaling.
TEST_F(PropertyTypeUpdateTests, AddKindOfQuantityToArrayProperty)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Gadget", R"xml(<ECArrayProperty propertyName="Counts" typeName="int" />)xml", R"xml(<ECArrayProperty propertyName="Counts" typeName="int" kindOfQuantity="Length" />)xml");
    ExpectAccepted(schemaXml);
    }

// E11 / E14. Adding a string enumerator is accepted; changing a string enumerator value is rejected like an
// integer one (E13), with its own issue id: ECDb_0401 for strings, ECDb_0400 for integers.
TEST_F(PropertyTypeUpdateTests, AddStringEnumerator)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Blue" value="blue" />)xml", R"xml(<ECEnumerator name="Blue" value="blue" /><ECEnumerator name="Green" value="green" />)xml");
    ExpectAccepted(schemaXml);
    }

TEST_F(PropertyTypeUpdateTests, ChangingStringEnumeratorValueIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Blue" value="blue" />)xml", R"xml(<ECEnumerator name="Blue" value="navy" />)xml");
    ExpectRejected(schemaXml, "ECDb_0401", SchemaVersionChange::Major);
    }

//***************************************************************************************
// Import paths that matter for iModels (Rob's I2, I7). DgnDomains::DoImportSchemas always adds
// DisallowMajorSchemaUpgrade and AllowMajorSchemaUpgradeForDynamicSchemas, so in an iModel the
// only way through gate G is the DynamicSchema custom attribute. The schema sync db imports with
// DoNotCreateOrUpdateDataTables, which refuses anything that can make stored values unreadable.
//***************************************************************************************

//---------------------------------------------------------------------------------------
// T01 / I2 / S08. With the iModel options a read version bump does not help a non-dynamic schema.
// The import is vetoed at the schema level (ECDb_0427, "major schema version changes are
// disabled") before the writer looks at the property. This is the situation every BIS domain
// schema is in.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, TypeChangeWithIModelOptionsIsRejectedForNonDynamicSchema)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "double");
    ExpectRejected(schemaXml, "ECDb_0427", SchemaVersionChange::Major, IModelImportOptions());
    }

//---------------------------------------------------------------------------------------
// T01 / I2. Same without a read version bump, which is what a connector would actually send:
// the schema-level veto does not fire, the type change itself is rejected (ECDb_0336).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, TypeChangeWithIModelOptionsAndMinorVersionIsRejectedForNonDynamicSchema)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "double");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Minor, IModelImportOptions());
    }

//---------------------------------------------------------------------------------------
// T01 / I2. A dynamic schema gets the type change through with the iModel options and without
// a read version bump. The data effect is the same as on the plain ECDb route.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, TypeChangeInDynamicSchemaWithIModelOptions)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    MakeDynamic(schemaXml);
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "double");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Count", IntegerAsDouble), IModelImportOptions());
    }

//---------------------------------------------------------------------------------------
// T01 / I2. The lossy case through the dynamic route: the customer scenario is reachable in an iModel.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, LossyTypeChangeInDynamicSchemaWithIModelOptions_ReadAsWrongDate)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    SeedWidgetNames([] (int index) { return Utf8PrintfString("2020-03-%02dT10:30:00.000Z", 10 + index); });

    Utf8String schemaXml = GetBaselineSchemaXml();
    MakeDynamic(schemaXml);
    ChangePropertyType(schemaXml, "Widget", "Name", "string", "dateTime");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges().ConvertValuesOf("TestSchema:Widget", "Name", StringReadAsDateTime), IModelImportOptions());
    }

//---------------------------------------------------------------------------------------
// E02 / I2. Enumeration deletion through the dynamic route.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeleteEnumerationInDynamicSchemaWithIModelOptions)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    MakeDynamic(schemaXml);
    RemoveStatusEnumeration(schemaXml);
    ChangePropertyType(schemaXml, "Gadget", "State", "Status", "int");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges(), IModelImportOptions());
    EXPECT_TRUE(GetEnumeration("Status") == nullptr);
    }

//---------------------------------------------------------------------------------------
// K08 / I2. KindOfQuantity deletion through the dynamic route.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeleteKindOfQuantityInDynamicSchemaWithIModelOptions)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    MakeDynamic(schemaXml);
    ReplaceInSchema(schemaXml, R"xml(<KindOfQuantity typeName="LengthInFeet" persistenceUnit="u:FT" relativeError="0.001" presentationUnits="f:DefaultRealU(4)[u:FT]" />)xml", "");
    ExpectAccepted(schemaXml, SchemaVersionChange::Minor, ExpectedDataChanges(), IModelImportOptions());
    EXPECT_TRUE(GetKindOfQuantity("LengthInFeet") == nullptr);
    }

//---------------------------------------------------------------------------------------
// E12 / I2. The dynamic route does not open gates that G does not open: enumerator deletion stays rejected.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, DeletingEnumeratorInDynamicSchemaIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    MakeDynamic(schemaXml);
    ReplaceInSchema(schemaXml, R"xml(<ECEnumerator name="Retired" value="2" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0402", SchemaVersionChange::Minor, IModelImportOptions());
    }

//---------------------------------------------------------------------------------------
// T01 / I7. With DoNotCreateOrUpdateDataTables (schema sync db) an otherwise legal type change is
// refused, because the import cannot take responsibility for values that become unreadable.
// SchemaWriter reports a second ECDb_0336 and sets the data deletion refused flag.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTypeUpdateTests, TypeChangeWithDoNotCreateOrUpdateDataTablesIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ChangePropertyType(schemaXml, "Widget", "Count", "int", "long");
    ExpectRejected(schemaXml, "ECDb_0336", SchemaVersionChange::Major,
        SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade | SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables);
    }


END_ECDBUNITTESTS_NAMESPACE
