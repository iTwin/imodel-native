/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECDb/SchemaImportAdapter.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct SchemaImportAdapterTests : ECDbTestFixture {
    static Utf8CP V1() {
        return R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="Test" alias="ts" version="01.00.00" displayLabel="Test schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEnumeration typeName="Color" backingTypeName="int" isStrict="true">
                    <ECEnumerator name="Red" value="1" displayLabel="Red"/>
                    <ECEnumerator name="Green" value="2"/>
                </ECEnumeration>
                <PropertyCategory typeName="Misc" displayLabel="Miscellaneous" priority="5"/>
                <ECCustomAttributeClass typeName="Tag" appliesTo="Any">
                    <ECProperty propertyName="Note" typeName="string"/>
                </ECCustomAttributeClass>
                <ECStructClass typeName="Point">
                    <ECProperty propertyName="X" typeName="double"/>
                    <ECProperty propertyName="Y" typeName="double"/>
                </ECStructClass>
                <ECEntityClass typeName="Model">
                    <ECProperty propertyName="Name" typeName="string"/>
                </ECEntityClass>
                <ECEntityClass typeName="Element" modifier="Abstract" description="Base element">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow></ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Code" typeName="string" category="Misc"/>
                    <ECNavigationProperty propertyName="Model" relationshipName="ModelContainsElements" direction="backward"/>
                </ECEntityClass>
                <ECEntityClass typeName="PhysicalElement">
                    <BaseClass>Element</BaseClass>
                    <ECCustomAttributes>
                        <Tag xmlns="Test.01.00.00"><Note>physical</Note></Tag>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Color" typeName="Color"/>
                    <ECStructProperty propertyName="Location" typeName="Point"/>
                    <ECArrayProperty propertyName="Labels" typeName="string" minOccurs="0" maxOccurs="unbounded"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
                    <Source multiplicity="(1..1)" roleLabel="contains" polymorphic="true"><Class class="Model"/></Source>
                    <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true"><Class class="Element"/></Target>
                </ECRelationshipClass>
            </ECSchema>)xml";
    }

    static Utf8CP V2() {
        return R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="Test" alias="ts" version="01.00.01" displayLabel="Test schema v2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEnumeration typeName="Color" backingTypeName="int" isStrict="true">
                    <ECEnumerator name="Red" value="1" displayLabel="Crimson"/>
                    <ECEnumerator name="Green" value="2"/>
                    <ECEnumerator name="Blue" value="3"/>
                </ECEnumeration>
                <ECEnumeration typeName="Shape" backingTypeName="string" isStrict="false">
                    <ECEnumerator name="Round" value="round"/>
                </ECEnumeration>
                <PropertyCategory typeName="Misc" displayLabel="Miscellaneous" priority="5"/>
                <ECCustomAttributeClass typeName="Tag" appliesTo="Any">
                    <ECProperty propertyName="Note" typeName="string"/>
                </ECCustomAttributeClass>
                <ECStructClass typeName="Point">
                    <ECProperty propertyName="X" typeName="double"/>
                    <ECProperty propertyName="Y" typeName="double"/>
                    <ECProperty propertyName="Z" typeName="double"/>
                </ECStructClass>
                <ECEntityClass typeName="Model">
                    <ECProperty propertyName="Name" typeName="string"/>
                </ECEntityClass>
                <ECEntityClass typeName="Element" modifier="Abstract" description="Base element, revised">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow></ShareColumns>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Code" typeName="string" category="Misc" displayLabel="Code value"/>
                    <ECNavigationProperty propertyName="Model" relationshipName="ModelContainsElements" direction="backward"/>
                </ECEntityClass>
                <ECEntityClass typeName="PhysicalElement">
                    <BaseClass>Element</BaseClass>
                    <ECCustomAttributes>
                        <Tag xmlns="Test.01.00.01"><Note>physical v2</Note></Tag>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Color" typeName="Color"/>
                    <ECStructProperty propertyName="Location" typeName="Point"/>
                    <ECArrayProperty propertyName="Labels" typeName="string" minOccurs="0" maxOccurs="unbounded"/>
                    <ECProperty propertyName="Mass" typeName="double"/>
                    <ECProperty propertyName="Shape" typeName="Shape"/>
                </ECEntityClass>
                <ECEntityClass typeName="Pipe">
                    <BaseClass>PhysicalElement</BaseClass>
                    <ECProperty propertyName="Diameter" typeName="double"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
                    <Source multiplicity="(1..1)" roleLabel="contains" polymorphic="true"><Class class="Model"/></Source>
                    <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true"><Class class="Element"/></Target>
                </ECRelationshipClass>
                <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="None">
                    <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true"><Class class="Element"/></Source>
                    <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true"><Class class="Element"/></Target>
                    <ECProperty propertyName="Weight" typeName="int"/>
                </ECRelationshipClass>
            </ECSchema>)xml";
    }

    static Utf8CP Quantities() {
        return R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="Quantities" alias="qt" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                <KindOfQuantity typeName="Length" persistenceUnit="u:M" relativeError="0.0001" presentationUnits="f:DefaultRealU(2)[u:M];f:DefaultRealU(2)[u:FT]"/>
                <ECEntityClass typeName="Beam">
                    <ECProperty propertyName="Span" typeName="double" kindOfQuantity="Length"/>
                </ECEntityClass>
            </ECSchema>)xml";
    }

    static Utf8String Describe(SchemaCommandResult const& result) {
        BeJsDocument json;
        result.ToJson(json);
        return json.Stringify();
    }

    static bool HasIssue(SchemaCommandResult const& result, Utf8CP code) {
        for (auto const& issue : result.GetIssues())
            if (issue.code.Equals(code))
                return true;
        return false;
    }

    static ECSchemaReadContextPtr Read(ECDbCR db, std::vector<Utf8CP> const& xmls) {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        BeFileName searchPath;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchPath);
        searchPath.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");
        context->AddSchemaPath(searchPath);
        context->AddSchemaLocater(db.GetSchemaLocater());
        TestIssueListener listener;
        context->Issues().AddListener(listener);
        for (auto xml : xmls) {
            ECSchemaPtr schema;
            auto status = ECSchema::ReadFromXmlString(schema, xml, *context);
            EXPECT_EQ(SchemaReadStatus::Success, status);
            if (status != SchemaReadStatus::Success)
                for (auto const& issue : listener.m_issues)
                    ADD_FAILURE() << "Schema read: " << issue.message.c_str();
        }
        context->Issues().RemoveListener();
        return context;
    }

    static BentleyStatus NativeImport(ECDbR db, std::vector<Utf8CP> const& xmls, SchemaImportOptions options = SchemaImportOptions::None) {
        auto context = Read(db, xmls);
        TestIssueListener listener;
        db.AddIssueListener(listener);
        auto status = db.Schemas().ImportSchemas(context->GetCache().GetSchemas(), options);
        db.RemoveIssueListener();
        if (SUCCESS != status) {
            for (auto const& issue : listener.m_issues)
                ADD_FAILURE() << "Native import: " << issue.message.c_str();
            db.AbandonChanges();
            return ERROR;
        }
        return BE_SQLITE_OK == db.SaveChanges() ? SUCCESS : ERROR;
    }

    static SchemaCommandResult AdapterImport(ECDbR db, std::vector<Utf8CP> const& xmls, bool commit = true, SchemaImportOptions options = SchemaImportOptions::None) {
        auto context = Read(db, xmls);
        SchemaImportCompletion completion;
        auto result = SchemaImportAdapter::ImportSchemas(db, context->GetCache().GetSchemas(), &completion, options);
        if (result.IsOk() && commit && completion.IsPending())
            EXPECT_EQ(BE_SQLITE_OK, completion.Commit());
        return result;
    }

    static Utf8String Query(ECDbCR db, Utf8CP sql) {
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(db, sql)) << sql << " " << db.GetLastError().c_str();
        Utf8String out;
        DbResult status;
        while (BE_SQLITE_ROW == (status = stmt.Step())) {
            for (int i = 0; i < stmt.GetColumnCount(); ++i) {
                out.append(i == 0 ? "" : "|");
                out.append(stmt.IsColumnNull(i) ? "<null>" : stmt.GetValueText(i));
            }
            out.append("\n");
        }
        EXPECT_EQ(BE_SQLITE_DONE, status) << sql << " " << db.GetLastError().c_str();
        return out;
    }

    static Utf8String ResolveIndexClassIds(ECDbCR db, Utf8String sql) {
        bmap<Utf8String, Utf8String> names;
        Statement classes;
        EXPECT_EQ(BE_SQLITE_OK, classes.Prepare(db, "SELECT CAST(c.Id AS TEXT),s.Name||':'||c.Name FROM ec_Class c JOIN ec_Schema s ON s.Id=c.SchemaId"));
        DbResult status;
        while ((status = classes.Step()) == BE_SQLITE_ROW)
            names[classes.GetValueText(0)] = classes.GetValueText(1);
        EXPECT_EQ(BE_SQLITE_DONE, status);
        size_t position = 0;
        while ((position = sql.find("ECClassId=", position)) != Utf8String::npos) {
            position += strlen("ECClassId=");
            auto end = position;
            while (end < sql.size() && sql[end] >= '0' && sql[end] <= '9')
                ++end;
            if (end == position)
                continue;
            auto found = names.find(sql.substr(position, end - position));
            EXPECT_TRUE(found != names.end()) << "Unknown class id in index SQL: " << sql.c_str();
            if (found == names.end()) {
                position = end;
                continue;
            }
            sql.replace(position, end - position, found->second);
            position += found->second.size();
        }
        // Partial-index class filters are unordered sets; their numeric-id order differs between imports.
        position = 0;
        while ((position = sql.find(" WHERE ECClassId=", position)) != Utf8String::npos) {
            position += strlen(" WHERE ");
            auto end = sql.find('\n', position);
            if (end == Utf8String::npos)
                end = sql.size();
            auto predicate = sql.substr(position, end - position);
            bvector<Utf8String> terms;
            size_t start = 0;
            bool classFilter = true;
            while (start < predicate.size()) {
                auto next = predicate.find(" OR ", start);
                auto term = predicate.substr(start, next == Utf8String::npos ? Utf8String::npos : next - start);
                if (term.find("ECClassId=") != 0 || term.find(' ') != Utf8String::npos) {
                    classFilter = false;
                    break;
                }
                terms.push_back(term);
                if (next == Utf8String::npos)
                    break;
                start = next + strlen(" OR ");
            }
            if (classFilter) {
                std::sort(terms.begin(), terms.end());
                Utf8String normalized;
                for (auto const& term : terms) {
                    if (!normalized.empty())
                        normalized.append(" OR ");
                    normalized.append(term);
                }
                sql.replace(position, end - position, normalized);
                position += normalized.size();
            } else
                position = end;
        }
        return sql;
    }

    // Name-resolved persisted metadata and mapping, independent of generated ids.
    static bvector<std::pair<Utf8String, Utf8String>> Dump(ECDbCR db) {
        static std::pair<Utf8CP, Utf8CP> const s_sections[] = {
            {"schemas", "SELECT Name,Alias,DisplayLabel,Description,VersionDigit1,VersionDigit2,VersionDigit3,OriginalECXmlVersionMajor,OriginalECXmlVersionMinor FROM ec_Schema ORDER BY Name"},
            {"references", "SELECT s.Name,r.Name FROM ec_SchemaReference x JOIN ec_Schema s ON s.Id=x.SchemaId JOIN ec_Schema r ON r.Id=x.ReferencedSchemaId ORDER BY 1,2"},
            {"classes", "SELECT s.Name,c.Name,c.DisplayLabel,c.Description,c.Type,c.Modifier,c.RelationshipStrength,c.RelationshipStrengthDirection,c.CustomAttributeContainerType FROM ec_Class c JOIN ec_Schema s ON s.Id=c.SchemaId ORDER BY 1,2"},
            {"bases", "SELECT c.Name,b.Name,x.Ordinal FROM ec_ClassHasBaseClasses x JOIN ec_Class c ON c.Id=x.ClassId JOIN ec_Class b ON b.Id=x.BaseClassId ORDER BY 1,3"},
            {"properties", "SELECT c.Name,p.Name,p.DisplayLabel,p.Description,p.IsReadonly,p.Priority,p.Ordinal,p.Kind,p.PrimitiveType,p.PrimitiveTypeMinLength,p.PrimitiveTypeMaxLength,p.PrimitiveTypeMinValue,p.PrimitiveTypeMaxValue,"
                           "(SELECT Name FROM ec_Enumeration WHERE Id=p.EnumerationId),(SELECT Name FROM ec_Class WHERE Id=p.StructClassId),p.ExtendedTypeName,(SELECT Name FROM ec_KindOfQuantity WHERE Id=p.KindOfQuantityId),"
                           "(SELECT Name FROM ec_PropertyCategory WHERE Id=p.CategoryId),p.ArrayMinOccurs,p.ArrayMaxOccurs,(SELECT Name FROM ec_Class WHERE Id=p.NavigationRelationshipClassId),p.NavigationDirection "
                           "FROM ec_Property p JOIN ec_Class c ON c.Id=p.ClassId ORDER BY 1,2"},
            {"constraints", "SELECT c.Name,x.RelationshipEnd,x.MultiplicityLowerLimit,x.MultiplicityUpperLimit,x.IsPolymorphic,x.RoleLabel,(SELECT Name FROM ec_Class WHERE Id=x.AbstractConstraintClassId),"
                            "(SELECT group_concat(k.Name) FROM (SELECT k.Name FROM ec_RelationshipConstraintClass y JOIN ec_Class k ON k.Id=y.ClassId WHERE y.ConstraintId=x.Id ORDER BY k.Name) k) "
                            "FROM ec_RelationshipConstraint x JOIN ec_Class c ON c.Id=x.RelationshipClassId ORDER BY 1,2"},
            {"enumerations", "SELECT Name,DisplayLabel,Description,UnderlyingPrimitiveType,IsStrict,EnumValues FROM ec_Enumeration ORDER BY Name"},
            {"koqs", "SELECT Name,DisplayLabel,Description,PersistenceUnit,RelativeError,PresentationUnits FROM ec_KindOfQuantity ORDER BY Name"},
            {"categories", "SELECT Name,DisplayLabel,Description,Priority FROM ec_PropertyCategory ORDER BY Name"},
            {"customAttributes", "SELECT x.ContainerType,COALESCE((SELECT Name FROM ec_Schema WHERE x.ContainerType=1 AND Id=x.ContainerId),"
                                 "(SELECT s.Name||':'||c.Name FROM ec_Class c JOIN ec_Schema s ON s.Id=c.SchemaId WHERE (x.ContainerType & 30)<>0 AND c.Id=x.ContainerId),"
                                 "(SELECT s.Name||':'||c.Name||'.'||p.Name FROM ec_Property p JOIN ec_Class c ON c.Id=p.ClassId JOIN ec_Schema s ON s.Id=c.SchemaId WHERE (x.ContainerType & 992)<>0 AND p.Id=x.ContainerId),"
                                 "(SELECT c.Name||'#'||r.RelationshipEnd FROM ec_RelationshipConstraint r JOIN ec_Class c ON c.Id=r.RelationshipClassId WHERE x.ContainerType IN (1024,2048) AND r.Id=x.ContainerId)),"
                                 "k.Name,x.Ordinal,x.Instance FROM ec_CustomAttribute x JOIN ec_Class k ON k.Id=x.ClassId ORDER BY 2,1,4,3"},
            {"classMaps", "SELECT c.Name,m.MapStrategy,m.ShareColumnsMode,m.MaxSharedColumnsBeforeOverflow,m.JoinedTableInfo FROM ec_ClassMap m JOIN ec_Class c ON c.Id=m.ClassId ORDER BY 1"},
            {"tables", "SELECT t.Name,t.Type,(SELECT Name FROM ec_Table WHERE Id=t.ParentTableId),(SELECT Name FROM ec_Class WHERE Id=t.ExclusiveRootClassId),t.UpdatableViewName FROM ec_Table t ORDER BY 1"},
            {"columns", "SELECT t.Name,c.Name,c.Type,c.IsVirtual,c.Ordinal,c.NotNullConstraint,c.UniqueConstraint,c.CheckConstraint,c.DefaultConstraint,c.CollationConstraint,c.OrdinalInPrimaryKey,c.ColumnKind "
                        "FROM ec_Column c JOIN ec_Table t ON t.Id=c.TableId ORDER BY 1,2"},
            {"propertyMaps", "SELECT c.Name,pp.AccessString,t.Name,col.Name FROM ec_PropertyMap m JOIN ec_Class c ON c.Id=m.ClassId JOIN ec_PropertyPath pp ON pp.Id=m.PropertyPathId "
                             "JOIN ec_Column col ON col.Id=m.ColumnId JOIN ec_Table t ON t.Id=col.TableId ORDER BY 1,2,3,4"},
            {"indexes", "SELECT i.Name,t.Name,i.IsUnique,i.AddNotNullWhereExp,i.IsAutoGenerated,(SELECT Name FROM ec_Class WHERE Id=i.ClassId),i.AppliesToSubclassesIfPartial,"
                        "(SELECT group_concat(n) FROM (SELECT col.Name n FROM ec_IndexColumn ic JOIN ec_Column col ON col.Id=ic.ColumnId WHERE ic.IndexId=i.Id ORDER BY ic.Ordinal)) "
                        "FROM ec_Index i JOIN ec_Table t ON t.Id=i.TableId ORDER BY 1"},
            {"sqlite", "SELECT type,name,tbl_name,sql FROM sqlite_master WHERE name NOT LIKE 'sqlite_%' AND name NOT LIKE 'ec\\_%' ESCAPE '\\' ORDER BY 1,2"},
            {"classHierarchyCache", "SELECT s.Name||':'||c.Name,bs.Name||':'||b.Name FROM ec_cache_ClassHierarchy h JOIN ec_Class c ON c.Id=h.ClassId JOIN ec_Schema s ON s.Id=c.SchemaId "
                                    "JOIN ec_Class b ON b.Id=h.BaseClassId JOIN ec_Schema bs ON bs.Id=b.SchemaId ORDER BY 1,2"},
            {"classHasTablesCache", "SELECT s.Name||':'||c.Name,t.Name FROM ec_cache_ClassHasTables x JOIN ec_Class c ON c.Id=x.ClassId JOIN ec_Schema s ON s.Id=c.SchemaId "
                                    "JOIN ec_Table t ON t.Id=x.TableId ORDER BY 1,2"},
        };
        bvector<std::pair<Utf8String, Utf8String>> dump;
        for (auto const& section : s_sections) {
            auto rows = Query(db, section.second);
            if (0 == strcmp(section.first, "sqlite"))
                rows = ResolveIndexClassIds(db, rows);
            dump.push_back({section.first, rows});
        }
        return dump;
    }

    static void ExpectSameDump(ECDbCR expected, ECDbCR actual, std::set<Utf8String> const& skippedSections = {}) {
        auto expectedDump = Dump(expected);
        auto actualDump = Dump(actual);
        ASSERT_EQ(expectedDump.size(), actualDump.size());
        for (size_t i = 0; i < expectedDump.size(); ++i)
            if (skippedSections.count(expectedDump[i].first) == 0)
                EXPECT_STREQ(expectedDump[i].second.c_str(), actualDump[i].second.c_str()) << "Section: " << expectedDump[i].first.c_str();
    }

    static Utf8String PlanJson(SchemaImportPlan const& plan) {
        BeJsDocument json;
        plan.ToJson(json);
        return json.Stringify(StringifyFormat::Indented);
    }

    void SetupPair(Utf8CP name, ECDb& native) {
        ASSERT_EQ(BE_SQLITE_OK, SetupECDb(Utf8PrintfString("%s.ecdb", name).c_str()));
        ASSERT_EQ(BE_SQLITE_OK, CreateECDb(native, Utf8PrintfString("%sNative.ecdb", name).c_str()));
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, NewSchemaMatchesNativeImport) {
    ECDb native;
    SetupPair("SchemaImportAdapterNew", native);
    ASSERT_EQ(SUCCESS, NativeImport(native, {V1()}));
    auto result = AdapterImport(m_ecdb, {V1()});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ExpectSameDump(native, m_ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Model(Name) VALUES('m')"));
    ECInstanceKey model;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(model));
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.PhysicalElement(Code,Color,Location.X,Model.Id) VALUES('c',2,1.5,?)"));
    stmt.BindId(1, model.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, CustomAttributeOrderIsPlannedOnlyWhenCreationOrderDiffers) {
    // Inline creation attributes keep source order (not alphabetical), so no reorder step is needed.
    // A mixin's IsMixin attribute is written by CreateMixin itself, so its container still needs one.
    Utf8CP xml = R"xml(<ECSchema schemaName="CaOrder" alias="cao" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
        <ECEntityClass typeName="Single">
            <ECCustomAttributes><HiddenClass xmlns="CoreCustomAttributes.01.00.03"/></ECCustomAttributes>
            <ECProperty propertyName="One" typeName="string">
                <ECCustomAttributes><HiddenProperty xmlns="CoreCustomAttributes.01.00.03"/></ECCustomAttributes>
            </ECProperty>
            <ECProperty propertyName="Two" typeName="string">
                <ECCustomAttributes>
                    <HiddenProperty xmlns="CoreCustomAttributes.01.00.03"/>
                    <Deprecated xmlns="CoreCustomAttributes.01.00.03"/>
                </ECCustomAttributes>
            </ECProperty>
        </ECEntityClass>
        <ECEntityClass typeName="IMix" modifier="Abstract">
            <ECCustomAttributes>
                <HiddenClass xmlns="CoreCustomAttributes.01.00.03"/>
                <IsMixin xmlns="CoreCustomAttributes.01.00.03"><AppliesToEntityClass>Single</AppliesToEntityClass></IsMixin>
            </ECCustomAttributes>
        </ECEntityClass>
    </ECSchema>)xml";
    ECDb native;
    SetupPair("SchemaImportAdapterCaOrder", native);
    auto context = Read(m_ecdb, {xml});
    SchemaImportPlan plan;
    auto result = SchemaImportAdapter::BuildPlan(m_ecdb, context->GetCache().GetSchemas(), plan);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    bvector<Utf8String> ordered;
    for (auto const& step : plan.GetSteps())
        if (step.m_facet == "CustomAttributeOrder")
            ordered.push_back(step.m_item);
    ASSERT_EQ(2u, ordered.size()) << PlanJson(plan).c_str();
    EXPECT_STREQ("CaOrder:IMix", ordered[0].c_str());
    EXPECT_STREQ("CaOrder:IMix", ordered[1].c_str());

    ASSERT_EQ(SUCCESS, NativeImport(native, {xml}));
    result = AdapterImport(m_ecdb, {xml});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ExpectSameDump(native, m_ecdb);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, SharedTableIndexesFollowSubclassesAddedInLaterGroups) {
    // Each new class maps in its own group. Index reconciliation is scoped to the new class and its
    // ancestors, so Leaf's partial index must pick up SubLeaf, and must stay unchanged while Other
    // (not a subclass of Leaf) is mapped.
    auto schema = [](bool upgraded) {
        Utf8String xml(Utf8PrintfString(R"xml(<ECSchema schemaName="SharedIndex" alias="si" version="01.00.%02d" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <DbIndexList xmlns="ECDbMap.02.00.00"><Indexes><DbIndex><Name>ix_si_Element_Code</Name><IsUnique>True</IsUnique><Properties><string>Code</string></Properties></DbIndex></Indexes></DbIndexList>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Leaf">
                <ECCustomAttributes>
                    <DbIndexList xmlns="ECDbMap.02.00.00"><Indexes><DbIndex><Name>ix_si_Leaf_Score</Name><IsUnique>False</IsUnique><Properties><string>Score</string></Properties></DbIndex></Indexes></DbIndexList>
                </ECCustomAttributes>
                <BaseClass>Element</BaseClass>
                <ECProperty propertyName="Score" typeName="int"/>
            </ECEntityClass>)xml", upgraded ? 1 : 0));
        if (upgraded)
            xml.append(R"xml(
            <ECEntityClass typeName="Other"><BaseClass>Element</BaseClass></ECEntityClass>
            <ECEntityClass typeName="SubLeaf"><BaseClass>Leaf</BaseClass><ECProperty propertyName="Extra" typeName="int"/></ECEntityClass>)xml");
        xml.append("</ECSchema>");
        return xml;
    };
    ECDb native;
    SetupPair("SchemaImportAdapterSharedIndex", native);
    auto v1 = schema(false);
    auto v2 = schema(true);
    ASSERT_EQ(SUCCESS, NativeImport(native, {v1.c_str()}));
    ASSERT_EQ(SUCCESS, NativeImport(native, {v2.c_str()}));
    auto result = AdapterImport(m_ecdb, {v1.c_str()});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    result = AdapterImport(m_ecdb, {v2.c_str()});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ExpectSameDump(native, m_ecdb);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO si.Other(Code) VALUES('dup')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO si.SubLeaf(Code,Score) VALUES('dup',1)"));
    EXPECT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, stmt.Step()) << "Element's unique index must cover subclasses added later";
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, ManyPrimitivePropertiesMatchNativeImport) {
    ECDb native;
    SetupPair("SchemaImportAdapterManyProperties", native);
    Utf8String xml(R"xml(<ECSchema schemaName="ManyProperties" alias="mp" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">)xml");
    for (int c = 0; c < 12; ++c) {
        xml.append(Utf8PrintfString("<ECEntityClass typeName=\"C%d\">", c));
        for (int p = 0; p < 64; ++p)
            xml.append(Utf8PrintfString("<ECProperty propertyName=\"P%d\" typeName=\"double\"/>", p));
        xml.append("</ECEntityClass>");
    }
    xml.append("</ECSchema>");
    ASSERT_EQ(SUCCESS, NativeImport(native, {xml.c_str()}));
    StopWatch timer(true);
    auto result = AdapterImport(m_ecdb, {xml.c_str()});
    timer.Stop();
    auto seconds = timer.GetElapsedSeconds();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    printf("SchemaImportAdapter: 12 classes, 768 primitive properties, %.3f seconds\n", seconds);
    RecordProperty("ImportSeconds", std::to_string(seconds));
    ExpectSameDump(native, m_ecdb);
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ExpectSameDump(native, m_ecdb);
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO mp.C11(P0,P63) VALUES(1.25,63.5)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT P0,P63 FROM mp.C11"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_DOUBLE_EQ(1.25, statement.GetValueDouble(0));
    EXPECT_DOUBLE_EQ(63.5, statement.GetValueDouble(1));
    EXPECT_EQ(BE_SQLITE_DONE, statement.Step());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, RootRelationshipsMapWithClassesInOneGroup) {
    // Classes and root relationships (with their navigation properties) are mapped in one pass; a derived
    // relationship needs its base's class map and therefore runs after that group.
    Utf8CP xml = R"xml(<ECSchema schemaName="RelBatch" alias="rb" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="Parent"><ECProperty propertyName="Code" typeName="string"/></ECEntityClass>
        <ECEntityClass typeName="Child">
            <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
            <ECProperty propertyName="Name" typeName="string"/>
            <ECNavigationProperty propertyName="Parent" relationshipName="Owns" direction="Backward"/>
        </ECEntityClass>
        <ECEntityClass typeName="Leaf"><BaseClass>Child</BaseClass><ECProperty propertyName="Extra" typeName="int"/></ECEntityClass>
        <ECRelationshipClass typeName="Owns" strength="embedding" modifier="None">
            <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true"><Class class="Parent"/></Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true"><Class class="Child"/></Target>
        </ECRelationshipClass>
        <ECRelationshipClass typeName="OwnsLeaf" strength="embedding" modifier="Sealed">
            <BaseClass>Owns</BaseClass>
            <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true"><Class class="Parent"/></Source>
            <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true"><Class class="Leaf"/></Target>
        </ECRelationshipClass>
        <ECRelationshipClass typeName="Links" strength="referencing" modifier="None">
            <Source multiplicity="(0..*)" roleLabel="links" polymorphic="true"><Class class="Parent"/></Source>
            <Target multiplicity="(0..*)" roleLabel="is linked by" polymorphic="true"><Class class="Leaf"/></Target>
        </ECRelationshipClass>
    </ECSchema>)xml";
    ECDb native;
    SetupPair("SchemaImportAdapterRelBatch", native);
    ASSERT_EQ(SUCCESS, NativeImport(native, {xml}));
    {
        auto context = Read(m_ecdb, {xml});
        SchemaImportPlan plan;
        ASSERT_TRUE(SchemaImportAdapter::BuildPlan(m_ecdb, context->GetCache().GetSchemas(), plan).IsOk());
        Utf8String items;
        for (auto const& step : plan.GetSteps())
            items.append(step.m_item).append(";");
        ASSERT_STREQ("RelBatch;RelBatch;RelBatch:Parent;RelBatch:Child;RelBatch:Leaf;RelBatch:Links;RelBatch:OwnsLeaf;", items.c_str());
    }
    uint32_t executions = 0;
    auto cancel = m_ecdb.Schemas().OnAfterSchemaChanges().AddListener([&](ECDbCR, SchemaChangeType) { ++executions; });
    auto result = AdapterImport(m_ecdb, {xml});
    cancel();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    // CreateSchema, AddSchemaReference, one group for Parent..Links (Child's group holds Owns and its
    // navigation property), then the derived relationship.
    EXPECT_EQ(4u, executions);
    ExpectSameDump(native, m_ecdb);

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO rb.Parent(ECInstanceId,Code) VALUES(1,'p')"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO rb.Leaf(ECInstanceId,Name,Extra,Parent) VALUES(2,'l',7,?)"));
    statement.BindNavigationValue(1, ECInstanceId(UINT64_C(1)), m_ecdb.Schemas().GetClassId("RelBatch", "OwnsLeaf"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO rb.Links(SourceECInstanceId,TargetECInstanceId) VALUES(1,2)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT COUNT(*) FROM rb.OwnsLeaf r JOIN rb.Links l ON l.TargetECInstanceId=r.TargetECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_EQ(1, statement.GetValueInt(0));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, PropertyMetadataUpgradesMapWithClassesInOneGroup) {
    // Upgrades that add properties and classes and edit property metadata, including a KoQ that keeps the
    // persistence unit, run as one group after the schema version change, reusing the working graph between children.
    Utf8CP v1 = R"xml(<ECSchema schemaName="MetaBatch" alias="mb" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
        <PropertyCategory typeName="Cat" priority="1"/>
        <KindOfQuantity typeName="Len" persistenceUnit="u:M" relativeError="0.1"/>
        <KindOfQuantity typeName="Len2" persistenceUnit="u:M" relativeError="0.01"/>
        <ECEntityClass typeName="Base">
            <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
        </ECEntityClass>
        <ECEntityClass typeName="Derived"><BaseClass>Base</BaseClass><ECProperty propertyName="D" typeName="double" kindOfQuantity="Len"/></ECEntityClass>
    </ECSchema>)xml";
    Utf8CP v2 = R"xml(<ECSchema schemaName="MetaBatch" alias="mb" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
        <PropertyCategory typeName="Cat" priority="1"/>
        <KindOfQuantity typeName="Len" persistenceUnit="u:M" relativeError="0.1"/>
        <KindOfQuantity typeName="Len2" persistenceUnit="u:M" relativeError="0.01"/>
        <ECEntityClass typeName="Base">
            <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
            <ECProperty propertyName="A" typeName="string" displayLabel="Alpha" priority="5" maximumLength="10"/>
            <ECProperty propertyName="B" typeName="int" category="Cat" readOnly="true"/>
            <ECProperty propertyName="C" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="Derived"><BaseClass>Base</BaseClass><ECProperty propertyName="D" typeName="double" description="dee" kindOfQuantity="Len2"/><ECProperty propertyName="E" typeName="long"/></ECEntityClass>
        <ECEntityClass typeName="Added"><BaseClass>Derived</BaseClass><ECProperty propertyName="F" typeName="int"/></ECEntityClass>
    </ECSchema>)xml";
    ECDb native;
    SetupPair("SchemaImportAdapterMetaBatch", native);
    ASSERT_EQ(SUCCESS, NativeImport(native, {v1}));
    ASSERT_EQ(SUCCESS, NativeImport(native, {v2}));
    auto result = AdapterImport(m_ecdb, {v1});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO mb.Derived(ECInstanceId,A,B,D) VALUES(1,'a',2,3.5)"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    {
        auto context = Read(m_ecdb, {v2});
        SchemaImportPlan plan;
        ASSERT_TRUE(SchemaImportAdapter::BuildPlan(m_ecdb, context->GetCache().GetSchemas(), plan).IsOk());
        Utf8String commands;
        for (auto const& step : plan.GetSteps()) {
            BeJsDocument json(step.m_commands.c_str());
            json.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst command) {
                commands.append(command["command"].asString()).append(";");
                return false;
            });
        }
        // Property changes are planned class by class, followed by new classes.
        ASSERT_STREQ("AlterSchema;AlterProperty;AlterProperty;AddProperty;AlterProperty;AddProperty;CreateEntityClass;", commands.c_str());
    }
    uint32_t executions = 0;
    auto cancel = m_ecdb.Schemas().OnAfterSchemaChanges().AddListener([&](ECDbCR, SchemaChangeType) { ++executions; });
    result = AdapterImport(m_ecdb, {v2});
    cancel();
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    // AlterSchema, then one group for every property and class change.
    EXPECT_EQ(2u, executions);
    ExpectSameDump(native, m_ecdb);
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO mb.Added(ECInstanceId,A,B,C,D,E,F) VALUES(2,'b',3,'c',4.5,5,6)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT COUNT(*),SUM(B) FROM mb.Derived WHERE D>3"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_EQ(2, statement.GetValueInt(0));
    EXPECT_EQ(5, statement.GetValueInt(1));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, AddingKindOfQuantityToBaseAndDerivedOverrideMatchesNative) {
    Utf8CP v1 = R"xml(<ECSchema schemaName="NullKoq" alias="nk" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
        <ECEntityClass typeName="Equipment">
            <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
            <ECProperty propertyName="Width" typeName="double"/>
        </ECEntityClass>
        <ECEntityClass typeName="ReservedArea"><BaseClass>Equipment</BaseClass></ECEntityClass>
        <ECEntityClass typeName="ReservedAreaPar">
            <BaseClass>ReservedArea</BaseClass>
            <ECProperty propertyName="Width" typeName="double"/>
        </ECEntityClass>
    </ECSchema>)xml";
    Utf8String v2(v1);
    v2.ReplaceAll("alias=\"nk\" version=\"01.00.00\"", "alias=\"nk\" version=\"01.00.01\"");
    v2.ReplaceAll("<ECEntityClass typeName=\"Equipment\">",
                  "<KindOfQuantity typeName=\"Length\" persistenceUnit=\"u:M\" relativeError=\"0.0001\"/><ECEntityClass typeName=\"Equipment\">");
    v2.ReplaceAll("propertyName=\"Width\" typeName=\"double\"", "propertyName=\"Width\" typeName=\"double\" kindOfQuantity=\"Length\"");
    ECDb native;
    SetupPair("SchemaImportAdapterNullKoqOverride", native);
    ASSERT_EQ(SUCCESS, NativeImport(native, {v1}));
    auto result = AdapterImport(m_ecdb, {v1});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ECDb* databases[] = {&native, &m_ecdb};
    for (auto db : databases) {
        ECSqlStatement insert;
        ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*db, "INSERT INTO nk.ReservedAreaPar(Width) VALUES(12.5)"));
        ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
        insert.Finalize();
        ASSERT_EQ(BE_SQLITE_OK, db->SaveChanges());
    }
    ASSERT_EQ(SUCCESS, NativeImport(native, {v2.c_str()}));
    result = AdapterImport(m_ecdb, {v2.c_str()});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ExpectSameDump(native, m_ecdb);
    for (auto db : databases) {
        for (auto className : {"Equipment", "ReservedAreaPar"}) {
            auto ecClass = db->Schemas().GetClass("NullKoq", className);
            ASSERT_NE(nullptr, ecClass);
            auto width = ecClass->GetPropertyP("Width", false);
            ASSERT_NE(nullptr, width);
            EXPECT_TRUE(width->IsKindOfQuantityDefinedLocally());
            ASSERT_NE(nullptr, width->GetKindOfQuantity());
            EXPECT_STREQ("Length", width->GetKindOfQuantity()->GetName().c_str());
        }
        ECSqlStatement query;
        ASSERT_EQ(ECSqlStatus::Success, query.Prepare(*db, "SELECT Width FROM nk.ReservedAreaPar"));
        ASSERT_EQ(BE_SQLITE_ROW, query.Step());
        EXPECT_DOUBLE_EQ(12.5, query.GetValueDouble(0));
        EXPECT_EQ(BE_SQLITE_DONE, query.Step());
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, NestedOverridesAcrossMappingBatchesMatchNative) {
    Utf8CP v1 = R"xml(<ECSchema schemaName="Overrides" alias="ov" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="Device">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>20</MaxSharedColumnsBeforeOverflow></ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="A_Code" typeName="string"/>
            <ECProperty propertyName="B_Name" typeName="string"/>
            <ECProperty propertyName="C_Number" typeName="string"/>
            <ECProperty propertyName="D_Area" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="Instrument"><BaseClass>Device</BaseClass></ECEntityClass>
        <ECEntityClass typeName="FlowElement"><BaseClass>Instrument</BaseClass></ECEntityClass>
        <ECEntityClass typeName="Leaf"><BaseClass>FlowElement</BaseClass></ECEntityClass>
    </ECSchema>)xml";
    Utf8String v2(v1);
    v2.ReplaceAll("alias=\"ov\" version=\"01.00.00\"", "alias=\"ov\" version=\"01.00.01\"");
    v2.ReplaceAll("<ECEntityClass typeName=\"Instrument\"><BaseClass>Device</BaseClass></ECEntityClass>", R"xml(
        <ECEntityClass typeName="Instrument"><BaseClass>Device</BaseClass>
            <ECProperty propertyName="A_Code" typeName="string"/>
            <ECProperty propertyName="C_Number" typeName="string"/>
            <ECProperty propertyName="D_Area" typeName="string"/>
        </ECEntityClass>)xml");
    // The class metadata command separates the two sets of property overrides into mapping batches.
    v2.ReplaceAll("<ECEntityClass typeName=\"FlowElement\"><BaseClass>Instrument</BaseClass></ECEntityClass>", R"xml(
        <ECEntityClass typeName="FlowElement" displayLabel="Flow element"><BaseClass>Instrument</BaseClass>
            <ECProperty propertyName="A_Code" typeName="string"/>
            <ECProperty propertyName="B_Name" typeName="string"/>
            <ECProperty propertyName="C_Number" typeName="string"/>
            <ECProperty propertyName="D_Area" typeName="string"/>
        </ECEntityClass>)xml");
    ECDb native;
    SetupPair("SchemaImportAdapterNestedOverrides", native);
    ASSERT_EQ(SUCCESS, NativeImport(native, {v1}));
    auto result = AdapterImport(m_ecdb, {v1});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ECDb* databases[] = {&native, &m_ecdb};
    for (auto db : databases) {
        int id = 0;
        for (auto className : {"Device", "Instrument", "FlowElement", "Leaf"}) {
            ECSqlStatement insert;
            auto sql = Utf8PrintfString("INSERT INTO ov.%s(ECInstanceId,A_Code,B_Name,C_Number,D_Area) VALUES(%d,'code','name','number','area')", className, ++id);
            ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*db, sql.c_str()));
            ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
        }
        ASSERT_EQ(BE_SQLITE_OK, db->SaveChanges());
    }
    ASSERT_EQ(SUCCESS, NativeImport(native, {v2.c_str()}));
    result = AdapterImport(m_ecdb, {v2.c_str()});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ExpectSameDump(native, m_ecdb);
    Utf8CP roots = "SELECT c.Name,pp.AccessString,root.Name FROM ec_PropertyMap m JOIN ec_Class c ON c.Id=m.ClassId "
                  "JOIN ec_Schema s ON s.Id=c.SchemaId JOIN ec_PropertyPath pp ON pp.Id=m.PropertyPathId "
                  "JOIN ec_Property p ON p.Id=pp.RootPropertyId JOIN ec_Class root ON root.Id=p.ClassId "
                  "WHERE s.Name='Overrides' ORDER BY 1,2,3";
    EXPECT_EQ(Query(native, roots), Query(m_ecdb, roots));
    for (auto db : databases) {
        ECSqlStatement query;
        ASSERT_EQ(ECSqlStatus::Success, query.Prepare(*db, "SELECT ECInstanceId,A_Code,B_Name,C_Number,D_Area FROM ov.Device ORDER BY ECInstanceId"));
        for (int id = 1; id <= 4; ++id) {
            ASSERT_EQ(BE_SQLITE_ROW, query.Step());
            EXPECT_EQ(id, query.GetValueInt(0));
            EXPECT_STREQ("code", query.GetValueText(1));
            EXPECT_STREQ("name", query.GetValueText(2));
            EXPECT_STREQ("number", query.GetValueText(3));
            EXPECT_STREQ("area", query.GetValueText(4));
        }
        EXPECT_EQ(BE_SQLITE_DONE, query.Step());
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, MetadataUpgradeRetainsNativeInheritedOverrideMappings) {
    Utf8CP v1 = R"xml(<ECSchema schemaName="Inherited" alias="ih" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="Root">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>20</MaxSharedColumnsBeforeOverflow></ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Name" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="Device">
            <BaseClass>Root</BaseClass>
            <ECProperty propertyName="Name" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="Leaf"><BaseClass>Device</BaseClass></ECEntityClass>
    </ECSchema>)xml";
    Utf8String v2(v1);
    v2.ReplaceAll("alias=\"ih\" version=\"01.00.00\"", "alias=\"ih\" version=\"01.00.01\"");
    v2.ReplaceAll("typeName=\"Device\"", "typeName=\"Device\" displayLabel=\"Device\"");
    ECDb native;
    SetupPair("SchemaImportAdapterInheritedRoots", native);
    ECDb* databases[] = {&native, &m_ecdb};
    Utf8CP roots = "SELECT c.Name,pp.AccessString,root.Name,m.Id,pp.Id FROM ec_PropertyMap m JOIN ec_Class c ON c.Id=m.ClassId "
                  "JOIN ec_Schema s ON s.Id=c.SchemaId JOIN ec_PropertyPath pp ON pp.Id=m.PropertyPathId "
                  "JOIN ec_Property p ON p.Id=pp.RootPropertyId JOIN ec_Class root ON root.Id=p.ClassId "
                  "WHERE s.Name='Inherited' ORDER BY 1,2,3";
    for (auto db : databases) {
        ASSERT_EQ(SUCCESS, NativeImport(*db, {v1}));
        ECSqlStatement insert;
        ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*db, "INSERT INTO ih.Leaf(Name) VALUES('retained')"));
        ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
        insert.Finalize();
        ASSERT_EQ(BE_SQLITE_OK, db->SaveChanges());
    }
    auto baseline = Query(m_ecdb, roots);
    ASSERT_EQ(SUCCESS, NativeImport(native, {v2.c_str()}));
    auto result = AdapterImport(m_ecdb, {v2.c_str()});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ExpectSameDump(native, m_ecdb);
    EXPECT_EQ(baseline, Query(m_ecdb, roots));
    ECSqlStatement query;
    ASSERT_EQ(ECSqlStatus::Success, query.Prepare(m_ecdb, "SELECT Name FROM ih.Leaf"));
    ASSERT_EQ(BE_SQLITE_ROW, query.Step());
    EXPECT_STREQ("retained", query.GetValueText(0));
    EXPECT_EQ(BE_SQLITE_DONE, query.Step());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, BisCoreMatchesNativeImportAndPreparesECSql) {
    ECDb native;
    SetupPair("SchemaImportAdapterBisCore", native);

    // BisCore maps SpatialIndex to an existing rtree, normally created by DgnDb before schema import.
    ECDb* databases[] = {&native, &m_ecdb};
    for (auto db : databases) {
        ASSERT_EQ(BE_SQLITE_OK, db->ExecuteSql("CREATE VIRTUAL TABLE dgn_SpatialIndex USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"));
        ASSERT_EQ(BE_SQLITE_OK, db->SaveChanges());
    }

    // Use the full BisCore 1.0.0 schema shipped with the ECDb test assets, not a simplified stand-in.
    BeFileName schemaDirectory;
    BeTest::GetHost().GetDocumentsRoot(schemaDirectory);
    schemaDirectory.AppendToPath(L"ECDb").AppendToPath(L"fileformatbenchmark").AppendToPath(L"schemas").AppendToPath(L"dgndb");
    BeFileName schemaFile(schemaDirectory);
    schemaFile.AppendToPath(L"BisCore.ecschema.xml");
    ASSERT_TRUE(schemaFile.DoesPathExist());

    auto nativeContext = Read(native, {});
    nativeContext->AddSchemaPath(schemaDirectory);
    ECSchemaPtr nativeSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlFile(nativeSchema, schemaFile.GetName(), *nativeContext));
    TestIssueListener listener;
    native.AddIssueListener(listener);
    auto nativeStatus = native.Schemas().ImportSchemas(nativeContext->GetCache().GetSchemas());
    native.RemoveIssueListener();
    for (auto const& issue : listener.m_issues)
        if (nativeStatus != SUCCESS)
            ADD_FAILURE() << "Native BisCore import: " << issue.message.c_str();
    if (nativeStatus != SUCCESS)
        native.AbandonChanges();
    ASSERT_EQ(SUCCESS, nativeStatus);
    ASSERT_EQ(BE_SQLITE_OK, native.SaveChanges());

    Utf8CP const queries[] = {
        "SELECT ECInstanceId,ECClassId,CodeValue,Model.Id,Parent.Id FROM bis.Element",
        "SELECT ECInstanceId,ECClassId FROM bis.Model",
        "SELECT ECInstanceId,ECClassId FROM bis.GeometricElement3d",
        "SELECT ECInstanceId,ECClassId FROM bis.PhysicalElement",
        "SELECT SourceECInstanceId,TargetECInstanceId FROM bis.ModelContainsElements",
        "SELECT SourceECInstanceId,TargetECInstanceId FROM bis.ElementRefersToElements",
        "SELECT ECInstanceId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ FROM bis.SpatialIndex",
        "SELECT e.ECInstanceId,m.ECInstanceId FROM bis.Element e JOIN bis.Model m ON e.Model.Id=m.ECInstanceId",
    };
    for (auto ecsql : queries) {
        SCOPED_TRACE(ecsql);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(native, ecsql));
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    auto adapterContext = Read(m_ecdb, {});
    adapterContext->AddSchemaPath(schemaDirectory);
    ECSchemaPtr adapterSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlFile(adapterSchema, schemaFile.GetName(), *adapterContext));
    auto baseline = Dump(m_ecdb);
    SchemaImportCompletion completion;
    auto result = SchemaImportAdapter::ImportSchemas(m_ecdb, adapterContext->GetCache().GetSchemas(), &completion);
    if (!result.IsOk()) {
        EXPECT_FALSE(completion.IsPending());
        EXPECT_TRUE(baseline == Dump(m_ecdb)) << "Failed BisCore import must roll back metadata and physical mappings.";
    }
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_TRUE(completion.IsPending());
    ASSERT_EQ(BE_SQLITE_OK, completion.Commit());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ExpectSameDump(native, m_ecdb);
    for (auto ecsql : queries) {
        SCOPED_TRACE(ecsql);
        ECSqlStatement nativeStatement, adapterStatement;
        ASSERT_EQ(ECSqlStatus::Success, nativeStatement.Prepare(native, ecsql));
        ASSERT_EQ(ECSqlStatus::Success, adapterStatement.Prepare(m_ecdb, ecsql));
        EXPECT_EQ(nativeStatement.GetColumnCount(), adapterStatement.GetColumnCount());
        EXPECT_EQ(BE_SQLITE_DONE, adapterStatement.Step());
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, LegacyEnumeratorRenameSurvivesECXmlVersionUpgrade) {
    Utf8CP legacy = R"xml(<ECSchema schemaName="LegacyEnum" alias="le" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEnumeration typeName="StatementType" backingTypeName="int" isStrict="true">
            <ECEnumerator value="3" displayLabel="ReadOnly"/>
            <ECEnumerator value="7" displayLabel="All"/>
        </ECEnumeration>
        <ECEnumeration typeName="Code" backingTypeName="string" isStrict="false">
            <ECEnumerator value="a" displayLabel="Alpha"/>
            <ECEnumerator value="b" displayLabel="Beta"/>
        </ECEnumeration>
    </ECSchema>)xml";
    ECDb native;
    SetupPair("SchemaImportAdapterLegacyEnum", native);
    ASSERT_EQ(SUCCESS, NativeImport(native, {legacy}));
    auto result = AdapterImport(m_ecdb, {legacy});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ExpectSameDump(native, m_ecdb);

    Utf8String upgrade(legacy);
    upgrade.ReplaceAll("ECXML.3.1", "ECXML.3.2");
    upgrade.ReplaceAll("version=\"01.00.00\"", "version=\"01.00.01\"");
    upgrade.ReplaceAll("<ECEnumerator value=\"3\"", "<ECEnumerator name=\"ReadOnly\" value=\"3\"");
    upgrade.ReplaceAll("<ECEnumerator value=\"7\"", "<ECEnumerator name=\"All\" value=\"7\"");
    upgrade.ReplaceAll("<ECEnumerator value=\"a\"", "<ECEnumerator name=\"Alpha\" value=\"a\"");
    upgrade.ReplaceAll("<ECEnumerator value=\"b\"", "<ECEnumerator name=\"Beta\" value=\"b\"");
    ASSERT_EQ(SUCCESS, NativeImport(native, {upgrade.c_str()}));
    result = AdapterImport(m_ecdb, {upgrade.c_str()});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ExpectSameDump(native, m_ecdb);
    auto schema = m_ecdb.Schemas().GetSchema("LegacyEnum");
    ASSERT_NE(nullptr, schema);
    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(2, schema->GetOriginalECXmlVersionMinor());
    auto enumeration = schema->GetEnumerationCP("StatementType");
    ASSERT_NE(nullptr, enumeration);
    auto enumerator = enumeration->FindEnumeratorByName("ReadOnly");
    ASSERT_NE(nullptr, enumerator);
    EXPECT_EQ(3, enumerator->GetInteger());
    enumeration = schema->GetEnumerationCP("Code");
    ASSERT_NE(nullptr, enumeration);
    enumerator = enumeration->FindEnumeratorByName("Alpha");
    ASSERT_NE(nullptr, enumerator);
    EXPECT_STREQ("a", enumerator->GetString().c_str());

    Utf8String forbidden(upgrade);
    forbidden.ReplaceAll("version=\"01.00.01\"", "version=\"01.00.02\"");
    forbidden.ReplaceAll("name=\"ReadOnly\"", "name=\"Renamed\"");
    auto context = Read(native, {forbidden.c_str()});
    ASSERT_EQ(ERROR, native.Schemas().ImportSchemas(context->GetCache().GetSchemas()));
    ASSERT_EQ(BE_SQLITE_OK, native.AbandonChanges());
    native.ClearECDbCache();
    result = AdapterImport(m_ecdb, {forbidden.c_str()});
    EXPECT_FALSE(result.IsOk()) << "A new session must enforce the now-persisted ECXML 3.2 rename restriction.";
    ExpectSameDump(native, m_ecdb);
}

