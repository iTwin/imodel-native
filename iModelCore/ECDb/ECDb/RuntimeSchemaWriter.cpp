/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "RuntimeSchemaWriter.h"
#include <pugixml/src/pugixml.hpp>
#include <cinttypes>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

static constexpr uint32_t MAGIC = 0x43534348; // "CSCH" - Indicates the file is an EC schema blob."
static constexpr uint8_t  FORMAT_VERSION = 1;

// Safely narrow int64_t SQLite row ID to uint32_t for the binary format.
// EC metadata IDs are small sequential values; overflow indicates data corruption.
static uint32_t SafeU32Id(int64_t val)
    {
    if (val >= 0 && val <= (int64_t)UINT32_MAX)
        return (uint32_t)val;
    ECDbLogger::Get().warningv("RuntimeSchemaWriter::SafeU32Id: EC metadata row ID %" PRIi64 " exceeds uint32_t range, using sentinel", val);
    return UINT32_MAX;
    }

// Helper: prepare a statement and return error on failure.
static DbResult PrepareStmt(Statement& stmt, DbCR db, Utf8CP sql)
    {
    auto rc = stmt.Prepare(db, sql);
    if (rc != BE_SQLITE_OK)
        ECDbLogger::Get().errorv("RuntimeSchemaWriter::PrepareStmt: failed to prepare statement (rc=%d): %.120s", (int)rc, sql);
    return rc;
    }

// Schemas excluded from the runtime binary blob. These fall into three categories:
// 1. Units/Formats: empty schemas whose items (Unit, Format, Phenomenon, UnitSystem)
//    are only referenced as strings from KindOfQuantity. Including them would add
//    zero useful classes/properties but bloat the string table.
// 2. ECDb-internal: ECDbSystem, ECDbMap, ECDbFileInfo, ECDbSchemaPolicies
//    describe the EC storage layer. No runtime consumer needs these.
//    Note: ECDbMeta is NOT excluded - consumers use it for metadata queries.
// 3. Pure CA schemas: CoreCustomAttributes, ECv3ConversionAttributes,
//    EditorCustomAttributes, BisCustomAttributes, SchemaLocalizationCustomAttributes,
//    SchemaUpgradeCustomAttributes. Their classes are only CustomAttribute and Struct
//    types used for decoration. Since the blob doesn't include CA instances, including
//    these schema definitions provides no value.
// The list is checked case-insensitively via BeStringUtilities::StricmpAscii.
static bool IsExcludedSchema(Utf8CP name)
    {
    struct CiLess {
        bool operator()(std::string const& a, std::string const& b) const { return BeStringUtilities::StricmpAscii(a.c_str(), b.c_str()) < 0; }
    };
    static const std::set<std::string, CiLess> s_excluded = {
        "BisCustomAttributes",
        "CoreCustomAttributes",
        "ECDbFileInfo",
        "ECDbMap",
        "ECDbSchemaPolicies",
        "ECDbSystem",
        "ECv3ConversionAttributes",
        "EditorCustomAttributes",
        "Formats",
        "SchemaLocalizationCustomAttributes",
        "SchemaUpgradeCustomAttributes",
        "Units",
    };
    return s_excluded.find(name) != s_excluded.end();
    }

//---------------------------------------------------------------------------------------
// Pre-collect IDs of schemas in the exclusion list. This avoids per-row name checks
// and enables SQL-level filtering in the property query.
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::CollectExcludedSchemaIds(DbCR db)
    {
    m_excludedSchemaIds.clear();
    Statement stmt;
    auto rc = PrepareStmt(stmt, db, "SELECT Id, Name FROM ec_Schema");
    if (rc != BE_SQLITE_OK)
        return rc;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        if (IsExcludedSchema(Safe(stmt.GetValueText(1))))
            m_excludedSchemaIds.insert(stmt.GetValueInt64(0));
        }
    return BE_SQLITE_OK;
    }