TEST_F(SchemaImportAdapterTests, MovingPropertiesToBaseDuringMinorUpgradeKeepsData) {
    Utf8CP initial = R"xml(<ECSchema schemaName="MoveProperty" alias="mp" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="Item"><ECProperty propertyName="Name" typeName="string"/></ECEntityClass>
        <ECEntityClass typeName="EntityBase" modifier="Abstract">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00"><ApplyToSubclassesOnly>true</ApplyToSubclassesOnly><MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow></ShareColumns>
            </ECCustomAttributes>
        </ECEntityClass>
        <ECEntityClass typeName="EntityDerived"><BaseClass>EntityBase</BaseClass><ECProperty propertyName="MemberPriority" typeName="int"/></ECEntityClass>
        <ECRelationshipClass typeName="RefersTo" modifier="Abstract" strength="referencing">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00"><ApplyToSubclassesOnly>true</ApplyToSubclassesOnly><MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow></ShareColumns>
            </ECCustomAttributes>
            <Source multiplicity="(0..*)" polymorphic="true" roleLabel="refers to"><Class class="Item"/></Source>
            <Target multiplicity="(0..*)" polymorphic="true" roleLabel="is referenced by"><Class class="Item"/></Target>
        </ECRelationshipClass>
        <ECRelationshipClass typeName="GroupsMembers" modifier="None" strength="referencing">
            <BaseClass>RefersTo</BaseClass>
            <ECProperty propertyName="MemberPriority" typeName="int"/>
            <Source multiplicity="(0..*)" polymorphic="true" roleLabel="groups"><Class class="Item"/></Source>
            <Target multiplicity="(0..*)" polymorphic="true" roleLabel="is grouped by"><Class class="Item"/></Target>
        </ECRelationshipClass>
    </ECSchema>)xml";
    ECDb native;
    SetupPair("SchemaImportAdapterMoveProperty", native);
    ASSERT_EQ(SUCCESS, NativeImport(native, {initial}));
    auto result = AdapterImport(m_ecdb, {initial});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ECDb* databases[] = {&native, &m_ecdb};
    for (auto db : databases) {
        ASSERT_EQ(BE_SQLITE_DONE, TestHelper(*db).ExecuteECSql("INSERT INTO mp.Item(ECInstanceId,Name) VALUES(1,'source')"));
        ASSERT_EQ(BE_SQLITE_DONE, TestHelper(*db).ExecuteECSql("INSERT INTO mp.Item(ECInstanceId,Name) VALUES(2,'target')"));
        ECSqlStatement insert;
        ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*db, "INSERT INTO mp.GroupsMembers(ECInstanceId,SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId,MemberPriority) VALUES(3,1,?,2,?,42)"));
        auto item = db->Schemas().GetClass("MoveProperty", "Item");
        ASSERT_NE(nullptr, item);
        ASSERT_EQ(ECSqlStatus::Success, insert.BindId(1, item->GetId()));
        ASSERT_EQ(ECSqlStatus::Success, insert.BindId(2, item->GetId()));
        ASSERT_EQ(BE_SQLITE_DONE, insert.Step()) << db->GetLastError().c_str();
        insert.Finalize();
        ASSERT_EQ(BE_SQLITE_DONE, TestHelper(*db).ExecuteECSql("INSERT INTO mp.EntityDerived(ECInstanceId,MemberPriority) VALUES(4,99)"));
        ASSERT_EQ(BE_SQLITE_OK, db->SaveChanges());
    }
    Utf8String upgrade(initial);
    upgrade.ReplaceAll("version=\"01.00.00\"", "version=\"01.00.01\"");
    upgrade.ReplaceAll("<ECProperty propertyName=\"MemberPriority\" typeName=\"int\"/>", "");
    upgrade.ReplaceAll("<ECEntityClass typeName=\"EntityBase\" modifier=\"Abstract\">",
                       "<ECEntityClass typeName=\"EntityBase\" modifier=\"Abstract\"><ECProperty propertyName=\"MemberPriority\" typeName=\"int\"/>");
    upgrade.ReplaceAll("<Source multiplicity=\"(0..*)\" polymorphic=\"true\" roleLabel=\"refers to\">",
                       "<ECProperty propertyName=\"MemberPriority\" typeName=\"int\"/><Source multiplicity=\"(0..*)\" polymorphic=\"true\" roleLabel=\"refers to\">");
    result = AdapterImport(m_ecdb, {upgrade.c_str()}, true, SchemaImportOptions::DisallowMajorSchemaUpgrade);
    EXPECT_EQ(SchemaCommandResult::Status::ERROR_DATA_TRANSFORM_REQUIRED, result.GetStatus()) << Describe(result);
    ExpectSameDump(native, m_ecdb);
    auto options = SchemaImportOptions::DisallowMajorSchemaUpgrade | SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade;
    ASSERT_EQ(SUCCESS, NativeImport(native, {upgrade.c_str()}, options));
    result = AdapterImport(m_ecdb, {upgrade.c_str()}, true, options);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ExpectSameDump(native, m_ecdb);
    auto derived = m_ecdb.Schemas().GetClass("MoveProperty", "GroupsMembers");
    ASSERT_NE(nullptr, derived);
    EXPECT_EQ(nullptr, derived->GetPropertyP("MemberPriority", false));
    ASSERT_NE(nullptr, derived->GetPropertyP("MemberPriority"));
    for (auto db : databases) {
        ECSqlStatement statement;
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*db, "SELECT ECInstanceId,SourceECInstanceId,TargetECInstanceId,MemberPriority FROM mp.RefersTo"));
        ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(3, statement.GetValueInt64(0));
        EXPECT_EQ(1, statement.GetValueInt64(1));
        EXPECT_EQ(2, statement.GetValueInt64(2));
        EXPECT_EQ(42, statement.GetValueInt(3));
        EXPECT_EQ(BE_SQLITE_DONE, statement.Step());
        statement.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(*db, "SELECT ECInstanceId,MemberPriority FROM mp.EntityBase"));
        ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(4, statement.GetValueInt64(0));
        EXPECT_EQ(99, statement.GetValueInt(1));
        EXPECT_EQ(BE_SQLITE_DONE, statement.Step());
    }
    Utf8String deletion(upgrade);
    deletion.ReplaceAll("version=\"01.00.01\"", "version=\"01.00.02\"");
    deletion.ReplaceAll("<ECProperty propertyName=\"MemberPriority\" typeName=\"int\"/>", "");
    result = AdapterImport(m_ecdb, {deletion.c_str()}, true, options);
    EXPECT_FALSE(result.IsOk()) << "Removing the inherited property itself still requires a major upgrade.";
    EXPECT_TRUE(HasIssue(result, "SchemaEditor.MajorUpgradeDisabled")) << Describe(result);
    ExpectSameDump(native, m_ecdb);
}

TEST_F(SchemaImportAdapterTests, PolicyExceptionsApplyBeforeNewItemIdsExist) {
    std::pair<Utf8CP, Utf8CP> cases[] = {
        {"NoAdditionalForeignKeyConstraints", "Addition:Allowed.Target"},
        {"NoAdditionalForeignKeyConstraints", "a:allowed.target"},
        {"NoAdditionalForeignKeyConstraints", "Addition:Allowed.*"},
        {"NoAdditionalForeignKeyConstraints", "Addition:*"},
        {"NoAdditionalRootEntityClasses", "Addition:Allowed"},
        {"NoAdditionalRootEntityClasses", "Addition"},
        {"NoAdditionalLinkTables", "Addition:AllowedHasTarget"},
        {"NoAdditionalLinkTables", "Addition:*"},
    };
    for (auto const& test : cases) {
        SCOPED_TRACE(test.first);
        SCOPED_TRACE(test.second);
        Utf8String policy = Utf8PrintfString(R"xml(
            <ECSchema schemaName="Policy" alias="p" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="ECDbSchemaPolicies" version="01.00.00" alias="policies"/>
                <ECCustomAttributes><%s xmlns="ECDbSchemaPolicies.01.00.00"><Exceptions><string>%s</string></Exceptions></%s></ECCustomAttributes>
                <ECEntityClass typeName="TargetNode"/>
            </ECSchema>)xml",
                                             test.first, test.second, test.first);
        Utf8String addition = R"xml(
            <ECSchema schemaName="Addition" alias="a" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="Policy" version="01.00.00" alias="p"/>
                <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                <ECEntityClass typeName="Allowed">
                    <ECNavigationProperty propertyName="Target" relationshipName="AllowedHasTarget" direction="Forward">
                        <ECCustomAttributes><ForeignKeyConstraint xmlns="ECDbMap.02.00.00"/></ECCustomAttributes>
                    </ECNavigationProperty>
                </ECEntityClass>
                <ECRelationshipClass typeName="AllowedHasTarget" modifier="None" strength="referencing">
                    <Source multiplicity="(0..*)" roleLabel="has target" polymorphic="true"><Class class="Allowed"/></Source>
                    <Target multiplicity="(0..1)" roleLabel="is target of" polymorphic="true"><Class class="p:TargetNode"/></Target>
                </ECRelationshipClass>
            </ECSchema>)xml";
        if (Utf8String(test.first).Equals("NoAdditionalLinkTables")) {
            auto begin = addition.find("<ECNavigationProperty");
            auto end = addition.find("</ECNavigationProperty>", begin);
            ASSERT_NE(Utf8String::npos, begin);
            ASSERT_NE(Utf8String::npos, end);
            addition.erase(begin, end + strlen("</ECNavigationProperty>") - begin);
            addition.ReplaceAll("<Target multiplicity=\"(0..1)\"", "<Target multiplicity=\"(0..*)\"");
        }
        ECDb native;
        SetupPair(Utf8PrintfString("SchemaImportAdapter%sException", test.first).c_str(), native);
        ASSERT_EQ(SUCCESS, NativeImport(native, {policy.c_str()}));
        auto result = AdapterImport(m_ecdb, {policy.c_str()});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_EQ(SUCCESS, NativeImport(native, {addition.c_str()}));
        result = AdapterImport(m_ecdb, {addition.c_str()});
        ASSERT_TRUE(result.IsOk()) << Describe(result);
        ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
        ExpectSameDump(native, m_ecdb);
        addition.ReplaceAll("schemaName=\"Addition\" alias=\"a\"", "schemaName=\"BlockedAddition\" alias=\"ba\"");
        result = AdapterImport(m_ecdb, {addition.c_str()});
        EXPECT_FALSE(result.IsOk());
        EXPECT_TRUE(HasIssue(result, Utf8String(test.first).Equals("NoAdditionalLinkTables") ? "ECDb_0271" : "SchemaEditor.Policy")) << Describe(result);
        ExpectSameDump(native, m_ecdb);
    }
}