// Binary record tags - must stay in sync with Tag enum in RuntimeSchemaBinaryReader.ts
// v2 flat format: each tag marks a table header (count-prefixed), not individual records.
namespace Tag
    {
    constexpr uint8_t PropertyDefTable = 0x0A;
    constexpr uint8_t SchemaTable  = 0x10;
    constexpr uint8_t EnumTable    = 0x20;
    constexpr uint8_t KoQTable     = 0x30;
    constexpr uint8_t PropCatTable = 0x31;
    constexpr uint8_t ClassTable   = 0x40;
    }

//---------------------------------------------------------------------------------------
// String interning
//---------------------------------------------------------------------------------------
uint32_t RuntimeSchemaWriter::Intern(Utf8CP str)
    {
    str = Safe(str);
    std::string key(str);
    auto it = m_stringIndex.find(key);
    if (it != m_stringIndex.end())
        return it->second;
    uint32_t idx = (uint32_t)m_stringTable.size();
    auto result = m_stringIndex.emplace(std::move(key), idx);
    m_stringTable.push_back(result.first->first.c_str()); // pointer into map key (stable)
    return idx;
    }

//---------------------------------------------------------------------------------------
// Resolve the ec_Class.Id of CoreCustomAttributes:HiddenProperty.
// If the CA class doesn't exist in this database, m_hiddenPropertyCaClassId remains empty
// and all isHidden flags will be 0.
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::ResolveHiddenPropertyCAClassId(DbCR db)
    {
    m_hiddenPropertyCAClassId.reset();

    Statement stmt;
    auto rc = PrepareStmt(stmt, db,
        "SELECT c.Id FROM ec_Class c "
        "JOIN ec_Schema s ON c.SchemaId=s.Id "
        "WHERE s.Name='CoreCustomAttributes' AND c.Name='HiddenProperty'");
    if (rc != BE_SQLITE_OK)
        return rc;

    if (stmt.Step() == BE_SQLITE_ROW)
        m_hiddenPropertyCAClassId = stmt.GetValueInt64(0);
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Parse the Instance XML of a HiddenProperty CA to determine if the property is hidden.
// The HiddenProperty CA has an optional `Show` boolean (default false).
// If Show is explicitly set to true, the property is NOT hidden.
// Returns true if the property should be marked as hidden.
//---------------------------------------------------------------------------------------
bool RuntimeSchemaWriter::IsHiddenFromInstanceXml(Utf8CP instanceXml)
    {
    if (!instanceXml || *instanceXml == '\0')
        return true; // CA present with no instance data -> hidden

    pugi::xml_document doc;
    auto parseResult = doc.load_string(instanceXml, pugi::parse_default | pugi::parse_ws_pcdata);
    if (!parseResult)
        return true; // unparsable -> treat as hidden (CA was present)

    // Look for <Show>true</Show> (case-insensitive) under the root element
    auto root = doc.first_child();
    auto showNode = root.child("Show");
    if (!showNode)
        return true; // no Show element -> hidden

    Utf8CP showText = showNode.child_value();
    return 0 != BeStringUtilities::StricmpAscii(showText, "true");
    }

//---------------------------------------------------------------------------------------
// Resolve the ec_Class.Id of ECDbMap:QueryView.
// If the CA class doesn't exist, m_queryViewCAClassId remains empty and no classes
// will be tagged as views.
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::ResolveQueryViewCAClassId(DbCR db)
    {
    m_queryViewCAClassId.reset();

    Statement stmt;
    auto rc = PrepareStmt(stmt, db,
        "SELECT c.Id FROM ec_Class c "
        "JOIN ec_Schema s ON c.SchemaId=s.Id "
        "WHERE s.Name='ECDbMap' AND c.Name='QueryView'");
    if (rc != BE_SQLITE_OK)
        return rc;

    if (stmt.Step() == BE_SQLITE_ROW)
        m_queryViewCAClassId = stmt.GetValueInt64(0);
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Property definition dedup - builds a table of unique PropertyDef records and per-class
// PropertyRef lists. Called before writing so the PropertyDef table can precede schema records.
//---------------------------------------------------------------------------------------
uint32_t RuntimeSchemaWriter::InternPropertyDef(PropertyDefRecord const& def)
    {
    auto it = m_propDefIndex.find(def);
    if (it != m_propDefIndex.end())
        return it->second;
    uint32_t idx = (uint32_t)m_propDefs.size();
    m_propDefs.push_back(def);
    m_propDefIndex[def] = idx;
    return idx;
    }

DbResult RuntimeSchemaWriter::CollectPropertyDefsAndRefs(DbCR db)
    {
    m_propDefs.clear();
    m_propDefIndex.clear();
    m_classPropRefs.clear();

    // Excluded schemas are filtered at the SQL level via NOT IN for efficiency.
    // Ordered by ClassId,Ordinal so m_classPropRefs preserves per-class order.
    // When the HiddenProperty CA class exists, an extra column (hp_ca.Instance) is added
    // between IsReadonly and Priority, shifting Priority from column 17 to 18.
    bool hasHiddenJoin = m_hiddenPropertyCAClassId.has_value();
    int priorityCol = hasHiddenJoin ? 18 : 17;

    Utf8String sql =
        "SELECT c.Id, "
        "p.Id, p.Name, p.DisplayLabel, p.Description, p.Kind, p.PrimitiveType, p.ExtendedTypeName, "
        "p.EnumerationId, p.StructClassId, p.KindOfQuantityId, p.CategoryId, "
        "p.ArrayMinOccurs, p.ArrayMaxOccurs, "
        "p.NavigationRelationshipClassId, p.NavigationDirection, "
        "p.IsReadonly";
    if (hasHiddenJoin)
        sql.append(", hp_ca.Instance");
    sql.append(
        ", p.Priority "
        "FROM ec_Property p "
        "JOIN ec_Class c ON p.ClassId=c.Id ");
    if (hasHiddenJoin)
        sql.append(Utf8PrintfString(
            "LEFT JOIN ec_CustomAttribute hp_ca ON hp_ca.ContainerId=p.Id "
            "AND hp_ca.ContainerType & %d <> 0 "
            "AND hp_ca.ClassId=%" PRIi64 " ",
            (int)ECN::CustomAttributeContainerType::AnyProperty,
            m_hiddenPropertyCAClassId.value()));
    if (!m_excludedSchemaIds.empty())
        {
        sql.append("WHERE c.SchemaId NOT IN (");
        bool first = true;
        for (auto id : m_excludedSchemaIds)
            {
            if (!first) sql.append(",");
            sql.append(Utf8PrintfString("%" PRIi64, id));
            first = false;
            }
        sql.append(") ");
        }
    sql.append("ORDER BY c.Id, p.Ordinal");

    Statement stmt;
    auto rc = PrepareStmt(stmt, db, sql.c_str());
    if (rc != BE_SQLITE_OK)
        return rc;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        int64_t classId = stmt.GetValueInt64(0);
        int64_t propertyId = stmt.GetValueInt64(1);

        PropertyDefRecord def;
        def.nameSid       = Intern(Safe(stmt.GetValueText(2)));
        def.descriptionSid= Intern(Safe(stmt.GetValueText(4)));
        def.kind          = (uint8_t)stmt.GetValueInt(5);
        def.primitiveType = (uint16_t)stmt.GetValueInt(6);
        def.extTypeSid    = Intern(Safe(stmt.GetValueText(7)));
        // FK columns are NULL when absent; GetValueInt64 returns 0 for NULL.
        // EC metadata IDs are 1-based, so 0 is a safe "no reference" sentinel.
        def.enumRowId     = SafeU32Id(stmt.GetValueInt64(8));
        def.structClassRowId = SafeU32Id(stmt.GetValueInt64(9));
        def.koqRowId      = SafeU32Id(stmt.GetValueInt64(10));
        def.catRowId       = SafeU32Id(stmt.GetValueInt64(11));
        // ArrayMinOccurs/MaxOccurs: NULL means "not specified" -> UINT32_MAX sentinel.
        // 0 is a valid value (e.g. minOccurs=0 means array can be empty).
        def.arrayMinOccurs= stmt.IsColumnNull(12) ? UINT32_MAX : (uint32_t)stmt.GetValueInt(12);
        def.arrayMaxOccurs= stmt.IsColumnNull(13) ? UINT32_MAX : (uint32_t)stmt.GetValueInt(13);
        def.navRelClassRowId = SafeU32Id(stmt.GetValueInt64(14));
        def.navDirection  = (uint8_t)stmt.GetValueInt(15);
        def.isReadonly    = stmt.GetValueInt(16) != 0 ? (uint8_t)1 : (uint8_t)0;
        // HiddenProperty CA: if the JOIN is present, column 17 is hp_ca.Instance
        if (hasHiddenJoin)
            {
            Utf8CP hiddenInstance = stmt.GetValueText(17);
            def.isHidden = (hiddenInstance != nullptr) ? (IsHiddenFromInstanceXml(hiddenInstance) ? (uint8_t)1 : (uint8_t)0) : (uint8_t)0;
            }
        else
            def.isHidden = (uint8_t)0;

        uint32_t defIdx = InternPropertyDef(def);

        PropertyRefRecord ref;
        ref.defIdx   = defIdx;
        ref.labelSid = Intern(Safe(stmt.GetValueText(3)));
        ref.priority = stmt.GetValueInt(priorityCol);
        ref.ecInstanceId = SafeU32Id(propertyId);

        m_classPropRefs[classId].push_back(ref);
        }
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Pre-collect IDs of mixin classes (Entity classes with CoreCustomAttributes:IsMixin CA).
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::CollectMixinClassIds(DbCR db)
    {
    m_mixinClassIds.clear();
    Statement stmt;
    auto rc = PrepareStmt(stmt, db, Utf8PrintfString(
        "SELECT ca.ContainerId FROM ec_CustomAttribute ca "
        "JOIN ec_Class cac ON ca.ClassId=cac.Id "
        "JOIN ec_Schema cas ON cac.SchemaId=cas.Id "
        "WHERE ca.ContainerType & %d <> 0 "
        "AND cas.Name='CoreCustomAttributes' AND cac.Name='IsMixin'",
        (int)ECN::CustomAttributeContainerType::AnyClass).c_str());
    if (rc != BE_SQLITE_OK)
        return rc;
    while (stmt.Step() == BE_SQLITE_ROW)
        m_mixinClassIds.insert(stmt.GetValueInt64(0));
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Pre-collect IDs of classes with the ECDbMap:QueryView CA (tagged as views).
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::CollectQueryViewClassIds(DbCR db)
    {
    m_queryViewClassIds.clear();
    if (!m_queryViewCAClassId.has_value())
        return BE_SQLITE_OK;
    Statement stmt;
    auto rc = PrepareStmt(stmt, db, Utf8PrintfString(
        "SELECT ContainerId FROM ec_CustomAttribute "
        "WHERE ContainerType & %d <> 0 AND ClassId=%" PRIi64,
        (int)ECN::CustomAttributeContainerType::AnyClass,
        m_queryViewCAClassId.value()).c_str());
    if (rc != BE_SQLITE_OK)
        return rc;
    while (stmt.Step() == BE_SQLITE_ROW)
        m_queryViewClassIds.insert(stmt.GetValueInt64(0));
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Helper: append WHERE <col> NOT IN (...) for excluded schemas.
//---------------------------------------------------------------------------------------
static void AppendExcludedSchemaFilter(Utf8StringR sql, Utf8CP column, std::unordered_set<int64_t> const& ids)
    {
    if (ids.empty())
        return;
    sql.append(" WHERE ");
    sql.append(column);
    sql.append(" NOT IN (");
    bool first = true;
    for (auto id : ids)
        {
        if (!first) sql.append(",");
        sql.append(Utf8PrintfString("%" PRIi64, id));
        first = false;
        }
    sql.append(")");
    }

//---------------------------------------------------------------------------------------
// Write the deduplicated PropertyDef table from pre-collected data.
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::WritePropertyDefTable()
    {
    PutU8(Tag::PropertyDefTable);
    PutU32((uint32_t)m_propDefs.size());
    for (auto const& def : m_propDefs)
        {
        PutU32(def.nameSid);
        PutU8(def.kind);
        PutU16(def.primitiveType);
        PutU32(def.extTypeSid);
        PutU32(def.enumRowId);
        PutU32(def.structClassRowId);
        PutU32(def.koqRowId);
        PutU32(def.catRowId);
        PutU32(def.arrayMinOccurs);
        PutU32(def.arrayMaxOccurs);
        PutU32(def.navRelClassRowId);
        PutU8(def.navDirection);
        PutU8(def.isReadonly);
        PutU8(def.isHidden);
        PutU32(def.descriptionSid);
        }
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Flat schema table: all non-excluded schemas, count-prefixed.
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::WriteSchemaTable(DbCR db)
    {
    Utf8String sql("SELECT Id,Name,DisplayLabel,Description,Alias,VersionDigit1,VersionDigit2,VersionDigit3 FROM ec_Schema");
    AppendExcludedSchemaFilter(sql, "Id", m_excludedSchemaIds);
    sql.append(" ORDER BY Id");

    Statement stmt;
    auto rc = PrepareStmt(stmt, db, sql.c_str());
    if (rc != BE_SQLITE_OK)
        return rc;

    PutU8(Tag::SchemaTable);
    size_t countPos = m_output.size();
    PutU32(0);
    uint32_t count = 0;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        PutSRef(Safe(stmt.GetValueText(1))); // name
        PutU16((uint16_t)stmt.GetValueInt(5));
        PutU16((uint16_t)stmt.GetValueInt(6));
        PutU16((uint16_t)stmt.GetValueInt(7));
        PutSRef(Safe(stmt.GetValueText(4))); // alias
        PutSRef(Safe(stmt.GetValueText(2))); // label
        PutSRef(Safe(stmt.GetValueText(3))); // description
        PutU32(SafeU32Id(stmt.GetValueInt64(0))); // ecInstanceId
        count++;
        }
    PatchU32(countPos, count);
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Flat enumeration table: all enumerations across all non-excluded schemas.
// Each record carries its schema's ecInstanceId for ownership resolution.
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::WriteEnumTable(DbCR db)
    {
    Utf8String sql("SELECT e.Id,e.Name,e.DisplayLabel,e.Description,e.UnderlyingPrimitiveType,e.IsStrict,e.EnumValues,e.SchemaId FROM ec_Enumeration e");
    AppendExcludedSchemaFilter(sql, "e.SchemaId", m_excludedSchemaIds);
    sql.append(" ORDER BY e.SchemaId, e.Id");

    Statement stmt;
    auto rc = PrepareStmt(stmt, db, sql.c_str());
    if (rc != BE_SQLITE_OK)
        return rc;

    PutU8(Tag::EnumTable);
    size_t countPos = m_output.size();
    PutU32(0);
    uint32_t count = 0;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        PutU32(SafeU32Id(stmt.GetValueInt64(7))); // schemaEcId
        PutSRef(Safe(stmt.GetValueText(1)));       // name
        PutU8((uint8_t)stmt.GetValueInt(4));        // primitiveType
        PutU8(stmt.GetValueInt(5) != 0 ? 1 : 0);  // isStrict
        PutSRef(Safe(stmt.GetValueText(2)));       // label
        PutSRef(Safe(stmt.GetValueText(3)));       // description
        PutSRef(Safe(stmt.GetValueText(6)));       // enumValues JSON
        PutU32(SafeU32Id(stmt.GetValueInt64(0)));  // ecInstanceId
        count++;
        }
    PatchU32(countPos, count);
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Flat KindOfQuantity table.
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::WriteKoqTable(DbCR db)
    {
    Utf8String sql("SELECT k.Id,k.Name,k.DisplayLabel,k.Description,k.PersistenceUnit,k.RelativeError,k.PresentationUnits,k.SchemaId FROM ec_KindOfQuantity k");
    AppendExcludedSchemaFilter(sql, "k.SchemaId", m_excludedSchemaIds);
    sql.append(" ORDER BY k.SchemaId, k.Id");

    Statement stmt;
    auto rc = PrepareStmt(stmt, db, sql.c_str());
    if (rc != BE_SQLITE_OK)
        return rc;

    PutU8(Tag::KoQTable);
    size_t countPos = m_output.size();
    PutU32(0);
    uint32_t count = 0;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        PutU32(SafeU32Id(stmt.GetValueInt64(7))); // schemaEcId
        PutSRef(Safe(stmt.GetValueText(1)));       // name
        PutSRef(Safe(stmt.GetValueText(2)));       // label
        PutSRef(Safe(stmt.GetValueText(3)));       // description
        PutSRef(Safe(stmt.GetValueText(4)));       // persistenceUnit
        PutF64(stmt.GetValueDouble(5));             // relativeError
        PutSRef(Safe(stmt.GetValueText(6)));       // presentationUnits
        PutU32(SafeU32Id(stmt.GetValueInt64(0)));  // ecInstanceId
        count++;
        }
    PatchU32(countPos, count);
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Flat PropertyCategory table.
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::WritePropCatTable(DbCR db)
    {
    Utf8String sql("SELECT c.Id,c.Name,c.DisplayLabel,c.Description,c.Priority,c.SchemaId FROM ec_PropertyCategory c");
    AppendExcludedSchemaFilter(sql, "c.SchemaId", m_excludedSchemaIds);
    sql.append(" ORDER BY c.SchemaId, c.Id");

    Statement stmt;
    auto rc = PrepareStmt(stmt, db, sql.c_str());
    if (rc != BE_SQLITE_OK)
        return rc;

    PutU8(Tag::PropCatTable);
    size_t countPos = m_output.size();
    PutU32(0);
    uint32_t count = 0;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        PutU32(SafeU32Id(stmt.GetValueInt64(5))); // schemaEcId
        PutSRef(Safe(stmt.GetValueText(1)));       // name
        PutSRef(Safe(stmt.GetValueText(2)));       // label
        PutSRef(Safe(stmt.GetValueText(3)));       // description
        PutI32(stmt.GetValueInt(4));                // priority
        PutU32(SafeU32Id(stmt.GetValueInt64(0)));  // ecInstanceId
        count++;
        }
    PatchU32(countPos, count);
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Flat class table with count-prefixed inline sub-items (base classes, property refs,
// relationship constraints). Class type adjustments (mixin, view) use pre-collected sets.
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::WriteClassTable(DbCR db)
    {
    Utf8String sql(
        "SELECT c.Id,c.Name,c.DisplayLabel,c.Description,c.Type,c.Modifier,"
        "c.RelationshipStrength,c.RelationshipStrengthDirection,c.SchemaId "
        "FROM ec_Class c");
    AppendExcludedSchemaFilter(sql, "c.SchemaId", m_excludedSchemaIds);
    sql.append(" ORDER BY c.SchemaId, c.Id");

    Statement classStmt;
    DbResult rc;
    if ((rc = PrepareStmt(classStmt, db, sql.c_str())) != BE_SQLITE_OK)
        return rc;

    Statement baseStmt;
    if ((rc = PrepareStmt(baseStmt, db,
        "SELECT s.Name, bc.Name, b.Ordinal "
        "FROM ec_ClassHasBaseClasses b "
        "JOIN ec_Class bc ON b.BaseClassId=bc.Id "
        "JOIN ec_Schema s ON bc.SchemaId=s.Id "
        "WHERE b.ClassId=? ORDER BY b.Ordinal")) != BE_SQLITE_OK)
        return rc;

    Statement constrStmt;
    if ((rc = PrepareStmt(constrStmt, db,
        "SELECT rc.Id, rc.RelationshipEnd, rc.MultiplicityLowerLimit, rc.MultiplicityUpperLimit, "
        "rc.IsPolymorphic, rc.RoleLabel, acs.Name, acc.Name "
        "FROM ec_RelationshipConstraint rc "
        "LEFT JOIN ec_Class acc ON rc.AbstractConstraintClassId=acc.Id "
        "LEFT JOIN ec_Schema acs ON acc.SchemaId=acs.Id "
        "WHERE rc.RelationshipClassId=? ORDER BY rc.RelationshipEnd")) != BE_SQLITE_OK)
        return rc;

    Statement constrClassStmt;
    if ((rc = PrepareStmt(constrClassStmt, db,
        "SELECT s.Name, c.Name FROM ec_RelationshipConstraintClass rcc "
        "JOIN ec_Class c ON rcc.ClassId=c.Id "
        "JOIN ec_Schema s ON c.SchemaId=s.Id "
        "WHERE rcc.ConstraintId=? ORDER BY rcc.Id")) != BE_SQLITE_OK)
        return rc;

    PutU8(Tag::ClassTable);
    size_t countPos = m_output.size();
    PutU32(0);
    uint32_t count = 0;

    while (classStmt.Step() == BE_SQLITE_ROW)
        {
        int64_t classId = classStmt.GetValueInt64(0);
        int64_t schemaId = classStmt.GetValueInt64(8);
        int classType = classStmt.GetValueInt(4);

        if (classType == 0 && m_mixinClassIds.count(classId))
            classType = 4; // Mixin
        if (m_queryViewClassIds.count(classId))
            classType = 5; // View

        PutU32(SafeU32Id(schemaId));                  // schemaEcId
        PutSRef(Safe(classStmt.GetValueText(1)));     // name
        PutU8((uint8_t)classType);
        PutU8((uint8_t)classStmt.GetValueInt(5));     // modifier
        PutSRef(Safe(classStmt.GetValueText(2)));     // label
        PutSRef(Safe(classStmt.GetValueText(3)));     // description
        if (classType == 1) // Relationship
            {
            PutU8((uint8_t)classStmt.GetValueInt(6)); // strength
            PutU8((uint8_t)classStmt.GetValueInt(7)); // strengthDirection
            }
        PutU32(SafeU32Id(classId));                   // ecInstanceId

        // Base classes (count-prefixed)
        baseStmt.Reset(); baseStmt.ClearBindings();
        baseStmt.BindInt64(1, classId);
        size_t baseCountPos = m_output.size();
        PutU16(0);
        uint16_t baseCount = 0;
        while (baseStmt.Step() == BE_SQLITE_ROW)
            {
            PutSRef(Safe(baseStmt.GetValueText(0))); // schema name
            PutSRef(Safe(baseStmt.GetValueText(1))); // class name
            PutU8((uint8_t)baseStmt.GetValueInt(2));  // ordinal
            if (++baseCount == UINT16_MAX)
                {
                ECDbLogger::Get().errorv("RuntimeSchemaWriter: class %" PRIi64 " has too many base classes", classId);
                return BE_SQLITE_ERROR;
                }
            }
        PatchU16(baseCountPos, baseCount);

        // Property refs (count-prefixed, from pre-collected map)
        auto propIt = m_classPropRefs.find(classId);
        if (propIt != m_classPropRefs.end() && propIt->second.size() > UINT16_MAX)
            {
            ECDbLogger::Get().errorv("RuntimeSchemaWriter: class %" PRIi64 " has %zu own properties, exceeding uint16_t limit", classId, propIt->second.size());
            return BE_SQLITE_ERROR;
            }
        uint16_t propCount = propIt != m_classPropRefs.end() ? (uint16_t)propIt->second.size() : 0;
        PutU16(propCount);
        if (propIt != m_classPropRefs.end())
            for (auto const& ref : propIt->second)
                {
                PutU32(ref.defIdx);
                PutU32(ref.labelSid);
                PutI32(ref.priority);
                PutU32(ref.ecInstanceId);
                }

        // Relationship constraints (count-prefixed, only for relationships)
        if (classType == 1)
            {
            constrStmt.Reset(); constrStmt.ClearBindings();
            constrStmt.BindInt64(1, classId);
            size_t constrCountPos = m_output.size();
            PutU8(0);
            uint8_t constrCount = 0;
            while (constrStmt.Step() == BE_SQLITE_ROW)
                {
                int64_t constraintId = constrStmt.GetValueInt64(0);
                PutU8((uint8_t)constrStmt.GetValueInt(1));  // relEnd
                PutU32((uint32_t)constrStmt.GetValueInt(2)); // multLower
                PutU32((uint32_t)constrStmt.GetValueInt(3)); // multUpper
                PutU8(constrStmt.GetValueInt(4) != 0 ? 1 : 0); // isPolymorphic
                PutSRef(Safe(constrStmt.GetValueText(5)));  // roleLabel
                PutSRef(Safe(constrStmt.GetValueText(6)));  // abstractSchemaName
                PutSRef(Safe(constrStmt.GetValueText(7)));  // abstractClassName

                // Constraint classes (count-prefixed)
                constrClassStmt.Reset(); constrClassStmt.ClearBindings();
                constrClassStmt.BindInt64(1, constraintId);
                size_t ccCountPos = m_output.size();
                PutU8(0);
                uint8_t ccCount = 0;
                while (constrClassStmt.Step() == BE_SQLITE_ROW)
                    {
                    PutSRef(Safe(constrClassStmt.GetValueText(0)));
                    PutSRef(Safe(constrClassStmt.GetValueText(1)));
                    ccCount++;
                    }
                m_output[ccCountPos] = ccCount;
                constrCount++;
                }
            m_output[constrCountPos] = constrCount;
            }

        count++;
        }
    PatchU32(countPos, count);
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// Write string table at end of blob and patch offset in header.
//---------------------------------------------------------------------------------------
void RuntimeSchemaWriter::WriteStringTable(size_t stOffsetPos)
    {
    uint32_t stOffset = (uint32_t)m_output.size();
    PutU32((uint32_t)m_stringTable.size());
    for (auto s : m_stringTable)
        {
        uint32_t len = (uint32_t)strlen(s);
        PutU32(len);
        if (len)
            m_output.insert(m_output.end(), (Byte const*)s, (Byte const*)s + len);
        }
    PatchU32(stOffsetPos, stOffset);
    }

//---------------------------------------------------------------------------------------
// Main orchestration - pre-collects metadata, then writes flat tables top-down.
//---------------------------------------------------------------------------------------
DbResult RuntimeSchemaWriter::WriteAllSchemas(DbCR db)
    {
    m_output.clear();
    m_output.reserve(2 * 1024 * 1024);
    m_stringTable.clear();
    m_stringIndex.clear();
    Intern(""); // index 0 = empty string

    // Pre-pass: collect metadata needed before writing
    DbResult rc;
    if ((rc = CollectExcludedSchemaIds(db)) != BE_SQLITE_OK) return rc;
    if ((rc = ResolveHiddenPropertyCAClassId(db)) != BE_SQLITE_OK) return rc;
    if ((rc = ResolveQueryViewCAClassId(db)) != BE_SQLITE_OK) return rc;
    if ((rc = CollectPropertyDefsAndRefs(db)) != BE_SQLITE_OK) return rc;
    if ((rc = CollectMixinClassIds(db)) != BE_SQLITE_OK) return rc;
    if ((rc = CollectQueryViewClassIds(db)) != BE_SQLITE_OK) return rc;

    // Header: magic(4) + version(1) + stringTableOffset placeholder(4)
    PutU32(MAGIC);
    PutU8(FORMAT_VERSION);
    size_t stOffsetPos = m_output.size();
    PutU32(0);

    // Write flat tables top-down: property defs, schemas, schema items, classes
    if ((rc = WritePropertyDefTable()) != BE_SQLITE_OK) return rc;
    if ((rc = WriteSchemaTable(db)) != BE_SQLITE_OK) return rc;
    if ((rc = WriteEnumTable(db)) != BE_SQLITE_OK) return rc;
    if ((rc = WriteKoqTable(db)) != BE_SQLITE_OK) return rc;
    if ((rc = WritePropCatTable(db)) != BE_SQLITE_OK) return rc;
    if ((rc = WriteClassTable(db)) != BE_SQLITE_OK) return rc;

    WriteStringTable(stOffsetPos);
    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