TEST_F(SchemaImportAdapterTests, UpgradeMatchesNativeImportAndKeepsData) {
    ECDb native;
    SetupPair("SchemaImportAdapterUpgrade", native);
    ASSERT_EQ(SUCCESS, NativeImport(native, {V1()}));
    ASSERT_EQ(SUCCESS, NativeImport(m_ecdb, {V1()}));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Model(Name) VALUES('m')"));
    ECInstanceKey model;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(model));
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.PhysicalElement(Code,Color,Location.Y,Model.Id) VALUES('keep',1,7.25,?)"));
    stmt.BindId(1, model.GetInstanceId());
    ECInstanceKey element;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(element));
    stmt.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    // Upgrade without data movement: physical layout must match native exactly.
    Utf8String withoutZ(V2());
    withoutZ.ReplaceAll("<ECProperty propertyName=\"Z\" typeName=\"double\"/>", "");
    ASSERT_EQ(SUCCESS, NativeImport(native, {withoutZ.c_str()}));
    auto result = AdapterImport(m_ecdb, {withoutZ.c_str()});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ExpectSameDump(native, m_ecdb);

    // Growing the struct moves existing values to the overflow table, which, as natively, needs explicit permission.
    Utf8String withZ(V2());
    withZ.ReplaceAll("01.00.01", "01.00.02");
    auto denied = AdapterImport(m_ecdb, {withZ.c_str()});
    ASSERT_FALSE(denied.IsOk());
    EXPECT_TRUE(HasIssue(denied, "SchemaEditor.DataTransformRequired")) << Describe(denied);

    ASSERT_EQ(SUCCESS, NativeImport(native, {withZ.c_str()}, SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    result = AdapterImport(m_ecdb, {withZ.c_str()}, true, SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    // Native reuses columns freed by the move in a different order (its single mapping pass allocates
    // before relocating); the metadata must still match exactly and values must be preserved.
    ExpectSameDump(native, m_ecdb, {"columns", "propertyMaps", "sqlite"});

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code,Color,Location.Y,Model.Id FROM ts.PhysicalElement WHERE ECInstanceId=?"));
    stmt.BindId(1, element.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("keep", stmt.GetValueText(0));
    EXPECT_EQ(1, stmt.GetValueInt(1));
    EXPECT_DOUBLE_EQ(7.25, stmt.GetValueDouble(2));
    EXPECT_EQ(model.GetInstanceId(), stmt.GetValueId<ECInstanceId>(3));
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Pipe(Code,Diameter,Shape,Model.Id) VALUES('p',0.5,'round',?)"));
    stmt.BindId(1, model.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, KindOfQuantityGraphInFreshDb) {
    ECDb native;
    SetupPair("SchemaImportAdapterKoq", native);
    ASSERT_EQ(SUCCESS, NativeImport(native, {Quantities()}));
    auto result = AdapterImport(m_ecdb, {Quantities()});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ExpectSameDump(native, m_ecdb);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, PropertyPolicyCacheRefreshesAfterSchemaAttributeChanges) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaImportAdapterCachedPolicies.ecdb"));
    ASSERT_EQ(SUCCESS, NativeImport(m_ecdb, {
        R"xml(<ECSchema schemaName="Policy" alias="p" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbSchemaPolicies" version="01.00.00" alias="policies"/>
            <ECEntityClass typeName="Root"/>
        </ECSchema>)xml",
        R"xml(<ECSchema schemaName="Target" alias="t" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Item"/>
        </ECSchema>)xml"
    }));
    {
        auto editor = SchemaEditor::Begin(m_ecdb);
        ASSERT_NE(nullptr, editor);
        BeJsDocument commands(R"json([
            {"command":"AddProperty","args":{"class":"t:Item","name":"A","type":"double"}},
            {"command":"AddProperty","args":{"class":"t:Item","name":"B","type":"double"}},
            {"command":"SetCustomAttribute","args":{"container":"p","caClass":"ECDbSchemaPolicies:NoAdditionalRootEntityClasses","instance":{}}},
            {"command":"CreateEntityClass","args":{"schema":"t","name":"Forbidden"}}
        ])json");
        auto result = editor->ExecuteGroup(commands);
        ASSERT_FALSE(result.IsOk());
        EXPECT_TRUE(HasIssue(result, "SchemaEditor.Policy")) << Describe(result);
        BeJsDocument diagnostic;
        result.ToJson(diagnostic);
        EXPECT_EQ(3, diagnostic["failedCommandIndex"].asUInt64());
    }
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("Target", "Forbidden"));
    auto item = m_ecdb.Schemas().GetClass("Target", "Item");
    ASSERT_NE(nullptr, item);
    EXPECT_EQ(nullptr, item->GetPropertyP("A"));
    EXPECT_EQ(nullptr, item->GetPropertyP("B"));
    auto policy = m_ecdb.Schemas().GetSchema("Policy");
    ASSERT_NE(nullptr, policy);
    EXPECT_FALSE(policy->GetCustomAttributeLocal("ECDbSchemaPolicies", "NoAdditionalRootEntityClasses").IsValid());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, PropertyWorkingGraphOwnsPresentationUnitOverrides) {
    ECDb native;
    SetupPair("SchemaImportAdapterCachedKoq", native);
    Utf8String xml(Quantities());
    auto end = xml.find("</ECEntityClass>");
    ASSERT_NE(Utf8String::npos, end);
    xml.insert(end, R"xml(
        <ECProperty propertyName="Width" typeName="double" kindOfQuantity="Length"/>
        <ECProperty propertyName="Height" typeName="double" kindOfQuantity="Length"/>
    )xml");
    ASSERT_EQ(SUCCESS, NativeImport(native, {xml.c_str()}));
    auto result = AdapterImport(m_ecdb, {xml.c_str()});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ExpectSameDump(native, m_ecdb);
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ExpectSameDump(native, m_ecdb);
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO qt.Beam(Span,Width,Height) VALUES(1.25,2.5,3.75)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Span,Width,Height FROM qt.Beam"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_DOUBLE_EQ(1.25, statement.GetValueDouble(0));
    EXPECT_DOUBLE_EQ(2.5, statement.GetValueDouble(1));
    EXPECT_DOUBLE_EQ(3.75, statement.GetValueDouble(2));
    EXPECT_EQ(BE_SQLITE_DONE, statement.Step());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, PreviewIsReadOnlyDeterministicAndReimportIsNoOp) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaImportAdapterPreview.ecdb"));
    auto before = Dump(m_ecdb);
    auto context = Read(m_ecdb, {V2()});
    auto schemas = context->GetCache().GetSchemas();
    SchemaImportPlan plan;
    auto result = SchemaImportAdapter::BuildPlan(m_ecdb, schemas, plan);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_TRUE(plan.IsBuilt());
    EXPECT_FALSE(plan.IsEmpty());
    EXPECT_GT(plan.GetCommandCount(), plan.GetSteps().size() - 1);
    auto after = Dump(m_ecdb);
    for (size_t i = 0; i < before.size(); ++i)
        EXPECT_STREQ(before[i].second.c_str(), after[i].second.c_str()) << before[i].first.c_str();

    bvector<ECSchemaCP> reversed(schemas.rbegin(), schemas.rend());
    SchemaImportPlan shuffled;
    ASSERT_TRUE(SchemaImportAdapter::BuildPlan(m_ecdb, reversed, shuffled).IsOk());
    EXPECT_STREQ(PlanJson(plan).c_str(), PlanJson(shuffled).c_str());

    SchemaImportCompletion completion;
    result = SchemaImportAdapter::ExecutePlan(m_ecdb, plan, &completion);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_TRUE(completion.IsPending());
    ASSERT_EQ(BE_SQLITE_OK, completion.Commit());

    SchemaImportPlan again;
    auto reread = Read(m_ecdb, {V2()});
    result = SchemaImportAdapter::BuildPlan(m_ecdb, reread->GetCache().GetSchemas(), again);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    EXPECT_TRUE(again.IsEmpty()) << PlanJson(again).c_str();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, StalePlanIsRejected) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaImportAdapterStale.ecdb"));
    auto context = Read(m_ecdb, {V1()});
    SchemaImportPlan plan;
    ASSERT_TRUE(SchemaImportAdapter::BuildPlan(m_ecdb, context->GetCache().GetSchemas(), plan).IsOk());
    ASSERT_EQ(SUCCESS, NativeImport(m_ecdb, {R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="Other" alias="ot" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Thing"/>
        </ECSchema>)xml"}));
    auto result = SchemaImportAdapter::ExecutePlan(m_ecdb, plan);
    EXPECT_FALSE(result.IsOk());
    EXPECT_TRUE(HasIssue(result, "SchemaImport.StalePlan")) << Describe(result);
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetSchema("Test"));

    SchemaImportPlan unbuilt;
    result = SchemaImportAdapter::ExecutePlan(m_ecdb, unbuilt);
    EXPECT_TRUE(HasIssue(result, "SchemaImport.PlanNotBuilt")) << Describe(result);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, CallerControlsCommitAndAbandon) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaImportAdapterCompletion.ecdb"));
    auto result = AdapterImport(m_ecdb, {V1()}, false);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    {
        auto context = Read(m_ecdb, {V1()});
        SchemaImportCompletion completion;
        result = SchemaImportAdapter::ImportSchemas(m_ecdb, context->GetCache().GetSchemas(), &completion);
        ASSERT_FALSE(result.IsOk()) << "the previous import is still pending, so the transaction is dirty";
    }
    m_ecdb.AbandonChanges();
    m_ecdb.ClearECDbCache();
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetSchema("Test"));

    auto context = Read(m_ecdb, {V1()});
    SchemaImportCompletion completion;
    result = SchemaImportAdapter::ImportSchemas(m_ecdb, context->GetCache().GetSchemas(), &completion);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_NE(nullptr, m_ecdb.Schemas().GetSchema("Test"));
    ASSERT_EQ(BE_SQLITE_OK, completion.Abandon());
    EXPECT_FALSE(completion.IsPending());
    EXPECT_EQ(BE_SQLITE_MISUSE, completion.Commit());
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetSchema("Test"));

    context = Read(m_ecdb, {V1()});
    result = SchemaImportAdapter::ImportSchemas(m_ecdb, context->GetCache().GetSchemas(), &completion);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ASSERT_EQ(BE_SQLITE_OK, completion.Commit());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    EXPECT_NE(nullptr, m_ecdb.Schemas().GetClass("Test", "PhysicalElement"));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, ProhibitedChangesFailAndRollBack) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaImportAdapterProhibited.ecdb"));
    ASSERT_EQ(SUCCESS, NativeImport(m_ecdb, {V1()}));
    auto before = Dump(m_ecdb);
    auto expectUnchanged = [&]() {
        auto after = Dump(m_ecdb);
        for (size_t i = 0; i < before.size(); ++i)
            EXPECT_STREQ(before[i].second.c_str(), after[i].second.c_str()) << before[i].first.c_str();
    };

    Utf8String v1(V1());
    Utf8String changedStrategy = v1;
    changedStrategy.ReplaceAll("version=\"01.00.00\" displayLabel", "version=\"01.00.01\" displayLabel");
    changedStrategy.ReplaceAll("<MapStrategy>TablePerHierarchy</MapStrategy>", "<MapStrategy>OwnTable</MapStrategy>");
    auto result = AdapterImport(m_ecdb, {changedStrategy.c_str()});
    EXPECT_FALSE(result.IsOk());
    expectUnchanged();

    Utf8CP lonely = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="Kind" alias="kd" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Lonely"><ECProperty propertyName="P" typeName="int"/></ECEntityClass>
        </ECSchema>)xml";
    ASSERT_EQ(SUCCESS, NativeImport(m_ecdb, {lonely}));
    before = Dump(m_ecdb);
    Utf8String changedKind(lonely);
    changedKind.ReplaceAll("version=\"01.00.00\"", "version=\"01.00.01\"");
    changedKind.ReplaceAll("ECEntityClass", "ECStructClass");
    auto context = Read(m_ecdb, {changedKind.c_str()});
    SchemaImportPlan plan;
    result = SchemaImportAdapter::BuildPlan(m_ecdb, context->GetCache().GetSchemas(), plan);
    EXPECT_FALSE(result.IsOk());
    EXPECT_TRUE(HasIssue(result, "SchemaImport.Unsupported")) << Describe(result);
    EXPECT_FALSE(plan.IsBuilt());
    expectUnchanged();

    Utf8String changedType = v1;
    changedType.ReplaceAll("version=\"01.00.00\" displayLabel", "version=\"01.00.01\" displayLabel");
    changedType.ReplaceAll("<ECProperty propertyName=\"Name\" typeName=\"string\"/>", "<ECProperty propertyName=\"Name\" typeName=\"int\"/>");
    result = AdapterImport(m_ecdb, {changedType.c_str()});
    EXPECT_FALSE(result.IsOk());
    expectUnchanged();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, MinorVersionDowngradeIsSkipped) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaImportAdapterDowngrade.ecdb"));
    ASSERT_EQ(SUCCESS, NativeImport(m_ecdb, {V2()}));
    auto context = Read(m_ecdb, {V1()});
    SchemaImportPlan plan;
    auto result = SchemaImportAdapter::BuildPlan(m_ecdb, context->GetCache().GetSchemas(), plan);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    EXPECT_TRUE(plan.IsEmpty()) << PlanJson(plan).c_str();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, MergedClassStepsReportFailingStep) {
    // Consecutive class-creation steps execute as one editor group. A failure in a later class must
    // still name that class's plan step, with the failed child index relative to the step.
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaImportAdapterMergedFailure.ecdb"));
    auto before = Dump(m_ecdb);
    Utf8CP xml = R"xml(<ECSchema schemaName="Merged" alias="mg" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Alpha"><ECProperty propertyName="A" typeName="int"/></ECEntityClass>
            <ECEntityClass typeName="Beta"><ECProperty propertyName="B" typeName="int"/></ECEntityClass>
            <ECEntityClass typeName="Gamma"><ECProperty propertyName="G" typeName="int"/><ECProperty propertyName="ECClassId" typeName="int"/></ECEntityClass>
        </ECSchema>)xml";
    auto context = Read(m_ecdb, {xml});
    SchemaImportPlan plan;
    auto result = SchemaImportAdapter::BuildPlan(m_ecdb, context->GetCache().GetSchemas(), plan);
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    size_t gamma = SIZE_MAX, groups = 0;
    for (size_t i = 0; i < plan.GetSteps().size(); ++i) {
        auto const& step = plan.GetSteps()[i];
        groups += step.m_group ? 1 : 0;
        if (step.m_item.EndsWith("Gamma"))
            gamma = i;
    }
    ASSERT_NE(SIZE_MAX, gamma) << PlanJson(plan).c_str();
    ASSERT_EQ(3, groups) << "Each class is its own plan step; execution merges them";
    EXPECT_FALSE(plan.GetSteps()[gamma].m_commands.Contains("AddProperty")) << plan.GetSteps()[gamma].m_commands.c_str();
    EXPECT_TRUE(plan.GetSteps()[gamma].m_commands.Contains("\"properties\"")) << plan.GetSteps()[gamma].m_commands.c_str();

    result = SchemaImportAdapter::ExecutePlan(m_ecdb, plan);
    ASSERT_FALSE(result.IsOk());
    EXPECT_STREQ("CreateEntityClass", result.GetFailedCommand().c_str()) << Describe(result);
    EXPECT_EQ(0, result.GetFailedCommandIndex()) << "Gamma's properties are inline in its CreateEntityClass";
    ASSERT_FALSE(result.GetIssues().empty());
    EXPECT_STREQ("properties[1].name", result.GetIssues().front().arg.c_str()) << Describe(result);
    auto expected = Utf8PrintfString("Plan step %" PRIu64 " (", (uint64_t)gamma);
    bool named = false;
    for (auto const& issue : result.GetIssues())
        named |= issue.code.Equals("SchemaImport.Step") && issue.message.StartsWith(expected.c_str()) && issue.message.Contains("Gamma");
    EXPECT_TRUE(named) << Describe(result);
    auto after = Dump(m_ecdb);
    ASSERT_EQ(before.size(), after.size());
    for (size_t i = 0; i < before.size(); ++i)
        EXPECT_STREQ(before[i].second.c_str(), after[i].second.c_str()) << before[i].first.c_str();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaImportAdapterTests, DropsMatchNativeImport) {
    ECDb native;
    SetupPair("SchemaImportAdapterDrops", native);
    ASSERT_EQ(SUCCESS, NativeImport(native, {V2()}));
    ASSERT_EQ(SUCCESS, NativeImport(m_ecdb, {V2()}));

    Utf8String v3(V2());
    v3.ReplaceAll("version=\"01.00.01\" displayLabel", "version=\"02.00.00\" displayLabel");
    v3.ReplaceAll("Test.01.00.01", "Test.02.00.00");
    v3.ReplaceAll("<ECProperty propertyName=\"Mass\" typeName=\"double\"/>", "");
    auto pipeStart = v3.find("<ECEntityClass typeName=\"Pipe\">");
    auto pipeEnd = v3.find("</ECEntityClass>", pipeStart);
    ASSERT_NE(Utf8String::npos, pipeStart);
    v3.erase(pipeStart, pipeEnd + strlen("</ECEntityClass>") - pipeStart);

    ASSERT_EQ(SUCCESS, NativeImport(native, {v3.c_str()}));
    auto result = AdapterImport(m_ecdb, {v3.c_str()});
    ASSERT_TRUE(result.IsOk()) << Describe(result);
    ExpectSameDump(native, m_ecdb);

    // Enumerator deletion is prohibited natively, even in a major version; the adaptor must reject it too.
    Utf8String v4(v3);
    v4.ReplaceAll("version=\"02.00.00\" displayLabel", "version=\"03.00.00\" displayLabel");
    v4.ReplaceAll("Test.02.00.00", "Test.03.00.00");
    v4.ReplaceAll("<ECEnumerator name=\"Blue\" value=\"3\"/>", "");
    ASSERT_NE(v3, v4);
    auto before = Dump(m_ecdb);
    auto rejected = AdapterImport(m_ecdb, {v4.c_str()});
    EXPECT_FALSE(rejected.IsOk());
    EXPECT_TRUE(before == Dump(m_ecdb));
}

END_ECDBUNITTESTS_NAMESPACE
